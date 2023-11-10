#include <core.p4>
#include <tna.p4>

const bit<10> CPU_INGRESS_MIRROR_ID = 98;
const bit<10> MC0_INGRESS_MIRROR_ID = 100;

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

header ipv4_t {
    bit<4>  version;
    bit<4>  ihl;
    bit<8>  diffserv;
    bit<16> totalLen;
    bit<16> identification;
    bit<3>  flags;
    bit<13> fragOffset;
    bit<8>  ttl;
    bit<8>  protocol;
    bit<16> hdrChecksum;
    bit<32> srcAddr;
    bit<32> dstAddr;
}

struct headers_t {
    ethernet_t           ethernet;
    ipv4_t               ipv4;
}

struct mirror_metadata_t {
    bit<10> mirror_id;   
}

parser SwitchIngressParser(packet_in pkt,
    out headers_t hdr,
    out mirror_metadata_t ig_md,
    out ingress_intrinsic_metadata_t ig_intr_md) {
    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            default: accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }
    state start {
        transition parse_ethernet;
    }
}

control SwitchIngressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in mirror_metadata_t mirror_metadata,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    Mirror() mirror;
    apply {
        if (ig_intr_dprsr_md.mirror_type == 3w1) {
            mirror.emit(mirror_metadata.mirror_id);
        }
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

control SwitchIngress(inout headers_t hdr,
    inout mirror_metadata_t mirror_metadata,
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    
    action aiFilter() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action tiLearnMAC_aiNoOp() {
    }
    action tiFilter_aiNoOp() {
    }
    action aiForward(bit<9> egress_port) {
        ig_intr_md_for_tm.ucast_egress_port = egress_port;
    }
    action aiForwardUnknown() {
        ig_intr_md_for_dprsr.mirror_type = (bit<3>)3w1;
        mirror_metadata.mirror_id = MC0_INGRESS_MIRROR_ID;
    }
    action aiSendDigest() {
        ig_intr_md_for_dprsr.mirror_type = (bit<3>)3w1;
        mirror_metadata.mirror_id = CPU_INGRESS_MIRROR_ID;
    }
    table tiFilter {
        actions = {
            aiFilter();
            tiFilter_aiNoOp();
        }
        key = {
            ig_intr_md.ingress_port            : exact;
            ig_intr_md_for_tm.ucast_egress_port: exact;
        }
        const default_action = tiFilter_aiNoOp();
    }
    table tiForward {
        actions = {
            aiForward();
            aiForwardUnknown();
        }
        key = {
            hdr.ethernet.dstAddr: exact;
        }
        const default_action = aiForwardUnknown();
    }
    table tiLearnMAC {
        actions = {
            tiLearnMAC_aiNoOp();
            aiSendDigest();
        }
        key = {
            hdr.ethernet.srcAddr: exact;
        }
        const default_action = aiSendDigest();
    }
    apply {
        tiLearnMAC.apply();
        tiForward.apply();
        tiFilter.apply();
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out mirror_metadata_t mirror_metadata,
    out egress_intrinsic_metadata_t eg_intr_md) {
    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            default: accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }
    state start {
        transition parse_ethernet;
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in mirror_metadata_t mirror_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout mirror_metadata_t mirror_metadata,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    apply {
    }
}


Pipeline(SwitchIngressParser(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;