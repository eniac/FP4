#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/primitives.p4>

#define ETHERTYPE_IPV4 0x0800
#define NUM_PORTS 2


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
        cur_ts : 32;
        interArrival : 32;
        current_reading: 32 (saturating);
        bitCounter : 1;
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
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4; 
        default : ingress;         
    }
}

#define IPV4_TCP_PROTOCOL 6

parser parse_ipv4 {
    extract(ipv4);
    return ingress;
}




control ingress {
    if (valid(ipv4)) {
        apply(ipv4_lpm);
    }
}

control egress {
    apply(teInitializeWma);
    apply(teWmaPhase1);
    apply(teWmaPhase2);
    apply(te_rate_limit);
    if (meta.current_reading == 0) {
        apply(te_drop);
    }
}

table te_drop {
    actions {
        drop_packet;
    }
    default_action : drop_packet();
    size: 1;
}

table te_rate_limit {
    actions {
        ae_rate_limit;
    }
    default_action : ae_rate_limit(1024);
    size: 1;
}

action ae_rate_limit(x) {
    subtract(meta.current_reading, x, meta.current_reading);
}

table teInitializeWma {
    actions { 
        aeInitializeWma; 
    }
    default_action : aeInitializeWma();
    size : 1;
}
action aeInitializeWma(){
    modify_field(meta.cur_ts, eg_intr_md_from_parser_aux.egress_global_tstamp);
    reBitCounter.execute_stateful_alu(ig_intr_md_for_tm.ucast_egress_port);
}

blackbox stateful_alu reBitCounter {
    reg: reg_bit_counter_egress;
    update_lo_1_value: register_lo + 1;
    output_value: register_lo;
    output_dst: meta.bitCounter;
}
register reg_bit_counter_egress {
    width : 8;
    instance_count : NUM_PORTS;
}

table teWmaPhase1 {
    actions { aeWmaPhase1; }
    default_action : aeWmaPhase1();
    size : 1;
}

action aeWmaPhase1() {
    reWmaPhase1.execute_stateful_alu(ig_intr_md_for_tm.ucast_egress_port);
}
blackbox stateful_alu reWmaPhase1 {
    reg: reg_last_timestamp_egress;
    update_lo_1_value: meta.cur_ts;  // lo register stores last ts.
    update_hi_1_value: meta.cur_ts - register_lo; // hi register stores interArrival: (current ts - last ts)
    output_value: alu_hi;  // write interArrival to metadata.
    output_dst: meta.interArrival;
}
register reg_last_timestamp_egress {
    width : 64;
    instance_count : NUM_PORTS;
}

table teWmaPhase2 {
    actions { aeWmaPhase2; }
    default_action : aeWmaPhase2();
    size : 1;
}
action aeWmaPhase2() {
    reWmaPhase2.execute_stateful_alu(ig_intr_md_for_tm.ucast_egress_port);
}

blackbox stateful_alu reWmaPhase2 {
    reg : reg_interval_wma_egress;
    condition_lo : meta.bitCounter == 0;
    // if (for reg_lo)
    update_lo_1_predicate : condition_lo;
    update_lo_1_value : register_lo + meta.interArrival;
    // else (for reg_lo)
    update_lo_2_predicate : not condition_lo;
    update_lo_2_value : math_unit;
    math_unit_input : register_lo;
    math_unit_exponent_shift : 0;
    math_unit_exponent_invert : false;
    math_unit_lookup_table : 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0;
    math_unit_output_scale : 60;
    // if (for reg_hi)
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : register_lo;

    output_value : alu_hi;
    output_dst : meta.current_reading;
}
register reg_interval_wma_egress {
    width : 64;
    instance_count : NUM_PORTS;
}
action drop_packet() {
    drop();
}

action ai_noOp() {
}

table ipv4_lpm {
    reads {
        ipv4.dstAddr: lpm;
    }
    actions  {
        ipv4_forward;
        drop_packet;
        ai_noOp;
    }
    default_action : drop_packet();
    size : 1024;
}

action ipv4_forward(dstAddr, port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    modify_field(ethernet.srcAddr, dstAddr);
    modify_field(ethernet.dstAddr, dstAddr);
    subtract_from_field(ipv4.ttl, 1);
}
