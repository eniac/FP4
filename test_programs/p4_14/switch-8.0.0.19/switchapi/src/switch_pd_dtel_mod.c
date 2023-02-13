/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2016 Barefoot Networks, Inc.

 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Barefoot Networks,
 * Inc.
 * and its suppliers and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is
 * strictly forbidden unless prior written permission is obtained from
 * Barefoot Networks, Inc.
 *
 * No warranty, explicit or implicit is provided, unless granted under a
 * written agreement with Barefoot Networks, Inc.
 *
 * $Id: $
 *
 ******************************************************************************/

#include "switch_internal.h"
#include "switch_pd_dtel.h"

switch_status_t switch_pd_mirror_on_drop_tables_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  switch_pd_hdl_t entry_hdl;
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_mod_watch_nodod_action_spec_t action_spec;
  action_spec.action_dod_watchlist = 0;
  pd_status =
      p4_pd_dc_mirror_on_drop_watchlist_set_default_action_mod_watch_nodod(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mirror on drop watchlist set default failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_DTEL_DROP_REPORT_ENABLE

#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Mirror on Drop table init success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop table init failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

#if defined(P4_DTEL_DROP_REPORT_ENABLE) || defined(P4_DTEL_QUEUE_REPORT_ENABLE)
static switch_pd_status_t switch_pd_mirror_on_drop_encap_add_update(
    switch_device_t device,
    p4_pd_dc_mirror_on_drop_encap_match_spec_t *match_spec,
    int priority,
    p4_pd_dc_mirror_on_drop_insert_action_spec_t *action_spec,
    bool path_tracking_flow,
    bool congested,
    p4_pd_entry_hdl_t **entry_hdl,
    bool add) {
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  if (add) {
    p4_pd_dev_target_t p4_pd_device;
    p4_pd_device.device_id = device;
    for (int pipe = 0;
         pipe < max_pipes && pd_status == SWITCH_PD_STATUS_SUCCESS;
         pipe++) {
      p4_pd_device.dev_pipe_id = pipe;
      action_spec->action_flags =
          switch_build_dtel_report_flags(0,
                                         DTEL_REPORT_NEXT_PROTO_MOD,
                                         true,
                                         congested,
                                         path_tracking_flow,
                                         0,
                                         0,
                                         pipe);
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_add_with_mirror_on_drop_insert(
              switch_cfg_sess_hdl,
              p4_pd_device,
              match_spec,
              priority,
              action_spec,
              *entry_hdl);
      (*entry_hdl)++;
    }
  } else {
    for (int pipe = 0;
         pipe < max_pipes && pd_status == SWITCH_PD_STATUS_SUCCESS;
         pipe++) {
      action_spec->action_flags =
          switch_build_dtel_report_flags(0,
                                         DTEL_REPORT_NEXT_PROTO_MOD,
                                         true,
                                         congested,
                                         path_tracking_flow,
                                         0,
                                         0,
                                         pipe);
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_modify_with_mirror_on_drop_insert(
              switch_cfg_sess_hdl, device, **entry_hdl, action_spec);
      (*entry_hdl)++;
    }
  }
  return pd_status;
}
#endif  // P4_DTEL_DROP_REPORT_ENABLE || P4_DTEL_QUEUE_REPORT_ENABLE

switch_status_t switch_pd_mirror_on_drop_encap_update(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    dtel_event_info_t *event_infos,
    bool add,
    p4_pd_entry_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(switch_id);
  UNUSED(dest_udp_port);
  UNUSED(event_infos);
  UNUSED(add);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_DTEL_QUEUE_REPORT_ENABLE) || defined(P4_DTEL_DROP_REPORT_ENABLE)
  bool path_tracking_flow;
  bool congested;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_entry_hdl_t default_entry_hdl;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  if (add) {
    p4_pd_tbl_prop_value_t prop_val;
    p4_pd_tbl_prop_args_t prop_arg;
    prop_val.scope = PD_ENTRY_SCOPE_SINGLE_PIPELINE;
    prop_arg.value = 0;
    pd_status = p4_pd_dc_mirror_on_drop_encap_set_property(
        switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Mirror on Drop encap add/update an entry failed on device %d"
          " cannot set table property\n",
          device);
      goto cleanup;
    }
  }

  p4_pd_dc_mirror_on_drop_encap_match_spec_t match_spec;
  p4_pd_dc_mirror_on_drop_insert_action_spec_t action_spec;
  action_spec.action_switch_id = switch_id;
  action_spec.action_udp_port = dest_udp_port;

  int common_priority = 2;
  int dod_priority = common_priority - 1;
  int nop_priority = dod_priority - 1;

  switch_uint8_t mod_dscp = 0;
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  mod_dscp = event_infos[SWITCH_DTEL_EVENT_TYPE_DROP_REPORT].dscp;
#endif  // P4_DTEL_DROP_REPORT_ENABLE

  // set default action
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    pd_status = p4_pd_dc_mirror_on_drop_encap_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &default_entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Mirror on Drop encap set default failed on device %d\n", device);
      goto cleanup;
    }
  }

/*********************************************************
 * INT_EP
 *********************************************************/
#ifdef P4_INT_EP_ENABLE
  if (add) {
    // high priority nop to filter drop_reason==0
    match_spec.ingress_metadata_drop_reason = 0;
    match_spec.ingress_metadata_drop_reason_mask = 0xff;
    match_spec.int_metadata_sink = 0;
    match_spec.int_metadata_sink_mask = 0;
    match_spec.int_metadata_source = 0;
    match_spec.int_metadata_source_mask = 0;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
    match_spec.dtel_md_queue_alert = 0;
    match_spec.dtel_md_queue_alert_mask = 0;
#endif
#ifdef P4_DTEL_DROP_REPORT_ENABLE
    match_spec.dtel_md_mod_watchlist_hit = 0;
    match_spec.dtel_md_mod_watchlist_hit_mask = 0;
#endif
    for (int pipe = 0; pipe < max_pipes; pipe++) {
      p4_pd_device.dev_pipe_id = pipe;
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_add_with_nop(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           nop_priority,
                                                           entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "Mirror on Drop encap add/update an entry failed on device %d"
            " for nop\n",
            device);
        goto cleanup;
      }
      entry_hdl++;
    }
  } else {
    entry_hdl += max_pipes;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // only int
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = true;
  congested = false;

  match_spec.int_metadata_source = 1;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 0;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for INT source\n",
        device);
    goto cleanup;
  }

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 1;
  match_spec.int_metadata_sink_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for INT sink\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // qalert dod without mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = DROP_TRAFFIC_MANAGER;
  match_spec.ingress_metadata_drop_reason_mask = 0xff;
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  match_spec.dtel_md_mod_watchlist_hit = 0;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;
#endif

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD without MOD\n",
        device);
    goto cleanup;
  }

  // int + qalert dod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_source = 1;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 0;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT source without MoD\n",
        device);
    goto cleanup;
  }

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 1;
  match_spec.int_metadata_sink_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT sink without MoD\n",
        device);
    goto cleanup;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // qalert dod + mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_mod_watchlist_hit = 2;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD with MoD watchlist\n",
        device);
    goto cleanup;
  }

  // int + qalert dod + mod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_source = 1;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 0;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT source + MoD Watchlist\n",
        device);
    goto cleanup;
  }

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 1;
  match_spec.int_metadata_sink_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT sink + MoD Watchlist\n",
        device);
    goto cleanup;
  }

  // anything not queue alert DoD must have matched MoD watchlist
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  // qalert
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert\n",
        device);
    goto cleanup;
  }

  // int + qalert
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_source = 1;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 0;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert + INT source\n",
        device);
    goto cleanup;
  }

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 1;
  match_spec.int_metadata_sink_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert + INT sink\n",
        device);
    goto cleanup;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // mod
  action_spec.action_dscp = mod_dscp;
  path_tracking_flow = false;
  congested = false;

  match_spec.int_metadata_source = 0;
  match_spec.int_metadata_source_mask = 1;
  match_spec.int_metadata_sink = 0;
  match_spec.int_metadata_sink_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap set default failed on device %d"
        " for MoD\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE

/*********************************************************
 * INT_TRANSIT
 *********************************************************/
#elif defined(P4_INT_TRANSIT_ENABLE)
  if (add) {
    // high priority nop to filter drop_reason==0
    match_spec.ingress_metadata_drop_reason = 0;
    match_spec.ingress_metadata_drop_reason_mask = 0xff;
    match_spec.int_metadata_path_tracking_flow = 0;
    match_spec.int_metadata_path_tracking_flow_mask = 0;

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
    match_spec.dtel_md_queue_alert = 0;
    match_spec.dtel_md_queue_alert_mask = 0;
#endif
#ifdef P4_DTEL_DROP_REPORT_ENABLE
    match_spec.dtel_md_mod_watchlist_hit = 0;
    match_spec.dtel_md_mod_watchlist_hit_mask = 0;
#endif
    for (int pipe = 0; pipe < max_pipes; pipe++) {
      p4_pd_device.dev_pipe_id = pipe;
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_add_with_nop(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           nop_priority,
                                                           entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "Mirror on Drop encap add/update an entry failed on device %d"
            " for nop\n",
            device);
        goto cleanup;
      }
      entry_hdl++;
    }
  } else {
    entry_hdl += max_pipes;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // only int
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = true;
  congested = false;

  match_spec.int_metadata_path_tracking_flow = 1;
  match_spec.int_metadata_path_tracking_flow_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for INT transit\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // qalert dod without mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = DROP_TRAFFIC_MANAGER;
  match_spec.ingress_metadata_drop_reason_mask = 0xff;
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  match_spec.dtel_md_mod_watchlist_hit = 0;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;
#endif

  match_spec.int_metadata_path_tracking_flow = 0;
  match_spec.int_metadata_path_tracking_flow_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD without MoD\n",
        device);
    goto cleanup;
  }

  // int + qalert dod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_path_tracking_flow = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT transit without MoD\n",
        device);
    goto cleanup;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // qalert dod + mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  match_spec.dtel_md_mod_watchlist_hit = 2;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;

  path_tracking_flow = false;
  congested = true;

  match_spec.int_metadata_path_tracking_flow = 0;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + MoD watchlist\n",
        device);
    goto cleanup;
  }

  // int + qalert dod + mod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_path_tracking_flow = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + INT transit + MoD watchlist\n",
        device);
    goto cleanup;
  }

  // anything not queue alert DoD must have matched MoD watchlist
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  // qalert
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;

  match_spec.int_metadata_path_tracking_flow = 0;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert\n",
        device);
    goto cleanup;
  }

  // int + qalert
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.int_metadata_path_tracking_flow = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert + INT transit\n",
        device);
    goto cleanup;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // mod
  action_spec.action_dscp = mod_dscp;
  path_tracking_flow = false;
  congested = false;

  match_spec.int_metadata_path_tracking_flow = 0;
  match_spec.int_metadata_path_tracking_flow_mask = 1;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for MoD\n",
        device);
    goto cleanup;
  }
#endif

/*********************************************************
 * POSTCARD
 *********************************************************/
#elif defined(P4_POSTCARD_ENABLE)
  if (add) {
    // high priority nop to filter drop_reason==0
    match_spec.ingress_metadata_drop_reason = 0;
    match_spec.ingress_metadata_drop_reason_mask = 0xff;
    match_spec.postcard_md_report = 0;
    match_spec.postcard_md_report_mask = 0;

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
    match_spec.dtel_md_queue_alert = 0;
    match_spec.dtel_md_queue_alert_mask = 0;
#endif
#ifdef P4_DTEL_DROP_REPORT_ENABLE
    match_spec.dtel_md_mod_watchlist_hit = 0;
    match_spec.dtel_md_mod_watchlist_hit_mask = 0;
#endif
    for (int pipe = 0; pipe < max_pipes; pipe++) {
      p4_pd_device.dev_pipe_id = pipe;
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_add_with_nop(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           nop_priority,
                                                           entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "Mirror on Drop encap add/update an entry failed on device %d"
            " for nop\n",
            device);
        goto cleanup;
      }
      entry_hdl++;
    }
  } else {
    entry_hdl += max_pipes;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // only postcard
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = true;
  congested = false;

  match_spec.postcard_md_report = 1;
  match_spec.postcard_md_report_mask = 1;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Postcard\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // qalert dod without MoD
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = DROP_TRAFFIC_MANAGER;
  match_spec.ingress_metadata_drop_reason_mask = 0xff;
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  match_spec.dtel_md_mod_watchlist_hit = 0;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;
#endif

  match_spec.postcard_md_report = 0;
  match_spec.postcard_md_report_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD without MoD\n",
        device);
    goto cleanup;
  }

  // postcard + qalert dod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.postcard_md_report = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + Postcard without MoD\n",
        device);
    goto cleanup;
  }
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // qalert_dod + mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_mod_watchlist_hit = 2;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;

  match_spec.postcard_md_report = 0;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + MoD\n",
        device);
    goto cleanup;
  }

  // postcard + qalert dod + mod
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.postcard_md_report = 1;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + Postcard + MoD\n",
        device);
    goto cleanup;
  }
  // anything not queue alert DoD must have matched MoD watchlist
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  // qalert
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  path_tracking_flow = false;
  congested = true;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;

  match_spec.postcard_md_report = 0;
  match_spec.postcard_md_report_mask = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert\n",
        device);
    goto cleanup;
  }

  // postcard + qalert
  if (action_spec.action_dscp <
      event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp) {
    action_spec.action_dscp =
        event_infos[SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS].dscp;
  }
  path_tracking_flow = true;
  congested = true;

  match_spec.postcard_md_report = 1;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert + postcard\n",
        device);
    goto cleanup;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // mod
  action_spec.action_dscp = mod_dscp;
  path_tracking_flow = false;
  congested = false;

  match_spec.postcard_md_report = 0;
  match_spec.postcard_md_report_mask = 1;
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for MoD\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE
#else
  /*********************************************************
   * PURE QUEUE ALERT AND MOD
   *********************************************************/
  if (add) {
    // high priority nop to filter drop_reason==0
    match_spec.ingress_metadata_drop_reason = 0;
    match_spec.ingress_metadata_drop_reason_mask = 0xff;

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
    match_spec.dtel_md_queue_alert = 0;
    match_spec.dtel_md_queue_alert_mask = 0;
#endif
#ifdef P4_DTEL_DROP_REPORT_ENABLE
    match_spec.dtel_md_mod_watchlist_hit = 0;
    match_spec.dtel_md_mod_watchlist_hit_mask = 0;
#endif
    for (int pipe = 0; pipe < max_pipes; pipe++) {
      p4_pd_device.dev_pipe_id = pipe;
      pd_status =
          p4_pd_dc_mirror_on_drop_encap_table_add_with_nop(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           nop_priority,
                                                           entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "Mirror on Drop encap add/update an entry failed on device %d"
            " for nop\n",
            device);
        goto cleanup;
      }
      entry_hdl++;
    }
  } else {
    entry_hdl += max_pipes;
  }

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  path_tracking_flow = false;
  congested = true;

  // qalert dod without mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = DROP_TRAFFIC_MANAGER;
  match_spec.ingress_metadata_drop_reason_mask = 0xff;
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  match_spec.dtel_md_mod_watchlist_hit = 0;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;
#endif

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD without MoD\n",
        device);
    goto cleanup;
  }

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // qalert dod + mod
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }
  match_spec.dtel_md_mod_watchlist_hit = 2;
  match_spec.dtel_md_mod_watchlist_hit_mask = 2;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        dod_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert DoD + MoD\n",
        device);
    goto cleanup;
  }

  // anything not queue alert DoD must have matched MoD watchlist
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

  // qalert
  action_spec.action_dscp =
      event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH].dscp;
  if (action_spec.action_dscp < mod_dscp) {
    action_spec.action_dscp = mod_dscp;
  }

  match_spec.dtel_md_queue_alert = 1;
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;

  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for Queue Alert\n",
        device);
    goto cleanup;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  // mod
  action_spec.action_dscp = mod_dscp;
  path_tracking_flow = false;
  congested = false;
  match_spec.dtel_md_mod_watchlist_hit = 1;
  match_spec.dtel_md_mod_watchlist_hit_mask = 1;

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  match_spec.dtel_md_queue_alert = 0;
  match_spec.dtel_md_queue_alert_mask = 1;
#endif
  match_spec.ingress_metadata_drop_reason = 0;
  match_spec.ingress_metadata_drop_reason_mask = 0;
  pd_status = switch_pd_mirror_on_drop_encap_add_update(device,
                                                        &match_spec,
                                                        common_priority,
                                                        &action_spec,
                                                        path_tracking_flow,
                                                        congested,
                                                        &entry_hdl,
                                                        add);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop encap add/update an entry failed on device %d"
        " for MoD\n",
        device);
    goto cleanup;
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE

#endif  // INT_EP_ENABLE -> INT_TRANSIT_ENABLE -> POSTCARD -> else

cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_DROP_REPORT_ENABLE || P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("Mirror on Drop set switch ID success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop set switch ID failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_drop_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_spec_t *twl_match,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(priority);
  UNUSED(watch);
  UNUSED(action_params);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  p4_pd_dc_mirror_on_drop_watchlist_match_spec_t match_spec;

  // temp fix
  match_spec.ipv4_diffserv = 0;
  match_spec.ipv4_diffserv_mask = 0;

  match_spec.ethernet_etherType = twl_match->ether_type;
  match_spec.ethernet_etherType_mask = twl_match->ether_type_mask;
  match_spec.ipv4_srcAddr = twl_match->ipv4_src;
  match_spec.ipv4_srcAddr_mask = twl_match->ipv4_src_mask;
  match_spec.ipv4_dstAddr = twl_match->ipv4_dst;
  match_spec.ipv4_dstAddr_mask = twl_match->ipv4_dst_mask;
  match_spec.ipv4_protocol = twl_match->ip_proto;
  match_spec.ipv4_protocol_mask = twl_match->ip_proto_mask;
  match_spec.ipv4_diffserv = twl_match->dscp;
  match_spec.ipv4_diffserv_mask = twl_match->dscp_mask;
  match_spec.l3_metadata_lkp_l4_sport_start = twl_match->l4_port_src_start;
  match_spec.l3_metadata_lkp_l4_sport_end = twl_match->l4_port_src_end;
  match_spec.l3_metadata_lkp_l4_dport_start = twl_match->l4_port_dst_start;
  match_spec.l3_metadata_lkp_l4_dport_end = twl_match->l4_port_dst_end;

  if (match_spec.ipv4_srcAddr_mask != 0 || match_spec.ipv4_dstAddr_mask != 0 ||
      match_spec.ipv4_protocol_mask != 0 ||
      match_spec.ipv4_diffserv_mask != 0 ||
      match_spec.l3_metadata_lkp_l4_sport_start != 0 ||
      match_spec.l3_metadata_lkp_l4_sport_end != 0xFFFF ||
      match_spec.l3_metadata_lkp_l4_dport_start != 0 ||
      match_spec.l3_metadata_lkp_l4_dport_end != 0xFFFF) {
    match_spec.ipv4_valid = 1;
    match_spec.ipv4_valid_mask = 1;
  } else {
    match_spec.ipv4_valid = 0;
    match_spec.ipv4_valid_mask = 0;
  }

  if (action_params != NULL && action_params->_mod.report_queue_tail_drops) {
    pd_status = p4_pd_dc_mirror_on_drop_watchlist_table_add_with_mod_watch_dod(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
  } else {
    p4_pd_dc_mod_watch_nodod_action_spec_t action_spec;
    action_spec.action_dod_watchlist = watch ? 1 : 0;
    pd_status =
        p4_pd_dc_mirror_on_drop_watchlist_table_add_with_mod_watch_nodod(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
            &action_spec,
            entry_hdl);
  }
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Mirror on Drop watchlist add success on device %d handle 0x%x\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop watchlist add failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_drop_watchlist_entry_update(
    switch_device_t device,
    switch_pd_hdl_t entry_hdl,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(watch);
  UNUSED(action_params);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  if (action_params != NULL && action_params->_mod.report_queue_tail_drops) {
    pd_status =
        p4_pd_dc_mirror_on_drop_watchlist_table_modify_with_mod_watch_dod(
            switch_cfg_sess_hdl, device, entry_hdl);
  } else {
    p4_pd_dc_mod_watch_nodod_action_spec_t action_spec;
    action_spec.action_dod_watchlist = watch ? 1 : 0;
    pd_status =
        p4_pd_dc_mirror_on_drop_watchlist_table_modify_with_mod_watch_nodod(
            switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Mirror on Drop watchlist update success on device %d handle 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop watchlist update failed on device %d : %s (pd: 0x%x) "
        "handle 0x%x",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_drop_watchlist_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_DTEL_DROP_REPORT_ENABLE
  pd_status = p4_pd_dc_mirror_on_drop_watchlist_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_DROP_REPORT_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Mirror on Drop watchlist delete success on device %d handle 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Mirror on Drop watchlist delete failed on device %d : %s (pd: 0x%x) "
        "handle 0x%x",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}
