#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>
header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}


header ethernet_t ethernet;


header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr : 32;
    }
}


header ipv4_t ipv4;


calculated_field ipv4.hdrChecksum{
    update    ipv4_chksum_calc;
}

field_list_calculation ipv4_chksum_calc {
    input    {
        ipv4_field_list;
    }
    algorithm:csum16;
    output_width:16;
}

field_list ipv4_field_list {
    ipv4.version;
    ipv4.ihl;
    ipv4.diffserv;
    ipv4.totalLen;
    ipv4.identification;
    ipv4.flags;
    ipv4.fragOffset;
    ipv4.ttl;
    ipv4.protocol;
    ipv4.srcAddr;
    ipv4.dstAddr;
}

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 3;
        ecn : 3;
        ctrl : 6;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}


header tcp_t tcp;


header_type metadata_t {
    fields {
        ecmp_select : 16;
    }
}


metadata metadata_t meta;


parser start {
    extract(fp4_visited);
    
    return parse_ethernet;

}


parser parse_ethernet {
    extract(ethernet);

    return select(latest.etherType) { 
        0x0800 : parse_ipv4;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return select(ipv4.protocol) { 
        6 : parse_tcp;
        default  : ingress;
    }

}


parser parse_tcp {
    extract(tcp);

    return ingress;

}


control ingress     {
    if (valid(ipv4))  {
        apply(ecmp_group);

        apply(ecmp_nhop);


    }

    apply(ti_port_correction);


}

table ecmp_group {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        drop_packet_fp4_ecmp_group;
        set_ecmp_select;
    }
    size:1024;
}



action set_ecmp_select() {
    modify_field_with_hash_based_offset(meta.ecmp_select, 0, ecmp_hasher, 2);
    modify_field(fp4_visited.set_ecmp_select, 1);
}

field_list ecmp_fields {
    ipv4.srcAddr;
    ipv4.dstAddr;
    ipv4.protocol;
    tcp.srcPort;
    tcp.dstPort;
}

field_list_calculation ecmp_hasher {
    input    {
        ecmp_fields;
    }
    algorithm:crc16;
    output_width:16;
}

table ecmp_nhop {
    reads {
        meta.ecmp_select : exact;
    }
    actions {
        drop_packet_fp4_ecmp_nhop;
        set_nhop;
    }
    size:2;
}


action set_nhop(nhop_dmac, nhop_ipv4, port) {
    modify_field(ethernet.dstAddr, nhop_dmac);
    modify_field(ipv4.dstAddr, nhop_ipv4);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    subtract_from_field(ipv4.ttl, 1);
    modify_field(fp4_visited.set_nhop, 1);
}

control egress     {
    apply(send_frame);

    apply(ti_set_visited_type);


}

table send_frame {
    reads {
        ig_intr_md_for_tm.ucast_egress_port : exact;
    }
    actions {
        rewrite_mac;
        drop_packet_fp4_send_frame;
    }
    size:256;
}


action drop_packet_fp4_ecmp_group() {
    modify_field(fp4_visited.drop_packet_fp4_ecmp_group, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action drop_packet_fp4_ecmp_nhop() {
    modify_field(fp4_visited.drop_packet_fp4_ecmp_nhop, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action drop_packet_fp4_send_frame() {
    modify_field(fp4_visited.drop_packet_fp4_send_frame, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

header_type fp4_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        drop_packet_fp4_ecmp_group : 1;
        drop_packet_fp4_ecmp_nhop : 1;
        drop_packet_fp4_send_frame : 1;
        rewrite_mac : 1;
        set_ecmp_select : 1;
        set_nhop : 1;
    }
}


header fp4_visited_t fp4_visited;


action rewrite_mac(smac) {
    modify_field(ethernet.srcAddr, smac);
    modify_field(fp4_visited.rewrite_mac, 1);
}


action ai_set_visited_type() {
    modify_field(fp4_visited.pkt_type, 1);
}

table ti_set_visited_type {
    actions { ai_set_visited_type; }
    default_action: ai_set_visited_type();
}

action ai_port_correction(outPort) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, outPort);
}

action ai_NoAction() {
}

table ti_port_correction {
    reads {
        ig_intr_md_for_tm.ucast_egress_port: exact;
  }
    actions {
        ai_port_correction;
        ai_NoAction;
    }
    default_action : ai_NoAction();
    size: 1;
}

