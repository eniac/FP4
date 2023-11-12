#include <core.p4>
#include <tna.p4>

struct metadata_t {
    bit<32> cur_ts;
    bit<32> interArrival;
    @saturating 
    bit<32> current_reading;
    bit<1>  bitCounter;
    bit<9> egress_port;
}

struct standard_metadata_t {
    bit<9>  ingress_port;
    bit<32> packet_length;
    bit<9>  egress_spec;
    bit<9>  egress_port;
    bit<16> egress_instance;
    bit<32> instance_type;
    bit<8>  parser_status;
    bit<8>  parser_error_location;
}

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
    ethernet_t ethernet; 
    ipv4_t     ipv4;
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

    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}


control SwitchIngress(inout headers_t hdr,
    inout metadata_t ingress_metadata, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    action ipv4_forward(bit<48> dstAddr, bit<9> port) {
        ig_intr_md_for_tm.ucast_egress_port = port;
        hdr.ethernet.srcAddr = dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 8w1;
    }
    action drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action ai_noOp() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    table ipv4_lpm {
        actions = {
            ipv4_forward();
            drop_packet();
            ai_noOp();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        size = 1024;
        const default_action = drop_packet();
    }
    apply {
        if (hdr.ipv4.isValid()) {
            ipv4_lpm.apply();
        }
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out metadata_t eg_md,
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
        in metadata_t ingress_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() checksum_0;
    apply {
        hdr.ipv4.hdrChecksum = checksum_0.update({ hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr });
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
    }
}

struct reWmaPhase1_layout {
    bit<32> hi;
    bit<32> lo;
}
struct reWmaPhase2_layout {
    bit<32> hi;
    bit<32> lo;
}

Register<bit<8>, bit<32>>(32w2) reg_bit_counter_egress;
Register<bit<64>, bit<32>>(32w2) reg_interval_wma_egress;
Register<bit<64>, bit<32>>(32w2) reg_last_timestamp_egress;

control SwitchEgress(inout headers_t hdr, 
    inout metadata_t meta,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    RegisterAction<bit<8>, bit<32>, bit<8>>(reg_bit_counter_egress) reBitCounter = {
        void apply(inout bit<8> value, out bit<8> rv) {
            rv = 8w0;
            bit<8> in_value;
            in_value = value;
            rv = in_value;
            value = in_value + 8w1;
        }
    };
    RegisterAction<reWmaPhase1_layout, bit<32>, bit<32>>(reg_last_timestamp_egress) reWmaPhase1 = {
        void apply(inout reWmaPhase1_layout value, out bit<32> rv) {
            rv = 32w0;
            reWmaPhase1_layout in_value;
            in_value = value;
            value.hi = meta.cur_ts - in_value.lo;
            value.lo = meta.cur_ts;
            rv = value.hi;
        }
    };
    MathUnit<bit<32>>(false, 2s0, -6s4, { 8w15, 8w14, 8w13, 8w12, 8w11, 8w10, 8w9, 8w8, 8w7, 8w6, 8w5, 8w4, 8w3, 8w2, 8w1, 8w0 }) reWmaPhase2_math_unit_0;
    RegisterAction<reWmaPhase2_layout, bit<32>, bit<32>>(reg_interval_wma_egress) reWmaPhase2 = {
        void apply(inout reWmaPhase2_layout value, out bit<32> rv) {
            rv = 32w0;
            reWmaPhase2_layout in_value;
            in_value = value;
            if (meta.bitCounter == 1w0) {
                value.hi = in_value.lo;
            }
            if (meta.bitCounter == 1w0) {
                value.lo = in_value.lo + meta.interArrival;
            }
            if (!(meta.bitCounter == 1w0)) {
                value.lo = (bit<32>)reWmaPhase2_math_unit_0.execute(in_value.lo);
            }
            rv = value.hi;
        }
    };
    action aeInitializeWma() {
        meta.cur_ts = (bit<32>)eg_intr_md_from_prsr.global_tstamp;
        meta.bitCounter = (bit<1>)reBitCounter.execute((bit<32>)meta.egress_port);
    }
    action aeWmaPhase1() {
        meta.interArrival = reWmaPhase1.execute((bit<32>)meta.egress_port);
    }
    action aeWmaPhase2() {
        meta.current_reading = reWmaPhase2.execute((bit<32>)meta.egress_port);
    }
    action drop_packet() {
        ig_intr_dprs_md.drop_ctl = 3w1;
    }
    action ae_rate_limit(bit<32> x) {
        meta.current_reading = x |-| meta.current_reading;
    }
    table teInitializeWma {
        actions = {
            aeInitializeWma();
        }
        size = 1;
        const default_action = aeInitializeWma();
    }
    table teWmaPhase1 {
        actions = {
            aeWmaPhase1();
        }
        size = 1;
        const default_action = aeWmaPhase1();
    }
    table teWmaPhase2 {
        actions = {
            aeWmaPhase2();
        }
        size = 1;
        const default_action = aeWmaPhase2();
    }
    table te_drop {
        actions = {
            drop_packet();
        }
        size = 1;
        const default_action = drop_packet();
    }
    table te_rate_limit {
        actions = {
            ae_rate_limit();
        }
        size = 1;
        default_action = ae_rate_limit(32w1024);
    }
    apply {
        teInitializeWma.apply();
        teWmaPhase1.apply();
        teWmaPhase2.apply();
        te_rate_limit.apply();
        if (meta.current_reading == 32w0) {
            te_drop.apply();
        }
    }
}

Pipeline(SwitchIngressParser(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;