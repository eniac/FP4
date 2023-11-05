#include <core.p4>
#include <tna.p4>

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

header ipv4_t {
    bit<4> version;
    bit<4> ihl;
    bit<8> diffserv;
    bit<16> total_len;
    bit<16> identification;
    bit<3> flags;
    bit<13> frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdr_checksum;
    bit<32> src_addr;
    bit<32> dst_addr;
}

struct ingress_metadata_t {
    bit<12> vrf;                   /* VRF */
    bit<16> bd;                     /* ingress BD */
    bit<16> nexthop_index;                    /* final next hop index */
    bit<1> on_miss_ipv4_fib;
}

struct headers_t {
    ethernet_t ethernet;
    ipv4_t ipv4;
}

// Constants
const bit<16> PORT_VLAN_TABLE_SIZE = 32768;
const bit<32> BD_TABLE_SIZE = 65536;
const bit<32> IPV4_HOST_TABLE_SIZE = 131072;
const bit<16> IPV4_LPM_TABLE_SIZE = 16384;
const bit<16> NEXTHOP_TABLE_SIZE = 32768;
const bit<16> REWRITE_MAC_TABLE_SIZE = 32768;

parser SwitchIngressParser(packet_in packet,
    out headers_t hdr,
    out ingress_metadata_t ig_md,
    out ingress_intrinsic_metadata_t ig_intr_md) {
    
    state start {
        transition parse_ethernet;
    }
    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x0800: parse_ipv4;
            default: accept;
        }
    }
    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition accept;
    }
}

control SwitchIngressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in ingress_metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    Checksum() ipv4_checksum;
    
    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.diffserv,
            hdr.ipv4.total_len,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.frag_offset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr
        });
    
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

control SwitchIngress(inout headers_t hdr,
    inout ingress_metadata_t ingress_metadata, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    action set_bd(bit<16> bd) {
        ingress_metadata.bd = bd;
    }

    action set_vrf(bit<12> vrf) {
        ingress_metadata.vrf = vrf;
    }

    action port_mapping_on_miss() {}
    action bd_on_miss() {}
    action ipv4_fib_lpm_on_miss() {}

    action ipv4_fib_on_miss() {
        ingress_metadata.on_miss_ipv4_fib = 1; 
    }

    action ipv4_fib_hit_nexthop(bit<16> nexthop_index) {
        ingress_metadata.nexthop_index = nexthop_index;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    action ipv4_fib_lpm_hit_nexthop(bit<16> nexthop_index) {
        ingress_metadata.nexthop_index = nexthop_index;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }

    action ai_drop() {
        ig_intr_dprsr_md.drop_ctl = 0x1; // Drop packet.
    }

    action set_egress_details(bit<9> egress_spec) {
        ig_intr_tm_md.ucast_egress_port = egress_spec;
    }

    table port_mapping {
        key = {
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            set_bd;
            port_mapping_on_miss;
        }
        default_action = port_mapping_on_miss();
        size = PORT_VLAN_TABLE_SIZE;
    }

    table bd {
        key = {
            ingress_metadata.bd : exact;
        }
        actions = {
            set_vrf;
            bd_on_miss;
        }
        default_action = bd_on_miss();
        size = BD_TABLE_SIZE;
    }

    table ipv4_fib {
        key = {
            ingress_metadata.vrf : exact;
            hdr.ipv4.dst_addr : exact;
        }
        actions = {
            ipv4_fib_on_miss;
            ipv4_fib_hit_nexthop;
        }
        default_action = ipv4_fib_on_miss();
        size = IPV4_HOST_TABLE_SIZE;
    }

    table ipv4_fib_lpm {
        key = {
            ingress_metadata.vrf : exact;
            hdr.ipv4.dst_addr : lpm;
        }
        actions = {
            ipv4_fib_lpm_on_miss;
            ipv4_fib_lpm_hit_nexthop;
        }
        default_action = ipv4_fib_lpm_on_miss();
        size = IPV4_LPM_TABLE_SIZE;
    }

    table nexthop {
        key = {
            ingress_metadata.nexthop_index : exact;
        }
        actions = {
            ai_drop;
            set_egress_details;
        }
        default_action = ai_drop();
        size = NEXTHOP_TABLE_SIZE;
    }

    apply {
        if (hdr.ipv4.isValid()) {
            port_mapping.apply();
            bd.apply();
            ipv4_fib.apply();
            if (ingress_metadata.on_miss_ipv4_fib == 1) {
                ipv4_fib_lpm.apply();
            }
            nexthop.apply();
        }
    }
} 

parser SwitchEgressParser(packet_in packet, 
    out headers_t hdr,
    out ingress_metadata_t eg_md,
    out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        transition parse_ethernet;
    }
    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x0800: parse_ipv4;
            default: accept;
        }
    }
    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition accept;
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in ingress_metadata_t ingress_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() ipv4_checksum;
    
    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.diffserv,
            hdr.ipv4.total_len,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.frag_offset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr
        });
    
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout ingress_metadata_t ingress_metadata,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    action rewrite_mac_on_miss() {}
    action rewrite_src_dst_mac(bit<48> smac, bit<48> dmac) {
        hdr.ethernet.srcAddr = smac;
        hdr.ethernet.dstAddr = dmac;
    }

    table rewrite_mac {
        key = {
            ingress_metadata.nexthop_index : exact;
        }
        actions = {
            rewrite_mac_on_miss;
            rewrite_src_dst_mac;
        }
        default_action = rewrite_mac_on_miss();
        size = REWRITE_MAC_TABLE_SIZE;
    }
    apply {
        rewrite_mac.apply();
    }
}

Pipeline(SwitchIngressParser(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;
