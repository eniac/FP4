#include <core.p4>
#include <tna.p4>

header cheetah_t {
    bit<16> flowId;
    bit<32> rowId;
    bit<32> key;
}

struct cheetah_md_t {
    bit<8>  prune;
    bit<8>  sketch_one_same_key;
    bit<8>  sketch_two_same_key;
    bit<8>  sketch_three_same_key;
    bit<8>  sketch_four_same_key;
    bit<8>  sketch_five_same_key;
    bit<32> hash_value;
}
struct headers_t {
    cheetah_t cheetah;
}

parser IngressParserImpl(packet_in pkt, 
    out headers hdr, 
    out cheetah_md_t cheetah_md, 
    out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        transition parse_cheetah;
    }
    state parse_cheetah {
        packet.extract(hdr.cheetah);
        transition accept;
    }   
}

control IngressDeparserImpl(
        packet_out pkt,
        inout headers_t hdr,
        incheetah_md_t cheetah_md,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {
    apply {
        pkt.emit(hdr.cheetah);
    }
}

Register<bit<32>, bit<32>>(32w128) reg_sketch_five_key;
Register<bit<32>, bit<32>>(32w128) reg_sketch_five_value;
Register<bit<32>, bit<32>>(32w128) reg_sketch_four_key;
Register<bit<32>, bit<32>>(32w128) reg_sketch_four_value;
Register<bit<32>, bit<32>>(32w128) reg_sketch_one_key;
Register<bit<32>, bit<32>>(32w128) reg_sketch_one_value;
Register<bit<32>, bit<32>>(32w128) reg_sketch_three_key;
Register<bit<32>, bit<32>>(32w128) reg_sketch_three_value;
Register<bit<32>, bit<32>>(32w128) reg_sketch_two_key;
Register<bit<32>, bit<32>>(32w128) reg_sketch_two_value;


control ingress(inout headers hdr, 
    out cheetah_md_t cheetah_md, 
    in ingress_intrinsic_metadata_t ig_intr_md, 
    in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_five_key) alu_sketch_five_key = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> alu_hi = 32w0;
            bit<32> in_value;
            in_value = value;
            if (hdr.cheetah.key - in_value == 32w0) {
                alu_hi = 32w1;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                alu_hi = 32w0;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                value = hdr.cheetah.key;
            }
            rv = alu_hi;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_five_value) alu_sketch_five_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_four_key) alu_sketch_four_key = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> alu_hi = 32w0;
            bit<32> in_value;
            in_value = value;
            if (hdr.cheetah.key - in_value == 32w0) {
                alu_hi = 32w1;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                alu_hi = 32w0;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                value = hdr.cheetah.key;
            }
            rv = alu_hi;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_four_value) alu_sketch_four_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_one_key) alu_sketch_one_key = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> alu_hi = 32w0;
            bit<32> in_value;
            in_value = value;
            alu_hi = 32w1;
            alu_hi = 32w0;
            value = hdr.cheetah.key;
            rv = alu_hi;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_one_value) alu_sketch_one_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_three_key) alu_sketch_three_key = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> alu_hi = 32w0;
            bit<32> in_value;
            in_value = value;
            if (hdr.cheetah.key - in_value == 32w0) {
                alu_hi = 32w1;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                alu_hi = 32w0;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                value = hdr.cheetah.key;
            }
            rv = alu_hi;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_three_value) alu_sketch_three_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_two_key) alu_sketch_two_key = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> alu_hi = 32w0;
            bit<32> in_value;
            in_value = value;
            if (hdr.cheetah.key - in_value == 32w0) {
                alu_hi = 32w1;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                alu_hi = 32w0;
            }
            if (!(hdr.cheetah.key - in_value == 32w0)) {
                value = hdr.cheetah.key;
            }
            rv = alu_hi;
        }
    };
    RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_two_value) alu_sketch_two_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };

    action table_hash_init_nop(){}
    action table_sketch_one_key_nop(){}
    action table_sketch_one_value_nop() {}
    action table_hash_init_two_nop(){}
    action table_sketch_three_key_nop(){}
    action table_sketch_three_value_nop(){}
    action table_sketch_four_key_nop() {}
    action table_sketch_four_value_nop() {}
    action table_sketch_five_key_nop(){}
    action table_sketch_five_value_nop(){}
    action table_sketch_five_value_nop(){}
    action table_hash_init_three_nop(){}
    action table_hash_init_four_nop(){}
    action table_hash_init_five_nop(){}
    action table_prune_nop(){}

    action modify_hash() {
        meta.cheetah_md.hash_value = hdr.cheetah.key;
    }

    action modify_hash_two() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w3;
    }

    action modify_hash_three() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }

    action modify_hash_four() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }

    action modify_hash_five() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }

    action prune() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }

    action action_sketch_one_key() {
        {
            bit<32> temp_13;
            temp_13 = hash_14.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_one_same_key = (bit<8>)alu_sketch_one_key.execute((bit<32>)temp_13);
        }
    }
    action action action_sketch_one_value() {
        {
            bit<32> temp_14;
            temp_14 = hash_15.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_one_value.execute((bit<32>)temp_14);
        }
    }

    action action_sketch_four_key() {
        {
            bit<17> temp_11;
            temp_11 = hash_12.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_four_same_key = (bit<8>)alu_sketch_four_key.execute((bit<32>)temp_11);
        }
    }

    action action_sketch_three_value() {
        {
            bit<17> temp_16;
            temp_16 = hash_17.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_three_value.execute((bit<32>)temp_16);
        }
    }

    action action action_sketch_three_key() {
        {
            bit<17> temp_15;
            temp_15 = hash_16.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_three_same_key = (bit<8>)alu_sketch_three_key.execute((bit<32>)temp_15);
        }
    }

    action action_sketch_four_value() {
        {
            bit<17> temp_12;
            temp_12 = hash_13.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_four_value.execute((bit<32>)temp_12);
        }
    }

    action action_sketch_five_value() {
        {
            bit<17> temp_10;
            temp_10 = hash_11.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_five_value.execute((bit<32>)temp_10);
        }
    }

    table table_hash_init {
        actions = {
            modify_hash();
            table_hash_init_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_hash_init_nop();
    }

    table table_sketch_one_key {
        actions = {
            action_sketch_one_key();
            table_sketch_one_key_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_one_key_nop();
    }

    table table_sketch_one_value {
        actions = {
            action_sketch_one_value();
            table_sketch_one_value_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_one_value_nop();
    }

    table table_hash_init_two {
        actions = {
            modify_hash_two();
            table_hash_init_two_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_hash_init_two_nop();
    }
    table table_sketch_two_key {
        actions = {
            action_sketch_two_key();
            table_sketch_two_key_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_two_key_nop();
    }

    table table_sketch_three_key {
        actions = {
            action_sketch_three_key();
            table_sketch_three_key_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_three_key_nop();
    }

    table table_sketch_three_value {
        actions = {
            action_sketch_three_value();
            table_sketch_three_value_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_three_value_nop();
    }

    table table_sketch_four_key {
        actions = {
            action_sketch_four_key();
            table_sketch_four_key_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_four_key_nop();
    }

    table table_sketch_four_value {
        actions = {
            action_sketch_four_value();
            table_sketch_four_value_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_four_value_nop();
    }
    table table_sketch_five_key {
        actions = {
            action_sketch_five_key();
            table_sketch_five_key_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_five_key_nop();
    }

    table table_sketch_five_value {
        actions = {
            action_sketch_five_value();
            table_sketch_five_value_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_sketch_five_value_nop();
    }

    table table_hash_init_three {
        actions = {
            modify_hash_three();
            table_hash_init_three_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_hash_init_three_nop();
    }

    table table_hash_init_four {
        actions = {
            modify_hash_four();
            table_hash_init_four_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_hash_init_four_nop();
    }

    table table_hash_init_five {
        actions = {
            modify_hash_five();
            table_hash_init_five_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_hash_init_five_nop();
    }

    table table_prune {
        actions = {
            prune();
            table_prune_nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = table_prune_nop();
    }

    apply {
        table_hash_init.apply();
        table_sketch_one_key.apply();
        table_sketch_one_value.apply();
        table_hash_init_two.apply();
        table_sketch_two_key.apply();
        table_sketch_two_value.apply();
        table_sketch_three_key.apply();
        table_sketch_three_value.apply();
        table_sketch_four_key.apply();
        table_sketch_four_value.apply();
        table_sketch_five_key.apply();
        table_sketch_five_value.apply();
        table_hash_init_three.apply();
        table_hash_init_four.apply();
        table_hash_init_five.apply();
        table_prune.apply();
    }
}


parser EgressParserImpl(packet_in pkt, 
    out headers hdr, 
    out cheetah_md_t cheetah_md, 
    out egress_intrinsic_metadata_t eg_intr_md) {

    state start {
        transition parse_cheetah;
    }
    state parse_cheetah {
        packet.extract(hdr.cheetah);
        transition accept;
    }   
}

control egress(inout headers hdr, 
    inout cheetah_md_t cheetah_md, 
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md
    ) {
    apply {
    }
}

control EgressDeparserImpl(packet_out pkt, 
    inout headers hdr,
    in cheetah_md_t cheetah_md,
    in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr
    ) {
    apply {
        pkt.emit(hdr.cheetah);
    }
}

Pipeline(
    IngressParserImpl(), 
    ingress(), 
    IngressDeparserImpl(), 
    EgressParserImpl(), 
    egress(), 
    EgressDeparserImpl()) pipe;

Switch(pipe) main;