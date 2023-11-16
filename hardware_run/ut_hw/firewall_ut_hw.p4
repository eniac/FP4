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
        ctrl : 3;
        rst : 1;
        syn : 1;
        fin : 1;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}


header tcp_t tcp;


header_type metadata_t {
    fields {
        direction : 1;
        check_ports_hit : 1;
        reg_val_one : 1;
        reg_val_two : 1;
        reg_pos_one : 16;
        reg_pos_two : 16;
        first : 32;
        second : 32;
        third : 16;
        fourth : 16;
    }
}


metadata metadata_t meta;


parser start {
    extract(pfuzz_visited);
    
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

    return select(ipv4.protocol) { 
        6 : parse_tcp;
        default  : ingress;
    }

}


parser parse_tcp {
    extract(tcp);

    return ingress;

}


control ingress     {
    if (valid(ipv4))  {
        apply(ipv4_lpm);

        apply(check_ports);

        if (meta.check_ports_hit == 1)  {
            apply(ti_mvbl_0_VIRTUAL_START_metametadirection0);

            if (meta.direction == 0)  {
                apply(ti_get_incoming_pos);

                apply(ti_calculate_hash1);

                if (tcp.syn == 1)  {
                    apply(ti_write_bloom_filter1);

                    apply(ti_write_bloom_filter2);


                }


            }
            else  {
                apply(ti_get_outgoing_pos);

                apply(ti_calculate_hash2);

                apply(ti_read_bloom_filter1);

                apply(ti_read_bloom_filter2);

                apply(ti_apply_filter);


            }


        }


    }


}

control egress     {

}

table ti_get_incoming_pos {
    actions {
        ai_get_incoming_pos_pfuzz_ti_get_incoming_pos;
    }
    default_action:ai_get_incoming_pos_pfuzz_ti_get_incoming_pos();
    size:1;
}


table ti_get_outgoing_pos {
    actions {
        ai_get_outgoing_pos_pfuzz_ti_get_outgoing_pos;
    }
    default_action:ai_get_outgoing_pos_pfuzz_ti_get_outgoing_pos();
    size:1;
}




table ti_calculate_hash1 {
    actions {
        ai_calculate_hash_pfuzz_ti_calculate_hash1;
    }
    default_action:ai_calculate_hash_pfuzz_ti_calculate_hash1();
    size:1;
}


table ti_calculate_hash2 {
    actions {
        ai_calculate_hash_pfuzz_ti_calculate_hash2;
    }
    default_action:ai_calculate_hash_pfuzz_ti_calculate_hash2();
    size:1;
}



field_list hash_fields {
    meta.first;
    meta.second;
    meta.third;
    meta.fourth;
    ipv4.protocol;
}

field_list_calculation hasher_64 {
    input    {
        hash_fields;
    }
    algorithm:crc_64;
    output_width:32;
}

field_list_calculation hasher_32 {
    input    {
        hash_fields;
    }
    algorithm:crc32;
    output_width:32;
}



table ipv4_lpm {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        ipv4_forward_pfuzz_ipv4_lpm;
        drop_packet_pfuzz_ipv4_lpm;
        ai_noOp_pfuzz_ipv4_lpm;
    }
    default_action:drop_packet_pfuzz_ipv4_lpm();
    size:1024;
}



table check_ports {
    reads {
        ig_intr_md.ingress_port : exact;
        ig_intr_md_for_tm.ucast_egress_port : exact;
    }
    actions {
        set_direction_pfuzz_check_ports;
        set_hit_pfuzz_check_ports;
    }
    default_action:set_hit_pfuzz_check_ports();
    size:1024;
}




table ti_read_bloom_filter1 {
    actions {
        ai_read_bloom_filter1_pfuzz_ti_read_bloom_filter1;
    }
    default_action:ai_read_bloom_filter1_pfuzz_ti_read_bloom_filter1();
    size:1;
}


table ti_read_bloom_filter2 {
    actions {
        ai_read_bloom_filter2_pfuzz_ti_read_bloom_filter2;
    }
    default_action:ai_read_bloom_filter2_pfuzz_ti_read_bloom_filter2();
    size:1;
}


register bloom_filter_1 {
    width:1;
    instance_count:4096;
}


register bloom_filter_2 {
    width:1;
    instance_count:4096;
}




blackbox stateful_alu rir_boom_filter1 {
    reg : bloom_filter_1;

    output_value : register_lo;

    output_dst : meta.reg_val_one;


}

blackbox stateful_alu rir_boom_filter2 {
    reg : bloom_filter_2;

    output_value : register_lo;

    output_dst : meta.reg_val_two;


}

table ti_apply_filter {
    reads {
        meta.reg_val_one : exact;
        meta.reg_val_two : exact;
    }
    actions {
        drop_packet_pfuzz_ti_apply_filter;
        ai_noOp_pfuzz_ti_apply_filter;
    }
    default_action:drop_packet_pfuzz_ti_apply_filter;
    size:4;
}


table ti_write_bloom_filter1 {
    actions {
        ai_write_bloom_filter1_pfuzz_ti_write_bloom_filter1;
    }
    default_action:ai_write_bloom_filter1_pfuzz_ti_write_bloom_filter1();
    size:1;
}


table ti_write_bloom_filter2 {
    actions {
        ai_write_bloom_filter2_pfuzz_ti_write_bloom_filter2;
    }
    default_action:ai_write_bloom_filter2_pfuzz_ti_write_bloom_filter2();
    size:1;
}


blackbox stateful_alu riw_boom_filter1 {
    reg : bloom_filter_1;

    update_lo_1_value : set_bit;


}

blackbox stateful_alu riw_boom_filter2 {
    reg : bloom_filter_2;

    update_lo_1_value : set_bit;


}


action ai_get_incoming_pos_pfuzz_ti_get_incoming_pos() {
    modify_field(meta.first, ipv4.srcAddr);
    modify_field(meta.second, ipv4.dstAddr);
    modify_field(meta.third, tcp.srcPort);
    modify_field(meta.fourth, tcp.dstPort);
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action ai_get_outgoing_pos_pfuzz_ti_get_outgoing_pos() {
    modify_field(meta.first, ipv4.dstAddr);
    modify_field(meta.second, ipv4.srcAddr);
    modify_field(meta.third, tcp.dstPort);
    modify_field(meta.fourth, tcp.srcPort);
    add_to_field(pfuzz_visited.encoding_i2, 1);
}

action ai_calculate_hash_pfuzz_ti_calculate_hash1() {
    modify_field_with_hash_based_offset(meta.reg_pos_one, 0, hasher_64, 1);
    modify_field_with_hash_based_offset(meta.reg_pos_two, 0, hasher_32, 1);
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action ai_calculate_hash_pfuzz_ti_calculate_hash2() {
    modify_field_with_hash_based_offset(meta.reg_pos_one, 0, hasher_64, 1);
    modify_field_with_hash_based_offset(meta.reg_pos_two, 0, hasher_32, 1);
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action ipv4_forward_pfuzz_ipv4_lpm(dstAddr, port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    modify_field(ethernet.srcAddr, dstAddr);
    modify_field(ethernet.dstAddr, dstAddr);
    subtract_from_field(ipv4.ttl, 1);
    add_to_field(pfuzz_visited.encoding_i3, 7);
}

action drop_packet_pfuzz_ipv4_lpm() {
    add_to_field(pfuzz_visited.encoding_i3, 4);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action ai_noOp_pfuzz_ipv4_lpm() {
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action set_direction_pfuzz_check_ports(dir) {
    modify_field(meta.direction, dir);
    modify_field(meta.check_ports_hit, 1);
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action set_hit_pfuzz_check_ports() {
    modify_field(meta.check_ports_hit, 1);
    modify_field(meta.direction, 1);
    add_to_field(pfuzz_visited.encoding_i1, 5);
}

action ai_read_bloom_filter1_pfuzz_ti_read_bloom_filter1() {
    rir_boom_filter1 . execute_stateful_alu ( meta.reg_pos_one );
}

action ai_read_bloom_filter2_pfuzz_ti_read_bloom_filter2() {
    rir_boom_filter2 . execute_stateful_alu ( meta.reg_pos_two );
    add_to_field(pfuzz_visited.encoding_i1, 2);
}

action drop_packet_pfuzz_ti_apply_filter() {
    add_to_field(pfuzz_visited.encoding_i1, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action ai_noOp_pfuzz_ti_apply_filter() {
}

action ai_write_bloom_filter1_pfuzz_ti_write_bloom_filter1() {
    riw_boom_filter1 . execute_stateful_alu ( meta.reg_pos_one );
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action ai_write_bloom_filter2_pfuzz_ti_write_bloom_filter2() {
    riw_boom_filter2 . execute_stateful_alu ( meta.reg_pos_two );
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




action ai_mvbl_0_VIRTUAL_START_metametadirection0() {
  add_to_field(pfuzz_visited.encoding_i0, 1);
}

table ti_mvbl_0_VIRTUAL_START_metametadirection0{
  actions {
    ai_mvbl_0_VIRTUAL_START_metametadirection0;
  }
  default_action: ai_mvbl_0_VIRTUAL_START_metametadirection0();
}

