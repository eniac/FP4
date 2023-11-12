#include <tna.p4>
#include <core.p4>

struct meta_t {
    bit<32> lock_id;
    bit<1>  routed;
    bit<8>  available;
}


header adm_hdr_t {
    bit<8>  op;
    bit<32> lock;
    bit<32> new_left;
    bit<32> new_right;
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

header nc_hdr_t {
    bit<8>  recirc_flag;
    bit<8>  op;
    bit<8>  mode;
    bit<8>  client_id;
    bit<32> tid;
    bit<32> lock;
    bit<32> timestamp_lo;
    bit<32> timestamp_hi;
}

header nlk_hdr_t {
    bit<8>  recirc_flag;
    bit<8>  op;
    bit<8>  mode;
    bit<8>  client_id;
    bit<32> tid;
    bit<32> lock;
    bit<32> timestamp_lo;
    bit<32> timestamp_hi;
}

header probe_hdr_t {
    bit<8>  failure_status;
    bit<8>  op;
    bit<8>  mode;
    bit<8>  client_id;
    bit<32> tid;
    bit<32> lock;
    bit<32> timestamp_lo;
    bit<32> timestamp_hi;
}

header recirculate_hdr_t {
    bit<8>  dequeued_mode;
    bit<32> cur_head;
    bit<32> cur_tail;
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

header udp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<16> pkt_length;
    bit<16> checksum;
}



struct headers_t {
    adm_hdr_t         adm_hdr;
    ethernet_t        ethernet;
    ipv4_t            ipv4;
    nc_hdr_t          nc_hdr;
    nlk_hdr_t         nlk_hdr;
    probe_hdr_t       probe_hdr;
    recirculate_hdr_t recirculate_hdr;
    tcp_t             tcp;
    udp_t             udp;
}

parser SwitchIngressParser(packet_in pkt,
    out headers_t hdr,
    out meta_t ig_md,
    out ingress_intrinsic_metadata_t ig_intr_md) {

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            8w17: parse_udp;
            default: accept;
        }
    }
    state parse_nc_hdr {
        pkt.extract(hdr.nc_hdr);
        transition accept;
    }
    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }
    state parse_udp {
        pkt.extract(hdr.udp);
        transition select(hdr.udp.dstPort) {
            16w6666: parse_nc_hdr;
            default: accept;
        }
    }
    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800:     parse_ipv4;
            default:    accept;    
        }
    }

    state start {
        transition parse_ethernet;
    }
}

control SwitchIngressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in meta_t ig_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.nc_hdr);
        pkt.emit(hdr.tcp);
    }
}

Register<bit<32>, bit<32>>(32w130000) lock_status_register;

control SwitchIngress(inout headers_t hdr,
    inout meta_t meta, 
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {

    RegisterAction<bit<32>, bit<32>, bit<32>>(lock_status_register) acquire_lock_alu = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(lock_status_register) release_lock_alu = {
        void apply(inout bit<32> value) {
            bit<32> in_value;
            in_value = value;
            value = 32w0;
        }
    };
    action acquire_lock_action() {
        meta.available = (bit<8>)acquire_lock_alu.execute(meta.lock_id);
    }
    action decode_action() {
        meta.lock_id = hdr.nc_hdr.lock;
    }
    action set_egress(bit<9> egress_spec) {
        ig_intr_md_for_tm.ucast_egress_port = egress_spec;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 8w1;
    }
    action ipv4_route_drop_action() {
        ig_intr_dprsr_md.drop_ctl = 3w1;
    }
    action release_lock_action() {
        release_lock_alu.execute(meta.lock_id);
    }
    action reply_to_client_action() {
        hdr.ipv4.dstAddr = hdr.ipv4.srcAddr;
    }
    action set_retry_action() {
        hdr.nc_hdr.op = 8w5;
    }
    table acquire_lock_table {
        actions = {
            acquire_lock_action();
        }
        default_action = acquire_lock_action();
    }
    table decode_table {
        actions = {
            decode_action();
        }
        default_action = decode_action();
    }
    table ipv4_route {
        actions = {
            set_egress();
            ipv4_route_drop_action();
        }
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        size = 256;
        default_action = ipv4_route_drop_action();
    }
    table release_lock_table {
        actions = {
            release_lock_action();
        }
        default_action = release_lock_action();
    }
    table reply_to_client_table {
        actions = {
            reply_to_client_action();
        }
        default_action = reply_to_client_action();
    }
    table set_retry_table {
        actions = {
            set_retry_action();
        }
        default_action = set_retry_action();
    }
    apply {
        if (hdr.nc_hdr.isValid()) {
            decode_table.apply();
            if (hdr.nc_hdr.op == 8w0) {
                acquire_lock_table.apply();
                if (meta.available != 8w0) {
                    set_retry_table.apply();
                }
            } else {
                if (hdr.nc_hdr.op == 8w1) {
                    release_lock_table.apply();
                }
            }
            reply_to_client_table.apply();
        }
        ipv4_route.apply();
    }
}

parser SwitchEgressParser(packet_in pkt, 
    out headers_t hdr,
    out meta_t eg_md,
    out egress_intrinsic_metadata_t eg_intr_md) {

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            8w6: parse_tcp;
            8w17: parse_udp;
            default: accept;
        }
    }
    state parse_nc_hdr {
        pkt.extract(hdr.nc_hdr);
        transition accept;
    }
    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }
    state parse_udp {
        pkt.extract(hdr.udp);
        transition select(hdr.udp.dstPort) {
            16w6666: parse_nc_hdr;
            default: accept;
        }
    }
    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            16w0x800: parse_ipv4;
            default: accept;
        }
    }
    state start {
        transition parse_ethernet;
    }
}

control SwitchEgressDeparser(
        packet_out pkt,
        inout headers_t hdr,
        in meta_t ingress_metadata,
        in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr) {
    Checksum() checksum_0;
    apply {
        hdr.ipv4.hdrChecksum = checksum_0.update({ hdr.ipv4.version, hdr.ipv4.ihl, hdr.ipv4.diffserv, hdr.ipv4.totalLen, hdr.ipv4.identification, hdr.ipv4.flags, hdr.ipv4.fragOffset, hdr.ipv4.ttl, hdr.ipv4.protocol, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr });
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.nc_hdr);
        pkt.emit(hdr.tcp);
    }
}

control SwitchEgress(inout headers_t hdr, 
    inout meta_t ingress_metadata,
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