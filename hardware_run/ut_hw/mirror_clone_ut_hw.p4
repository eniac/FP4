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
    extract(pfuzz_visited);
    
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



table tiSetOutputPort {
    actions {
        aiSetOutputPort_pfuzz_tiSetOutputPort;
    }
    default_action:aiSetOutputPort_pfuzz_tiSetOutputPort();
}


field_list flLastSeen {
    meta.count_mirror;
    meta.count_original;
}


table tiSendClone {
    reads {
        ethernet.etherType : exact;
    }
    actions {
        aiSendClone_pfuzz_tiSendClone;
    }
    default_action:aiSendClone_pfuzz_tiSendClone();
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


table teReadMirror {
    actions {
        aeReadMirror_pfuzz_teReadMirror;
    }
    default_action:aeReadMirror_pfuzz_teReadMirror();
}


blackbox stateful_alu bb_mirror_update {
    reg : reg_mirror;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_mirror;


}


table teUpdateMirror {
    actions {
        aeUpdateMirror_pfuzz_teUpdateMirror;
    }
    default_action:aeUpdateMirror_pfuzz_teUpdateMirror();
}


blackbox stateful_alu bb_original_read {
    reg : reg_original;

    output_value : register_lo;

    output_dst : meta.count_original;


}


table teReadOriginal {
    actions {
        aeReadOriginal_pfuzz_teReadOriginal;
    }
    default_action:aeReadOriginal_pfuzz_teReadOriginal();
}


blackbox stateful_alu bb_original_update {
    reg : reg_original;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_original;


}


table teUpdateOriginal {
    actions {
        aeUpdateOriginal_pfuzz_teUpdateOriginal;
    }
    default_action:aeUpdateOriginal_pfuzz_teUpdateOriginal();
}



table teGetDiff {
    actions {
        aeGetDiff_pfuzz_teGetDiff;
    }
    default_action:aeGetDiff_pfuzz_teGetDiff();
}


action aiSetOutputPort_pfuzz_tiSetOutputPort() {
    add_to_field(ig_intr_md_for_tm.ucast_egress_port, 4);
}

action aiSendClone_pfuzz_tiSendClone() {
    clone_i2e(98, flLastSeen);
}

action aeReadMirror_pfuzz_teReadMirror() {
    bb_mirror_read . execute_stateful_alu ( 0 );
    add_to_field(pfuzz_visited.encoding_e2, 1);
}

action aeUpdateMirror_pfuzz_teUpdateMirror() {
    bb_mirror_update . execute_stateful_alu ( 0 );
    add_to_field(pfuzz_visited.encoding_e5, 1);
}

action aeReadOriginal_pfuzz_teReadOriginal() {
    bb_original_read . execute_stateful_alu ( 0 );
    add_to_field(pfuzz_visited.encoding_e4, 1);
}

action aeUpdateOriginal_pfuzz_teUpdateOriginal() {
    bb_original_update . execute_stateful_alu ( 0 );
    add_to_field(pfuzz_visited.encoding_e3, 1);
}

action aeGetDiff_pfuzz_teGetDiff() {
    subtract(meta.diff1, meta.count_original, meta.count_mirror);
    subtract(meta.diff2, meta.count_mirror, meta.count_original);
}

header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        encoding_e0 : 8;
        encoding_e1 : 8;
        encoding_e2 : 8;
        encoding_e3 : 8;
        encoding_e4 : 8;
        encoding_e5 : 8;
        encoding_i0 : 8;
        encoding_i1 : 8;
        pkt_type : 2;
        __pad : 6;
    }
}


header pfuzz_visited_t pfuzz_visited;


control egress     {
    if (eg_intr_md.egress_port == 40)  {
        apply(teUpdateMirror);

        apply(teReadOriginal);


    }
    else  {
        apply(teReadMirror);

        apply(teUpdateOriginal);


    }

    apply(teGetDiff);


}


