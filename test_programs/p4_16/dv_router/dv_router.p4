#include <core.p4>
#include <tna.p4>

header arp_t {
    bit<16> htype;
    bit<16> ptype;
    bit<8>  hlen;
    bit<8>  plen;
    bit<16> oper;
    bit<48> senderHA;
    bit<32> senderPA;
    bit<48> targetHA;
    bit<32> targetPA;
}

header distance_vec_t {
    bit<32>  src;
    bit<16>  length_dist;
    bit<144> data;
}

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}


struct cis553_metadata_t {
    bit<1>  forMe;
    bit<7>  __pad;
    bit<32> temp;
    bit<32> nextHop;
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

header bridged_header_t {
    bit<9> temp_port;
    bit<7> __pad;
}

struct dv_router_metadata_t {
    cis553_metadata_t cis553_metadata;
    bridged_header_t bridged_header;
}

struct headers_t {
    bridged_header_t     bridged_header;
    ethernet_t           ethernet;
    arp_t                arp;
    distance_vec_t       distance_vec;
    ipv4_t               ipv4;
}

const bit<10> CPU_INGRESS_MIRROR_ID = 98;


parser IngressParserImpl(packet_in pkt, 
    out headers_t hdr, 
    out dv_router_metadata_t dv_router_metadata,
    out ingress_intrinsic_metadata_t ig_intr_md,
    out ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm,
    out ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr) {
    Checksum() checksum_1;

    state parse_arp {
        pkt.extract(hdr.arp);
        transition accept;
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        checksum_1.add(hdr.ipv4.version);
        checksum_1.add(hdr.ipv4.ihl);
        checksum_1.add(hdr.ipv4.diffserv);
        checksum_1.add(hdr.ipv4.totalLen);
        checksum_1.add(hdr.ipv4.identification);
        checksum_1.add(hdr.ipv4.flags);
        checksum_1.add(hdr.ipv4.fragOffset);
        checksum_1.add(hdr.ipv4.ttl);
        checksum_1.add(hdr.ipv4.protocol);
        checksum_1.add(hdr.ipv4.srcAddr);
        checksum_1.add(hdr.ipv4.dstAddr);
        checksum_1.add(hdr.ipv4.hdrChecksum);
        ig_intr_md_from_prsr.parser_err[12:12] = (bit<1>)checksum_1.verify();
        transition accept;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            16w0x806: parse_arp;
            16w0x553: parse_routing;
            default: accept;
        }
    }

    state parse_routing {
        pkt.extract(hdr.distance_vec);
        transition accept;
    }

    state start {
        pkt.extract(ig_intr_md);
        transition select(ig_intr_md.ingress_port) {
            9w192: parse_routing;
            default: parse_ethernet;
        }
    }
}


control SwitchIngress(
        inout headers_t hdr,
        inout dv_router_metadata_t dv_router_metadata,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    action tiDrop_aDrop() {
        ig_intr_dprsr_md.drop_ctl = 3w1;
    }
    action tiHandleIncomingArpReqest_part_two_aDrop() {
        ig_intr_dprsr_md.drop_ctl = 3w1;    
    }
    action tiHandleIncomingEthernet_aDrop() {
        ig_intr_dprsr_md.drop_ctl = 3w1;   
    }
    action tiHandleIpv4_aDrop() {
        ig_intr_dprsr_md.drop_ctl = 3w1;       
    }
    action tiHandleOutgoingRouting_ai_nop() {
    }
    action tiHandleIncomingArpReqest_part_one_ai_nop(){}
    action tiHandleOutgoingEthernet_ai_nop() {}
    action aiHandleIncomingArpReqest_part_one(bit<48> mac_sa) {
        hdr.arp.oper = 16w2;
        hdr.ethernet.srcAddr = mac_sa;
        hdr.ethernet.dstAddr = hdr.arp.senderHA;
        dv_router_metadata.cis553_metadata.temp = hdr.arp.targetPA;
        hdr.arp.targetHA = hdr.arp.senderHA;
        hdr.arp.senderHA = mac_sa;
        ig_intr_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
    }
    
    action aiHandleIncomingArpReqest_part_two() {
        hdr.arp.targetPA = hdr.arp.senderPA;
        hdr.arp.senderPA = dv_router_metadata.cis553_metadata.temp;
    }
    action aiHandleIncomingArpResponse() {
        {
            ig_intr_dprsr_md.mirror_type = (bit<3>)3w1;
        }
    }
    action aiForMe() {
        dv_router_metadata.cis553_metadata.forMe = 1w1;
    }
    action aiHandleIncomingRouting() {
        {
            ig_intr_dprsr_md.mirror_type = (bit<3>)3w1;
        }
    }
    action aiFindNextL3Hop(bit<32> nextHop) {
        dv_router_metadata.cis553_metadata.nextHop = nextHop;
    }
    action aiSendToLastHop() {
        dv_router_metadata.cis553_metadata.nextHop = hdr.ipv4.dstAddr;
    }
    action aiForward(bit<48> mac_sa, bit<48> mac_da, bit<9> egress_port) {
        hdr.ethernet.srcAddr = mac_sa;
        hdr.ethernet.dstAddr = mac_da;
        ig_intr_tm_md.ucast_egress_port = egress_port;
    }
    action aiArpMiss(bit<32> local_ip, bit<48> local_mac, bit<9> local_port) {
        ig_intr_tm_md.ucast_egress_port = local_port;
        hdr.ethernet.dstAddr = 48w0xffffffffffff;
        hdr.ethernet.etherType = 16w0x806;
        hdr.ipv4.setInvalid();
        hdr.arp.setValid();
        hdr.arp.htype = 16w1;
        hdr.arp.ptype = 16w0x800;
        hdr.arp.hlen = 8w6;
        hdr.arp.plen = 8w4;
        hdr.arp.oper = 16w1;
        hdr.arp.senderHA = local_mac;
        hdr.arp.senderPA = local_ip;
        hdr.arp.targetHA = 48w0;
        hdr.arp.targetPA = dv_router_metadata.cis553_metadata.nextHop;
    }
    action aiHandleOutgoingRouting(bit<9> egress_port) {
        ig_intr_tm_md.ucast_egress_port = egress_port;
        hdr.ethernet.setValid();
        hdr.ethernet.dstAddr = 48w0xffffffffffff;
        hdr.ethernet.etherType = 16w0x553;
        dv_router_metadata.cis553_metadata.forMe = 1w0;
    }
    table tiDrop {
        actions = {
            tiDrop_aDrop();
        }
        const default_action = tiDrop_aDrop();
    }
    table tiHandleIncomingArpReqest_part_one {
        actions = {
            aiHandleIncomingArpReqest_part_one();
            tiHandleIncomingArpReqest_part_one_ai_nop();
        }
        key = {
            hdr.arp.targetPA       : exact;
            ig_intr_md.ingress_port: exact;
        }
        const default_action = tiHandleIncomingArpReqest_part_one_ai_nop();
    }
    table tiHandleIncomingArpReqest_part_two {
        actions = {
            aiHandleIncomingArpReqest_part_two();
            tiHandleIncomingArpReqest_part_two_aDrop();
        }
        key = {
            hdr.arp.targetPA       : exact;
            ig_intr_md.ingress_port: exact;
        }
        const default_action = tiHandleIncomingArpReqest_part_two_aDrop();
    }
    table tiHandleIncomingArpResponse {
        actions = {
            aiHandleIncomingArpResponse();
        }
        const default_action = aiHandleIncomingArpResponse();
    }
    table tiHandleIncomingEthernet {
        actions = {
            aiForMe();
            tiHandleIncomingEthernet_aDrop();
        }
        key = {
            hdr.ethernet.dstAddr   : exact;
            ig_intr_md.ingress_port: exact;
        }
        const default_action = tiHandleIncomingEthernet_aDrop();
    }
    table tiHandleIncomingRouting {
        actions = {
            aiHandleIncomingRouting();
        }
        const default_action = aiHandleIncomingRouting();
    }
    table tiHandleIpv4 {
        actions = {
            aiFindNextL3Hop();
            aiSendToLastHop();
            tiHandleIpv4_aDrop();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        const default_action = tiHandleIpv4_aDrop();
    }
    table tiHandleOutgoingEthernet {
        actions = {
            aiForward();
            aiArpMiss();
            tiHandleOutgoingEthernet_ai_nop();
        }
        key = {
            dv_router_metadata.cis553_metadata.nextHop: lpm;
        }
        const default_action = tiHandleOutgoingEthernet_ai_nop();
    }
    table tiHandleOutgoingRouting {
        actions = {
            aiHandleOutgoingRouting();
            tiHandleOutgoingRouting_ai_nop();
        }
        key = {
            hdr.distance_vec.src: exact;
        }
        const default_action = tiHandleOutgoingRouting_ai_nop();
    }

    apply {
        if (hdr.ethernet.isValid()) {
            tiHandleIncomingEthernet.apply();
        } else {
            tiHandleOutgoingRouting.apply();
        }
        if (dv_router_metadata.cis553_metadata.forMe == 1w0) {
        } else {
            if (hdr.ipv4.isValid()) {
                tiHandleIpv4.apply();
                tiHandleOutgoingEthernet.apply();
            } else {
                if (hdr.arp.isValid() && hdr.arp.oper == 16w1) {
                    tiHandleIncomingArpReqest_part_one.apply();
                    tiHandleIncomingArpReqest_part_two.apply();
                } else {
                    if (hdr.arp.isValid() && hdr.arp.oper == 16w2) {
                        tiHandleIncomingArpResponse.apply();
                    } else {
                        if (hdr.distance_vec.isValid()) {
                            tiHandleIncomingRouting.apply();
                        } else {
                            tiDrop.apply();
                        }
                    }
                }
            }
        }
        dv_router_metadata.bridged_header.setValid();
        dv_router_metadata.bridged_header.temp_port = ig_intr_md.ingress_port;
    }
}

control SwitchIngressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in dv_router_metadata_t dv_router_metadata,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr) {
    Mirror() mirror;
    apply {
        if (ig_intr_md_for_dprsr.mirror_type == 3w1) {
            mirror.emit(CPU_INGRESS_MIRROR_ID);
        }
        pkt.emit(dv_router_metadata.bridged_header);
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.arp);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.distance_vec);
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out dv_router_metadata_t dv_router_metadata,
    out egress_intrinsic_metadata_t eg_intr_md) {

    state parse_arp {
        pkt.extract(hdr.arp);
        transition accept;
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition accept;
    }

    state parse_routing {
        pkt.extract(hdr.distance_vec);
        transition accept;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            16w0x806: parse_arp;
            16w0x553: parse_routing;
            default: accept;
        }
    }

    state parse_bridged {
        pkt.extract(dv_router_metadata.bridged_header);
        transition select(dv_router_metadata.bridged_header.temp_port) {
            9w192: parse_routing;
            default: parse_ethernet;
        }
    }

    state start {
        transition parse_bridged;
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in dv_router_metadata_t dv_router_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() checksum_0;
    apply {
        hdr.ipv4.hdrChecksum = checksum_0.update({ hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr });
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.arp);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.distance_vec);
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout dv_router_metadata_t dv_router_metadata,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    apply {
    }   
}


Pipeline(IngressParserImpl(),
    SwitchIngress(),
    SwitchIngressDeparser(),
    SwitchEgressParser(), 
    SwitchEgress(),
    SwitchEgressDeparser()) pipe;

Switch(pipe) main;
