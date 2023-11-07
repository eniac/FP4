error {
    NoError,
    PacketTooShort,
    NoMatch,
    StackOutOfBounds,
    HeaderTooShort,
    ParserTimeout,
    ParserInvalidArgument,
    CounterRange,
    Timeout,
    PhvOwner,
    MultiWrite,
    IbufOverflow,
    IbufUnderflow
}

extern packet_in {
    void extract<T>(out T hdr);
    void extract<T>(out T variableSizeHeader, in bit<32> variableFieldSizeInBits);
    T lookahead<T>();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit<T>(in T hdr);
}

extern void verify(in bool check, in error toSignal);
action NoAction() {
}
match_kind {
    exact,
    ternary,
    lpm
}

typedef bit<9> PortId_t;
typedef bit<16> MulticastGroupId_t;
typedef bit<5> QueueId_t;
typedef bit<10> MirrorId_t;
typedef bit<16> ReplicationId_t;
typedef error ParserError_t;
const bit<32> PORT_METADATA_SIZE = 32w64;
const bit<16> PARSER_ERROR_OK = 16w0x0;
const bit<16> PARSER_ERROR_NO_TCAM = 16w0x1;
const bit<16> PARSER_ERROR_PARTIAL_HDR = 16w0x2;
const bit<16> PARSER_ERROR_CTR_RANGE = 16w0x4;
const bit<16> PARSER_ERROR_TIMEOUT_USER = 16w0x8;
const bit<16> PARSER_ERROR_TIMEOUT_HW = 16w0x10;
const bit<16> PARSER_ERROR_SRC_EXT = 16w0x20;
const bit<16> PARSER_ERROR_DST_CONT = 16w0x40;
const bit<16> PARSER_ERROR_PHV_OWNER = 16w0x80;
const bit<16> PARSER_ERROR_MULTIWRITE = 16w0x100;
const bit<16> PARSER_ERROR_ARAM_MBE = 16w0x400;
const bit<16> PARSER_ERROR_FCS = 16w0x800;
enum MeterType_t {
    PACKETS,
    BYTES
}

enum bit<8> MeterColor_t {
    GREEN = 8w0,
    YELLOW = 8w1,
    RED = 8w3
}

enum CounterType_t {
    PACKETS,
    BYTES,
    PACKETS_AND_BYTES
}

enum SelectorMode_t {
    FAIR,
    RESILIENT
}

enum HashAlgorithm_t {
    IDENTITY,
    RANDOM,
    CRC8,
    CRC16,
    CRC32,
    CRC64,
    CUSTOM
}

match_kind {
    range,
    selector,
    atcam_partition_index
}

@__intrinsic_metadata header ingress_intrinsic_metadata_t {
    bit<1>  resubmit_flag;
    @padding 
    bit<1>  _pad1;
    bit<2>  packet_version;
    @padding 
    bit<3>  _pad2;
    bit<9>  ingress_port;
    bit<48> ingress_mac_tstamp;
}

@__intrinsic_metadata struct ingress_intrinsic_metadata_for_tm_t {
    bit<9>             ucast_egress_port;
    bit<1>             bypass_egress;
    bit<1>             deflect_on_drop;
    bit<3>             ingress_cos;
    bit<5>             qid;
    bit<3>             icos_for_copy_to_cpu;
    bit<1>             copy_to_cpu;
    bit<2>             packet_color;
    bit<1>             disable_ucast_cutthru;
    bit<1>             enable_mcast_cutthru;
    MulticastGroupId_t mcast_grp_a;
    MulticastGroupId_t mcast_grp_b;
    bit<13>            level1_mcast_hash;
    bit<13>            level2_mcast_hash;
    bit<16>            level1_exclusion_id;
    bit<9>             level2_exclusion_id;
    bit<16>            rid;
}

@__intrinsic_metadata struct ingress_intrinsic_metadata_from_parser_t {
    bit<48> global_tstamp;
    bit<32> global_ver;
    bit<16> parser_err;
}

@__intrinsic_metadata struct ingress_intrinsic_metadata_for_deparser_t {
    bit<3> drop_ctl;
    bit<3> digest_type;
    bit<3> resubmit_type;
    bit<3> mirror_type;
}

@__intrinsic_metadata header egress_intrinsic_metadata_t {
    @padding 
    bit<7>  _pad0;
    bit<9>  egress_port;
    @padding 
    bit<5>  _pad1;
    bit<19> enq_qdepth;
    @padding 
    bit<6>  _pad2;
    bit<2>  enq_congest_stat;
    @padding 
    bit<14> _pad3;
    bit<18> enq_tstamp;
    @padding 
    bit<5>  _pad4;
    bit<19> deq_qdepth;
    @padding 
    bit<6>  _pad5;
    bit<2>  deq_congest_stat;
    bit<8>  app_pool_congest_stat;
    @padding 
    bit<14> _pad6;
    bit<18> deq_timedelta;
    bit<16> egress_rid;
    @padding 
    bit<7>  _pad7;
    bit<1>  egress_rid_first;
    @padding 
    bit<3>  _pad8;
    bit<5>  egress_qid;
    @padding 
    bit<5>  _pad9;
    bit<3>  egress_cos;
    @padding 
    bit<7>  _pad10;
    bit<1>  deflection_flag;
    bit<16> pkt_length;
}

@__intrinsic_metadata struct egress_intrinsic_metadata_from_parser_t {
    bit<48> global_tstamp;
    bit<32> global_ver;
    bit<16> parser_err;
}

@__intrinsic_metadata struct egress_intrinsic_metadata_for_deparser_t {
    bit<3> drop_ctl;
    bit<3> mirror_type;
    bit<1> coalesce_flush;
    bit<7> coalesce_length;
}

@__intrinsic_metadata struct egress_intrinsic_metadata_for_output_port_t {
    bit<1> capture_tstamp_on_tx;
    bit<1> update_delay_on_tx;
    bit<1> force_tx_error;
}

header pktgen_timer_header_t {
    @padding 
    bit<3>  _pad1;
    bit<2>  pipe_id;
    bit<3>  app_id;
    @padding 
    bit<8>  _pad2;
    bit<16> batch_id;
    bit<16> packet_id;
}

header pktgen_port_down_header_t {
    @padding 
    bit<3>  _pad1;
    bit<2>  pipe_id;
    bit<3>  app_id;
    @padding 
    bit<15> _pad2;
    bit<9>  port_num;
    bit<16> packet_id;
}

header pktgen_recirc_header_t {
    @padding 
    bit<3>  _pad1;
    bit<2>  pipe_id;
    bit<3>  app_id;
    bit<24> key;
    bit<16> packet_id;
}

header ptp_metadata_t {
    bit<8>  udp_cksum_byte_offset;
    bit<8>  cf_byte_offset;
    bit<48> updated_cf;
}

extern Checksum {
    Checksum();
    void add<T>(in T data);
    void subtract<T>(in T data);
    bool verify();
    bit<16> get();
    bit<16> update<T>(in T data, @optional in bool zeros_as_ones);
}

extern ParserCounter {
    ParserCounter();
    void set<T>(in T value);
    void set<T>(in T field, in bit<8> max, in bit<8> rotate, in bit<3> mask, in bit<8> add);
    bool is_zero();
    bool is_negative();
    void increment(in bit<8> value);
    void decrement(in bit<8> value);
}

extern ParserPriority {
    ParserPriority();
    void set(in bit<3> prio);
}

extern CRCPolynomial<T> {
    CRCPolynomial(T coeff, bool reversed, bool msb, bool extended, T init, T xor);
}

extern Hash<W> {
    Hash(HashAlgorithm_t algo);
    Hash(HashAlgorithm_t algo, CRCPolynomial<_> poly);
    W get<D>(in D data);
}

extern Random<W> {
    Random();
    W get();
}

extern T max<T>(in T t1, in T t2);
extern T min<T>(in T t1, in T t2);
extern void invalidate<T>(in T field);
extern T port_metadata_unpack<T>(packet_in pkt);
extern bit<32> sizeInBits<H>(in H h);
extern bit<32> sizeInBytes<H>(in H h);
extern Counter<W, I> {
    Counter(bit<32> size, CounterType_t type);
    void count(in I index);
}

extern DirectCounter<W> {
    DirectCounter(CounterType_t type);
    void count();
}

extern Meter<I> {
    Meter(bit<32> size, MeterType_t type);
    Meter(bit<32> size, MeterType_t type, bit<8> red, bit<8> yellow, bit<8> green);
    bit<8> execute(in I index, in MeterColor_t color);
    bit<8> execute(in I index);
}

extern DirectMeter {
    DirectMeter(MeterType_t type);
    DirectMeter(MeterType_t type, bit<8> red, bit<8> yellow, bit<8> green);
    bit<8> execute(in MeterColor_t color);
    bit<8> execute();
}

extern Lpf<T, I> {
    Lpf(bit<32> size);
    T execute(in T val, in I index);
}

extern DirectLpf<T> {
    DirectLpf();
    T execute(in T val);
}

extern Wred<T, I> {
    Wred(bit<32> size, bit<8> drop_value, bit<8> no_drop_value);
    bit<8> execute(in T val, in I index);
}

extern DirectWred<T> {
    DirectWred(bit<8> drop_value, bit<8> no_drop_value);
    bit<8> execute(in T val);
}

extern Register<T, I> {
    Register(bit<32> size);
    Register(bit<32> size, T initial_value);
    T read(in I index);
    void write(in I index, in T value);
}

extern DirectRegister<T> {
    DirectRegister();
    DirectRegister(T initial_value);
    T read();
    void write(in T value);
}

extern RegisterParam<T> {
    RegisterParam(T initial_value);
    T read();
}

enum MathOp_t {
    MUL,
    SQR,
    SQRT,
    DIV,
    RSQR,
    RSQRT
}

extern MathUnit<T> {
    MathUnit(bool invert, int<2> shift, int<6> scale, tuple<bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>, bit<8>> data);
    MathUnit(MathOp_t op, int factor);
    MathUnit(MathOp_t op, int A, int B);
    T execute(in T x);
}

extern RegisterAction<T, I, U> {
    RegisterAction(Register<T, I> reg);
    U execute(in I index);
    U execute_log();
    @synchronous(execute, execute_log) abstract void apply(inout T value, @optional out U rv);
    U predicate(@optional in bool cmplo, @optional in bool cmphi);
}

extern DirectRegisterAction<T, U> {
    DirectRegisterAction(DirectRegister<T> reg);
    U execute();
    @synchronous(execute) abstract void apply(inout T value, @optional out U rv);
    U predicate(@optional in bool cmplo, @optional in bool cmphi);
}

extern ActionProfile {
    ActionProfile(bit<32> size);
}

extern ActionSelector {
    ActionSelector(ActionProfile action_profile, Hash<_> hash, SelectorMode_t mode, bit<32> max_group_size, bit<32> num_groups);
    ActionSelector(ActionProfile action_profile, Hash<_> hash, SelectorMode_t mode, Register<bit<1>, _> reg, bit<32> max_group_size, bit<32> num_groups);
    @deprecated("ActionSelector must be specified with an associated ActionProfile") ActionSelector(bit<32> size, Hash<_> hash, SelectorMode_t mode);
    @deprecated("ActionSelector must be specified with an associated ActionProfile") ActionSelector(bit<32> size, Hash<_> hash, SelectorMode_t mode, Register<bit<1>, _> reg);
}

extern SelectorAction {
    SelectorAction(ActionSelector sel);
    bit<1> execute(@optional in bit<32> index);
    @synchronous(execute) abstract void apply(inout bit<1> value, @optional out bit<1> rv);
}

extern Mirror {
    Mirror();
    void emit(in MirrorId_t session_id);
    void emit<T>(in MirrorId_t session_id, in T hdr);
}

extern Resubmit {
    Resubmit();
    void emit();
    void emit<T>(in T hdr);
}

extern Digest<T> {
    Digest();
    void pack(in T data);
}

extern Atcam {
    Atcam(@optional bit<32> number_partitions);
}

extern Alpm {
    Alpm(@optional bit<32> number_partitions, @optional bit<32> subtrees_per_partition);
}

parser IngressParserT<H, M>(packet_in pkt, out H hdr, out M ig_md, @optional out ingress_intrinsic_metadata_t ig_intr_md, @optional out ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm, @optional out ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr);
parser EgressParserT<H, M>(packet_in pkt, out H hdr, out M eg_md, @optional out egress_intrinsic_metadata_t eg_intr_md, @optional out egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr);
control IngressT<H, M>(inout H hdr, inout M ig_md, @optional in ingress_intrinsic_metadata_t ig_intr_md, @optional in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr, @optional inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr, @optional inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm);
control EgressT<H, M>(inout H hdr, inout M eg_md, @optional in egress_intrinsic_metadata_t eg_intr_md, @optional in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr, @optional inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr, @optional inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport);
control IngressDeparserT<H, M>(packet_out pkt, inout H hdr, in M metadata, @optional in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr, @optional in ingress_intrinsic_metadata_t ig_intr_md);
control EgressDeparserT<H, M>(packet_out pkt, inout H hdr, in M metadata, @optional in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr, @optional in egress_intrinsic_metadata_t eg_intr_md, @optional in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr);
package Pipeline<IH, IM, EH, EM>(IngressParserT<IH, IM> ingress_parser, IngressT<IH, IM> ingress, IngressDeparserT<IH, IM> ingress_deparser, EgressParserT<EH, EM> egress_parser, EgressT<EH, EM> egress, EgressDeparserT<EH, EM> egress_deparser);
@pkginfo(arch="TNA", version="1.0.1") package Switch<IH0, IM0, EH0, EM0, IH1, IM1, EH1, EM1, IH2, IM2, EH2, EM2, IH3, IM3, EH3, EM3>(Pipeline<IH0, IM0, EH0, EM0> pipe0, @optional Pipeline<IH1, IM1, EH1, EM1> pipe1, @optional Pipeline<IH2, IM2, EH2, EM2> pipe2, @optional Pipeline<IH3, IM3, EH3, EM3> pipe3);
package IngressParsers<H, M>(IngressParserT<H, M> prsr0, @optional IngressParserT<H, M> prsr1, @optional IngressParserT<H, M> prsr2, @optional IngressParserT<H, M> prsr3, @optional IngressParserT<H, M> prsr4, @optional IngressParserT<H, M> prsr5, @optional IngressParserT<H, M> prsr6, @optional IngressParserT<H, M> prsr7, @optional IngressParserT<H, M> prsr8, @optional IngressParserT<H, M> prsr9, @optional IngressParserT<H, M> prsr10, @optional IngressParserT<H, M> prsr11, @optional IngressParserT<H, M> prsr12, @optional IngressParserT<H, M> prsr13, @optional IngressParserT<H, M> prsr14, @optional IngressParserT<H, M> prsr15, @optional IngressParserT<H, M> prsr16, @optional IngressParserT<H, M> prsr17);
package EgressParsers<H, M>(EgressParserT<H, M> prsr0, @optional EgressParserT<H, M> prsr1, @optional EgressParserT<H, M> prsr2, @optional EgressParserT<H, M> prsr3, @optional EgressParserT<H, M> prsr4, @optional EgressParserT<H, M> prsr5, @optional EgressParserT<H, M> prsr6, @optional EgressParserT<H, M> prsr7, @optional EgressParserT<H, M> prsr8, @optional EgressParserT<H, M> prsr9, @optional EgressParserT<H, M> prsr10, @optional EgressParserT<H, M> prsr11, @optional EgressParserT<H, M> prsr12, @optional EgressParserT<H, M> prsr13, @optional EgressParserT<H, M> prsr14, @optional EgressParserT<H, M> prsr15, @optional EgressParserT<H, M> prsr16, @optional EgressParserT<H, M> prsr17);
package MultiParserPipeline<IH, IM, EH, EM>(IngressParsers<IH, IM> ig_prsr, IngressT<IH, IM> ingress, IngressDeparserT<IH, IM> ingress_deparser, EgressParsers<EH, EM> eg_prsr, EgressT<EH, EM> egress, EgressDeparserT<EH, EM> egress_deparser);
package MultiParserSwitch<IH0, IM0, EH0, EM0, IH1, IM1, EH1, EM1, IH2, IM2, EH2, EM2, IH3, IM3, EH3, EM3>(MultiParserPipeline<IH0, IM0, EH0, EM0> pipe0, @optional MultiParserPipeline<IH1, IM1, EH1, EM1> pipe1, @optional MultiParserPipeline<IH2, IM2, EH2, EM2> pipe2, @optional MultiParserPipeline<IH3, IM3, EH3, EM3> pipe3);
struct compiler_generated_metadata_t {
    bit<10> mirror_id;
    bit<8>  mirror_source;
    bit<8>  resubmit_source;
    bit<4>  clone_src;
    bit<4>  clone_digest_id;
    bit<32> instance_type;
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

header cheetah_t {
    bit<16> flowId;
    bit<32> rowId;
    bit<32> key;
}

@name("generator_metadata_t") header generator_metadata_t_0 {
    bit<16> app_id;
    bit<16> batch_id;
    bit<16> instance_id;
}

struct metadata {
    @name(".__bfp4c_compiler_generated_meta") 
    compiler_generated_metadata_t               __bfp4c_compiler_generated_meta;
    @name(".cheetah_md") 
    cheetah_md_t                                cheetah_md;
    @name(".eg_intr_md") 
    egress_intrinsic_metadata_t                 eg_intr_md;
    @name(".eg_intr_md_for_dprsr") 
    egress_intrinsic_metadata_for_deparser_t    eg_intr_md_for_dprsr;
    @name(".eg_intr_md_for_oport") 
    egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport;
    @name(".eg_intr_md_from_parser_aux") 
    egress_intrinsic_metadata_from_parser_t     eg_intr_md_from_parser_aux;
    @name(".ig_intr_md") 
    ingress_intrinsic_metadata_t                ig_intr_md;
    @name(".ig_intr_md_for_tm") 
    ingress_intrinsic_metadata_for_tm_t         ig_intr_md_for_tm;
    @name(".ig_intr_md_from_parser_aux") 
    ingress_intrinsic_metadata_from_parser_t    ig_intr_md_from_parser_aux;
    @name(".standard_metadata") 
    standard_metadata_t                         standard_metadata;
}

struct headers {
    @name(".cheetah") 
    cheetah_t cheetah;
}

parser IngressParserImpl(packet_in pkt, out headers hdr, out metadata meta, out ingress_intrinsic_metadata_t ig_intr_md, out ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm, out ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr) {
    @name("parse_cheetah") state parse_cheetah {
        pkt.extract(hdr.cheetah);
        transition accept;
    }
    @name("start") state __ingress_p4_entry_point {
        transition parse_cheetah;
    }
    @name("$skip_to_packet") state __skip_to_packet {
        pkt.advance(32w0);
        transition __ingress_p4_entry_point;
    }
    @name("$phase0") state __phase0 {
        pkt.advance(32w64);
        transition __skip_to_packet;
    }
    @name("$resubmit") state __resubmit {
        transition __ingress_p4_entry_point;
    }
    @name("$check_resubmit") state __check_resubmit {
        transition select(ig_intr_md.resubmit_flag) {
            1w0 &&& 1w1: __phase0;
            1w1 &&& 1w1: __resubmit;
        }
    }
    @name("$ingress_metadata") state __ingress_metadata {
        pkt.extract(ig_intr_md);
        transition __check_resubmit;
    }
    @name("$ingress_tna_entry_point") state start {
        transition __ingress_metadata;
    }
}

parser EgressParserImpl(packet_in pkt, out headers hdr, out metadata meta, out egress_intrinsic_metadata_t eg_intr_md, out egress_intrinsic_metadata_from_parser_t eg_intr_md_from_parser_aux) {
    @name("parse_cheetah") state parse_cheetah {
        pkt.extract(hdr.cheetah);
        transition accept;
    }
    @name("start") state __egress_p4_entry_point {
        transition parse_cheetah;
    }
    @name("$bridged_metadata") state __bridged_metadata {
        transition __egress_p4_entry_point;
    }
    @name("$mirrored") state __mirrored {
        transition __egress_p4_entry_point;
    }
    @name("$check_mirrored") state __check_mirrored {
        transition select(pkt.lookahead<bit<8>>()) {
            8w0 &&& 8w8: __bridged_metadata;
            8w8 &&& 8w8: __mirrored;
        }
    }
    @name("$egress_metadata") state __egress_metadata {
        pkt.extract(eg_intr_md);
        transition __check_mirrored;
    }
    @name("$egress_tna_entry_point") state start {
        transition __egress_metadata;
    }
}

@name(".reg_sketch_five_key") Register<bit<32>, bit<32>>(32w128) reg_sketch_five_key;

@name(".reg_sketch_five_value") Register<bit<32>, bit<32>>(32w128) reg_sketch_five_value;

@name(".reg_sketch_four_key") Register<bit<32>, bit<32>>(32w128) reg_sketch_four_key;

@name(".reg_sketch_four_value") Register<bit<32>, bit<32>>(32w128) reg_sketch_four_value;

@name(".reg_sketch_one_key") Register<bit<32>, bit<32>>(32w128) reg_sketch_one_key;

@name(".reg_sketch_one_value") Register<bit<32>, bit<32>>(32w128) reg_sketch_one_value;

@name(".reg_sketch_three_key") Register<bit<32>, bit<32>>(32w128) reg_sketch_three_key;

@name(".reg_sketch_three_value") Register<bit<32>, bit<32>>(32w128) reg_sketch_three_value;

@name(".reg_sketch_two_key") Register<bit<32>, bit<32>>(32w128) reg_sketch_two_key;

@name(".reg_sketch_two_value") Register<bit<32>, bit<32>>(32w128) reg_sketch_two_value;

control ingress(inout headers hdr, inout metadata meta, in ingress_intrinsic_metadata_t ig_intr_md, in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_parser_aux, inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr, inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm) {
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_10;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_11;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_12;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_13;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_14;
    Hash<bit<32>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_15;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_16;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_17;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_18;
    Hash<bit<17>>(HashAlgorithm_t.CUSTOM, CRCPolynomial<bit<32>>(32w79764919, true, false, false, 32w4294967295, 32w4294967295)) hash_19;
    @name(".alu_sketch_five_key") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_five_key) alu_sketch_five_key = {
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
    @name(".alu_sketch_five_value") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_five_value) alu_sketch_five_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    @name(".alu_sketch_four_key") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_four_key) alu_sketch_four_key = {
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
    @name(".alu_sketch_four_value") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_four_value) alu_sketch_four_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    @name(".alu_sketch_one_key") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_one_key) alu_sketch_one_key = {
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
    @name(".alu_sketch_one_value") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_one_value) alu_sketch_one_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    @name(".alu_sketch_three_key") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_three_key) alu_sketch_three_key = {
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
    @name(".alu_sketch_three_value") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_three_value) alu_sketch_three_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    @name(".alu_sketch_two_key") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_two_key) alu_sketch_two_key = {
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
    @name(".alu_sketch_two_value") RegisterAction<bit<32>, bit<32>, bit<32>>(reg_sketch_two_value) alu_sketch_two_value = {
        void apply(inout bit<32> value, out bit<32> rv) {
            rv = 32w0;
            bit<32> in_value;
            in_value = value;
            rv = in_value;
            value = 32w1;
        }
    };
    @name(".modify_hash") action modify_hash() {
        meta.cheetah_md.hash_value = hdr.cheetah.key;
    }
    @name(".nop") action nop() {
    }
    @name(".modify_hash_five") action modify_hash_five() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }
    @name(".modify_hash_four") action modify_hash_four() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }
    @name(".modify_hash_three") action modify_hash_three() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w2;
    }
    @name(".modify_hash_two") action modify_hash_two() {
        meta.cheetah_md.hash_value = meta.cheetah_md.hash_value + 32w3;
    }
    @name(".prune") action prune() {
        ig_intr_md_for_dprsr.drop_ctl = 3w1;
    }
    @name(".action_sketch_five_key") action action_sketch_five_key() {
        {
            bit<17> temp_9;
            temp_9 = hash_10.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_five_same_key = (bit<8>)alu_sketch_five_key.execute((bit<32>)temp_9);
        }
    }
    @name(".action_sketch_five_value") action action_sketch_five_value() {
        {
            bit<17> temp_10;
            temp_10 = hash_11.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_five_value.execute((bit<32>)temp_10);
        }
    }
    @name(".action_sketch_four_key") action action_sketch_four_key() {
        {
            bit<17> temp_11;
            temp_11 = hash_12.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_four_same_key = (bit<8>)alu_sketch_four_key.execute((bit<32>)temp_11);
        }
    }
    @name(".action_sketch_four_value") action action_sketch_four_value() {
        {
            bit<17> temp_12;
            temp_12 = hash_13.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_four_value.execute((bit<32>)temp_12);
        }
    }
    @name(".action_sketch_one_key") action action_sketch_one_key() {
        {
            bit<32> temp_13;
            temp_13 = hash_14.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_one_same_key = (bit<8>)alu_sketch_one_key.execute((bit<32>)temp_13);
        }
    }
    @name(".action_sketch_one_value") action action_sketch_one_value() {
        {
            bit<32> temp_14;
            temp_14 = hash_15.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_one_value.execute((bit<32>)temp_14);
        }
    }
    @name(".action_sketch_three_key") action action_sketch_three_key() {
        {
            bit<17> temp_15;
            temp_15 = hash_16.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_three_same_key = (bit<8>)alu_sketch_three_key.execute((bit<32>)temp_15);
        }
    }
    @name(".action_sketch_three_value") action action_sketch_three_value() {
        {
            bit<17> temp_16;
            temp_16 = hash_17.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_three_value.execute((bit<32>)temp_16);
        }
    }
    @name(".action_sketch_two_key") action action_sketch_two_key() {
        {
            bit<17> temp_17;
            temp_17 = hash_18.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.sketch_two_same_key = (bit<8>)alu_sketch_two_key.execute((bit<32>)temp_17);
        }
    }
    @name(".action_sketch_two_value") action action_sketch_two_value() {
        {
            bit<17> temp_18;
            temp_18 = hash_19.get({ meta.cheetah_md.hash_value });
            meta.cheetah_md.prune = (bit<8>)alu_sketch_two_value.execute((bit<32>)temp_18);
        }
    }
    @name(".table_hash_init") table table_hash_init {
        actions = {
            modify_hash();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_hash_init_five") table table_hash_init_five {
        actions = {
            modify_hash_five();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_hash_init_four") table table_hash_init_four {
        actions = {
            modify_hash_four();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_hash_init_three") table table_hash_init_three {
        actions = {
            modify_hash_three();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_hash_init_two") table table_hash_init_two {
        actions = {
            modify_hash_two();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_prune") table table_prune {
        actions = {
            prune();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_five_key") table table_sketch_five_key {
        actions = {
            action_sketch_five_key();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_five_value") table table_sketch_five_value {
        actions = {
            action_sketch_five_value();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_four_key") table table_sketch_four_key {
        actions = {
            action_sketch_four_key();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_four_value") table table_sketch_four_value {
        actions = {
            action_sketch_four_value();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_one_key") table table_sketch_one_key {
        actions = {
            action_sketch_one_key();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_one_value") table table_sketch_one_value {
        actions = {
            action_sketch_one_value();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_three_key") table table_sketch_three_key {
        actions = {
            action_sketch_three_key();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_three_value") table table_sketch_three_value {
        actions = {
            action_sketch_three_value();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_two_key") table table_sketch_two_key {
        actions = {
            action_sketch_two_key();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
    }
    @name(".table_sketch_two_value") table table_sketch_two_value {
        actions = {
            action_sketch_two_value();
            nop();
        }
        key = {
            hdr.cheetah.flowId: exact;
        }
        default_action = nop();
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

control egress(inout headers hdr, inout metadata meta, in egress_intrinsic_metadata_t eg_intr_md, in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_parser_aux, inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr, inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport) {
    apply {
    }
}

control IngressDeparserImpl(packet_out pkt, inout headers hdr, in metadata meta, in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr, in ingress_intrinsic_metadata_t ig_intr_md) {
    apply {
        pkt.emit(hdr.cheetah);
    }
}

control EgressDeparserImpl(packet_out pkt, inout headers hdr, in metadata meta, in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr, in egress_intrinsic_metadata_t eg_intr_md, in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_parser_aux) {
    apply {
        pkt.emit(hdr.cheetah);
    }
}

Pipeline(IngressParserImpl(), ingress(), IngressDeparserImpl(), EgressParserImpl(), egress(), EgressDeparserImpl()) pipe;

@pa_auto_init_metadata Switch(pipe) main;

