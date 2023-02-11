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


header_type metadata_t {
    fields {
        count_mirror : 32;
        count_original : 32;
        diff1 : 32 (saturating);
        diff2 : 32 (saturating);
    }
}


metadata metadata_t meta;


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

parser start {
    extract(fp4_visited);
    
    return parse_ethernet;

}


parser parse_ethernet {
    extract(ethernet);

    return select(ethernet.etherType) { 
        0x0800 : parse_ipv4;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return ingress;

}


action aiSetOutputPort() {
    add_to_field(ig_intr_md_for_tm.ucast_egress_port, 4);
    modify_field(fp4_visited.aiSetOutputPort, 1);
}

table tiSetOutputPort {
    actions {
        aiSetOutputPort;
    }
    default_action:aiSetOutputPort();
}


field_list flLastSeen {
    meta.count_mirror;
    meta.count_original;
}

action aiSendClone() {
    clone_i2e(98, flLastSeen);
    modify_field(fp4_visited.aiSendClone, 1);
}

table tiSendClone {
    reads {
        ethernet.etherType : exact;
    }
    actions {
        aiSendClone;
    }
    default_action:aiSendClone();
}


control ingress     {
    apply(tiSendClone);

    apply(tiSetOutputPort);


}

register reg_mirror {
    width:32;
    instance_count:1;
}


register reg_original {
    width:32;
    instance_count:1;
}


blackbox stateful_alu bb_mirror_read {
    reg : reg_mirror;

    output_value : register_lo;

    output_dst : meta.count_mirror;


}

action aeReadMirror() {
    bb_mirror_read . execute_stateful_alu ( 0 );
}

table teReadMirror {
    actions {
        aeReadMirror;
    }
    default_action:aeReadMirror();
}


blackbox stateful_alu bb_mirror_update {
    reg : reg_mirror;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_mirror;


}

action aeUpdateMirror() {
    bb_mirror_update . execute_stateful_alu ( 0 );
}

table teUpdateMirror {
    actions {
        aeUpdateMirror;
    }
    default_action:aeUpdateMirror();
}


blackbox stateful_alu bb_original_read {
    reg : reg_original;

    output_value : register_lo;

    output_dst : meta.count_original;


}

action aeReadOriginal() {
    bb_original_read . execute_stateful_alu ( 0 );
}

table teReadOriginal {
    actions {
        aeReadOriginal;
    }
    default_action:aeReadOriginal();
}


blackbox stateful_alu bb_original_update {
    reg : reg_original;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_original;


}

action aeUpdateOriginal() {
    bb_original_update . execute_stateful_alu ( 0 );
}

table teUpdateOriginal {
    actions {
        aeUpdateOriginal;
    }
    default_action:aeUpdateOriginal();
}


action aeGetDiff() {
    subtract(meta.diff1, meta.count_original, meta.count_mirror);
    subtract(meta.diff2, meta.count_mirror, meta.count_original);
    modify_field(fp4_visited.aeGetDiff, 1);
}

table teGetDiff {
    actions {
        aeGetDiff;
    }
    default_action:aeGetDiff();
}


header_type fp4_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        aeGetDiff : 1;
        aeReadMirror : 1;
        aeReadOriginal : 1;
        aeUpdateMirror : 1;
        aeUpdateOriginal : 1;
        aiSendClone : 1;
        aiSetOutputPort : 1;
        __pad : 7;
    }
}


header fp4_visited_t fp4_visited;


control egress     {
    if (eg_intr_md.egress_port == 40)  {
        apply(teUpdateMirror);

        apply(tstate_teUpdateMirror);

        apply(teReadOriginal);

        apply(tstate_teReadOriginal);


    }
    else  {
        apply(teReadMirror);

        apply(tstate_teReadMirror);

        apply(teUpdateOriginal);

        apply(tstate_teUpdateOriginal);


    }

    apply(teGetDiff);


}


action astate_teReadMirror() {
    modify_field(fp4_visited.aeReadMirror, 1);
}

table tstate_teReadMirror {
    actions {
        astate_teReadMirror;
    }
    default_action : astate_teReadMirror();
    size: 0;
}

action astate_teReadOriginal() {
    modify_field(fp4_visited.aeReadOriginal, 1);
}

table tstate_teReadOriginal {
    actions {
        astate_teReadOriginal;
    }
    default_action : astate_teReadOriginal();
    size: 0;
}

action astate_teUpdateMirror() {
    modify_field(fp4_visited.aeUpdateMirror, 1);
}

table tstate_teUpdateMirror {
    actions {
        astate_teUpdateMirror;
    }
    default_action : astate_teUpdateMirror();
    size: 0;
}

action astate_teUpdateOriginal() {
    modify_field(fp4_visited.aeUpdateOriginal, 1);
}

table tstate_teUpdateOriginal {
    actions {
        astate_teUpdateOriginal;
    }
    default_action : astate_teUpdateOriginal();
    size: 0;
}

