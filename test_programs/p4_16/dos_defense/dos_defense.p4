#include <core.p4>
#include <tna.p4>

const bit<10> CPU_INGRESS_MIRROR_ID = 98;

struct metadata_t {
    bit<32> reg_val_one;
    bit<32> reg_val_two;
    bit<10> reg_pos_one;
    bit<6>  __pad_1;
    bit<10> reg_pos_two;
    bit<6>  __pad_2;
    bit<8>  reg_val_one_warning;
    bit<8>  reg_val_two_warning;
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
    ethernet_t  ethernet;
    ipv4_t      ipv4;
    tcp_t       tcp;
}

parser SwitchIngressParser(packet_in packet,
    out headers_t hdr,
    out metadata_t ig_md,
    out ingress_intrinsic_metadata_t ig_intr_md) {
    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            default: accept;
        }
    }

    state parse_tcp {
        packet.extract(hdr.tcp);
        transition accept;
    }
}

Register<bit<32>, bit<32>>(1024) flow_counter_1;
Register<bit<32>, bit<32>>(1024) flow_counter_2;
Register<bit<32>, bit<32>>(1024) flow_counter_warning_1;
Register<bit<32>, bit<32>>(1024) flow_counter_warning_2;

control ingress(inout headers_t hdr, 
    inout metadata_t my_meta, 
    in ingress_intrinsic_metadata_t ig_intr_md, 
    in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_parser_aux, 
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr, 
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    Hash<bit<16>>(HashAlgorithm_t.CRC16) hash_2;
    Hash<bit<32>>(HashAlgorithm_t.CRC32) hash_3;

    RegisterAction<bit<32>, bit<32>, bit<32>>(flow_counter_1) riw_flow_counter_1 = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = in_value + 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(flow_counter_2) riw_flow_counter_2 = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = in_value + 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(flow_counter_warning_1) riw_flow_counter_warning_1 = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            if (my_meta.reg_val_one > 32w64) {
                value = 32w1;
            }
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(flow_counter_warning_2) riw_flow_counter_warning_2 = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            if (my_meta.reg_val_two > 32w64) {
                value = 32w1;
            }
        }
    };
    action ipv4_forward(bit<48> dstAddr, bit<9> port) {
        ig_intr_md_for_tm.ucast_egress_port = port;
        hdr.ethernet.srcAddr = dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 8w1;
    }
    action drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action ai_handle_blacklist() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action aiSendDigest() {
        ig_intr_md_for_dprsr.mirror_type = (bit<3>)3w1;
    }
    action ai_calculate_hash() {
        {
            my_meta.reg_pos_one = hash_2.get({ my_meta.first, my_meta.second, my_meta.third, my_meta.fourth, hdr.ipv4.protocol })[9:0];
        }
        {
            my_meta.reg_pos_two = hash_3.get({ my_meta.first, my_meta.second, my_meta.third, my_meta.fourth, hdr.ipv4.protocol })[9:0];
        }
    }
    action ai_get_incoming_pos() {
        my_meta.first = hdr.ipv4.srcAddr;
        my_meta.second = hdr.ipv4.dstAddr;
        my_meta.third = hdr.tcp.srcPort;
        my_meta.fourth = hdr.tcp.dstPort;
    }
    action ai_write_flow_counter_1() {
        my_meta.reg_val_one = riw_flow_counter_1.execute((bit<32>)my_meta.reg_pos_one);
    }
    action ai_write_flow_counter_2() {
        my_meta.reg_val_two = riw_flow_counter_2.execute((bit<32>)my_meta.reg_pos_two);
    }
    action ai_write_flow_counter_warning_1() {
        my_meta.reg_val_one_warning = (bit<8>)riw_flow_counter_warning_1.execute((bit<32>)my_meta.reg_pos_one);
    }
    action ai_write_flow_counter_warning_2() {
        my_meta.reg_val_two_warning = (bit<8>)riw_flow_counter_warning_2.execute((bit<32>)my_meta.reg_pos_two);
    }
    table ipv4_lpm {
        actions = {
            ipv4_forward();
            drop_packet();
            ai_handle_blacklist();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        size = 1024;
        const default_action = drop_packet();
    }
    table tiSendDigest_1 {
        actions = {
            aiSendDigest();
        }
        const default_action = aiSendDigest();
    }
    table tiSendDigest_2 {
        actions = {
            aiSendDigest();
        }
        const default_action = aiSendDigest();
    }
    table ti_calculate_hash {
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
    table ti_write_flow_counter_1 {
        actions = {
            ai_write_flow_counter_1();
        }
        size = 1;
        const default_action = ai_write_flow_counter_1();
    }
    table ti_write_flow_counter_2 {
        actions = {
            ai_write_flow_counter_2();
        }
        size = 1;
        const default_action = ai_write_flow_counter_2();
    }
    table ti_write_flow_counter_warning_1 {
        actions = {
            ai_write_flow_counter_warning_1();
        }
        size = 1;
        const default_action = ai_write_flow_counter_warning_1();
    }
    table ti_write_flow_counter_warning_2 {
        actions = {
            ai_write_flow_counter_warning_2();
        }
        size = 1;
        const default_action = ai_write_flow_counter_warning_2();
    }
    apply {
        if (hdr.ipv4.isValid()) {
            ipv4_lpm.apply();
            ti_get_incoming_pos.apply();
            ti_calculate_hash.apply();
            ti_write_flow_counter_1.apply();
            ti_write_flow_counter_warning_1.apply();
            ti_write_flow_counter_2.apply();
            ti_write_flow_counter_warning_2.apply();
            if (my_meta.reg_val_one_warning == 8w1) {
                tiSendDigest_1.apply();
            }
            if (my_meta.reg_val_two_warning == 8w1) {
                tiSendDigest_2.apply();
            }
        }
    }
}


control SwitchIngressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr) {
    Mirror() mirror;
    apply {
        if (ig_intr_md_for_dprsr.mirror_type == 3w1) {
            mirror.emit(CPU_INGRESS_MIRROR_ID);
        }
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.tcp);
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in metadata_t my_meta,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() ipv4_checksum;
    
    apply {
        hdr.ipv4.hdrChecksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.diffserv,
            hdr.ipv4.totalLen,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.fragOffset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.srcAddr,
            hdr.ipv4.dstAddr
        });
    
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
    apply {}
}

parser EgressParserImpl(packet_in packet,
    out headers_t hdr,
    out metadata_t ig_md,
    out egress_intrinsic_metadata_t ig_intr_md) {
    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            default: accept;
        }
    }

    state parse_tcp {
        packet.extract(hdr.tcp);
        transition accept;
    }
}

Pipeline(
    SwitchIngressParser(), 
    ingress(), 
    SwitchIngressDeparser(), 
    EgressParserImpl(), 
    SwitchEgress(),
    SwitchEgressDeparser()
) pipe;

Switch(pipe) main;
