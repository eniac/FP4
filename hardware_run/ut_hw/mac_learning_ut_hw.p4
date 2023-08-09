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


parser start {
    extract(pfuzz_visited);
    
    return parse_ethernet;

}


parser parse_ethernet {
    extract(ethernet);

    return select(ethernet.etherType) { 
        0x0800 : parse_ipv4;
        default  : ingress;
    }

}


parser parse_ipv4 {
    extract(ipv4);

    return ingress;

}




table tiLearnMAC {
    reads {
        ethernet.srcAddr : exact;
    }
    actions {
        aiNoOp_pfuzz_tiLearnMAC;
        aiSendDigest_pfuzz_tiLearnMAC;
    }
    default_action:aiSendDigest_pfuzz_tiLearnMAC();
}




table tiForward {
    reads {
        ethernet.dstAddr : exact;
    }
    actions {
        aiForward_pfuzz_tiForward;
        aiForwardUnknown_pfuzz_tiForward;
    }
    default_action:aiForwardUnknown_pfuzz_tiForward();
}



table tiFilter {
    reads {
        ig_intr_md.ingress_port : exact;
        ig_intr_md_for_tm.ucast_egress_port : exact;
    }
    actions {
        aiFilter_pfuzz_tiFilter;
        aiNoOp_pfuzz_tiFilter;
    }
    default_action:aiNoOp_pfuzz_tiFilter();
}


control ingress     {
    apply(tiLearnMAC);

    apply(tiForward);

    apply(tiFilter);


}

action aiNoOp_pfuzz_tiLearnMAC() {
}

action aiSendDigest_pfuzz_tiLearnMAC() {
    clone_i2e(98);
    add_to_field(pfuzz_visited.encoding_i0, 4);
}

action aiForward_pfuzz_tiForward(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
    add_to_field(pfuzz_visited.encoding_i0, 2);
}

action aiForwardUnknown_pfuzz_tiForward() {
    clone_i2e(100);
}

action aiFilter_pfuzz_tiFilter() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);
}

action aiNoOp_pfuzz_tiFilter() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, pfuzz_visited.temp_port);    
    add_to_field(pfuzz_visited.encoding_i0, 1);
}

header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_i0 : 8;
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;


control egress     {

}


