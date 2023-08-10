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
    extract(pfuzz_visited);
    
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





table tiDrop {
    actions {
        aDrop_pfuzz_tiDrop;
    }
    default_action:aDrop_pfuzz_tiDrop();
}


table tiHandleIncomingEthernet {
    reads {
        ethernet.dstAddr : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiForMe_pfuzz_tiHandleIncomingEthernet;
        aDrop_pfuzz_tiHandleIncomingEthernet;
    }
    default_action:aDrop_pfuzz_tiHandleIncomingEthernet();
}



table tiHandleOutgoingRouting {
    reads {
        distance_vec.src : exact;
    }
    actions {
        aiHandleOutgoingRouting_pfuzz_tiHandleOutgoingRouting;
        ai_nop_pfuzz_tiHandleOutgoingRouting;
    }
    default_action:ai_nop_pfuzz_tiHandleOutgoingRouting();
}



table tiHandleIncomingArpReqest_part_one {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_one_pfuzz_tiHandleIncomingArpReqest_part_one;
        ai_nop_pfuzz_tiHandleIncomingArpReqest_part_one;
    }
    default_action:ai_nop_pfuzz_tiHandleIncomingArpReqest_part_one();
}



table tiHandleIncomingArpReqest_part_two {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_two_pfuzz_tiHandleIncomingArpReqest_part_two;
        aDrop_pfuzz_tiHandleIncomingArpReqest_part_two;
    }
    default_action:aDrop_pfuzz_tiHandleIncomingArpReqest_part_two();
}



table tiHandleIncomingArpResponse {
    actions {
        aiHandleIncomingArpResponse_pfuzz_tiHandleIncomingArpResponse;
    }
    default_action:aiHandleIncomingArpResponse_pfuzz_tiHandleIncomingArpResponse();
}




table tiHandleIpv4 {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        aiFindNextL3Hop_pfuzz_tiHandleIpv4;
        aiSendToLastHop_pfuzz_tiHandleIpv4;
        aDrop_pfuzz_tiHandleIpv4;
    }
    default_action:aDrop_pfuzz_tiHandleIpv4();
}




table tiHandleOutgoingEthernet {
    reads {
        cis553_metadata.nextHop : lpm;
    }
    actions {
        aiForward_pfuzz_tiHandleOutgoingEthernet;
        aiArpMiss_pfuzz_tiHandleOutgoingEthernet;
        ai_nop_pfuzz_tiHandleOutgoingEthernet;
    }
    default_action:ai_nop_pfuzz_tiHandleOutgoingEthernet();
}



table tiHandleIncomingRouting {
    actions {
        aiHandleIncomingRouting_pfuzz_tiHandleIncomingRouting;
    }
    default_action:aiHandleIncomingRouting_pfuzz_tiHandleIncomingRouting();
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
    else  {
        apply(ti_mvbl_3_VIRTUAL_START_hdripv4isValid);
        if (valid(ipv4))  {
            apply(tiHandleIpv4);

            apply(tiHandleOutgoingEthernet);


        }
        else  {
            apply(ti_mvbl_4_VIRTUAL_START_hdrarpisValidhdrarpoper1);
            if (valid(arp) && arp.oper == 1)  {
                apply(tiHandleIncomingArpReqest_part_one);
                apply(ti_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_tiHandleIncomingArpReqest_part_two);
                apply(tiHandleIncomingArpReqest_part_two);


            }
            else  {
                apply(ti_mvbl_0_VIRTUAL_START_hdrarpisValidhdrarpoper2);
                if (valid(arp) && arp.oper == 2)  {
                    apply(tiHandleIncomingArpResponse);


                }
                else  {
                    apply(ti_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_hdrdistance_vecisValid);
                    if (valid(distance_vec))  {
                        apply(tiHandleIncomingRouting);


                    }


                }


            }


        }


    }
    apply(ti_temp_port);
}
action ai_temp_port() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}
table ti_temp_port {
    actions {
        ai_temp_port;
    }
    default_action: ai_temp_port();
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

action aDrop_pfuzz_tiDrop() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action aiForMe_pfuzz_tiHandleIncomingEthernet() {
    modify_field(cis553_metadata.forMe, 1);
    add_to_field(pfuzz_visited.encoding_i1, 5);
}

action aDrop_pfuzz_tiHandleIncomingEthernet() {
    add_to_field(pfuzz_visited.encoding_i1, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action aiHandleOutgoingRouting_pfuzz_tiHandleOutgoingRouting(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
    add_header(ethernet);
    modify_field(ethernet.dstAddr, 0xffffffffffff);
    modify_field(ethernet.etherType, 0x553);
    modify_field(cis553_metadata.forMe, 0);
    add_to_field(pfuzz_visited.encoding_i8, 1);
}

action ai_nop_pfuzz_tiHandleOutgoingRouting() {
    add_to_field(pfuzz_visited.encoding_i8, 5);
}

action aiHandleIncomingArpReqest_part_one_pfuzz_tiHandleIncomingArpReqest_part_one(mac_sa) {
    modify_field(arp.oper, 2);
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, arp.senderHA);
    modify_field(cis553_metadata.temp, arp.targetPA);
    modify_field(arp.targetHA, arp.senderHA);
    modify_field(arp.senderHA, mac_sa);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, ig_intr_md.ingress_port);
    add_to_field(pfuzz_visited.encoding_i2, 1);
}

action ai_nop_pfuzz_tiHandleIncomingArpReqest_part_one() {
    add_to_field(pfuzz_visited.encoding_i2, 2);
}

action aiHandleIncomingArpReqest_part_two_pfuzz_tiHandleIncomingArpReqest_part_two() {
    modify_field(arp.targetPA, arp.senderPA);
    modify_field(arp.senderPA, cis553_metadata.temp);
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action aDrop_pfuzz_tiHandleIncomingArpReqest_part_two() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action aiHandleIncomingArpResponse_pfuzz_tiHandleIncomingArpResponse() {
    clone_i2e(98);
    add_to_field(pfuzz_visited.encoding_i6, 1);
}

action aiFindNextL3Hop_pfuzz_tiHandleIpv4(nextHop) {
    modify_field(cis553_metadata.nextHop, nextHop);
    add_to_field(pfuzz_visited.encoding_i8, 2);
}

action aiSendToLastHop_pfuzz_tiHandleIpv4() {
    modify_field(cis553_metadata.nextHop, ipv4.dstAddr);
    add_to_field(pfuzz_visited.encoding_i8, 3);
}

action aDrop_pfuzz_tiHandleIpv4() {
    add_to_field(pfuzz_visited.encoding_i8, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action aiForward_pfuzz_tiHandleOutgoingEthernet(mac_sa, mac_da, egress_port) {
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(standard_metadata.egress_spec, egress_port);
    add_to_field(pfuzz_visited.encoding_i3, 2);
}

action aiArpMiss_pfuzz_tiHandleOutgoingEthernet(local_ip, local_mac, local_port) {
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
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action ai_nop_pfuzz_tiHandleOutgoingEthernet() {
    add_to_field(pfuzz_visited.encoding_i3, 3);
}

action aiHandleIncomingRouting_pfuzz_tiHandleIncomingRouting() {
    clone_i2e(98);
    add_to_field(pfuzz_visited.encoding_i7, 1);
}

header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_i0 : 8;
        encoding_i1 : 8;
        encoding_i2 : 8;
        encoding_i3 : 8;
        encoding_i4 : 8;
        encoding_i5 : 8;
        encoding_i6 : 8;
        encoding_i7 : 8;
        encoding_i8 : 8;
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;


calculated_field ipv4.hdrChecksum{
    verify    ipv4_checksum;
    update    ipv4_checksum;
}


action ai_mvbl_0_VIRTUAL_START_hdrarpisValidhdrarpoper2() {
  add_to_field(pfuzz_visited.encoding_i0, 1);
}

table ti_mvbl_0_VIRTUAL_START_hdrarpisValidhdrarpoper2{
  actions {
    ai_mvbl_0_VIRTUAL_START_hdrarpisValidhdrarpoper2;
  }
  default_action: ai_mvbl_0_VIRTUAL_START_hdrarpisValidhdrarpoper2();
}

action ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_hdrdistance_vecisValid() {
  add_to_field(pfuzz_visited.encoding_i1, 1);
}

table ti_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_hdrdistance_vecisValid{
  actions {
    ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_hdrdistance_vecisValid;
  }
  default_action: ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_hdrdistance_vecisValid();
}

action ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_tiHandleIncomingArpReqest_part_two() {
  add_to_field(pfuzz_visited.encoding_i1, 2);
}

table ti_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_tiHandleIncomingArpReqest_part_two{
  actions {
    ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_tiHandleIncomingArpReqest_part_two;
  }
  default_action: ai_mvbl_1_aDrop_pfuzz_tiHandleIncomingEthernet_tiHandleIncomingArpReqest_part_two();
}

action ai_mvbl_3_VIRTUAL_START_hdripv4isValid() {
  add_to_field(pfuzz_visited.encoding_i3, 1);
}

table ti_mvbl_3_VIRTUAL_START_hdripv4isValid{
  actions {
    ai_mvbl_3_VIRTUAL_START_hdripv4isValid;
  }
  default_action: ai_mvbl_3_VIRTUAL_START_hdripv4isValid();
}

action ai_mvbl_4_VIRTUAL_START_hdrarpisValidhdrarpoper1() {
  add_to_field(pfuzz_visited.encoding_i4, 1);
}

table ti_mvbl_4_VIRTUAL_START_hdrarpisValidhdrarpoper1{
  actions {
    ai_mvbl_4_VIRTUAL_START_hdrarpisValidhdrarpoper1;
  }
  default_action: ai_mvbl_4_VIRTUAL_START_hdrarpisValidhdrarpoper1();
}

