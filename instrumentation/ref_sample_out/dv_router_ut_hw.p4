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


header_type arp_t {
    fields {
        htype : 16;
        ptype : 16;
        hlen : 8;
        plen : 8;
        oper : 16;
        senderHA : 48;
        senderPA : 32;
        targetHA : 48;
        targetPA : 32;
    }
}


header arp_t arp;


header_type distance_vec_t {
    fields {
        src : 32;
        length_dist : 16;
        data : 144;
    }
}


header distance_vec_t distance_vec;


header_type cis553_metadata_t {
    fields {
        forMe : 1;
        __pad : 7;
        temp : 32;
        nextHop : 32;
    }
}


metadata cis553_metadata_t cis553_metadata;


parser start {
    extract(fp4_visited);
    
    return select(ig_intr_md.ingress_port) { 
        192 : parse_routing;
        default  : parse_ethernet;
    }

}


parser parse_ethernet {
    extract(ethernet);

    return select(ethernet.etherType) { 
        0x0800 : parse_ipv4;
        0x0806 : parse_arp;
        0x0553 : parse_routing;
        default  : ingress;
    }

}


parser parse_arp {
    extract(arp);

    return ingress;

}


parser parse_ipv4 {
    extract(ipv4);

    return ingress;

}


parser parse_routing {
    extract(distance_vec);

    return ingress;

}


action aiForMe() {
    modify_field(cis553_metadata.forMe, 1);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 1);
}


table tiDrop {
    actions {
        aDrop_fp4_tiDrop;
    }
}


table tiHandleIncomingEthernet {
    reads {
        ethernet.dstAddr : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiForMe;
        aDrop_fp4_tiHandleIncomingEthernet;
    }
}


action aiHandleOutgoingRouting(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
    add_header(ethernet);
    modify_field(ethernet.dstAddr, 0xffffffffffff);
    modify_field(ethernet.etherType, 0x553);
    modify_field(cis553_metadata.forMe, 0);
    add(fp4_visited.encoding1, fp4_visited.encoding1, 1);
}

table tiHandleOutgoingRouting {
    reads {
        distance_vec.src : exact;
    }
    actions {
        aiHandleOutgoingRouting;
    }
}


action aiHandleIncomingArpReqest_part_one(mac_sa) {
    modify_field(arp.oper, 2);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, arp.senderHA);
    modify_field(cis553_metadata.temp, arp.targetPA);
    modify_field(arp.targetHA, arp.senderHA);
    modify_field(arp.senderHA, mac_sa);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);
    add(fp4_visited.encoding1, fp4_visited.encoding1, 2);
}

table tiHandleIncomingArpReqest_part_one {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_one;
    }
}


action aiHandleIncomingArpReqest_part_two() {
    modify_field(arp.targetPA, arp.senderPA);
    modify_field(arp.senderPA, cis553_metadata.temp);
    add(fp4_visited.encoding0, fp4_visited.encoding0, -1);
}

table tiHandleIncomingArpReqest_part_two {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_two;
        aDrop_fp4_tiHandleIncomingArpReqest_part_two;
    }
}


action aiHandleIncomingArpResponse() {
    clone_i2e(98);
    add(fp4_visited.encoding2, fp4_visited.encoding2, 1);
}

table tiHandleIncomingArpResponse {
    actions {
        aiHandleIncomingArpResponse;
    }
}


action aiFindNextL3Hop(nextHop) {
    modify_field(cis553_metadata.nextHop, nextHop);
    add(fp4_visited.encoding4, fp4_visited.encoding4, 1);
}

action aiSendToLastHop() {
    modify_field(cis553_metadata.nextHop, ipv4.dstAddr);
    add(fp4_visited.encoding4, fp4_visited.encoding4, 2);
}

table tiHandleIpv4 {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        aiFindNextL3Hop;
        aiSendToLastHop;
        aDrop_fp4_tiHandleIpv4;
    }
}


action aiForward(mac_sa, mac_da, egress_port) {
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(standard_metadata.egress_spec, egress_port);
    add(fp4_visited.encoding1, fp4_visited.encoding1, 4);
}

action aiArpMiss(local_ip, local_mac, local_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, local_port);
    modify_field(ethernet.dstAddr, 0xffffffffffff);
    modify_field(ethernet.etherType, 0x0806);
    remove_header(ipv4);
    add_header(arp);
    modify_field(arp.htype, 1);
    modify_field(arp.ptype, 0x0800);
    modify_field(arp.hlen, 6);
    modify_field(arp.plen, 4);
    modify_field(arp.oper, 1);
    modify_field(arp.senderHA, local_mac);
    modify_field(arp.senderPA, local_ip);
    modify_field(arp.targetHA, 0);
    modify_field(arp.targetPA, cis553_metadata.nextHop);
    add(fp4_visited.encoding1, fp4_visited.encoding1, 8);
}

table tiHandleOutgoingEthernet {
    reads {
        cis553_metadata.nextHop : lpm;
    }
    actions {
        aiForward;
        aiArpMiss;
    }
}


action aiHandleIncomingRouting() {
    clone_i2e(98);
    add(fp4_visited.encoding3, fp4_visited.encoding3, 1);
}

table tiHandleIncomingRouting {
    actions {
        aiHandleIncomingRouting;
    }
}


control ingress     {
    if (valid(ethernet))  {
        apply(tiHandleIncomingEthernet);


    }
    else  {
        apply(tiHandleOutgoingRouting);


    }

    if (cis553_metadata.forMe == 0)  {

    }
    else     if (valid(ipv4))  {
        apply(tiHandleIpv4);

        apply(tiHandleOutgoingEthernet);


    }
    else     if (valid(arp) && arp.oper == 1)  {
        apply(tiHandleIncomingArpReqest_part_one);

        apply(tiHandleIncomingArpReqest_part_two);


    }
    else     if (valid(arp) && arp.oper == 2)  {
        apply(tiHandleIncomingArpResponse);


    }
    else     if (valid(distance_vec))  {
        apply(tiHandleIncomingRouting);


    }
    else  {
        apply(tiDrop);


    }


}

control egress     {

}

field_list ipv4_checksum_list {
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

field_list_calculation ipv4_checksum {
    input    {
        ipv4_checksum_list;
    }
    algorithm:csum16;
    output_width:16;
}

action aDrop_fp4_tiDrop() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 4);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action aDrop_fp4_tiHandleIncomingEthernet() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 2);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action aDrop_fp4_tiHandleIncomingArpReqest_part_two() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 8);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action aDrop_fp4_tiHandleIpv4() {
    add(fp4_visited.encoding4, fp4_visited.encoding4, 4);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

header_type fp4_visited_t {
    fields {
        preamble : 48;
        encoding0 : 32;
        encoding4 : 32;
        encoding1 : 32;
        encoding2 : 32;
        encoding3 : 32;
        pkt_type : 2;
        __pad : 6;
    }
}


header fp4_visited_t fp4_visited;


calculated_field ipv4.hdrChecksum{
    verify    ipv4_checksum;
    update    ipv4_checksum;
}


