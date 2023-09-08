#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>

/* -*- P4_14 -*- */

#define CPU_PORT 192
#define CPU_INGRESS_MIRROR_ID 98

/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/

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

// Each element of the distance vector in `data` is:
// bit<32> prefix
// bit<8>  length
// bit<8>  cost
// Total: 48 bits / 6 bytes (max 42 entries)
header_type distance_vec_t {
    fields {
        src : 32;
        length_dist : 16;
        data: 144;
    }
}
header distance_vec_t distance_vec;

// Declare local variables here
header_type cis553_metadata_t {
    fields {
        forMe : 1;
        __pad : 7;
        temp : 32;
        nextHop : 32;
    }
}
metadata cis553_metadata_t cis553_metadata;


/*************************************************************************
***********************  P A R S E   P A C K E T *************************
*************************************************************************/

parser start {
    return select(ig_intr_md.ingress_port) {
        CPU_PORT: parse_routing;
        default: parse_ethernet;
    }
}

parser parse_ethernet {
    extract(ethernet);
    return select(ethernet.etherType) {
        0x0800: parse_ipv4;
        0x0806: parse_arp;
        0x0553: parse_routing;
        default: ingress;
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


/*************************************************************************
***********************  I N G R E S S  **********************************
*************************************************************************/


action aiForMe() {
    modify_field(cis553_metadata.forMe, 1);
}

action aDrop() {
    drop();
}

action ai_nop() {
}

table tiDrop {
    actions {
        aDrop;
    }
    default_action: aDrop();
}

table tiHandleIncomingEthernet {
    reads {
        ethernet.dstAddr : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiForMe;
        aDrop;
    }
    default_action: aDrop();
}


action aiHandleOutgoingRouting(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
    add_header(ethernet);
    modify_field(ethernet.dstAddr, 0xffffffffffff);
    modify_field(ethernet.etherType, 0x553);
    modify_field(cis553_metadata.forMe, 0);
}

table tiHandleOutgoingRouting {
    reads {
        distance_vec.src : exact;
    }
    actions {
        aiHandleOutgoingRouting;
        ai_nop;
    }
    default_action: ai_nop();
}


action aiHandleIncomingArpReqest_part_one(mac_sa) {
    /* ARP Request's oper == 1 and ARP Response's oper == 2 */
    modify_field(arp.oper, 2);

    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, arp.senderHA);

    modify_field(cis553_metadata.temp, arp.targetPA);

    modify_field(arp.targetHA, arp.senderHA);
    modify_field(arp.senderHA, mac_sa);

    modify_field(ig_intr_md_for_tm.ucast_egress_port ,ig_intr_md.ingress_port);

}

table tiHandleIncomingArpReqest_part_one {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_one;
        ai_nop;
    }
    default_action: ai_nop();
}

action aiHandleIncomingArpReqest_part_two() {
    modify_field(arp.targetPA, arp.senderPA);
    modify_field(arp.senderPA, cis553_metadata.temp);
}

table tiHandleIncomingArpReqest_part_two {
    reads {
        arp.targetPA : exact;
        ig_intr_md.ingress_port : exact;
    }
    actions {
        aiHandleIncomingArpReqest_part_two;
        aDrop;
    }
    default_action: aDrop();
}

// field_list arp_digest {
    // ig_intr_md.ingress_port;
//     arp.targetHA;
//     arp.senderHA;
//     arp.senderPA;
// }

action aiHandleIncomingArpResponse() {
    // add_header(port_information);,
    // modify_field(port_information.ingress_port, ig_intr_md.ingress_port);
    clone_i2e(CPU_INGRESS_MIRROR_ID);
}

table tiHandleIncomingArpResponse {
    actions {
        aiHandleIncomingArpResponse;
    }
    default_action: aiHandleIncomingArpResponse();
}


action aiFindNextL3Hop(nextHop) {
    modify_field(cis553_metadata.nextHop, nextHop);
}

action aiSendToLastHop() {
    modify_field(cis553_metadata.nextHop, ipv4.dstAddr);
}

table tiHandleIpv4 {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        aiFindNextL3Hop;
        aiSendToLastHop;
        aDrop;
    }
    default_action: aDrop();
}


action aiForward(mac_sa, mac_da, egress_port) {
    modify_field(ethernet.srcAddr, mac_sa);
    modify_field(ethernet.dstAddr, mac_da);
    modify_field(standard_metadata.egress_spec, egress_port);
}

action aiArpMiss(local_ip, local_mac, local_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, local_port);

    modify_field(ethernet.dstAddr,0xffffffffffff);
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
}

table tiHandleOutgoingEthernet {
    reads {
        cis553_metadata.nextHop : lpm;
    }
    actions {
        aiForward;
        aiArpMiss;
        ai_nop;
    }
    default_action: ai_nop();
}


// field_list routing_digest {
    // distance_vec.src;
//     distance_vec.length_dist;
//     distance_vec.data;
// }

action aiHandleIncomingRouting() {
    // generate_digest(0, routing_digest);
    clone_i2e(CPU_INGRESS_MIRROR_ID);
}

table tiHandleIncomingRouting {
    actions {
        aiHandleIncomingRouting;
    }
    default_action: aiHandleIncomingRouting();
}


control ingress {
    // Check the first header
    if (valid(ethernet)) {
        apply(tiHandleIncomingEthernet);
    } else {
        // routing packets from the control plane come raw (no L2 header)
        apply(tiHandleOutgoingRouting);
    }
    // Check the second header if the Ethernet dst is us
    // if (cis553_metadata.forMe == 0) {
    //     // Don't do anything with it
    // } else if (valid(ipv4)) {
    //    apply(tiHandleIpv4);
    //    apply(tiHandleOutgoingEthernet);
    // } else if (valid(arp) && arp.oper == 1) {
    //     apply(tiHandleIncomingArpReqest_part_one);
    //     apply(tiHandleIncomingArpReqest_part_two);
    // } else if (valid(arp) && arp.oper == 2) {
    //     apply(tiHandleIncomingArpResponse);
    // } else if (valid(distance_vec)) {
    //     apply(tiHandleIncomingRouting);
    // } else {
    //     apply(tiDrop);
    // }
    if (cis553_metadata.forMe == 0) {
        // Don't do anything with it
    } else {
        if (valid(ipv4)) {
            apply(tiHandleIpv4);
            apply(tiHandleOutgoingEthernet);
        } else {
            if (valid(arp) && arp.oper == 1) {
                apply(tiHandleIncomingArpReqest_part_one);
                apply(tiHandleIncomingArpReqest_part_two);
            } else {
                if (valid(arp) && arp.oper == 2) {
                    apply(tiHandleIncomingArpResponse);
                } else {
                    if (valid(distance_vec)) {
                        apply(tiHandleIncomingRouting);
                    }
                    else {
                        apply(tiDrop);
                    }                
                }
            }
        }
    }

}


/*************************************************************************
***********************  E G R E S S  ************************************
*************************************************************************/

control egress {
}


/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   ***************
*************************************************************************/

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
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.hdrChecksum  {
    verify ipv4_checksum;
    update ipv4_checksum;
}