#include <tofino/intrinsic_metadata.p4>
#include <tofino/stateful_alu_blackbox.p4>

parser start {
    extract(pfuzz_visited);
    
    return ingress;

}


control ingress {
  if (pfuzz_visited.pkt_type == 1 || pfuzz_visited.pkt_type == 3) {
    apply(ti_path_assertion);
  } else {
    apply(ti_set_port);
    apply(ti_set_visited_type);
  }
}

header_type pfuzz_visited_t {
    fields {
        preamble : 48;
        pkt_type : 2;
        __pad : 6;
        encoding_i0 : 8;
        temp_port : 16;
    }
}


header pfuzz_visited_t pfuzz_visited;

control egress {
    apply(te_update_count);
}

table ti_path_assertion {
  actions {
    ai_send_to_control_plane;
  }
  default_action : ai_send_to_control_plane();
}

action ai_send_to_control_plane() {
  modify_field(ig_intr_md_for_tm.ucast_egress_port, 192);
  modify_field(pfuzz_visited.preamble, 14593470);
  exit();
}

table ti_set_port {
  actions {
    ai_set_port;
  }
  default_action : ai_set_port();
}

action ai_set_port() {
  modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
  modify_field(pfuzz_visited.temp_port, 0);
}

action ai_set_visited_type() {
    modify_field(pfuzz_visited.pkt_type, 1);
}

table ti_set_visited_type {
    actions { ai_set_visited_type; }
    default_action: ai_set_visited_type();
}

register forward_count_register {
  width: 32;
  instance_count: 1;
}
blackbox stateful_alu forward_count_register_alu {
    reg: forward_count_register;
    output_value: register_lo;
    update_lo_1_value: register_lo + 1;
}
action ae_update_count() {
    forward_count_register_alu.execute_stateful_alu(0);
}
table te_update_count{
  actions {
    ae_update_count;
  }
  default_action: ae_update_count();
  size: 0;
}

