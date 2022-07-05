#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>

#define ETHERTYPE_IPV4 0x0800
#define NUM_UPLINKS 2

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
        hdrChecksum : 16; // here
        srcAddr : 32;
        dstAddr: 32;
    }
}
header ipv4_t ipv4;


calculated_field ipv4.hdrChecksum {
    update ipv4_chksum_calc;
}


field_list_calculation ipv4_chksum_calc {
    input {
        ipv4_field_list;
    }
    algorithm : csum16;
    output_width: 16;
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
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4; 
        default : ingress;         
    }
}

#define IPV4_TCP_PROTOCOL 6

parser parse_ipv4 {
    extract(ipv4);
    return select(ipv4.protocol) {
        IPV4_TCP_PROTOCOL : parse_tcp;
        default: ingress;
    }
}


parser parse_tcp {
    extract(tcp);
    return ingress;
}

control ingress {
    if (valid(ipv4)) {
        apply(ecmp_group);
        apply(ecmp_nhop);
    }
}

table ecmp_group {
    reads {
        ipv4.dstAddr: lpm;
    }
    actions {
        drop_packet;
        set_ecmp_select;
    }
    size : 1024;
}

action drop_packet() {
    drop();
}

action set_ecmp_select() {
    modify_field_with_hash_based_offset(meta.ecmp_select, 0, ecmp_hasher, NUM_UPLINKS);
}


field_list ecmp_fields {
    ipv4.srcAddr;
    ipv4.dstAddr;
    ipv4.protocol;
    tcp.srcPort;
    tcp.dstPort;
}

field_list_calculation ecmp_hasher {
    input {
        ecmp_fields;
    }
    algorithm : crc16;
    output_width : 16;
}


table ecmp_nhop {
    reads {
        meta.ecmp_select: exact;
    }
    actions{
        drop_packet;
        set_nhop;
    }
    size : 2;
}


action set_nhop(nhop_dmac, nhop_ipv4,port) {
    modify_field(ethernet.dstAddr, nhop_dmac);
    modify_field(ipv4.dstAddr, nhop_ipv4);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    subtract_from_field(ipv4.ttl, 1);
}

control egress {
    apply(send_frame);
}

table send_frame {
    reads {
        ig_intr_md_for_tm.ucast_egress_port: exact;
    }
    actions {
        rewrite_mac;
        drop_packet;
    }
    size : 256;
}

action rewrite_mac(smac) {
    modify_field(ethernet.srcAddr, smac);
}


