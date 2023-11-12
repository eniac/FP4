#include <core.p4>
#include <tna.p4>

const bit<10> MC1_INGRESS_MIRROR_ID =  101;

const bit<8> PKT_TYPE_NORMAL = 1;
const bit<8> PKT_TYPE_MIRROR = 2;


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

header ig_mirror_header_t {
    bit<8>  pkt_type;
    bit<32> meta_count_mirror;
    bit<32> meta_count_original;
}

header bridged_header_t {
    bit<8>  pkt_type;
}

struct metadata_t {
    bit<32> count_mirror;
    bit<32> count_original;
    // @saturating 
    bit<32> diff1;
    // @saturating 
    bit<32> diff2;
    ig_mirror_header_t ig_mirror_header;
}

struct headers_t {
    ethernet_t           ethernet;
    bridged_header_t bridged_header;
    ipv4_t               ipv4;
}

parser TofinoEgressParser(
        packet_in pkt,
        out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }
}

parser SwitchIngressParser(packet_in pkt,
    out headers_t hdr,
    out metadata_t ig_md,
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
        in metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    Mirror() mirror;
    apply {
        if (ig_intr_dprsr_md.mirror_type == 3w1) {
            mirror.emit<ig_mirror_header_t>(MC1_INGRESS_MIRROR_ID, ig_md.ig_mirror_header);
        }
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

control SwitchIngress(inout headers_t hdr,
    inout metadata_t metadata, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    action aiSendClone() {
        ig_intr_dprsr_md.mirror_type = (bit<3>)3w1;
        hdr.bridged_header.pkt_type = PKT_TYPE_MIRROR;
    }
    action aiSetOutputPort() {
        ig_intr_md_for_tm.ucast_egress_port = ig_intr_md_for_tm.ucast_egress_port + 9w4;

    }
    action aiAddHeader() {
        hdr.bridged_header.setValid();
        hdr.bridged_header.pkt_type = PKT_TYPE_NORMAL;
    }
    table tiSendClone {
        actions = {
            aiSendClone();
        }
        key = {
            hdr.ethernet.etherType: exact;
        }
        const default_action = aiSendClone();
    }
    table tiSetOutputPort {
        actions = {
            aiSetOutputPort();
        }
        const default_action = aiSetOutputPort();
    }
    table tiAddHeader {
        actions = {
            aiAddHeader();
        }
        const default_action = aiAddHeader();
    }
    apply {
        tiSendClone.apply();
        tiSetOutputPort.apply();
        tiAddHeader.apply();
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out metadata_t eg_md,
    out egress_intrinsic_metadata_t eg_intr_md) {

    TofinoEgressParser() tofino_parser;

    state start {
        tofino_parser.apply(pkt, eg_intr_md);
        transition parse_metadata;
    }

    state parse_metadata {
        bridged_header_t bridged_header = pkt.lookahead<bridged_header_t>();
        transition select(bridged_header.pkt_type) {
            PKT_TYPE_MIRROR : parse_mirror_md;
            PKT_TYPE_NORMAL : parse_bridged_md;
            default : accept;
        }
    }

    state parse_bridged_md {
        pkt.extract(hdr.bridged_header);
        transition parse_ethernet;
    }

    state parse_mirror_md {
        ig_mirror_header_t ig_mirror_header;
        pkt.extract(ig_mirror_header);
        transition parse_ethernet;
    }

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
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in metadata_t ingress_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() checksum_0;
    apply {
        hdr.ipv4.hdrChecksum = checksum_0.update({ hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr });
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

Register<bit<32>, bit<32>>(32w1) reg_mirror;
Register<bit<32>, bit<32>>(32w1) reg_original;

control SwitchEgress(inout headers_t hdr, 
    inout metadata_t ingress_metadata,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_mirror) bb_mirror_read = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_mirror) bb_mirror_update = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = in_value + 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_original) bb_original_read = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_original) bb_original_update = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = in_value + 32w1;
        }
    };
    action aeGetDiff() {
        ingress_metadata.diff1 = ingress_metadata.count_original |-| ingress_metadata.count_mirror;
        ingress_metadata.diff2 = ingress_metadata.count_mirror |-| ingress_metadata.count_original;
    }
    action aeReadMirror() {
        ingress_metadata.count_mirror = bb_mirror_read.execute(32w0);
    }
    action aeReadOriginal() {
        ingress_metadata.count_original = bb_original_read.execute(32w0);
    }
    action aeUpdateMirror() {
        ingress_metadata.count_mirror = bb_mirror_update.execute(32w0);
    }
    action aeUpdateOriginal() {
        ingress_metadata.count_original = bb_original_update.execute(32w0);
    }
    table teGetDiff {
        actions = {
            aeGetDiff();
        }
        const default_action = aeGetDiff();
    }
    table teReadMirror {
        actions = {
            aeReadMirror();
        }
        const default_action = aeReadMirror();
    }
    table teReadOriginal {
        actions = {
            aeReadOriginal();
        }
        const default_action = aeReadOriginal();
    }
    table teUpdateMirror {
        actions = {
            aeUpdateMirror();
        }
        const default_action = aeUpdateMirror();
    }
    table teUpdateOriginal {
        actions = {
            aeUpdateOriginal();
        }
        const default_action = aeUpdateOriginal();
    }
    apply {
        if (eg_intr_md.egress_port == 9w40) {
            teUpdateMirror.apply();
            teReadOriginal.apply();
        } else {
            teReadMirror.apply();
            teUpdateOriginal.apply();
        }
        teGetDiff.apply();
    }
}


Pipeline(SwitchIngressParser(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;