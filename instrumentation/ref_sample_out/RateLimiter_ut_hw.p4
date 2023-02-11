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
        cur_ts : 32;
        interArrival : 32;
        current_reading : 32 (saturating);
        bitCounter : 1;
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

    return select(latest.etherType) { 
        0x0800 : parse_ipv4;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return ingress;

}


control ingress     {
    if (valid(ipv4))  {
        apply(ipv4_lpm);


    }


}

control egress     {
    apply(teInitializeWma);

    apply(teWmaPhase1);

    apply(tstate_teWmaPhase1);

    apply(teWmaPhase2);

    apply(tstate_teWmaPhase2);

    apply(te_rate_limit);

    if (meta.current_reading == 0)  {
        apply(te_drop);


    }


}

table te_drop {
    actions {
        drop_packet_fp4_te_drop;
    }
    default_action:drop_packet_fp4_te_drop();
    size:1;
}


table te_rate_limit {
    actions {
        ae_rate_limit;
    }
    default_action:ae_rate_limit(1024);
    size:1;
}


action ae_rate_limit(x) {
    subtract(meta.current_reading, x, meta.current_reading);
    modify_field(fp4_visited.ae_rate_limit, 1);
}

table teInitializeWma {
    actions {
        aeInitializeWma;
    }
    default_action:aeInitializeWma();
    size:1;
}


action aeInitializeWma() {
    modify_field(meta.cur_ts, eg_intr_md_from_parser_aux.egress_global_tstamp);
    reBitCounter . execute_stateful_alu ( ig_intr_md_for_tm.ucast_egress_port );
    modify_field(fp4_visited.aeInitializeWma, 1);
}

blackbox stateful_alu reBitCounter {
    reg : reg_bit_counter_egress;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.bitCounter;


}

register reg_bit_counter_egress {
    width:8;
    instance_count:2;
}


table teWmaPhase1 {
    actions {
        aeWmaPhase1;
    }
    default_action:aeWmaPhase1();
    size:1;
}


action aeWmaPhase1() {
    reWmaPhase1 . execute_stateful_alu ( ig_intr_md_for_tm.ucast_egress_port );
}

blackbox stateful_alu reWmaPhase1 {
    reg : reg_last_timestamp_egress;

    update_lo_1_value : meta.cur_ts;

    update_hi_1_value : meta.cur_ts - register_lo;

    output_value : alu_hi;

    output_dst : meta.interArrival;


}

register reg_last_timestamp_egress {
    width:64;
    instance_count:2;
}


table teWmaPhase2 {
    actions {
        aeWmaPhase2;
    }
    default_action:aeWmaPhase2();
    size:1;
}


action aeWmaPhase2() {
    reWmaPhase2 . execute_stateful_alu ( ig_intr_md_for_tm.ucast_egress_port );
}

blackbox stateful_alu reWmaPhase2 {
    reg : reg_interval_wma_egress;

    condition_lo : meta.bitCounter == 0;

    update_lo_1_predicate : condition_lo;

    update_lo_1_value : register_lo + meta.interArrival;

    update_lo_2_predicate : not   condition_lo;

    update_lo_2_value : math_unit;

    math_unit_input : register_lo;

    math_unit_exponent_shift : 0;

    math_unit_exponent_invert : false;

    math_unit_lookup_table : 15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0;

    math_unit_output_scale : 60;

    update_hi_1_predicate : condition_lo;

    update_hi_1_value : register_lo;

    output_value : alu_hi;

    output_dst : meta.current_reading;


}

register reg_interval_wma_egress {
    width:64;
    instance_count:2;
}



action ai_noOp() {
    modify_field(fp4_visited.ai_noOp, 1);
}

table ipv4_lpm {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        ipv4_forward;
        drop_packet_fp4_ipv4_lpm;
        ai_noOp;
    }
    default_action:drop_packet_fp4_ipv4_lpm();
    size:1024;
}


action drop_packet_fp4_te_drop() {
    modify_field(fp4_visited.drop_packet_fp4_te_drop, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action drop_packet_fp4_ipv4_lpm() {
    modify_field(fp4_visited.drop_packet_fp4_ipv4_lpm, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

header_type fp4_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        aeInitializeWma : 1;
        aeWmaPhase1 : 1;
        aeWmaPhase2 : 1;
        ae_rate_limit : 1;
        ai_noOp : 1;
        drop_packet_fp4_ipv4_lpm : 1;
        drop_packet_fp4_te_drop : 1;
        ipv4_forward : 1;
        __pad : 6;
    }
}


header fp4_visited_t fp4_visited;


action ipv4_forward(dstAddr, port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    modify_field(ethernet.srcAddr, dstAddr);
    modify_field(ethernet.dstAddr, dstAddr);
    subtract_from_field(ipv4.ttl, 1);
    modify_field(fp4_visited.ipv4_forward, 1);
}


action astate_teWmaPhase1() {
    modify_field(fp4_visited.aeWmaPhase1, 1);
}

table tstate_teWmaPhase1 {
    actions {
        astate_teWmaPhase1;
    }
    default_action : astate_teWmaPhase1();
    size: 0;
}

action astate_teWmaPhase2() {
    modify_field(fp4_visited.aeWmaPhase2, 1);
}

table tstate_teWmaPhase2 {
    actions {
        astate_teWmaPhase2;
    }
    default_action : astate_teWmaPhase2();
    size: 0;
}

