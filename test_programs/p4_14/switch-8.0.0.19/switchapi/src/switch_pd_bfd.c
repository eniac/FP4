/*
 * Copyright 2016-present Barefoot Networks, Inc.
 */
#include "switchapi/switch_bfd.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

static int32_t switch_pd_bfd_session_count[SWITCH_MAX_DEVICE][SWITCH_MAX_PIPES];

// create a bfd pkt with some extra info in pktgen buffer
void switch_pd_bfd_pktgen_pkt_init(uint8_t *buffer) {
  switch_pktgen_ext_header_t *ext_header;
  ipv4_header_t *ipv4_header;
  udp_header_t *udp_header;
  bfd_header_t *bfd_header;

  ext_header = (switch_pktgen_ext_header_t *)buffer;
  memset(ext_header, 0, sizeof(switch_pktgen_ext_header_t));
  ext_header->ether_type = htons(ETHERTYPE_BF_PKTGEN);

  ipv4_header = (ipv4_header_t *)(ext_header + 1);
  memset(ipv4_header, 0, sizeof(ipv4_header_t));
  ipv4_header->ver_ihl = 0x45;
  ipv4_header->diffserv = 48;  // DSCP/CS6
  ipv4_header->total_len = htons(
      (sizeof(ipv4_header_t) + sizeof(udp_header_t) + sizeof(bfd_header_t)));
  ipv4_header->ipid = htons(1);
  ipv4_header->flags_offset = 0;
  ipv4_header->ttl = 255;
  ipv4_header->proto = IP_PROTOCOLS_UDP;
  ipv4_header->hdr_chksum = 0;
  ipv4_header->sip = 0;  // programmed by the hardware, based on session_id
  ipv4_header->dip = 0;  // programmed by the hardware, based on session_id

  udp_header = (udp_header_t *)(ipv4_header + 1);
  udp_header->sport = 0;
  ;
  udp_header->dport = 0;
  ;
  udp_header->len = htons(sizeof(bfd_header_t) + sizeof(udp_header_t));
  udp_header->chksum = 0;

  bfd_header = (bfd_header_t *)(udp_header + 1);
  bfd_header->ver_diag = (1 << 5) | 0x00;     // ver(3), diag(5)
  bfd_header->state_flags = (3 << 6) | 0x00;  // state(3 = up), flags(6)
  bfd_header->detect_mult = 0;
  bfd_header->len = 24;
  bfd_header->my_disc = 0;
  bfd_header->your_disc = 0;
  bfd_header->desired_tx_interval = 0;
  bfd_header->required_rx_interval = 0;
  bfd_header->required_echo_rx_interval = 0;
}

switch_status_t switch_pd_bfd_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef P4_BFD_OFFLOAD_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_entry_hdl_t entry_hdl;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  uint8_t *buffer = NULL;
  int p;
  p4_pd_tbl_prop_value_t prop_val;
  p4_pd_tbl_prop_args_t prop_arg;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  struct p4_pd_pktgen_app_cfg bfd_app_config;
  int bfd_pkt_len = sizeof(switch_pktgen_ext_header_t) + sizeof(ipv4_header_t) +
                    sizeof(udp_header_t) + sizeof(bfd_header_t);
  uint16_t pkt_offset =
      switch_pd_pktgen_app_buffer_offset(p4_pd_device, P4_PKTGEN_APP_BFD);
  // book-keeping of sessions per pipe
  for (p = 0; p < max_pipes; p++) {
    switch_pd_bfd_session_count[device][p] = 0;
  }
  // init bfd_app_config
  bfd_app_config.trigger_type = PD_PKTGEN_TRIGGER_TIMER_PERIODIC;
  bfd_app_config.batch_count = 0;  // zero based
  bfd_app_config.packets_per_batch = (SWITCH_MAX_BFD_SESSIONS / max_pipes) - 1;
  bfd_app_config.pattern_value = 0;
  bfd_app_config.pattern_mask = 0;
  bfd_app_config.timer_nanosec = (SWITCH_PKTGEN_BFD_TIMER_USEC * 1000);
  bfd_app_config.ibg = 1000;
  bfd_app_config.ibg_jitter = 100;
  bfd_app_config.ipg = 10;  // tweak it if needed
  bfd_app_config.ipg_jitter = 10;
  bfd_app_config.source_port = 0;
  bfd_app_config.increment_source_port = 0;
  bfd_app_config.pkt_buffer_offset = pkt_offset;
  bfd_app_config.length = bfd_pkt_len;

  buffer = SWITCH_MALLOC(device, bfd_pkt_len, 1);

  p4_pd_pktgen_cfg_app(
      switch_cfg_sess_hdl, p4_pd_device, P4_PKTGEN_APP_BFD, bfd_app_config);
  // setup ipv4 pkt buffer for bfd
  // XXX - ipv6 not done, use another app_id for ipv6 later
  switch_pd_bfd_pktgen_pkt_init(buffer);
  p4_pd_pktgen_write_pkt_buffer(
      switch_cfg_sess_hdl, p4_pd_device, pkt_offset, bfd_pkt_len, buffer);

  // setup default tables in the pipeline
  p4_pd_dc_bfd_tx_timer_set_default_action_bfd_drop_pkt(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  p4_pd_dc_bfd_recirc_egress_set_default_action_bfd_recirc_skip_egress(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  p4_pd_dc_bfd_tx_timer_action_set_default_action_bfd_tx_egress_drop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  p4_pd_dc_bfd_fix_pkt_hdrs_set_default_action_bfd_tx_to_cpu(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  {
    // Rx pkt actions

    // bfd_pkt_action : default action = bfd_drop_pkt (all pipes)
    //  for each pipe :
    //      setup all actions as per the decision table (pipe_id)

    p4_pd_dc_bfd_pkt_action_match_spec_t match_spec;
    p4_pd_dc_bfd_pkt_to_cpu_action_spec_t action_pkt_to_cpu;

    prop_val.scope = PD_ENTRY_SCOPE_SINGLE_PIPELINE;
    prop_arg.value = 0;
    // set up assmetric table - per pipe programming
    p4_pd_dc_bfd_pkt_action_set_property(
        switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);

    // once mode is set to assymmetric, all entries must be programmed
    // per pipe
    for (p = 0; p < max_pipes; p++) {
      // Per pipe programming for the rx pkt
      // set the pipe_id
      p4_pd_device.dev_pipe_id = p;

      p4_pd_dc_bfd_pkt_action_set_default_action_bfd_drop_pkt(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

      match_spec.bfd_meta_pkt_tx = 0;
      match_spec.bfd_meta_pkt_action = 0;
      match_spec.bfd_meta_pktgen_pipe = p + 1; /* this pipe */
      match_spec.bfd_meta_pktgen_pipe_mask = ~0;
      p4_pd_dc_bfd_pkt_action_table_add_with_bfd_drop_pkt(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 0, &entry_hdl);

      match_spec.bfd_meta_pkt_tx = 0;
      match_spec.bfd_meta_pkt_action = 0;
      match_spec.bfd_meta_pktgen_pipe = 0;
      match_spec.bfd_meta_pktgen_pipe_mask = 0;  // masked, !this_pipe

      p4_pd_dc_bfd_pkt_action_table_add_with_bfd_recirc_to_pktgen_pipe(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 10, &entry_hdl);

      match_spec.bfd_meta_pkt_tx = 0;
      match_spec.bfd_meta_pkt_action = 1;
      match_spec.bfd_meta_pktgen_pipe = 0;
      match_spec.bfd_meta_pktgen_pipe_mask = 0;  // masked, !this_pipe

      p4_pd_dc_bfd_pkt_action_table_add_with_nop(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 20, &entry_hdl);

      // Tx pkt actions
      match_spec.bfd_meta_pkt_tx = 1;
      match_spec.bfd_meta_pkt_action = 0;
      match_spec.bfd_meta_pktgen_pipe = 0;
      match_spec.bfd_meta_pktgen_pipe_mask = 0;

      p4_pd_dc_bfd_pkt_action_table_add_with_bfd_tx_pkt(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 30, &entry_hdl);

      match_spec.bfd_meta_pkt_tx = 1;
      match_spec.bfd_meta_pkt_action = 1;
      match_spec.bfd_meta_pktgen_pipe = 0;
      match_spec.bfd_meta_pktgen_pipe_mask = 0;

      action_pkt_to_cpu.action_cpu_mirror_id = SWITCH_CPU_MIRROR_SESSION_ID;
      action_pkt_to_cpu.action_reason_code =
          SWITCH_HOSTIF_REASON_CODE_BFD_EVENT;

      p4_pd_dc_bfd_pkt_action_table_add_with_bfd_pkt_to_cpu(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            40,
                                                            &action_pkt_to_cpu,
                                                            &entry_hdl);
    }
  }

  // set up assmetric table - per pipe programming
  p4_pd_dc_bfd_rx_timers_set_property(
      switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);

  p4_pd_dc_bfd_tx_session_set_property(
      switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);

  for (p = 0; p < max_pipes; p++) {
    p4_pd_device.dev_pipe_id = p;
    // bfd_rx_timers: default action = nop
    p4_pd_dc_bfd_rx_timers_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

    // bfd_tx_session: default action = bfd_drop_pkt (all pipes)
    p4_pd_dc_bfd_tx_session_set_default_action_bfd_tx_drop_pkt(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  }
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  // bfd_rx_session: default action = bfd_session_miss
  p4_pd_dc_bfd_rx_session_set_default_action_bfd_session_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif
  return SWITCH_STATUS_SUCCESS;
}

int32_t switch_pd_bfd_num_sessions_get(switch_pd_target_t pd_dev) {
  return switch_pd_bfd_session_count[pd_dev.device_id][pd_dev.dev_pipe_id];
}

int32_t switch_pd_bfd_session_increment(switch_pd_target_t pd_dev) {
  return ++switch_pd_bfd_session_count[pd_dev.device_id][pd_dev.dev_pipe_id];
}

int32_t switch_pd_bfd_session_decrement(switch_pd_target_t pd_dev) {
  return --switch_pd_bfd_session_count[pd_dev.device_id][pd_dev.dev_pipe_id];
}

switch_status_t switch_pd_bfd_session_update(switch_device_t device,
                                             switch_bfd_info_t *bfd_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
#ifdef P4_BFD_OFFLOAD_ENABLE
  p4_pd_dev_target_t p4_pd_device;

  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  int pktgen_pipe = bfd_info->session_id % max_pipes;
  int local_session_id = bfd_info->session_id / max_pipes;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  // check the contraint
  assert(((SWITCH_MAX_BFD_SESSIONS / max_pipes) * max_pipes) ==
         SWITCH_MAX_BFD_SESSIONS);

  // program tables in the reverse order (of executaion)
  {
    // bfd_tx_timer : (all pipes)
    //  key{pkt_tx=0, action=01, session_id} => bfd_tx_timer_check
    p4_pd_dc_bfd_tx_timer_match_spec_t match_spec;
    p4_pd_dc_bfd_tx_timer_check_action_spec_t action;

    match_spec.bfd_meta_pkt_tx = 1;
    match_spec.bfd_meta_pkt_action = 0;
    match_spec.bfd_meta_session_id = bfd_info->session_id;
    match_spec.bfd_meta_session_id_mask = ~0;

    action.action_session_id = bfd_info->session_id;
    action.action_myDisc = bfd_info->api_info.my_disc;
    action.action_yourDisc = bfd_info->api_info.your_disc;
    action.action_minTx = bfd_info->api_info.desired_tx_interval;
    action.action_minRx = bfd_info->api_info.min_rx_interval;
    action.action_detectMult = bfd_info->api_info.detect_mult;

    status = p4_pd_dc_bfd_tx_timer_table_add_with_bfd_tx_timer_check(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        0,
        &action,
        &bfd_info->tx_timer_table_entry_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }
  }
  {
    // init stateful register with rxMultiplier
    uint8_t rx_timer_val = bfd_info->rx_mult;
    p4_pd_device.dev_pipe_id = pktgen_pipe;
    status = p4_pd_dc_register_write_bfd_rx_session_timer(
        switch_cfg_sess_hdl, p4_pd_device, local_session_id, &rx_timer_val);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }
  }
  {
    // bfd_rx_timers: 2 entries per session only on pktgen_pipe
    //  key{offload = 1pkt_tx=0, session_id} =>
    //      rx_timers_reset(local_session_id)
    //  key{offload = 1pkt_tx=1, session_id} =>
    //      rx_timers_check(local_session_id, sip, dip, sport, dport)
    p4_pd_dc_bfd_rx_timers_match_spec_t match_spec;
    p4_pd_dc_bfd_rx_timer_reset_action_spec_t action;
    p4_pd_dc_bfd_rx_timer_check_action_spec_t action2;

    match_spec.bfd_meta_session_offload = 1;
    match_spec.bfd_meta_pkt_tx = 0;
    match_spec.bfd_meta_session_id = bfd_info->session_id;

    action.action_local_session_id = local_session_id;

    p4_pd_device.dev_pipe_id = pktgen_pipe;
    status = p4_pd_dc_bfd_rx_timers_table_add_with_bfd_rx_timer_reset(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action,
        &bfd_info->rx_timers_table_reset_entry_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }

    match_spec.bfd_meta_session_offload = 1;
    match_spec.bfd_meta_pkt_tx = 1;
    match_spec.bfd_meta_session_id = bfd_info->session_id;

    action2.action_local_session_id = local_session_id;
    action2.action_sip = bfd_info->api_info.sip.ip.v4addr;
    action2.action_dip = bfd_info->api_info.dip.ip.v4addr;
    action2.action_sport = bfd_info->api_info.sport;
    action2.action_dport = bfd_info->api_info.dport;

    status = p4_pd_dc_bfd_rx_timers_table_add_with_bfd_rx_timer_check(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action2,
        &bfd_info->rx_timers_table_check_entry_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }
  }
  {
    // bfd_tx_session: (only on one pipe)
    //  key{app_id=BFD, packet_id=0..N} =>
    //      bfd_update_tx_session_info(session_id, rx_mult, tx_mult)
    p4_pd_dc_bfd_tx_session_match_spec_t match_spec;
    p4_pd_dc_bfd_update_tx_session_info_action_spec_t action;

    match_spec.pktgen_generic_app_id = P4_PKTGEN_APP_BFD;
    match_spec.pktgen_generic_packet_id = local_session_id;

    action.action_session_id = bfd_info->session_id;
    action.action_rx_mult = bfd_info->rx_mult;
    action.action_tx_mult = bfd_info->tx_mult;
    action.action_vrf = handle_to_id(bfd_info->api_info.vrf_hdl);
    action.action_rmac_group = handle_to_id(bfd_info->api_info.rmac_hdl);
    memcpy(action.action_rmac, bfd_info->api_info.rmac.mac_addr, 6);

    p4_pd_device.dev_pipe_id = pktgen_pipe;
    status = p4_pd_dc_bfd_tx_session_table_add_with_bfd_update_tx_session_info(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action,
        &bfd_info->tx_session_table_entry_hdl);

    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }
  }
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  {
    // bfd_rx_session: (all pipes)
    //  key{myDisc, yourDisc} =>
    //      bfd_update_rx_session_info(session_id, rx_mult, pktgen_pipe);
    p4_pd_dc_bfd_rx_session_match_spec_t match_spec;
    p4_pd_dc_bfd_update_rx_session_info_action_spec_t action;

    match_spec.bfd_header_valid = 1;
    match_spec.bfd_header_myDiscriminator = bfd_info->api_info.your_disc;
    match_spec.bfd_header_yourDiscriminator = bfd_info->api_info.my_disc;
    match_spec.bfd_header_version = 1;
    match_spec.bfd_header_state_flags = 0xC0;
    match_spec.bfd_header_desiredMinTxInterval =
        bfd_info->api_info.remote_desired_tx_interval;
    match_spec.bfd_header_requiredMinRxInterval =
        bfd_info->api_info.remote_min_rx_interval;

    action.action_session_id = bfd_info->session_id;
    action.action_rx_mult = bfd_info->rx_mult;
    action.action_pktgen_pipe = pktgen_pipe + 1;
    action.action_recirc_port = SWITCH_PD_PKTGEN_RECIRC_PORT(pktgen_pipe);

    status = p4_pd_dc_bfd_rx_session_table_add_with_bfd_update_rx_session_info(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action,
        &bfd_info->rx_session_table_entry_hdl);

    if (status != SWITCH_STATUS_SUCCESS) {
      goto err_end;
    }
  }
  // enable_app on the pktgen_pipe (always is ok)
  p4_pd_device.dev_pipe_id = pktgen_pipe;
  if (switch_pd_bfd_session_increment(p4_pd_device) == 1) {
    // first session on this {dev, pipe}
    status = p4_pd_pktgen_app_enable(
        switch_cfg_sess_hdl, p4_pd_device, P4_PKTGEN_APP_BFD);
  }

err_end:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif
  return status;
}

switch_status_t switch_pd_bfd_session_delete(switch_device_t device,
                                             switch_bfd_info_t *bfd_info) {
#ifdef P4_BFD_OFFLOAD_ENABLE
  p4_pd_dev_target_t p4_pd_device;
  SWITCH_FAST_RECONFIG(device)
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  int pktgen_pipe = bfd_info->session_id % max_pipes;

  p4_pd_device.device_id = device;
  // delete entries that are common on all pipes
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  if (bfd_info->tx_timer_table_entry_hdl != SWITCH_PD_INVALID_HANDLE) {
    p4_pd_dc_bfd_tx_timer_table_delete(
        switch_cfg_sess_hdl, device, bfd_info->tx_timer_table_entry_hdl);
  }
  if (bfd_info->rx_session_table_entry_hdl != SWITCH_PD_INVALID_HANDLE) {
    p4_pd_dc_bfd_rx_session_table_delete(
        switch_cfg_sess_hdl, device, bfd_info->rx_session_table_entry_hdl);
  }

  // delete entries that are programmed per pipe
  p4_pd_device.dev_pipe_id = pktgen_pipe;

  if (bfd_info->rx_timers_table_reset_entry_hdl != SWITCH_PD_INVALID_HANDLE) {
    p4_pd_dc_bfd_rx_timers_table_delete(
        switch_cfg_sess_hdl, device, bfd_info->rx_timers_table_reset_entry_hdl);
  }
  if (bfd_info->rx_timers_table_check_entry_hdl != SWITCH_PD_INVALID_HANDLE) {
    p4_pd_dc_bfd_rx_timers_table_delete(
        switch_cfg_sess_hdl, device, bfd_info->rx_timers_table_check_entry_hdl);
  }
  if (bfd_info->tx_session_table_entry_hdl != SWITCH_PD_INVALID_HANDLE) {
    p4_pd_dc_bfd_tx_session_table_delete(
        switch_cfg_sess_hdl, device, bfd_info->tx_session_table_entry_hdl);
  }

  if (switch_pd_bfd_session_decrement(p4_pd_device) == 0) {
    // last session got deleted on this {dev, pipe}
    p4_pd_pktgen_app_disable(
        switch_cfg_sess_hdl, p4_pd_device, P4_PKTGEN_APP_BFD);
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif
  return SWITCH_STATUS_SUCCESS;
}
