blackbox stateful_alu acquire_lock_alu {
    reg: lock_status_register;

    update_lo_1_value: 1;

    output_value: register_lo;
    output_dst  : meta.available;
}

blackbox stateful_alu release_lock_alu {
    reg: lock_status_register;

    update_lo_1_value: 0;
}