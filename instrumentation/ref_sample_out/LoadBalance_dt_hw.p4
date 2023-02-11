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

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 3;
        ecn : 3;
        ctrl : 6;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}


header tcp_t tcp;


header_type metadata_t {
    fields {
        ecmp_select : 16;
    }
}


metadata metadata_t meta;


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

    return select(ipv4_clone.protocol) { 
        6 : parse_tcp_clone;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return select(ipv4.protocol) { 
        6 : parse_tcp;
        default  : start_clone;
    }

}


parser parse_tcp_clone {
    extract(tcp_clone);

    return ingress;

}


parser parse_tcp {
    extract(tcp);

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




field_list ecmp_fields {
    ipv4.srcAddr;
    ipv4.dstAddr;
    ipv4.protocol;
    tcp.srcPort;
    tcp.dstPort;
}

field_list_calculation ecmp_hasher {
    input    {
        ecmp_fields;
    }
    algorithm:crc16;
    output_width:16;
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
    apply(ti_set_visited_type);
    apply(te_update_count);
}





header_type fp4_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        drop_packet_fp4_ecmp_group : 1;
        drop_packet_fp4_ecmp_nhop : 1;
        drop_packet_fp4_send_frame : 1;
        rewrite_mac : 1;
        set_ecmp_select : 1;
        set_nhop : 1;
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
header tcp_t tcp_clone;

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
  fp4_visited.drop_packet_fp4_ecmp_group;
  fp4_visited.drop_packet_fp4_ecmp_nhop;
  fp4_visited.drop_packet_fp4_send_frame;
  fp4_visited.rewrite_mac;
  fp4_visited.set_ecmp_select;
  fp4_visited.set_nhop;
}


field_list fi_bf_hash_fields_32 {
  fp4_visited.drop_packet_fp4_ecmp_group;
  fp4_visited.drop_packet_fp4_ecmp_nhop;
  fp4_visited.drop_packet_fp4_send_frame;
  fp4_visited.rewrite_mac;
  fp4_visited.set_ecmp_select;
  fp4_visited.set_nhop;
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
  remove_header(tcp);
  remove_header(tcp_clone);
  modify_field(fp4_visited.pkt_type, 0);
  modify_field(fp4_visited.drop_packet_fp4_ecmp_group, 0);
  modify_field(fp4_visited.drop_packet_fp4_ecmp_nhop, 0);
  modify_field(fp4_visited.drop_packet_fp4_send_frame, 0);
  modify_field(fp4_visited.rewrite_mac, 0);
  modify_field(fp4_visited.set_ecmp_select, 0);
  modify_field(fp4_visited.set_nhop, 0);
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
    ai_add_ethernet_ipv4_tcp;
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

action ai_add_ethernet_ipv4_tcp() {
  add_header(ethernet);
  add_header(ethernet_clone);
  add_header(ipv4);
  add_header(ipv4_clone);
  add_header(tcp);
  add_header(tcp_clone);
  modify_field(ethernet.etherType, 0x0800);
  modify_field(ipv4.protocol, 6);
}

table ti_add_fields {
  reads {
    fp4_metadata.seed_num: exact;
  }
  actions {
    ai_NoAction;
    ai_add_fixed_ethernet;
    ai_add_fixed_ethernet_ipv4;
    ai_add_fixed_ethernet_ipv4_tcp;
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

action ai_add_fixed_ethernet_ipv4_tcp(ethernetdstAddr, ethernetsrcAddr, ethernetetherType, ipv4version, ipv4ihl, ipv4diffserv, ipv4totalLen, ipv4identification, ipv4flags, ipv4fragOffset, ipv4ttl, ipv4protocol, ipv4hdrChecksum, ipv4srcAddr, ipv4dstAddr, tcpsrcPort, tcpdstPort, tcpseqNo, tcpackNo, tcpdataOffset, tcpres, tcpecn, tcpctrl, tcpwindow, tcpchecksum, tcpurgentPtr) {
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
  modify_field(tcp.srcPort, tcpsrcPort);
  modify_field(tcp.dstPort, tcpdstPort);
  modify_field(tcp.seqNo, tcpseqNo);
  modify_field(tcp.ackNo, tcpackNo);
  modify_field(tcp.dataOffset, tcpdataOffset);
  modify_field(tcp.res, tcpres);
  modify_field(tcp.ecn, tcpecn);
  modify_field(tcp.ctrl, tcpctrl);
  modify_field(tcp.window, tcpwindow);
  modify_field(tcp.checksum, tcpchecksum);
  modify_field(tcp.urgentPtr, tcpurgentPtr);
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
    ae_move_send_frame;
  }
  default_action : ai_NoAction();
}

action ae_move_send_frame(){
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
    ae_move_back_send_frame;
    ae_move_back_fix_send_frame;
  }
  default_action : ai_NoAction();
  size: 2048;
}

action ae_move_back_send_frame(){
  modify_field(ipv4.dstAddr, fp4_metadata.max_bits_field0);
  modify_field(ipv4_clone.dstAddr, fp4_metadata.max_bits_field0);
}
action ae_move_back_fix_send_frame(ipv4_dstAddr){
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
    tcp.valid: exact;
  }
  // I think the default action should be ai_NoAction
  actions {
    ai_NoAction;
    ai_add_clone_ethernet;
    ai_add_clone_ethernet_ipv4;
    ai_add_clone_ethernet_ipv4_tcp;
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

action ai_add_clone_ethernet_ipv4_tcp() {
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
  modify_field(tcp_clone.srcPort, tcp.srcPort);
  modify_field(tcp_clone.dstPort, tcp.dstPort);
  modify_field(tcp_clone.seqNo, tcp.seqNo);
  modify_field(tcp_clone.ackNo, tcp.ackNo);
  modify_field(tcp_clone.dataOffset, tcp.dataOffset);
  modify_field(tcp_clone.res, tcp.res);
  modify_field(tcp_clone.ecn, tcp.ecn);
  modify_field(tcp_clone.ctrl, tcp.ctrl);
  modify_field(tcp_clone.window, tcp.window);
  modify_field(tcp_clone.checksum, tcp.checksum);
  modify_field(tcp_clone.urgentPtr, tcp.urgentPtr);
}

action ai_set_visited_type() {
    modify_field(fp4_visited.pkt_type, 1);
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
