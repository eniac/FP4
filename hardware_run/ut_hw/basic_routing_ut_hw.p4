#include <tofino/intrinsic_metadata.p4>
#include <tofino/stateful_alu_blackbox.p4>
header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}


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


parser start {
    extract(pfuzz_visited);
    
    return parse_ethernet;

}


header ethernet_t ethernet;


parser parse_ethernet {
    extract(ethernet);

    return select(latest.etherType) { 
        0x0800 : parse_ipv4;
        default  : ingress;
    }

}


header ipv4_t ipv4;


field_list ipv4_checksum_list {
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

field_list_calculation ipv4_checksum {
    input    {
        ipv4_checksum_list;
    }
    algorithm:csum16;
    output_width:16;
}

calculated_field ipv4.hdrChecksum{
    verify    ipv4_checksum;
    update    ipv4_checksum;
}

parser parse_ipv4 {
    extract(ipv4);

    return ingress;

}


header_type ingress_metadata_t {
    fields {
        vrf : 12;
        bd : 16;
        nexthop_index : 16;
        on_miss_ipv4_fib : 1;
    }
}


metadata ingress_metadata_t ingress_metadata;




table port_mapping {
    reads {
        ig_intr_md.ingress_port : exact;
    }
    actions {
        set_bd_pfuzz_port_mapping;
        on_miss_pfuzz_port_mapping;
    }
    default_action:on_miss_pfuzz_port_mapping();
    size:32768;
}



table bd {
    reads {
        ingress_metadata.bd : exact;
    }
    actions {
        set_vrf_pfuzz_bd;
        on_miss_pfuzz_bd;
    }
    default_action:on_miss_pfuzz_bd();
    size:65536;
}




table ipv4_fib {
    reads {
        ingress_metadata.vrf : exact;
        ipv4.dstAddr : exact;
    }
    actions {
        on_miss_ipv4_fib_pfuzz_ipv4_fib;
        fib_hit_nexthop_pfuzz_ipv4_fib;
    }
    default_action:on_miss_ipv4_fib_pfuzz_ipv4_fib();
    size:131072;
}


table ipv4_fib_lpm {
    reads {
        ingress_metadata.vrf : exact;
        ipv4.dstAddr : lpm;
    }
    actions {
        on_miss_pfuzz_ipv4_fib_lpm;
        fib_hit_nexthop_pfuzz_ipv4_fib_lpm;
    }
    default_action:on_miss_pfuzz_ipv4_fib_lpm();
    size:16384;
}




table ti_drop {
    actions {
        ai_drop_pfuzz_ti_drop;
    }
    default_action:ai_drop_pfuzz_ti_drop();
}


table nexthop {
    reads {
        ingress_metadata.nexthop_index : exact;
    }
    actions {
        ai_drop_pfuzz_nexthop;
        set_egress_details_pfuzz_nexthop;
    }
    default_action:ai_drop_pfuzz_nexthop();
    size:32768;
}


control ingress     {
    if (valid(ipv4))  {
        apply(port_mapping);

        apply(bd);

        apply(ipv4_fib);

        if (ingress_metadata.on_miss_ipv4_fib == 1)  {
            apply(ipv4_fib_lpm);


        }

        apply(nexthop);


    }


}


table rewrite_mac {
    reads {
        ingress_metadata.nexthop_index : exact;
    }
    actions {
        on_miss_pfuzz_rewrite_mac;
        rewrite_src_dst_mac_pfuzz_rewrite_mac;
    }
    default_action:on_miss_pfuzz_rewrite_mac();
    size:32768;
}


action set_bd_pfuzz_port_mapping(bd) {
    modify_field(ingress_metadata.bd, bd);
    add_to_field(pfuzz_visited.encoding_i1, 9);
}

action on_miss_pfuzz_port_mapping() {
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action set_vrf_pfuzz_bd(vrf) {
    modify_field(ingress_metadata.vrf, vrf);
    add_to_field(pfuzz_visited.encoding_i1, 4);
}

action on_miss_pfuzz_bd() {
}

action on_miss_ipv4_fib_pfuzz_ipv4_fib() {
    modify_field(ingress_metadata.on_miss_ipv4_fib, 1);
    add_to_field(pfuzz_visited.encoding_i1, 2);
}

action fib_hit_nexthop_pfuzz_ipv4_fib(nexthop_index) {
    modify_field(ingress_metadata.nexthop_index, nexthop_index);
    subtract_from_field(ipv4.ttl, 1);
}

action on_miss_pfuzz_ipv4_fib_lpm() {
    add_to_field(pfuzz_visited.encoding_i0, 2);
}

action fib_hit_nexthop_pfuzz_ipv4_fib_lpm(nexthop_index) {
    modify_field(ingress_metadata.nexthop_index, nexthop_index);
    subtract_from_field(ipv4.ttl, 1);
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

action ai_drop_pfuzz_ti_drop() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action ai_drop_pfuzz_nexthop() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action set_egress_details_pfuzz_nexthop(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
    add_to_field(pfuzz_visited.encoding_i1, 1);
}

action on_miss_pfuzz_rewrite_mac() {
}

action rewrite_src_dst_mac_pfuzz_rewrite_mac(smac, dmac) {
    modify_field(ethernet.srcAddr, smac);
    modify_field(ethernet.dstAddr, dmac);
    add_to_field(pfuzz_visited.encoding_e0, 1);
}

header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_e0 : 8;
        encoding_i0 : 8;
        encoding_i1 : 8;
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;


control egress     {
    apply(rewrite_mac);


}


