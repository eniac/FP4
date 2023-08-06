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

    return select(latest.protocol) { 
        6 : parse_tcp;
        17 : parse_udp;
        default  : ingress;
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

parser parse_tcp {
    extract(tcp);

    return ingress;

}


parser parse_udp {
    extract(udp);

    return select(latest.dstPort) { 
        6666 : parse_nc_hdr;
        default  : ingress;
    }

}


parser parse_nc_hdr {
    extract(nc_hdr);

    return ingress;

}



table ipv4_route {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        set_egress_pfuzz_ipv4_route;
        drop_action_pfuzz_ipv4_route;
    }
    default_action:drop_action_pfuzz_ipv4_route;
    size:256;
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



table decode_table {
    actions {
        decode_action_pfuzz_decode_table;
    }
    default_action:decode_action_pfuzz_decode_table;
}



table release_lock_table {
    actions {
        release_lock_action_pfuzz_release_lock_table;
    }
    default_action:release_lock_action_pfuzz_release_lock_table;
}



table acquire_lock_table {
    actions {
        acquire_lock_action_pfuzz_acquire_lock_table;
    }
    default_action:acquire_lock_action_pfuzz_acquire_lock_table;
}



table set_retry_table {
    actions {
        set_retry_action_pfuzz_set_retry_table;
    }
    default_action:set_retry_action_pfuzz_set_retry_table;
}



table reply_to_client_table {
    actions {
        reply_to_client_action_pfuzz_reply_to_client_table;
    }
    default_action:reply_to_client_action_pfuzz_reply_to_client_table;
}


control ingress     {
    if (valid(nc_hdr))  {
        apply(decode_table);

        if (nc_hdr.op == 0)  {
            apply(acquire_lock_table);

            if (meta.available != 0)  {
                apply(set_retry_table);


            }


        }
        else  {
            apply(ti_mvbl_1_hdrnc_hdrisValid_hdrnc_hdrop1);
            if (nc_hdr.op == 1)  {
                apply(release_lock_table);


            }


        }

        apply(reply_to_client_table);


    }

    apply(ipv4_route);


}

action set_egress_pfuzz_ipv4_route(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
    add_to_field(ipv4.ttl, -1);
    add_to_field(pfuzz_visited.encoding_i2, 1);
}

action drop_action_pfuzz_ipv4_route() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action decode_action_pfuzz_decode_table() {
    modify_field(meta.lock_id, nc_hdr.lock);
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action release_lock_action_pfuzz_release_lock_table() {
    release_lock_alu . execute_stateful_alu ( meta.lock_id );
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action acquire_lock_action_pfuzz_acquire_lock_table() {
    acquire_lock_alu . execute_stateful_alu ( meta.lock_id );
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action set_retry_action_pfuzz_set_retry_table() {
    modify_field(nc_hdr.op, 5);
    add_to_field(pfuzz_visited.encoding_i3, 1);
}

action reply_to_client_action_pfuzz_reply_to_client_table() {
    modify_field(ipv4.dstAddr, ipv4.srcAddr);
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
    }
}


header pfuzz_visited_t pfuzz_visited;


control egress     {

}


action ai_mvbl_1_hdrnc_hdrisValid_hdrnc_hdrop1() {
  add_to_field(pfuzz_visited.encoding_i1, 1);
}

table ti_mvbl_1_hdrnc_hdrisValid_hdrnc_hdrop1{
  actions {
    ai_mvbl_1_hdrnc_hdrisValid_hdrnc_hdrop1;
  }
  default_action: ai_mvbl_1_hdrnc_hdrisValid_hdrnc_hdrop1();
}

action ai_mvbl_1_hdrnc_hdrisValid_metametaavailable0() {
  add_to_field(pfuzz_visited.encoding_i1, 2);
}

table ti_mvbl_1_hdrnc_hdrisValid_metametaavailable0{
  actions {
    ai_mvbl_1_hdrnc_hdrisValid_metametaavailable0;
  }
  default_action: ai_mvbl_1_hdrnc_hdrisValid_metametaavailable0();
}

