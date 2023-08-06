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

parser start_clone {
    
    return parse_ethernet_clone;

}


parser start {
    extract(pfuzz_visited);
    
    return parse_ethernet;

}


parser parse_ethernet_clone {
    extract(ethernet_clone);

    return select(ethernet_clone.etherType) { 
        0x0800 : parse_ipv4_clone;
        default  : ingress;
    }

}


parser parse_ethernet {
    extract(ethernet);

    return select(ethernet.etherType) { 
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




field_list flLastSeen {
    meta.count_mirror;
    meta.count_original;
}



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



blackbox stateful_alu bb_mirror_update {
    reg : reg_mirror;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_mirror;


}



blackbox stateful_alu bb_original_read {
    reg : reg_original;

    output_value : register_lo;

    output_dst : meta.count_original;


}



blackbox stateful_alu bb_original_update {
    reg : reg_original;

    update_lo_1_value : register_lo + 1;

    output_value : register_lo;

    output_dst : meta.count_original;


}












header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_e0 : 8;
        encoding_e1 : 8;
        encoding_e2 : 8;
        encoding_e3 : 8;
        encoding_e4 : 8;
        encoding_e5 : 8;
        encoding_i0 : 8;
        encoding_i1 : 8;
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


header ethernet_t ethernet_clone;
header ipv4_t ipv4_clone;

action ai_get_reg_pos() {
  modify_field_with_hash_based_offset(pfuzz_metadata.reg_pos_one, 0, bloom_filter_hash_16, 4096);
  modify_field_with_hash_based_offset(pfuzz_metadata.reg_pos_two, 0, bloom_filter_hash_32, 4096);
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
  pfuzz_visited.encoding_e2;
  pfuzz_visited.encoding_e4;
  pfuzz_visited.encoding_e5;
  pfuzz_visited.encoding_e3;
}


field_list fi_bf_hash_fields_32 {
  pfuzz_visited.encoding_e2;
  pfuzz_visited.encoding_e4;
  pfuzz_visited.encoding_e5;
  pfuzz_visited.encoding_e3;
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
    output_dst: pfuzz_metadata.reg_val_one;
    update_lo_1_value: set_bit;
}
blackbox stateful_alu ri_bloom_filter_2_alu_update {
    reg: ri_bloom_filter_2;
    output_value: alu_lo;
    output_dst: pfuzz_metadata.reg_val_two;
    update_lo_1_value: set_bit;
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
  remove_header(ethernet);
  remove_header(ethernet_clone);
  remove_header(ipv4);
  remove_header(ipv4_clone);
  modify_field(pfuzz_visited.pkt_type, 0);
  modify_field(pfuzz_visited.encoding_e2, 0);
  modify_field(pfuzz_visited.encoding_e4, 0);
  modify_field(pfuzz_visited.encoding_e5, 0);
  modify_field(pfuzz_visited.encoding_e3, 0);
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
    pfuzz_metadata.seed_num: exact;
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
    ae_move_tiSendClone;
  }
  default_action : ai_NoAction();
}

action ae_move_tiSendClone(){
  modify_field(pfuzz_metadata.max_bits_field0, ethernet.etherType);
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
    ae_move_back_tiSendClone;
    ae_move_back_fix_tiSendClone;
  }
  default_action : ai_NoAction();
  size: 2048;
}

action ae_move_back_tiSendClone(){
  modify_field(ethernet.etherType, pfuzz_metadata.max_bits_field0);
  modify_field(ethernet_clone.etherType, pfuzz_metadata.max_bits_field0);
}
action ae_move_back_fix_tiSendClone(ethernet_etherType){
  modify_field(ethernet.etherType, ethernet_etherType);
  modify_field(ethernet_clone.etherType, ethernet_etherType);
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

action ai_set_visited_type() {
    modify_field(pfuzz_visited.pkt_type, 1);
}

table ti_set_visited_type {
    actions { ai_set_visited_type; }
    default_action: ai_set_visited_type();
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

