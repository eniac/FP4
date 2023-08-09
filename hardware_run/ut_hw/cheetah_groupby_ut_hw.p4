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


parser start {
    extract(pfuzz_visited);
    
    return parse_cheetah;

}


parser parse_cheetah {
    extract(cheetah);

    return ingress;

}


action set_egr(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
}


action _drop() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
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



table table_sketch_one_key {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_one_key_pfuzz_table_sketch_one_key;
        nop_pfuzz_table_sketch_one_key;
    }
    default_action:nop_pfuzz_table_sketch_one_key;
}


table table_sketch_one_value {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_one_value_pfuzz_table_sketch_one_value;
        nop_pfuzz_table_sketch_one_value;
    }
    default_action:nop_pfuzz_table_sketch_one_value;
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



table table_sketch_two_key {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_two_key_pfuzz_table_sketch_two_key;
        nop_pfuzz_table_sketch_two_key;
    }
    default_action:nop_pfuzz_table_sketch_two_key;
}


table table_sketch_two_value {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_two_value_pfuzz_table_sketch_two_value;
        nop_pfuzz_table_sketch_two_value;
    }
    default_action:nop_pfuzz_table_sketch_two_value;
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



table table_sketch_three_key {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_three_key_pfuzz_table_sketch_three_key;
        nop_pfuzz_table_sketch_three_key;
    }
    default_action:nop_pfuzz_table_sketch_three_key;
}


table table_sketch_three_value {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_three_value_pfuzz_table_sketch_three_value;
        nop_pfuzz_table_sketch_three_value;
    }
    default_action:nop_pfuzz_table_sketch_three_value;
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



table table_sketch_four_key {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_four_key_pfuzz_table_sketch_four_key;
        nop_pfuzz_table_sketch_four_key;
    }
    default_action:nop_pfuzz_table_sketch_four_key;
}


table table_sketch_four_value {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_four_value_pfuzz_table_sketch_four_value;
        nop_pfuzz_table_sketch_four_value;
    }
    default_action:nop_pfuzz_table_sketch_four_value;
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



table table_sketch_five_key {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_five_key_pfuzz_table_sketch_five_key;
        nop_pfuzz_table_sketch_five_key;
    }
    default_action:nop_pfuzz_table_sketch_five_key;
}


table table_sketch_five_value {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        action_sketch_five_value_pfuzz_table_sketch_five_value;
        nop_pfuzz_table_sketch_five_value;
    }
    default_action:nop_pfuzz_table_sketch_five_value;
}







table table_hash_init {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        modify_hash_pfuzz_table_hash_init;
        nop_pfuzz_table_hash_init;
    }
    default_action:nop_pfuzz_table_hash_init;
}


table table_hash_init_two {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        modify_hash_two_pfuzz_table_hash_init_two;
        nop_pfuzz_table_hash_init_two;
    }
    default_action:nop_pfuzz_table_hash_init_two;
}


table table_hash_init_three {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        modify_hash_three_pfuzz_table_hash_init_three;
        nop_pfuzz_table_hash_init_three;
    }
    default_action:nop_pfuzz_table_hash_init_three;
}


table table_hash_init_four {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        modify_hash_four_pfuzz_table_hash_init_four;
        nop_pfuzz_table_hash_init_four;
    }
    default_action:nop_pfuzz_table_hash_init_four;
}


table table_hash_init_five {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        modify_hash_five_pfuzz_table_hash_init_five;
        nop_pfuzz_table_hash_init_five;
    }
    default_action:nop_pfuzz_table_hash_init_five;
}


table table_prune {
    reads {
        cheetah.flowId : exact;
    }
    actions {
        prune_pfuzz_table_prune;
        nop_pfuzz_table_prune;
    }
    default_action:nop_pfuzz_table_prune;
}


action action_sketch_one_key_pfuzz_table_sketch_one_key() {
    alu_sketch_one_key . execute_stateful_alu_from_hash ( hash_one );
}

action nop_pfuzz_table_sketch_one_key() {
    add_to_field(pfuzz_visited.encoding_i2, 3);
}

action action_sketch_one_value_pfuzz_table_sketch_one_value() {
    alu_sketch_one_value . execute_stateful_alu_from_hash ( hash_one );
}

action nop_pfuzz_table_sketch_one_value() {
    add_to_field(pfuzz_visited.encoding_i4, 5);
}

action action_sketch_two_key_pfuzz_table_sketch_two_key() {
    alu_sketch_two_key . execute_stateful_alu_from_hash ( hash_two );
}

action nop_pfuzz_table_sketch_two_key() {
    add_to_field(pfuzz_visited.encoding_i0, 8);
}

action action_sketch_two_value_pfuzz_table_sketch_two_value() {
    alu_sketch_two_value . execute_stateful_alu_from_hash ( hash_two );
}

action nop_pfuzz_table_sketch_two_value() {
    add_to_field(pfuzz_visited.encoding_i0, 4);
}

action action_sketch_three_key_pfuzz_table_sketch_three_key() {
    alu_sketch_three_key . execute_stateful_alu_from_hash ( hash_three );
}

action nop_pfuzz_table_sketch_three_key() {
    add_to_field(pfuzz_visited.encoding_i2, 1);
}

action action_sketch_three_value_pfuzz_table_sketch_three_value() {
    alu_sketch_three_value . execute_stateful_alu_from_hash ( hash_three );
}

action nop_pfuzz_table_sketch_three_value() {
    add_to_field(pfuzz_visited.encoding_i4, 2);
}

action action_sketch_four_key_pfuzz_table_sketch_four_key() {
    alu_sketch_four_key . execute_stateful_alu_from_hash ( hash_four );
}

action nop_pfuzz_table_sketch_four_key() {
    add_to_field(pfuzz_visited.encoding_i4, 1);
}

action action_sketch_four_value_pfuzz_table_sketch_four_value() {
    alu_sketch_four_value . execute_stateful_alu_from_hash ( hash_four );
}

action nop_pfuzz_table_sketch_four_value() {
    add_to_field(pfuzz_visited.encoding_i3, 4);
}

action action_sketch_five_key_pfuzz_table_sketch_five_key() {
    alu_sketch_five_key . execute_stateful_alu_from_hash ( hash_five );
}

action nop_pfuzz_table_sketch_five_key() {
    add_to_field(pfuzz_visited.encoding_i3, 2);
}

action action_sketch_five_value_pfuzz_table_sketch_five_value() {
    alu_sketch_five_value . execute_stateful_alu_from_hash ( hash_five );
}

action nop_pfuzz_table_sketch_five_value() {
    add_to_field(pfuzz_visited.encoding_i0, 2);
}

action modify_hash_pfuzz_table_hash_init() {
    modify_field(cheetah_md.hash_value, cheetah.key);
}

action nop_pfuzz_table_hash_init() {
    add_to_field(pfuzz_visited.encoding_i2, 6);
}

action modify_hash_two_pfuzz_table_hash_init_two() {
    add_to_field(cheetah_md.hash_value, 3);
}

action nop_pfuzz_table_hash_init_two() {
    add_to_field(pfuzz_visited.encoding_i3, 10);
}

action modify_hash_three_pfuzz_table_hash_init_three() {
    add_to_field(cheetah_md.hash_value, 2);
}

action nop_pfuzz_table_hash_init_three() {
    add_to_field(pfuzz_visited.encoding_i1, 3);
}

action modify_hash_four_pfuzz_table_hash_init_four() {
    add_to_field(cheetah_md.hash_value, 2);
}

action nop_pfuzz_table_hash_init_four() {
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action modify_hash_five_pfuzz_table_hash_init_five() {
    add_to_field(cheetah_md.hash_value, 2);
}

action nop_pfuzz_table_hash_init_five() {
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action prune_pfuzz_table_prune() {
    add_to_field(pfuzz_visited.encoding_i1, 2);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action nop_pfuzz_table_prune() {
    add_to_field(pfuzz_visited.encoding_i1, 1);
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


control ingress     {
    apply(table_hash_init);

    apply(table_sketch_one_key);

    apply(table_sketch_one_value);

    apply(table_hash_init_two);

    if (cheetah_md.prune == 0x0)  {
        apply(table_sketch_two_key);

        apply(table_sketch_two_value);

        apply(table_sketch_three_key);

        apply(table_sketch_three_value);

        apply(table_sketch_four_key);

        apply(table_sketch_four_value);

        apply(table_sketch_five_key);

        apply(table_sketch_five_value);


    }

    apply(table_hash_init_three);

    apply(table_hash_init_four);

    apply(table_hash_init_five);

    if (cheetah_md.prune == 0x1)  {
        apply(table_prune);


    }


}


action ai_mvbl_2_action_sketch_one_key_pfuzz_table_sketch_one_key_table_sketch_three_key() {
  add_to_field(pfuzz_visited.encoding_i2, 1);
}

table ti_mvbl_2_action_sketch_one_key_pfuzz_table_sketch_one_key_table_sketch_three_key{
  actions {
    ai_mvbl_2_action_sketch_one_key_pfuzz_table_sketch_one_key_table_sketch_three_key;
  }
  default_action: ai_mvbl_2_action_sketch_one_key_pfuzz_table_sketch_one_key_table_sketch_three_key();
}

action ai_mvbl_3_nop_pfuzz_table_hash_init_two_table_sketch_four_value() {
  add_to_field(pfuzz_visited.encoding_i3, 2);
}

table ti_mvbl_3_nop_pfuzz_table_hash_init_two_table_sketch_four_value{
  actions {
    ai_mvbl_3_nop_pfuzz_table_hash_init_two_table_sketch_four_value;
  }
  default_action: ai_mvbl_3_nop_pfuzz_table_hash_init_two_table_sketch_four_value();
}

action ai_mvbl_4_nop_pfuzz_table_sketch_one_value_table_sketch_three_value() {
  add_to_field(pfuzz_visited.encoding_i4, 1);
}

table ti_mvbl_4_nop_pfuzz_table_sketch_one_value_table_sketch_three_value{
  actions {
    ai_mvbl_4_nop_pfuzz_table_sketch_one_value_table_sketch_three_value;
  }
  default_action: ai_mvbl_4_nop_pfuzz_table_sketch_one_value_table_sketch_three_value();
}

