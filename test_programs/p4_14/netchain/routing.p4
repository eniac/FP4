action set_egress(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
    add_to_field(ipv4.ttl, -1);
}

table ipv4_route {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        set_egress;
        drop_action;
    }
    default_action:drop_action;
    size : 256;
}

action drop_action() {
    drop();
}
