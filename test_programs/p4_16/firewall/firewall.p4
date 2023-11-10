#include <core.p4>
#include <tna.p4>

struct metadata_t {
    bit<1>  direction;
    bit<1>  check_ports_hit;
    bit<1>  reg_val_one;
    bit<1>  reg_val_two;
    bit<12> reg_pos_one;
    bit<12> reg_pos_two;
    bit<4>  __pad;
    bit<32> first;
    bit<32> second;
    bit<16> third;
    bit<16> fourth;
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

header tcp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<32> seqNo;
    bit<32> ackNo;
    bit<4>  dataOffset;
    bit<3>  res;
    bit<3>  ecn;
    bit<3>  ctrl;
    bit<1>  rst;
    bit<1>  syn;
    bit<1>  fin;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgentPtr;
}


struct headers_t {
    ethernet_t ethernet; 
    ipv4_t     ipv4;
    tcp_t      tcp;
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
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            default: accept;
        }
    }
    state parse_tcp {
        pkt.extract(hdr.tcp);
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
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr) {
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.tcp);
    }
}

Register<bit<1>, bit<32>>(32w4096) bloom_filter_1;

Register<bit<1>, bit<32>>(32w4096) bloom_filter_2;



control SwitchIngress(inout headers_t hdr,
    inout metadata_t ingress_metadata, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    Hash<bit<16>>(HashAlgorithm_t.CRC16) hash_2;
    Hash<bit<32>>(HashAlgorithm_t.CRC32) hash_3;

    RegisterAction<bit<1>, bit<32>, bit<1>>(bloom_filter_1) rir_boom_filter1 = {
        void apply(inout bit<1> value, out bit<1> rv) {
            rv = 1w0;
            bit<1> in_value;
            in_value = value;
            rv = in_value;
        }
    };
    RegisterAction<bit<1>, bit<32>, bit<1>>(bloom_filter_2) rir_boom_filter2 = {
        void apply(inout bit<1> value, out bit<1> rv) {
            rv = 1w0;
            bit<1> in_value;
            in_value = value;
            rv = in_value;
        }
    };
    RegisterAction<bit<1>, bit<32>, bit<1>>(bloom_filter_1) riw_boom_filter1 = {
        void apply(inout bit<1> value) {
            bit<1> in_value;
            in_value = value;
            value = 1w1;
        }
    };
    RegisterAction<bit<1>, bit<32>, bit<1>>(bloom_filter_2) riw_boom_filter2 = {
        void apply(inout bit<1> value) {
            bit<1> in_value;
            in_value = value;
            value = 1w1;
        }
    };
    action set_direction(bit<1> dir) {
        ingress_metadata.direction = dir;
        ingress_metadata.check_ports_hit = 1w1;
    }
    action set_hit() {
        ingress_metadata.check_ports_hit = 1w1;
        ingress_metadata.direction = 1w1;
    }
    action ipv4_forward(bit<48> dstAddr, bit<9> port) {
        ig_intr_md_for_tm.ucast_egress_port = port;
        hdr.ethernet.srcAddr = dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 8w1;
    }
    action ti_apply_filter_drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action ipv4_lpm_drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;   
    }
    action ipv4_lpm_ai_noOp() {}
    action ti_apply_filter_ai_noOp() {}

    action ai_calculate_hash() {
        {
            ingress_metadata.reg_pos_one = hash_2.get({ ingress_metadata.first, ingress_metadata.second, ingress_metadata.third, ingress_metadata.fourth, hdr.ipv4.protocol })[11:0];
        }
        {
            ingress_metadata.reg_pos_two = hash_3.get({ ingress_metadata.first, ingress_metadata.second, ingress_metadata.third, ingress_metadata.fourth, hdr.ipv4.protocol })[11:0];
        }
    }
    action ai_get_incoming_pos() {
        ingress_metadata.first = hdr.ipv4.srcAddr;
        ingress_metadata.second = hdr.ipv4.dstAddr;
        ingress_metadata.third = hdr.tcp.srcPort;
        ingress_metadata.fourth = hdr.tcp.dstPort;
    }
    action ai_get_outgoing_pos() {
        ingress_metadata.first = hdr.ipv4.dstAddr;
        ingress_metadata.second = hdr.ipv4.srcAddr;
        ingress_metadata.third = hdr.tcp.dstPort;
        ingress_metadata.fourth = hdr.tcp.srcPort;
    }
    action ai_read_bloom_filter1() {
        ingress_metadata.reg_val_one = rir_boom_filter1.execute((bit<32>)ingress_metadata.reg_pos_one);
    }
    action ai_read_bloom_filter2() {
        ingress_metadata.reg_val_two = rir_boom_filter2.execute((bit<32>)ingress_metadata.reg_pos_two);
    }
    action ai_write_bloom_filter1() {
        riw_boom_filter1.execute((bit<32>)ingress_metadata.reg_pos_one);
    }
    action ai_write_bloom_filter2() {
        riw_boom_filter2.execute((bit<32>)ingress_metadata.reg_pos_two);
    }
    table check_ports {
        actions = {
            set_direction();
            set_hit();
        }
        key = {
            ig_intr_md.ingress_port            : exact;
            ig_intr_md_for_tm.ucast_egress_port: exact;
        }
        size = 1024;
        const default_action = set_hit();
    }
    table ipv4_lpm {
        actions = {
            ipv4_forward();
            ipv4_lpm_drop_packet();
            ipv4_lpm_ai_noOp();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        size = 1024;
        const default_action = ipv4_lpm_drop_packet();
    }
    
    table ti_apply_filter {
        actions = {
            ti_apply_filter_drop_packet();
            ti_apply_filter_ai_noOp();
        }
        key = {
            ingress_metadata.reg_val_one: exact;
            ingress_metadata.reg_val_two: exact;
        }
        size = 4;
        default_action = ti_apply_filter_drop_packet();
    }
    table ti_calculate_hash1 {
        actions = {
            ai_calculate_hash();
        }
        size = 1;
        const default_action = ai_calculate_hash();
    }
    table ti_calculate_hash2 {
        actions = {
            ai_calculate_hash();
        }
        size = 1;
        const default_action = ai_calculate_hash();
    }
    table ti_get_incoming_pos {
        actions = {
            ai_get_incoming_pos();
        }
        size = 1;
        const default_action = ai_get_incoming_pos();
    }
    table ti_get_outgoing_pos {
        actions = {
            ai_get_outgoing_pos();
        }
        size = 1;
        const default_action = ai_get_outgoing_pos();
    }
    table ti_read_bloom_filter1 {
        actions = {
            ai_read_bloom_filter1();
        }
        size = 1;
        const default_action = ai_read_bloom_filter1();
    }
    table ti_read_bloom_filter2 {
        actions = {
            ai_read_bloom_filter2();
        }
        size = 1;
        const default_action = ai_read_bloom_filter2();
    }
    table ti_write_bloom_filter1 {
        actions = {
            ai_write_bloom_filter1();
        }
        size = 1;
        const default_action = ai_write_bloom_filter1();
    }
    table ti_write_bloom_filter2 {
        actions = {
            ai_write_bloom_filter2();
        }
        size = 1;
        const default_action = ai_write_bloom_filter2();
    }
    apply {
        if (hdr.ipv4.isValid()) {
            ipv4_lpm.apply();
            check_ports.apply();
            if (ingress_metadata.check_ports_hit == 1w1) {
                if (ingress_metadata.direction == 1w0) {
                    ti_get_incoming_pos.apply();
                    ti_calculate_hash1.apply();
                    if (hdr.tcp.syn == 1w1) {
                        ti_write_bloom_filter1.apply();
                        ti_write_bloom_filter2.apply();
                    }
                } else {
                    ti_get_outgoing_pos.apply();
                    ti_calculate_hash2.apply();
                    ti_read_bloom_filter1.apply();
                    ti_read_bloom_filter2.apply();
                    ti_apply_filter.apply();
                }
            }
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
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            default: accept;
        }
    }
    state parse_tcp {
        pkt.extract(hdr.tcp);
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
        pkt.emit(hdr.tcp);
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout metadata_t ingress_metadata,
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