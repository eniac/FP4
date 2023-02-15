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
    extract(fp4_visited);
    
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
    }
}


metadata ingress_metadata_t ingress_metadata;



action set_bd(bd) {
    modify_field(ingress_metadata.bd, bd);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 1);
}

table port_mapping {
    reads {
        ig_intr_md.ingress_port : exact;
    }
    actions {
        set_bd;
    }
    size:32768;
}


action set_vrf(vrf) {
    modify_field(ingress_metadata.vrf, vrf);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 2);
}

table bd {
    reads {
        ingress_metadata.bd : exact;
    }
    actions {
        set_vrf;
    }
    size:65536;
}



table ipv4_fib {
    reads {
        ingress_metadata.vrf : exact;
        ipv4.dstAddr : exact;
    }
    actions {
        on_miss_fp4_ipv4_fib;
        fib_hit_nexthop_fp4_ipv4_fib;
    }
    size:131072;
}


table ipv4_fib_lpm {
    reads {
        ingress_metadata.vrf : exact;
        ipv4.dstAddr : lpm;
    }
    actions {
        on_miss_fp4_ipv4_fib_lpm;
        fib_hit_nexthop_fp4_ipv4_fib_lpm;
    }
    size:16384;
}


action set_egress_details(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 32);
}

action ai_drop() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 16);
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

table nexthop {
    reads {
        ingress_metadata.nexthop_index : exact;
    }
    actions {
        ai_drop;
        set_egress_details;
    }
    size:32768;
}


control ingress     {
    if (valid(ipv4))  {
        apply(port_mapping);

        apply(bd);

        apply(ipv4_fib) {
 on_miss_fp4_ipv4_fib {
    apply(ipv4_fib_lpm);


}
         }

        apply(nexthop);


    }


}

action rewrite_src_dst_mac(smac, dmac) {
    modify_field(ethernet.srcAddr, smac);
    modify_field(ethernet.dstAddr, dmac);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 128);
}

table rewrite_mac {
    reads {
        ingress_metadata.nexthop_index : exact;
    }
    actions {
        on_miss_fp4_rewrite_mac;
        rewrite_src_dst_mac;
    }
    size:32768;
}


action on_miss_fp4_ipv4_fib() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 4);
}

action fib_hit_nexthop_fp4_ipv4_fib(nexthop_index) {
    modify_field(ingress_metadata.nexthop_index, nexthop_index);
    subtract_from_field(ipv4.ttl, 1);
    add(fp4_visited.encoding0, fp4_visited.encoding0, 8);
}

action on_miss_fp4_ipv4_fib_lpm() {
    add(fp4_visited.encoding1, fp4_visited.encoding1, 1);
}

action fib_hit_nexthop_fp4_ipv4_fib_lpm(nexthop_index) {
    modify_field(ingress_metadata.nexthop_index, nexthop_index);
    subtract_from_field(ipv4.ttl, 1);
    add(fp4_visited.encoding1, fp4_visited.encoding1, 2);
}

action on_miss_fp4_rewrite_mac() {
    add(fp4_visited.encoding0, fp4_visited.encoding0, 64);
}

header_type fp4_visited_t {
    fields {
        preamble : 48;
        encoding0 : 32;
        encoding1 : 32;
        pkt_type : 2;
        __pad : 6;
    }
}


header fp4_visited_t fp4_visited;


control egress     {
    apply(rewrite_mac);


}


