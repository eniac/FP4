#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>

/* -*- P4_14 -*- */

#define CPU_PORT 192
#define CPU_INGRESS_MIRROR_ID 98
#define MC0_INGRESS_MIRROR_ID 100

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

/*************************************************************************
***********************  P A R S E   P A C K E T *************************
*************************************************************************/

parser start {
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return select(ethernet.etherType) {
        0x0800: parse_ipv4;
        default: ingress;
    }
}

parser parse_ipv4 {
    extract(ipv4);
    return ingress;
}
/*************************************************************************
***********************  I N G R E S S  **********************************
*************************************************************************/

action aiNoOp() {}

action aiSendDigest() {
    // TODO: Should include ingress port in the digest
    clone_i2e(CPU_INGRESS_MIRROR_ID);
}

table tiLearnMAC {
    reads {
        ethernet.srcAddr : exact;
    }
    actions {
        aiNoOp;
        aiSendDigest;
    }
    default_action: aiSendDigest();
}

action aiForward(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
}

action aiForwardUnknown() {
    // Goup ID should be the ingress port
    clone_i2e(MC0_INGRESS_MIRROR_ID);
}

table tiForward {
    reads {
        ethernet.dstAddr : exact;
    }
    actions {
        aiForward;
        aiForwardUnknown;
    }

    default_action: aiForwardUnknown();
}


action aiFilter() {
    drop();
}
table tiFilter {
    reads {
        ig_intr_md.ingress_port : exact;
        ig_intr_md_for_tm.ucast_egress_port : exact;
    }
    actions {
        aiFilter;
        aiNoOp;
    }
    default_action: aiNoOp();
}

control ingress {
    apply(tiLearnMAC);
    apply(tiForward);
    apply(tiFilter);
}

/*************************************************************************
***********************  E G R E S S  ************************************
*************************************************************************/

control egress {
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   ***************
*************************************************************************/

