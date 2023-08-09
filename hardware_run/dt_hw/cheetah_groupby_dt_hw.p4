#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>
header_type cheetah_t {
    fields {
        flowId : 16;
        rowId : 32;
        key : 32;
    }
}


header_type cheetah_md_t {
    fields {
        prune : 8;
        sketch_one_same_key : 8;
        sketch_two_same_key : 8;
        sketch_three_same_key : 8;
        sketch_four_same_key : 8;
        sketch_five_same_key : 8;
        hash_value : 32;
    }
}


metadata cheetah_md_t cheetah_md;


header cheetah_t cheetah;


parser start_clone {
    
    return parse_cheetah_clone;

}


parser start {
    extract(pfuzz_visited);
    
    return parse_cheetah;

}


parser parse_cheetah_clone {
    extract(cheetah_clone);

    return ingress;

}


parser parse_cheetah {
    extract(cheetah);

    return start_clone;

}






register reg_sketch_one_key {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_one_key {
    reg : reg_sketch_one_key;

    update_lo_1_value : cheetah.key;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.sketch_one_same_key;


}

register reg_sketch_one_value {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_one_value {
    reg : reg_sketch_one_value;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.prune;


}

field_list hash_list_one {
    cheetah_md.hash_value;
}

field_list_calculation hash_one {
    input    {
        hash_list_one;
    }
    algorithm:crc32;
    output_width:32;
}





register reg_sketch_two_key {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_two_key {
    reg : reg_sketch_two_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not   condition_lo;

    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;

    update_hi_1_value : 1;

    update_hi_2_predicate : not   condition_lo;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.sketch_two_same_key;


}

register reg_sketch_two_value {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_two_value {
    reg : reg_sketch_two_value;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.prune;


}

field_list hash_list_two {
    cheetah_md.hash_value;
}

field_list_calculation hash_two {
    input    {
        hash_list_two;
    }
    algorithm:crc32;
    output_width:17;
}





register reg_sketch_three_key {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_three_key {
    reg : reg_sketch_three_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not   condition_lo;

    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;

    update_hi_1_value : 1;

    update_hi_2_predicate : not   condition_lo;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.sketch_three_same_key;


}

register reg_sketch_three_value {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_three_value {
    reg : reg_sketch_three_value;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.prune;


}

field_list hash_list_three {
    cheetah_md.hash_value;
}

field_list_calculation hash_three {
    input    {
        hash_list_three;
    }
    algorithm:crc32;
    output_width:17;
}





register reg_sketch_four_key {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_four_key {
    reg : reg_sketch_four_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not   condition_lo;

    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;

    update_hi_1_value : 1;

    update_hi_2_predicate : not   condition_lo;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.sketch_four_same_key;


}

register reg_sketch_four_value {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_four_value {
    reg : reg_sketch_four_value;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.prune;


}

field_list hash_list_four {
    cheetah_md.hash_value;
}

field_list_calculation hash_four {
    input    {
        hash_list_four;
    }
    algorithm:crc32;
    output_width:17;
}





register reg_sketch_five_key {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_five_key {
    reg : reg_sketch_five_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not   condition_lo;

    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;

    update_hi_1_value : 1;

    update_hi_2_predicate : not   condition_lo;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.sketch_five_same_key;


}

register reg_sketch_five_value {
    width:32;
    instance_count:128;
}


blackbox stateful_alu alu_sketch_five_value {
    reg : reg_sketch_five_value;

    update_hi_1_value : 1;

    update_hi_2_value : 0;

    output_value : alu_hi;

    output_dst : cheetah_md.prune;


}

field_list hash_list_five {
    cheetah_md.hash_value;
}

field_list_calculation hash_five {
    input    {
        hash_list_five;
    }
    algorithm:crc32;
    output_width:17;
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
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;


header_type pfuzz_metadata_t {
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
        max_bits_field0 : 16;
    }
}


metadata pfuzz_metadata_t pfuzz_metadata;


control ingress {
  if (pfuzz_visited.pkt_type == 1 || pfuzz_visited.pkt_type == 3) {
    apply(ti_get_reg_pos);
    apply(ti_read_values_1);
    apply(ti_read_values_2);
    apply(ti_path_assertion);
  }
  if (pfuzz_visited.pkt_type == 0) {
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


header cheetah_t cheetah_clone;

action ai_get_reg_pos() {
  modify_field_with_hash_based_offset(pfuzz_metadata.reg_pos_one, 0, bloom_filter_hash_16, 65536);
  modify_field_with_hash_based_offset(pfuzz_metadata.reg_pos_two, 0, bloom_filter_hash_32, 65536);
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
  pfuzz_visited.encoding_i3;
  pfuzz_visited.encoding_i0;
  pfuzz_visited.encoding_i4;
  pfuzz_visited.encoding_i1;
  pfuzz_visited.encoding_i2;
}


field_list fi_bf_hash_fields_32 {
  pfuzz_visited.encoding_i3;
  pfuzz_visited.encoding_i0;
  pfuzz_visited.encoding_i4;
  pfuzz_visited.encoding_i1;
  pfuzz_visited.encoding_i2;
}


register ri_bloom_filter_1 {
  width: 1;
  instance_count : 65536;
}

register ri_bloom_filter_2 {
  width: 1;
  instance_count : 65536;
}

blackbox stateful_alu ri_bloom_filter_1_alu_update {
    reg: ri_bloom_filter_1;
    output_value: alu_lo;
    output_dst: pfuzz_metadata.reg_val_one;
    update_lo_1_value: set_bit;
    initial_register_lo_value : 0;
}
blackbox stateful_alu ri_bloom_filter_2_alu_update {
    reg: ri_bloom_filter_2;
    output_value: alu_lo;
    output_dst: pfuzz_metadata.reg_val_two;
    update_lo_1_value: set_bit;
    initial_register_lo_value : 0;
}
action ai_read_values_1() {
    ri_bloom_filter_1_alu_update.execute_stateful_alu(pfuzz_metadata.reg_pos_one);
}

action ai_read_values_2() {
    ri_bloom_filter_2_alu_update.execute_stateful_alu(pfuzz_metadata.reg_pos_two);
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
    pfuzz_metadata.reg_val_one : exact;
    pfuzz_metadata.reg_val_two : exact;
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
  modify_field(pfuzz_visited.preamble, 14593470);
  exit();
}

action ai_recycle_packet() {
  remove_header(cheetah);
  remove_header(cheetah_clone);
  modify_field(pfuzz_visited.pkt_type, 0);
  modify_field(pfuzz_visited.encoding_i3, 0);
  modify_field(pfuzz_visited.encoding_i0, 0);
  modify_field(pfuzz_visited.encoding_i4, 0);
  modify_field(pfuzz_visited.encoding_i1, 0);
  modify_field(pfuzz_visited.encoding_i2, 0);
}

table ti_get_random_seed {
  actions {
    ai_get_random_seed;
  }
  default_action : ai_get_random_seed();
  size: 0;
}

action ai_get_random_seed() {
  modify_field_rng_uniform(pfuzz_metadata.seed_num, 0 , 255);
  modify_field_rng_uniform(pfuzz_metadata.make_clone, 0, 255);
  modify_field_rng_uniform(pfuzz_metadata.fixed_header_seed, 0, 255);
}

table ti_turn_on_mutation {
  actions {
    ai_turn_on_mutation;
  }
  default_action : ai_turn_on_mutation();
  size: 0;
}

action ai_turn_on_mutation() {
  modify_field(pfuzz_metadata.apply_mutations, 1);
  modify_field_rng_uniform(pfuzz_metadata.temp_port, 0, 15);
}

action ai_set_seed_num(real_seed_num) {
  modify_field(pfuzz_metadata.seed_num, real_seed_num);
}

table ti_set_seed_num {
  reads {
    pfuzz_metadata.seed_num : ternary;
  }
  actions {
    ai_set_seed_num;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 64;
}

action ai_set_resubmit(real_resubmit) {
  modify_field(pfuzz_metadata.make_clone, real_resubmit);
}

table ti_set_resubmit {
  reads {
    pfuzz_metadata.make_clone : ternary;
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
    pfuzz_metadata.make_clone : range;
    pfuzz_metadata.fixed_header_seed : ternary;
  }
  actions {
    ai_set_fixed_header;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 1;
}

action ai_set_fixed_header(real_fixed_header) {
  modify_field(pfuzz_metadata.fixed_header_seed, real_fixed_header);
}

table ti_create_packet {
  reads {
    pfuzz_metadata.seed_num: exact;
  }
  actions {
    ai_drop_packet;
    ai_add_cheetah;
  }
  default_action : ai_drop_packet();
  size: 64;
}

action ai_add_cheetah() {
  add_header(cheetah);
  add_header(cheetah_clone);
}

table ti_add_fields {
  reads {
    pfuzz_metadata.seed_num: exact;
  }
  actions {
    ai_NoAction;
    ai_add_fixed_cheetah;
  }
  default_action : ai_NoAction();
  size: 64;
}

action ai_add_fixed_cheetah(cheetahflowId, cheetahrowId, cheetahkey) {
  modify_field(cheetah.flowId, cheetahflowId);
  modify_field(cheetah.rowId, cheetahrowId);
  modify_field(cheetah.key, cheetahkey);
}

table ti_set_port {
  reads { 
    pfuzz_metadata.temp_port : exact;
  }
    actions {
    ai_set_port;
  }
  default_action : ai_set_port(0);
  size: 16;
}

action ai_set_port(outPort) {
  modify_field(ig_intr_md_for_tm.ucast_egress_port, outPort);
  modify_field(pfuzz_visited.temp_port, outPort);
}

table te_get_table_seed {
  actions {
    ae_get_table_seed;
  }
  default_action : ae_get_table_seed();
}

action ae_get_table_seed() {
  modify_field_rng_uniform(pfuzz_metadata.table_seed, 0, 15);
}

action ae_set_table_seed(real_table_seed) {
  modify_field(pfuzz_metadata.table_seed, real_table_seed);
}

table te_set_table_seed {
  reads {
    pfuzz_metadata.seed_num : exact;
    pfuzz_metadata.table_seed : ternary;
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
    pfuzz_metadata.seed_num: exact;
    pfuzz_metadata.table_seed: exact;
  }
  actions {
    ai_NoAction;
    ae_move_table_sketch_two_value;
  }
  default_action : ai_NoAction();
}

action ae_move_table_sketch_two_value(){
  modify_field(pfuzz_metadata.max_bits_field0, cheetah.flowId);
}
table te_apply_mutations0 {
  actions {
    ae_apply_mutations0;
  }
  default_action : ae_apply_mutations0();
}

action ae_apply_mutations0() {
  modify_field_rng_uniform(pfuzz_metadata.max_bits_field0, 0,0xffff);
}

table te_move_back_fields {
  reads {
    pfuzz_metadata.seed_num: exact;
    pfuzz_metadata.table_seed: exact;
    pfuzz_metadata.fixed_header_seed: ternary;
  }
  actions {
    ai_NoAction;
    ai_drop_packet;
    ae_move_back_table_sketch_two_value;
    ae_move_back_fix_table_sketch_two_value;
  }
  default_action : ai_NoAction();
  size: 2048;
}

action ae_move_back_table_sketch_two_value(){
  modify_field(cheetah.flowId, pfuzz_metadata.max_bits_field0);
  modify_field(cheetah_clone.flowId, pfuzz_metadata.max_bits_field0);
}
action ae_move_back_fix_table_sketch_two_value(cheetah_flowId){
  modify_field(cheetah.flowId, cheetah_flowId);
  modify_field(cheetah_clone.flowId, cheetah_flowId);
}

field_list fe_resub_fields {
  pfuzz_metadata.apply_mutations;
  pfuzz_metadata.seed_num;
}

table te_do_resubmit {
  reads {
    pfuzz_metadata.make_clone: exact;
  }
  actions {
    ae_resubmit;
    ai_NoAction;
  }
  default_action : ai_NoAction();
  size: 256;
}

action ae_resubmit() {
  modify_field(pfuzz_visited.pkt_type, 3);
  clone_egress_pkt_to_egress(99, fe_resub_fields);
}

action ai_drop_packet() {
  drop();
}

action ai_NoAction() {
}

table ti_add_clones {
  reads {
    cheetah.valid: exact;
  }
  // I think the default action should be ai_NoAction
  actions {
    ai_NoAction;
    ai_add_clone_cheetah;
  }
  default_action : ai_NoAction();
  size: 16;
}

action ai_add_clone_cheetah() {
  modify_field(cheetah_clone.flowId, cheetah.flowId);
  modify_field(cheetah_clone.rowId, cheetah.rowId);
  modify_field(cheetah_clone.key, cheetah.key);
}

action ai_set_visited_type() {
    modify_field(pfuzz_visited.pkt_type, 1);
}

table ti_set_visited_type {
    actions { ai_set_visited_type; }
    default_action: ai_set_visited_type();
}

control egress {
  if (pfuzz_metadata.apply_mutations == 1) {
    apply(te_get_table_seed);
    apply(te_set_table_seed);
    apply(te_move_fields);
    apply(te_apply_mutations0);
    apply(te_move_back_fields);
    apply(te_do_resubmit);
  }
    apply(ti_set_visited_type);
    apply(te_update_count);
}

register forward_count_register {
  width: 32;
  instance_count: 1;
}
blackbox stateful_alu forward_count_register_alu {
    reg: forward_count_register;
    output_value: register_lo;
    output_dst: pfuzz_metadata.temp_data;
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

