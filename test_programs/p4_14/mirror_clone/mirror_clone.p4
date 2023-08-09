#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>

#define CPU_PORT 192 // For Hardware
#define CPU_INGRESS_MIRROR_ID 98
#define CPU_EGRESS_MIRROR_ID 99
#define MC1_INGRESS_MIRROR_ID 101
#define ETHERTYPE_IPV4 0x0800


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


header_type metadata_t {
    fields {
        count_mirror : 32;
        count_original : 32;
        diff1 : 32 (saturating) ;
        diff2 : 32 (saturating) ;
    }
}

metadata metadata_t meta;

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


parser start {
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return select(ethernet.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4; 
        default : ingress;
    }
}

parser parse_ipv4 {
    extract(ipv4);
    return ingress;
}

action aiSetOutputPort() {
    add_to_field(ig_intr_md_for_tm.ucast_egress_port, 4);
}


table tiSetOutputPort {
    actions {
        aiSetOutputPort;
    }
    default_action: aiSetOutputPort();
}

field_list flLastSeen {
    meta.count_mirror;
    meta.count_original;
}

action aiSendClone() {
    // clone_i2e(CPU_INGRESS_MIRROR_ID, flLastSeen);
    // Should still clone the full packet, otherwise the cloned packet will be invalid syntax
    // clone_i2e(MC1_INGRESS_MIRROR_ID, flLastSeen);
    clone_i2e(MC1_INGRESS_MIRROR_ID);
}


table tiSendClone {
    reads {
        ethernet.etherType : exact;
    }
    actions {
        aiSendClone;
    }
    default_action: aiSendClone(); 
}

control ingress {
    apply(tiSendClone);
    apply(tiSetOutputPort);
}

register reg_mirror {
    width : 32;
    instance_count : 1;
}

register reg_original {
    width : 32;
    instance_count : 1;
}

blackbox stateful_alu bb_mirror_read {
    reg: reg_mirror;
    output_value: register_lo;
    output_dst: meta.count_mirror;
}

action aeReadMirror() {
    bb_mirror_read.execute_stateful_alu(0);
}

table teReadMirror {
    actions {
        aeReadMirror;
    }
    default_action: aeReadMirror();
}

blackbox stateful_alu bb_mirror_update {
    reg: reg_mirror;
    update_lo_1_value: register_lo + 1;
    output_value: register_lo;
    output_dst: meta.count_mirror;
}

action aeUpdateMirror() {
    bb_mirror_update.execute_stateful_alu(0);
}

table teUpdateMirror {
    actions {
        aeUpdateMirror;
    }
    default_action: aeUpdateMirror();
}

blackbox stateful_alu bb_original_read {
    reg: reg_original;
    output_value: register_lo;
    output_dst: meta.count_original;
}

action aeReadOriginal() {
    bb_original_read.execute_stateful_alu(0);
}

table teReadOriginal {
    actions {
        aeReadOriginal;
    }
    default_action: aeReadOriginal();
}

blackbox stateful_alu bb_original_update {
    reg: reg_original;
    update_lo_1_value: register_lo + 1;
    output_value: register_lo;
    output_dst: meta.count_original;
}

action aeUpdateOriginal() {
    bb_original_update.execute_stateful_alu(0);
}

table teUpdateOriginal {
    actions {
        aeUpdateOriginal;
    }
    default_action: aeUpdateOriginal();
}

action aeGetDiff() {
    subtract(meta.diff1, meta.count_original, meta.count_mirror);
    subtract(meta.diff2, meta.count_mirror, meta.count_original);
}

table teGetDiff {
    actions {
        aeGetDiff;
    }
    default_action: aeGetDiff();
}

// action ae_no_op() {}

// action ae_mark_assertion_one() {
//     modify_field(fp4_metadata.assertion_one, 1);
// }

// table te_check_assertion1 {
//     reads {
//         meta.diff1 : range;
//     }
//     actions {
//         ae_no_op;
//         ae_mark_assertion_one;
//     }
// }

// action ae_mark_assertion_two() {
//     modify_field(fp4_metadata.assertion_two, 1);
// }

// table te_check_assertion1 {
//     reads {
//         meta.diff2 : range;
//     }
//     actions {
//         ae_no_op;
//         ae_mark_assertion_two;
//     }
// }


control egress {
    if (eg_intr_md.egress_port == 40) {
        apply(teUpdateMirror);
        apply(teReadOriginal);
    } else {
        apply(teReadMirror);
        apply(teUpdateOriginal);
    }
    apply(teGetDiff);
    
    // apply(te_check_assertion1);  <--  assert(meta.diff1 <= 1);
    
    // apply(te_check_assertion2);  <--  assert(meta.diff2 <= 1);
}
