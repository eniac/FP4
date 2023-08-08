#include <tofino/constants.p4>
#include <tofino/intrinsic_metadata.p4>
#include <tofino/primitives.p4>
#include <tofino/stateful_alu_blackbox.p4>
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


header_type udp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        pkt_length : 16;
        checksum : 16;
    }
}


header udp_t udp;


header_type nlk_hdr_t {
    fields {
        recirc_flag : 8;
        op : 8;
        mode : 8;
        client_id : 8;
        tid : 32;
        lock : 32;
        timestamp_lo : 32;
        timestamp_hi : 32;
    }
}


header nlk_hdr_t nlk_hdr;


header_type adm_hdr_t {
    fields {
        op : 8;
        lock : 32;
        new_left : 32;
        new_right : 32;
    }
}


header adm_hdr_t adm_hdr;


header_type recirculate_hdr_t {
    fields {
        dequeued_mode : 8;
        cur_head : 32;
        cur_tail : 32;
    }
}


header recirculate_hdr_t recirculate_hdr;


header_type probe_hdr_t {
    fields {
        failure_status : 8;
        op : 8;
        mode : 8;
        client_id : 8;
        tid : 32;
        lock : 32;
        timestamp_lo : 32;
        timestamp_hi : 32;
    }
}


header probe_hdr_t probe_hdr;


header_type nc_hdr_t {
    fields {
        recirc_flag : 8;
        op : 8;
        mode : 8;
        client_id : 8;
        tid : 32;
        lock : 32;
        timestamp_lo : 32;
        timestamp_hi : 32;
    }
}


header nc_hdr_t nc_hdr;


parser start_clone {
    
    return parse_ethernet_clone;

}


parser start {
    extract(pfuzz_visited);
    
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

    return select(latest.protocol) { 
        6 : parse_tcp_clone;
        17 : parse_udp_clone;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return select(latest.protocol) { 
        6 : parse_tcp;
        17 : parse_udp;
        default  : start_clone;
    }

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

field_list_calculation ipv4_chksum_calc {
    input    {
        ipv4_field_list;
    }
    algorithm:csum16;
    output_width:16;
}

calculated_field ipv4.hdrChecksum{
    update    ipv4_chksum_calc;
}

parser parse_tcp_clone {
    extract(tcp_clone);

    return ingress;

}


parser parse_tcp {
    extract(tcp);

    return start_clone;

}


parser parse_udp_clone {
    extract(udp_clone);

    return select(latest.dstPort) { 
        6666 : parse_nc_hdr_clone;
        default  : ingress;
    }

}


parser parse_udp {
    extract(udp);

    return select(latest.dstPort) { 
        6666 : parse_nc_hdr;
        default  : start_clone;
    }

}


parser parse_nc_hdr_clone {
    extract(nc_hdr_clone);

    return ingress;

}


parser parse_nc_hdr {
    extract(nc_hdr);

    return start_clone;

}





blackbox stateful_alu acquire_lock_alu {
    reg : lock_status_register;

    update_lo_1_value : 1;

    output_value : register_lo;

    output_dst : meta.available;


}

blackbox stateful_alu release_lock_alu {
    reg : lock_status_register;

    update_lo_1_value : 0;


}

header_type meta_t {
    fields {
        lock_id : 32;
        routed : 1;
        available : 8;
    }
}


metadata meta_t meta;


register lock_status_register {
    width:32;
    instance_count:130000;
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








header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_i0 : 8;
        encoding_i1 : 8;
        encoding_i2 : 8;
        encoding_i3 : 8;
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
        max_bits_field0 : 32;
        max_bits_field1 : 32;
    }
}


metadata pfuzz_metadata_t pfuzz_metadata;


control egress {
  if (pfuzz_metadata.apply_mutations == 1) {
    apply(te_get_table_seed);
    apply(te_set_table_seed);
    apply(te_move_fields);
    apply(te_apply_mutations0);
    apply(te_apply_mutations1);
    apply(te_move_back_fields);
    apply(te_do_resubmit);
  }
    apply(ti_set_visited_type);
    apply(te_update_count);
}


header adm_hdr_t adm_hdr_clone;
header ethernet_t ethernet_clone;
header ipv4_t ipv4_clone;
header nc_hdr_t nc_hdr_clone;
header nlk_hdr_t nlk_hdr_clone;
header probe_hdr_t probe_hdr_clone;
header recirculate_hdr_t recirculate_hdr_clone;
header tcp_t tcp_clone;
header udp_t udp_clone;

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
  pfuzz_visited.encoding_i2;
}


field_list fi_bf_hash_fields_32 {
  pfuzz_visited.encoding_i3;
  pfuzz_visited.encoding_i0;
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
  remove_header(adm_hdr);
  remove_header(adm_hdr_clone);
  remove_header(ethernet);
  remove_header(ethernet_clone);
  remove_header(ipv4);
  remove_header(ipv4_clone);
  remove_header(nc_hdr);
  remove_header(nc_hdr_clone);
  remove_header(nlk_hdr);
  remove_header(nlk_hdr_clone);
  remove_header(probe_hdr);
  remove_header(probe_hdr_clone);
  remove_header(recirculate_hdr);
  remove_header(recirculate_hdr_clone);
  remove_header(tcp);
  remove_header(tcp_clone);
  remove_header(udp);
  remove_header(udp_clone);
  modify_field(pfuzz_visited.pkt_type, 0);
  modify_field(pfuzz_visited.encoding_i3, 0);
  modify_field(pfuzz_visited.encoding_i0, 0);
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
    ai_add_ethernet;
    ai_add_ethernet_ipv4;
    ai_add_ethernet_ipv4_tcp;
    ai_add_ethernet_ipv4_udp;
    ai_add_ethernet_ipv4_udp_nc_hdr;
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

action ai_add_ethernet_ipv4_udp() {
  add_header(ethernet);
  add_header(ethernet_clone);
  add_header(ipv4);
  add_header(ipv4_clone);
  add_header(udp);
  add_header(udp_clone);
  modify_field(ethernet.etherType, 0x0800);
  modify_field(ipv4.protocol, 17);
}

action ai_add_ethernet_ipv4_udp_nc_hdr() {
  add_header(ethernet);
  add_header(ethernet_clone);
  add_header(ipv4);
  add_header(ipv4_clone);
  add_header(udp);
  add_header(udp_clone);
  add_header(nc_hdr);
  add_header(nc_hdr_clone);
  modify_field(ethernet.etherType, 0x0800);
  modify_field(ipv4.protocol, 17);
  modify_field(udp.dstPort, 6666);
}

table ti_add_fields {
  reads {
    pfuzz_metadata.seed_num: exact;
  }
  actions {
    ai_NoAction;
    ai_add_fixed_ethernet;
    ai_add_fixed_ethernet_ipv4;
    ai_add_fixed_ethernet_ipv4_tcp;
    ai_add_fixed_ethernet_ipv4_udp;
    ai_add_fixed_ethernet_ipv4_udp_nc_hdr;
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

action ai_add_fixed_ethernet_ipv4_udp(ethernetdstAddr, ethernetsrcAddr, ethernetetherType, ipv4version, ipv4ihl, ipv4diffserv, ipv4totalLen, ipv4identification, ipv4flags, ipv4fragOffset, ipv4ttl, ipv4protocol, ipv4hdrChecksum, ipv4srcAddr, ipv4dstAddr, udpsrcPort, udpdstPort, udppkt_length, udpchecksum) {
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
  modify_field(udp.srcPort, udpsrcPort);
  modify_field(udp.dstPort, udpdstPort);
  modify_field(udp.pkt_length, udppkt_length);
  modify_field(udp.checksum, udpchecksum);
}

action ai_add_fixed_ethernet_ipv4_udp_nc_hdr(ethernetdstAddr, ethernetsrcAddr, ethernetetherType, ipv4version, ipv4ihl, ipv4diffserv, ipv4totalLen, ipv4identification, ipv4flags, ipv4fragOffset, ipv4ttl, ipv4protocol, ipv4hdrChecksum, ipv4srcAddr, ipv4dstAddr, udpsrcPort, udpdstPort, udppkt_length, udpchecksum, nc_hdrrecirc_flag, nc_hdrop, nc_hdrmode, nc_hdrclient_id, nc_hdrtid, nc_hdrlock, nc_hdrtimestamp_lo, nc_hdrtimestamp_hi) {
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
  modify_field(udp.srcPort, udpsrcPort);
  modify_field(udp.dstPort, udpdstPort);
  modify_field(udp.pkt_length, udppkt_length);
  modify_field(udp.checksum, udpchecksum);
  modify_field(nc_hdr.recirc_flag, nc_hdrrecirc_flag);
  modify_field(nc_hdr.op, nc_hdrop);
  modify_field(nc_hdr.mode, nc_hdrmode);
  modify_field(nc_hdr.client_id, nc_hdrclient_id);
  modify_field(nc_hdr.tid, nc_hdrtid);
  modify_field(nc_hdr.lock, nc_hdrlock);
  modify_field(nc_hdr.timestamp_lo, nc_hdrtimestamp_lo);
  modify_field(nc_hdr.timestamp_hi, nc_hdrtimestamp_hi);
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
    ae_move_IF_CONDITION_2_parser_4;
    ae_move_IF_CONDITION_3_parser_4;
    ae_move_ipv4_route_parser_4;
  }
  default_action : ai_NoAction();
}

action ae_move_IF_CONDITION_2_parser_4(){
  modify_field(pfuzz_metadata.max_bits_field0, nc_hdr.lock);
}
action ae_move_IF_CONDITION_3_parser_4(){
  modify_field(pfuzz_metadata.max_bits_field0, nc_hdr.op);
}
action ae_move_ipv4_route_parser_4(){
  modify_field(pfuzz_metadata.max_bits_field0, ipv4.dstAddr);
  modify_field(pfuzz_metadata.max_bits_field1, ipv4.srcAddr);
}
table te_apply_mutations0 {
  actions {
    ae_apply_mutations0;
  }
  default_action : ae_apply_mutations0();
}

action ae_apply_mutations0() {
  modify_field_rng_uniform(pfuzz_metadata.max_bits_field0, 0,0xffffffff);
}

table te_apply_mutations1 {
  actions {
    ae_apply_mutations1;
  }
  default_action : ae_apply_mutations1();
}

action ae_apply_mutations1() {
  modify_field_rng_uniform(pfuzz_metadata.max_bits_field1, 0,0xffffffff);
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
    ae_move_back_IF_CONDITION_2_parser_4;
    ae_move_back_fix_IF_CONDITION_2_parser_4;
    ae_move_back_IF_CONDITION_3_parser_4;
    ae_move_back_fix_IF_CONDITION_3_parser_4;
    ae_move_back_ipv4_route_parser_4;
    ae_move_back_fix_ipv4_route_parser_4;
  }
  default_action : ai_NoAction();
  size: 2048;
}

action ae_move_back_IF_CONDITION_2_parser_4(){
  modify_field(nc_hdr.lock, pfuzz_metadata.max_bits_field0);
  modify_field(nc_hdr_clone.lock, pfuzz_metadata.max_bits_field0);
}
action ae_move_back_IF_CONDITION_3_parser_4(){
  modify_field(nc_hdr.op, pfuzz_metadata.max_bits_field0);
  modify_field(nc_hdr_clone.op, pfuzz_metadata.max_bits_field0);
}
action ae_move_back_ipv4_route_parser_4(){
  modify_field(ipv4.dstAddr, pfuzz_metadata.max_bits_field0);
  modify_field(ipv4_clone.dstAddr, pfuzz_metadata.max_bits_field0);
  modify_field(ipv4.srcAddr, pfuzz_metadata.max_bits_field1);
  modify_field(ipv4_clone.srcAddr, pfuzz_metadata.max_bits_field1);
}
action ae_move_back_fix_IF_CONDITION_2_parser_4(nc_hdr_lock){
  modify_field(nc_hdr.lock, nc_hdr_lock);
  modify_field(nc_hdr_clone.lock, nc_hdr_lock);
}
action ae_move_back_fix_IF_CONDITION_3_parser_4(nc_hdr_op){
  modify_field(nc_hdr.op, nc_hdr_op);
  modify_field(nc_hdr_clone.op, nc_hdr_op);
}
action ae_move_back_fix_ipv4_route_parser_4(ipv4_dstAddr, ipv4_srcAddr){
  modify_field(ipv4.dstAddr, ipv4_dstAddr);
  modify_field(ipv4_clone.dstAddr, ipv4_dstAddr);
  modify_field(ipv4.srcAddr, ipv4_srcAddr);
  modify_field(ipv4_clone.srcAddr, ipv4_srcAddr);
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
    adm_hdr.valid: exact;
    ethernet.valid: exact;
    ipv4.valid: exact;
    nc_hdr.valid: exact;
    nlk_hdr.valid: exact;
    probe_hdr.valid: exact;
    recirculate_hdr.valid: exact;
    tcp.valid: exact;
    udp.valid: exact;
  }
  // I think the default action should be ai_NoAction
  actions {
    ai_NoAction;
    ai_add_clone_ethernet;
    ai_add_clone_ethernet_ipv4;
    ai_add_clone_ethernet_ipv4_tcp;
    ai_add_clone_ethernet_ipv4_udp;
    ai_add_clone_ethernet_ipv4_udp_nc_hdr;
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

action ai_add_clone_ethernet_ipv4_udp() {
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
  modify_field(udp_clone.srcPort, udp.srcPort);
  modify_field(udp_clone.dstPort, udp.dstPort);
  modify_field(udp_clone.pkt_length, udp.pkt_length);
  modify_field(udp_clone.checksum, udp.checksum);
}

action ai_add_clone_ethernet_ipv4_udp_nc_hdr() {
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
  modify_field(udp_clone.srcPort, udp.srcPort);
  modify_field(udp_clone.dstPort, udp.dstPort);
  modify_field(udp_clone.pkt_length, udp.pkt_length);
  modify_field(udp_clone.checksum, udp.checksum);
  modify_field(nc_hdr_clone.recirc_flag, nc_hdr.recirc_flag);
  modify_field(nc_hdr_clone.op, nc_hdr.op);
  modify_field(nc_hdr_clone.mode, nc_hdr.mode);
  modify_field(nc_hdr_clone.client_id, nc_hdr.client_id);
  modify_field(nc_hdr_clone.tid, nc_hdr.tid);
  modify_field(nc_hdr_clone.lock, nc_hdr.lock);
  modify_field(nc_hdr_clone.timestamp_lo, nc_hdr.timestamp_lo);
  modify_field(nc_hdr_clone.timestamp_hi, nc_hdr.timestamp_hi);
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

