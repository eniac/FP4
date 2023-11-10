#include <core.p4>
#include <tna.p4>

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
    bit<6>  ctrl;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgentPtr;
}

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}


struct headers_t {
    ethernet_t ethernet;
    ipv4_t     ipv4;
    tcp_t      tcp;
}

// If changing the number of up links, also modify the bitwidth of ecmp_select
const bit<2> NUM_UPLINKS = 2;

struct ecmp_metadata_t {
    bit<1> ecmp_select;
}

header bridged_header_t {
    bit<9> temp_port;
    bit<7> __pad;
}

struct firewall_metadata_t {
    ecmp_metadata_t ecmp_metadata;
    bridged_header_t bridged_header;
}

parser SwitchIngressParser(packet_in pkt,
    out headers_t hdr,
    out firewall_metadata_t ig_md,
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
        in firewall_metadata_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr) {
    apply {
        pkt.emit(ig_md.bridged_header);
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.tcp);
    }
}


control SwitchIngress(inout headers_t hdr,
    inout firewall_metadata_t ig_md, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    Hash<bit<8>>(HashAlgorithm_t.CRC8) hash_1;
    
    action ecmp_group_drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action ecmp_nhop_drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    action set_ecmp_select() {
        {
            ig_md.ecmp_metadata.ecmp_select = hash_1.get({ hdr.ipv4.srcAddr, hdr.ipv4.dstAddr, hdr.ipv4.protocol, hdr.tcp.srcPort, hdr.tcp.dstPort })[0:0];
        }
    }
    action set_nhop(bit<48> nhop_dmac, bit<32> nhop_ipv4, bit<9> port) {
        hdr.ethernet.dstAddr = nhop_dmac;
        hdr.ipv4.dstAddr = nhop_ipv4;
        ig_intr_md_for_tm.ucast_egress_port = port;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 8w1;
    }
    table ecmp_group {
        actions = {
            ecmp_group_drop_packet();
            set_ecmp_select();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        size = 1024;
        const default_action = ecmp_group_drop_packet();
    }
    table ecmp_nhop {
        actions = {
            ecmp_nhop_drop_packet();
            set_nhop();
        }
        key = {
            ig_md.ecmp_metadata.ecmp_select: exact;
        }
        size = 2;
        const default_action = ecmp_nhop_drop_packet();
    }
    apply {
        if (hdr.ipv4.isValid()) {
            ecmp_group.apply();
            ecmp_nhop.apply();
        }
        ig_md.bridged_header.setValid();
        ig_md.bridged_header.temp_port = ig_intr_md.ingress_port;
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out firewall_metadata_t eg_md,
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
        pkt.extract(eg_md.bridged_header);
        transition parse_ethernet;
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout firewall_metadata_t eg_md,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {

    action rewrite_mac(bit<48> smac) {
        hdr.ethernet.srcAddr = smac;
    }
    action send_frame_drop_packet() {
        eg_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    table send_frame {
        actions = {
            rewrite_mac();
            send_frame_drop_packet();
        }
        key = {
            eg_md.bridged_header.temp_port: exact;
        }
        size = 256;
        const default_action = send_frame_drop_packet();
    }
    apply {
        send_frame.apply();
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in firewall_metadata_t eg_md,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {

    Checksum() checksum_0;
    apply {
        hdr.ipv4.hdrChecksum = checksum_0.update({ hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr });
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.tcp);
    }
}


Pipeline(SwitchIngressParser(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;
