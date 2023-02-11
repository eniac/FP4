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

parser start_clone {
    
    return parse_ethernet_clone;

}


parser start {
    extract(fp4_visited);
    
    return parse_ethernet;

}


parser parse_ethernet_clone {
    extract(ethernet_clone);

    return select(latest.etherType) { 
        0x0800 : parse_ipv4_clone;
        default  : ingress;
    }

}


parser parse_ethernet {
    extract(ethernet);

    return select(latest.etherType) { 
        0x0800 : parse_ipv4;
        default  : start_clone;
    }

}


parser parse_ipv4_clone {
    extract(ipv4_clone);

    return ingress;

}


parser parse_ipv4 {
    extract(ipv4);

    return start_clone;

}


control ingress {
  if (fp4_visited.pkt_type == 1 || fp4_visited.pkt_type == 3) {
    apply(ti_get_reg_pos);
    apply(ti_read_values_1);
    apply(ti_read_values_2);
    apply(ti_path_assertion);
  }
  if (fp4_visited.pkt_type == 0) {
    apply(ti_get_random_seed);
    apply(ti_turn_on_mutation);
    apply(ti_set_seed_num);
    apply(ti_set_fixed_header);
    apply(ti_set_resubmit);
    apply(ti_create_packet);
    apply(ti_add_fields);
  }
  apply(ti_set_port);
  apply(ti_add_clones);
}

control egress {
  if (fp4_metadata.apply_mutations == 1) {
    apply(te_get_table_seed);
    apply(te_set_table_seed);
    apply(te_move_fields);
    apply(te_apply_mutations0);
    apply(te_move_back_fields);
    apply(te_do_resubmit);
  }
    apply(te_update_count);
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


header_type fp4_metadata_t {
    fields {
        reg_pos_one : 12;
        reg_pos_two : 12;
        reg_val_one : 1;
        reg_val_two : 1;
        apply_mutations : 2;
        temp_port : 4;
        make_clone : 8;
        seed_num : 8;
        fixed_header_seed : 8;
        table_seed : 4;
        __pad1 : 4;
        temp_data : 32;
        max_bits_field0 : 32;
    }
}


metadata fp4_metadata_t fp4_metadata;




header ethernet_t ethernet_clone;
header ipv4_t ipv4_clone;

action ai_get_reg_pos() {
  modify_field_with_hash_based_offset(fp4_metadata.reg_pos_one, 0, bloom_filter_hash_16, 4096);
  modify_field_with_hash_based_offset(fp4_metadata.reg_pos_two, 0, bloom_filter_hash_32, 4096);
}

table ti_get_reg_pos {
  actions {
    ai_get_reg_pos;
  }
  default_action : ai_get_reg_pos();
  size: 0;
}

field_list_calculation bloom_filter_hash_16{
  input { fi_bf_hash_fields_16; }
  algorithm: crc16;
  output_width : 16;
}

field_list_calculation bloom_filter_hash_32{
  input { fi_bf_hash_fields_32; }
  algorithm: crc32;
  output_width : 16;
}

field_list fi_bf_hash_fields_16 {
  fp4_visited.aeInitializeWma;
  fp4_visited.aeWmaPhase1;
  fp4_visited.aeWmaPhase2;
  fp4_visited.ae_rate_limit;
  fp4_visited.ai_noOp;
  fp4_visited.drop_packet_fp4_ipv4_lpm;
  fp4_visited.drop_packet_fp4_te_drop;
  fp4_visited.ipv4_forward;
}


field_list fi_bf_hash_fields_32 {
  fp4_visited.aeInitializeWma;
  fp4_visited.aeWmaPhase1;
  fp4_visited.aeWmaPhase2;
  fp4_visited.ae_rate_limit;
  fp4_visited.ai_noOp;
  fp4_visited.drop_packet_fp4_ipv4_lpm;
  fp4_visited.drop_packet_fp4_te_drop;
  fp4_visited.ipv4_forward;
}


register ri_bloom_filter_1 {
  width: 1;
  instance_count : 4096;
}

register ri_bloom_filter_2 {
  width: 1;
  instance_count : 4096;
}

blackbox stateful_alu ri_bloom_filter_1_alu_update {
    reg: ri_bloom_filter_1;
    output_value: alu_lo;
    output_dst: fp4_metadata.reg_val_one;
    update_lo_1_value: set_bit;
}
blackbox stateful_alu ri_bloom_filter_2_alu_update {
    reg: ri_bloom_filter_2;
    output_value: alu_lo;
    output_dst: fp4_metadata.reg_val_two;
    update_lo_1_value: set_bit;
}
action ai_read_values_1() {
    ri_bloom_filter_1_alu_update.execute_stateful_alu(fp4_metadata.reg_pos_one);
}

action ai_read_values_2() {
    ri_bloom_filter_2_alu_update.execute_stateful_alu(fp4_metadata.reg_pos_two);
}

table ti_read_values_1 {
  actions {
    ai_read_values_1;
  }
  default_action : ai_read_values_1();
  size: 0;
}

table ti_read_values_2 {
  actions {
    ai_read_values_2;
  }
  default_action : ai_read_values_2();
  size: 0;
}

table ti_path_assertion {
  reads {
    fp4_metadata.reg_val_one : exact;
    fp4_metadata.reg_val_two : exact;
  }
  actions {
    ai_send_to_control_plane;
    ai_recycle_packet;
  }
  default_action : ai_recycle_packet();
  size: 4;
}

action ai_send_to_control_plane() {
  modify_field(ig_intr_md_for_tm.ucast_egress_port, 192);
  modify_field(fp4_visited.preamble, 14593470);
  exit();
}

action ai_recycle_packet() {
  remove_header(ethernet);
  remove_header(ethernet_clone);
  remove_header(ipv4);
  remove_header(ipv4_clone);
  modify_field(fp4_visited.pkt_type, 0);
  modify_field(fp4_visited.aeInitializeWma, 0);
  modify_field(fp4_visited.aeWmaPhase1, 0);
  modify_field(fp4_visited.aeWmaPhase2, 0);
  modify_field(fp4_visited.ae_rate_limit, 0);
  modify_field(fp4_visited.ai_noOp, 0);
  modify_field(fp4_visited.drop_packet_fp4_ipv4_lpm, 0);
  modify_field(fp4_visited.drop_packet_fp4_te_drop, 0);
  modify_field(fp4_visited.ipv4_forward, 0);
}

table ti_get_random_seed {
  actions {
    ai_get_random_seed;
  }
  default_action : ai_get_random_seed();
  size: 0;
}

action ai_get_random_seed() {
  modify_field_rng_uniform(fp4_metadata.seed_num, 0 , 255);
  modify_field_rng_uniform(fp4_metadata.make_clone, 0, 255);
  modify_field_rng_uniform(fp4_metadata.fixed_header_seed, 0, 255);
}

table ti_turn_on_mutation {
  actions {
    ai_turn_on_mutation;
  }
  default_action : ai_turn_on_mutation();
  size: 0;
}

action ai_turn_on_mutation() {
  modify_field(fp4_metadata.apply_mutations, 1);
  modify_field_rng_uniform(fp4_metadata.temp_port, 0, 15);
}

action ai_set_seed_num(real_seed_num) {
  modify_field(fp4_metadata.seed_num, real_seed_num);
}

table ti_set_seed_num {
  reads {
    fp4_metadata.seed_num : ternary;
  }
  actions {
    ai_set_seed_num;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 64;
}

action ai_set_resubmit(real_resubmit) {
  modify_field(fp4_metadata.make_clone, real_resubmit);
}

table ti_set_resubmit {
  reads {
    fp4_metadata.make_clone : ternary;
  }
  actions {
    ai_set_resubmit;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 1;
}

table ti_set_fixed_header {
  reads {
    fp4_metadata.make_clone : range;
    fp4_metadata.fixed_header_seed : ternary;
  }
  actions {
    ai_set_fixed_header;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 1;
}

action ai_set_fixed_header(real_fixed_header) {
  modify_field(fp4_metadata.fixed_header_seed, real_fixed_header);
}

table ti_create_packet {
  reads {
    fp4_metadata.seed_num: exact;
  }
  actions {
    ai_drop_packet;
    ai_add_ethernet;
    ai_add_ethernet_ipv4;
  }
  default_action : ai_drop_packet();
  size: 64;
}

action ai_add_ethernet() {
  add_header(ethernet);
  add_header(ethernet_clone);
}

action ai_add_ethernet_ipv4() {
  add_header(ethernet);
  add_header(ethernet_clone);
  add_header(ipv4);
  add_header(ipv4_clone);
  modify_field(ethernet.etherType, 0x0800);
}

table ti_add_fields {
  reads {
    fp4_metadata.seed_num: exact;
  }
  actions {
    ai_NoAction;
    ai_add_fixed_ethernet;
    ai_add_fixed_ethernet_ipv4;
  }
  default_action : ai_NoAction();
  size: 64;
}

action ai_add_fixed_ethernet(ethernetdstAddr, ethernetsrcAddr, ethernetetherType) {
  modify_field(ethernet.dstAddr, ethernetdstAddr);
  modify_field(ethernet.srcAddr, ethernetsrcAddr);
  modify_field(ethernet.etherType, ethernetetherType);
}

action ai_add_fixed_ethernet_ipv4(ethernetdstAddr, ethernetsrcAddr, ethernetetherType, ipv4version, ipv4ihl, ipv4diffserv, ipv4totalLen, ipv4identification, ipv4flags, ipv4fragOffset, ipv4ttl, ipv4protocol, ipv4hdrChecksum, ipv4srcAddr, ipv4dstAddr) {
  modify_field(ethernet.dstAddr, ethernetdstAddr);
  modify_field(ethernet.srcAddr, ethernetsrcAddr);
  modify_field(ethernet.etherType, ethernetetherType);
  modify_field(ipv4.version, ipv4version);
  modify_field(ipv4.ihl, ipv4ihl);
  modify_field(ipv4.diffserv, ipv4diffserv);
  modify_field(ipv4.totalLen, ipv4totalLen);
  modify_field(ipv4.identification, ipv4identification);
  modify_field(ipv4.flags, ipv4flags);
  modify_field(ipv4.fragOffset, ipv4fragOffset);
  modify_field(ipv4.ttl, ipv4ttl);
  modify_field(ipv4.protocol, ipv4protocol);
  modify_field(ipv4.hdrChecksum, ipv4hdrChecksum);
  modify_field(ipv4.srcAddr, ipv4srcAddr);
  modify_field(ipv4.dstAddr, ipv4dstAddr);
}

table ti_set_port {
  reads { 
    fp4_metadata.temp_port : exact;
  }
    actions {
    ai_set_port;
  }
  default_action : ai_set_port(0);
  size: 16;
}

action ai_set_port(outPort) {
  modify_field(ig_intr_md_for_tm.ucast_egress_port, outPort);
}

table te_get_table_seed {
  actions {
    ae_get_table_seed;
  }
  default_action : ae_get_table_seed();
}

action ae_get_table_seed() {
  modify_field_rng_uniform(fp4_metadata.table_seed, 0, 15);
}

action ae_set_table_seed(real_table_seed) {
  modify_field(fp4_metadata.table_seed, real_table_seed);
}

table te_set_table_seed {
  reads {
    fp4_metadata.seed_num : exact;
    fp4_metadata.table_seed : ternary;
  }
  actions {
    ae_set_table_seed;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 256;
}

table te_move_fields {
  reads {
    fp4_metadata.seed_num: exact;
    fp4_metadata.table_seed: exact;
  }
  actions {
    ai_NoAction;
    ae_move_ipv4_lpm;
  }
  default_action : ai_NoAction();
}

action ae_move_ipv4_lpm(){
  modify_field(fp4_metadata.max_bits_field0, ipv4.dstAddr);
}
table te_apply_mutations0 {
  actions {
    ae_apply_mutations0;
  }
  default_action : ae_apply_mutations0();
}

action ae_apply_mutations0() {
  modify_field_rng_uniform(fp4_metadata.max_bits_field0, 0,0xffffffff);
}

table te_move_back_fields {
  reads {
    fp4_metadata.seed_num: exact;
    fp4_metadata.table_seed: exact;
    fp4_metadata.fixed_header_seed: ternary;
  }
  actions {
    ai_NoAction;
    ai_drop_packet;
    ae_move_back_ipv4_lpm;
    ae_move_back_fix_ipv4_lpm;
  }
  default_action : ai_NoAction();
  size: 2048;
}

action ae_move_back_ipv4_lpm(){
  modify_field(ipv4.dstAddr, fp4_metadata.max_bits_field0);
  modify_field(ipv4_clone.dstAddr, fp4_metadata.max_bits_field0);
}
action ae_move_back_fix_ipv4_lpm(ipv4_dstAddr){
  modify_field(ipv4.dstAddr, ipv4_dstAddr);
  modify_field(ipv4_clone.dstAddr, ipv4_dstAddr);
}

field_list fe_resub_fields {
  fp4_metadata.apply_mutations;
  fp4_metadata.seed_num;
}

table te_do_resubmit {
  reads {
    fp4_metadata.make_clone: exact;
  }
  actions {
    ae_resubmit;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 256;
}

action ae_resubmit() {
  modify_field(fp4_visited.pkt_type, 3);
  clone_egress_pkt_to_egress(99, fe_resub_fields);
}

action ai_drop_packet() {
  drop();
}

action ai_NoAction() {
}

table ti_add_clones {
  reads {
    ethernet.valid: exact;
    ipv4.valid: exact;
  }
  // I think the default action should be ai_NoAction
  actions {
    ai_NoAction;
    ai_add_clone_ethernet;
    ai_add_clone_ethernet_ipv4;
  }
  default_action : ai_NoAction();
  size: 16;
}

action ai_add_clone_ethernet() {
  modify_field(ethernet_clone.dstAddr, ethernet.dstAddr);
  modify_field(ethernet_clone.srcAddr, ethernet.srcAddr);
  modify_field(ethernet_clone.etherType, ethernet.etherType);
}

action ai_add_clone_ethernet_ipv4() {
  modify_field(ethernet_clone.dstAddr, ethernet.dstAddr);
  modify_field(ethernet_clone.srcAddr, ethernet.srcAddr);
  modify_field(ethernet_clone.etherType, ethernet.etherType);
  modify_field(ipv4_clone.version, ipv4.version);
  modify_field(ipv4_clone.ihl, ipv4.ihl);
  modify_field(ipv4_clone.diffserv, ipv4.diffserv);
  modify_field(ipv4_clone.totalLen, ipv4.totalLen);
  modify_field(ipv4_clone.identification, ipv4.identification);
  modify_field(ipv4_clone.flags, ipv4.flags);
  modify_field(ipv4_clone.fragOffset, ipv4.fragOffset);
  modify_field(ipv4_clone.ttl, ipv4.ttl);
  modify_field(ipv4_clone.protocol, ipv4.protocol);
  modify_field(ipv4_clone.hdrChecksum, ipv4.hdrChecksum);
  modify_field(ipv4_clone.srcAddr, ipv4.srcAddr);
  modify_field(ipv4_clone.dstAddr, ipv4.dstAddr);
}

register forward_count_register {
  width: 32;
  instance_count: 1;
}
blackbox stateful_alu forward_count_register_alu {
    reg: forward_count_register;
    output_value: register_lo;
    output_dst: fp4_metadata.temp_data;
    update_lo_1_value: register_lo + 1;
}
action ae_update_count() {
    forward_count_register_alu.execute_stateful_alu(0);
}
table te_update_count{
  actions {
    ae_update_count;
  }
  default_action: ae_update_count();
  size: 0;
}

