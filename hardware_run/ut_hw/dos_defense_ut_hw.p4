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
        reg_val_one : 32;
        reg_val_two : 32;
        reg_pos_one : 16;
        reg_pos_two : 16;
        reg_val_one_warning : 8;
        reg_val_two_warning : 8;
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



table tiSendDigest_1 {
    actions {
        aiSendDigest_pfuzz_tiSendDigest_1;
    }
    default_action:aiSendDigest_pfuzz_tiSendDigest_1();
}


table tiSendDigest_2 {
    actions {
        aiSendDigest_pfuzz_tiSendDigest_2;
    }
    default_action:aiSendDigest_pfuzz_tiSendDigest_2();
}


control ingress     {
    if (valid(ipv4))  {
        apply(ipv4_lpm);

        apply(ti_get_incoming_pos);

        apply(ti_calculate_hash);

        apply(ti_write_flow_counter_1);

        apply(ti_write_flow_counter_warning_1);

        apply(ti_write_flow_counter_2);

        apply(ti_write_flow_counter_warning_2);

        if (meta.reg_val_one_warning == 1)  {
            apply(tiSendDigest_1);


        }

        if (meta.reg_val_two_warning == 1)  {
            apply(tiSendDigest_2);


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



table ti_calculate_hash {
    actions {
        ai_calculate_hash_pfuzz_ti_calculate_hash;
    }
    default_action:ai_calculate_hash_pfuzz_ti_calculate_hash();
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
        ai_handle_blacklist_pfuzz_ipv4_lpm;
    }
    default_action:drop_packet_pfuzz_ipv4_lpm();
    size:1024;
}



register flow_counter_1 {
    width:32;
    instance_count:1024;
}


register flow_counter_2 {
    width:32;
    instance_count:1024;
}


register flow_counter_warning_1 {
    width:32;
    instance_count:1024;
}


register flow_counter_warning_2 {
    width:32;
    instance_count:1024;
}


table ti_write_flow_counter_1 {
    actions {
        ai_write_flow_counter_1_pfuzz_ti_write_flow_counter_1;
    }
    default_action:ai_write_flow_counter_1_pfuzz_ti_write_flow_counter_1();
    size:1;
}


table ti_write_flow_counter_2 {
    actions {
        ai_write_flow_counter_2_pfuzz_ti_write_flow_counter_2;
    }
    default_action:ai_write_flow_counter_2_pfuzz_ti_write_flow_counter_2();
    size:1;
}


blackbox stateful_alu riw_flow_counter_1 {
    reg : flow_counter_1;

    update_lo_1_value : register_lo + 1;

    output_dst : meta.reg_val_one;


}

blackbox stateful_alu riw_flow_counter_2 {
    reg : flow_counter_2;

    update_lo_1_value : register_lo + 1;

    output_dst : meta.reg_val_two;


}



table ti_write_flow_counter_warning_1 {
    actions {
        ai_write_flow_counter_warning_1_pfuzz_ti_write_flow_counter_warning_1;
    }
    default_action:ai_write_flow_counter_warning_1_pfuzz_ti_write_flow_counter_warning_1();
    size:1;
}


table ti_write_flow_counter_warning_2 {
    actions {
        ai_write_flow_counter_warning_2_pfuzz_ti_write_flow_counter_warning_2;
    }
    default_action:ai_write_flow_counter_warning_2_pfuzz_ti_write_flow_counter_warning_2();
    size:1;
}


blackbox stateful_alu riw_flow_counter_warning_1 {
    reg : flow_counter_warning_1;

    condition_lo : meta.reg_val_one > 64;

    update_lo_1_predicate : condition_lo;

    update_lo_1_value : 1;

    output_dst : meta.reg_val_one_warning;


}

blackbox stateful_alu riw_flow_counter_warning_2 {
    reg : flow_counter_warning_2;

    condition_lo : meta.reg_val_two > 64;

    update_lo_1_predicate : condition_lo;

    update_lo_1_value : 1;

    output_dst : meta.reg_val_two_warning;


}


action aiSendDigest_pfuzz_tiSendDigest_1() {
    clone_i2e(98);
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action aiSendDigest_pfuzz_tiSendDigest_2() {
    clone_i2e(98);
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action ai_get_incoming_pos_pfuzz_ti_get_incoming_pos() {
    modify_field(meta.first, ipv4.srcAddr);
    modify_field(meta.second, ipv4.dstAddr);
    modify_field(meta.third, tcp.srcPort);
    modify_field(meta.fourth, tcp.dstPort);
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action ai_calculate_hash_pfuzz_ti_calculate_hash() {
    modify_field_with_hash_based_offset(meta.reg_pos_one, 0, hasher_64, 1024);
    modify_field_with_hash_based_offset(meta.reg_pos_two, 0, hasher_32, 1024);
}

action ipv4_forward_pfuzz_ipv4_lpm(dstAddr, port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, port);
    modify_field(ethernet.srcAddr, dstAddr);
    modify_field(ethernet.dstAddr, dstAddr);
    subtract_from_field(ipv4.ttl, 1);
    add_to_field(pfuzz_visited.encoding_i2, 3);
}

action drop_packet_pfuzz_ipv4_lpm() {
    add_to_field(pfuzz_visited.encoding_i2, 2);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action ai_handle_blacklist_pfuzz_ipv4_lpm() {
    add_to_field(pfuzz_visited.encoding_i2, 1);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action ai_write_flow_counter_1_pfuzz_ti_write_flow_counter_1() {
    riw_flow_counter_1 . execute_stateful_alu ( meta.reg_pos_one );
}

action ai_write_flow_counter_2_pfuzz_ti_write_flow_counter_2() {
    riw_flow_counter_2 . execute_stateful_alu ( meta.reg_pos_two );
}

action ai_write_flow_counter_warning_1_pfuzz_ti_write_flow_counter_warning_1() {
    riw_flow_counter_warning_1 . execute_stateful_alu ( meta.reg_pos_one );
}

action ai_write_flow_counter_warning_2_pfuzz_ti_write_flow_counter_warning_2() {
    riw_flow_counter_warning_2 . execute_stateful_alu ( meta.reg_pos_two );
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
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;




