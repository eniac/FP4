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

#define ERSPAN_FT_D_OTHER_POSTCARD 0x0400
#define ERSPAN_FT_D_OTHER_POSTCARD_QALERT 0x0C00
#define ERSPAN_FT_D_OTHER_QALERT 0x3800

switch_status_t switch_pd_dtel_postcard_tables_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#ifdef P4_POSTCARD_ENABLE
  switch_pd_hdl_t entry_hdl;
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_tbl_prop_value_t prop_val;
  p4_pd_tbl_prop_args_t prop_arg;
  prop_val.scope = PD_ENTRY_SCOPE_SINGLE_PIPELINE;
  prop_arg.value = 0;
  pd_status = p4_pd_dc_dtel_postcard_insert_set_property(
      switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Postcard dtel_postcard_insert set property failed on device %d\n",
        device);
    goto cleanup;
  }

  pd_status = p4_pd_dc_postcard_watchlist_set_default_action_postcard_not_watch(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("postcard watchlist set default action failed\n");
    goto cleanup;
  }

  // add sampling threshold values for default percents in registers
  for (int i = 0; i <= 100; i++) {
    uint32_t value = (uint32_t)(0xFFFFFFFFLL * (i / 100.0));
    pd_status = p4_pd_dc_register_write_dtel_postcard_sample_rate(
        switch_cfg_sess_hdl, p4_pd_device, i, &value);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel watchlist sampling set sample rate register failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

  pd_status = p4_pd_dc_dtel_postcard_e2e_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("postcard e2e set default action failed\n");
    goto cleanup;
  }

  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    pd_status = p4_pd_dc_dtel_postcard_insert_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR("postcard insert set default action failed\n");
      goto cleanup;
    }
  }
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_POSTCARD_ENABLE

#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard table init success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard table init failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_e2e_enable(switch_device_t device,
                                                   switch_list_t *event_infos) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(event_infos);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  // must remove previous entries (qalert could be there by default)
  // DSCP changes priority and requires entry removal and addtion again
  switch_pd_dtel_postcard_e2e_clear(device);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "postcard disabling postcard_e2e before enabling failed"
        "on device %d : %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_pd_hdl_t entry_hdl;
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  dtel_event_info_t *event_info = NULL;
  switch_node_t *node = NULL;

  int priority = 0;
  p4_pd_dc_dtel_postcard_e2e_match_spec_t match_spec;
  p4_pd_dc_postcard_e2e_action_spec_t action_spec;

  FOR_EACH_IN_LIST((*event_infos), node) {
    event_info = node->data;
    action_spec.action_dscp_report = event_info->dscp;

    match_spec.postcard_md_report = 1;
    match_spec.postcard_md_report_mask = 1;

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
    match_spec.postcard_md_suppress_enb = 0;
    match_spec.postcard_md_suppress_enb_mask = 0;
    match_spec.dtel_md_bfilter_output_mask = 0;
    match_spec.dtel_md_bfilter_output = 0;
#endif
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
    match_spec.dtel_md_queue_alert_mask = 0;
    match_spec.dtel_md_queue_alert = 0;
#endif

    match_spec.tcp_valid = 0;
    match_spec.tcp_valid_mask = 0;
    match_spec.tcp_flags = 0;
    match_spec.tcp_flags_mask = 0;

#ifdef P4_DTEL_WATCH_INNER_ENABLE
    match_spec.inner_tcp_info_valid_mask = 0;
    match_spec.inner_tcp_info_valid = 0;
    match_spec.inner_tcp_info_flags_mask = 0;
    match_spec.inner_tcp_info_flags = 0;
#endif
    switch (event_info->type) {
      case SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE: {
#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
        match_spec.postcard_md_suppress_enb = 1;
        match_spec.postcard_md_suppress_enb_mask = 1;
        match_spec.dtel_md_bfilter_output_mask = 2;
        match_spec.dtel_md_bfilter_output = 2;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (new flow) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        match_spec.dtel_md_bfilter_output_mask = 3;
        match_spec.dtel_md_bfilter_output = 0;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (flow change) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
#endif
      } break;
      case SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS: {
#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
        match_spec.postcard_md_suppress_enb = 0;
        match_spec.postcard_md_suppress_enb_mask = 1;
#endif
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (report all) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
      } break;
      case SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG: {
        match_spec.tcp_valid_mask = 1;
        match_spec.tcp_valid = 1;
        match_spec.tcp_flags = 1;
        match_spec.tcp_flags_mask = 1;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (outer TCP FIN) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        match_spec.tcp_flags = 2;
        match_spec.tcp_flags_mask = 2;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (outer TCP SYN) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        match_spec.tcp_flags = 4;
        match_spec.tcp_flags_mask = 4;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (outer TCP RST) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
#ifdef P4_DTEL_WATCH_INNER_ENABLE
        match_spec.inner_tcp_info_valid_mask = 1;
        match_spec.inner_tcp_info_valid = 1;
        match_spec.tcp_valid_mask = 0;
        match_spec.tcp_valid = 0;
        match_spec.tcp_flags_mask = 0;
        match_spec.tcp_flags = 0;
        match_spec.inner_tcp_info_flags_mask = 1;
        match_spec.inner_tcp_info_flags = 1;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (inner TCP FIN) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        match_spec.inner_tcp_info_flags_mask = 2;
        match_spec.inner_tcp_info_flags = 2;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (inner TCP SYN) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        match_spec.inner_tcp_info_flags_mask = 4;
        match_spec.inner_tcp_info_flags = 4;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (inner TCP RST) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
#endif  // P4_DTEL_WATCH_INNER_ENABLE
      } break;
      case SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH: {
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
        match_spec.dtel_md_queue_alert_mask = 1;
        match_spec.dtel_md_queue_alert = 1;
        match_spec.postcard_md_report_mask = 0;
        pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority++,
            &action_spec,
            &entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "DTel postcard_e2e (qalert) enable failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
      } break;
      default:
        break;
    }
  }
  FOR_EACH_IN_LIST_END();

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_POSTCARD_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard enable e2e success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard enable e2e failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_e2e_disable(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  switch_pd_hdl_t entry_hdl;
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  status = switch_pd_dtel_postcard_e2e_clear(device);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel postcard_e2e clearing for disable failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // get context
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel get dscp for telelemety event failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  p4_pd_dc_dtel_postcard_e2e_match_spec_t match_spec;
  p4_pd_dc_postcard_e2e_action_spec_t action_spec;
  action_spec.action_dscp_report =
      dtel_ctx->event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH]
          .dscp;
  match_spec.tcp_valid = 0;
  match_spec.tcp_valid_mask = 0;
  match_spec.tcp_flags = 0;
  match_spec.tcp_flags_mask = 0;

#ifdef P4_DTEL_WATCH_INNER_ENABLE
  match_spec.inner_tcp_info_valid_mask = 0;
  match_spec.inner_tcp_info_valid = 0;
  match_spec.inner_tcp_info_flags_mask = 0;
  match_spec.inner_tcp_info_flags = 0;
#endif
#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
  match_spec.postcard_md_suppress_enb = 0;
  match_spec.postcard_md_suppress_enb_mask = 0;
  match_spec.dtel_md_bfilter_output_mask = 0;
  match_spec.dtel_md_bfilter_output = 0;
#endif
  match_spec.dtel_md_queue_alert_mask = 1;
  match_spec.dtel_md_queue_alert = 1;
  match_spec.postcard_md_report_mask = 0;
  match_spec.postcard_md_report = 0;
  pd_status = p4_pd_dc_dtel_postcard_e2e_table_add_with_postcard_e2e(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      1,
      &action_spec,
      &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel postcard_e2e (qalert) enable failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard disable e2e success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard disable e2e failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_e2e_clear(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  switch_pd_hdl_t entry_hdl;
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

#ifdef BMV2TOFINO
  pd_status = p4_pd_dc_dtel_postcard_e2e_clear_entries(switch_cfg_sess_hdl,
                                                       p4_pd_device);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Postcard clearing dtel_postcard_e2e failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#else
  int index = 0;
  while (index >= 0) {
    pd_status = p4_pd_dc_dtel_postcard_e2e_get_first_entry_handle(
        switch_cfg_sess_hdl, p4_pd_device, &index);

    if (pd_status == SWITCH_PD_OBJ_NOT_FOUND) {
      pd_status = SWITCH_PD_STATUS_SUCCESS;
      break;
    }
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Postcard deleting an entry from dtel_postcard_e2e failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    if (index >= 0) {
      entry_hdl = (switch_pd_hdl_t)index;
      pd_status = p4_pd_dc_dtel_postcard_e2e_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "Postcard deleting an entry from dtel_postcard_e2e failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    }
  }
#endif  // BMV2TOFINO

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard disable e2e success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard disable e2e failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_insert_table_add(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(switch_id);
  UNUSED(dest_udp_port);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  p4_pd_dc_postcard_insert_action_spec_t action_spec;
  p4_pd_dc_dtel_postcard_insert_match_spec_t match_spec;
  action_spec.action_switch_id = switch_id;
  action_spec.action_udp_port = dest_udp_port;

  match_spec.postcard_md_report = 0;
  match_spec.dtel_md_queue_alert = 1;
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, true, false, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_add_with_postcard_insert(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert qalert entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    entry_hdl++;
  }

  match_spec.postcard_md_report = 1;
  match_spec.dtel_md_queue_alert = 0;
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, false, true, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_add_with_postcard_insert(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert postcard entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    entry_hdl++;
  }

  match_spec.postcard_md_report = 1;
  match_spec.dtel_md_queue_alert = 1;
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, true, true, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_add_with_postcard_insert(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert postcard+qalert entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    entry_hdl++;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("postcard insert add entries success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard insert add entries failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_insert_table_update(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(switch_id);
  UNUSED(dest_udp_port);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  int i = 0;
  p4_pd_dc_postcard_insert_action_spec_t action_spec;
  action_spec.action_switch_id = switch_id;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  action_spec.action_udp_port = dest_udp_port;
  for (int pipe = 0; pipe < max_pipes; pipe++) {
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, true, false, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_modify_with_postcard_insert(
        switch_cfg_sess_hdl, device, entry_hdl[i++], &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert qalert entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

  for (int pipe = 0; pipe < max_pipes; pipe++) {
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, false, true, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_modify_with_postcard_insert(
        switch_cfg_sess_hdl, device, entry_hdl[i++], &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert postcard entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

  for (int pipe = 0; pipe < max_pipes; pipe++) {
    action_spec.action_flags = switch_build_dtel_report_flags(
        0, DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL, false, true, true, 0, 0, pipe);
    pd_status = p4_pd_dc_dtel_postcard_insert_table_modify_with_postcard_insert(
        switch_cfg_sess_hdl, device, entry_hdl[i++], &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel postcard_insert postcard+qalert entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("postcard insert modify entries success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard insert modify entries failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_watchlist_entry_create(
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
#ifdef P4_POSTCARD_ENABLE
  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  p4_pd_dc_postcard_watchlist_match_spec_t match_spec;

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

#ifdef P4_DTEL_WATCH_INNER_ENABLE
  match_spec.tunnel_metadata_tunnel_vni = twl_match->tunnel_vni;
  match_spec.tunnel_metadata_tunnel_vni_mask = twl_match->tunnel_vni_mask;
  match_spec.inner_ethernet_etherType = twl_match->inner_ether_type;
  match_spec.inner_ethernet_etherType_mask = twl_match->inner_ether_type_mask;
  match_spec.inner_ipv4_srcAddr = twl_match->inner_ipv4_src;
  match_spec.inner_ipv4_srcAddr_mask = twl_match->inner_ipv4_src_mask;
  match_spec.inner_ipv4_dstAddr = twl_match->inner_ipv4_dst;
  match_spec.inner_ipv4_dstAddr_mask = twl_match->inner_ipv4_dst_mask;
  match_spec.inner_ipv4_protocol = twl_match->inner_ip_proto;
  match_spec.inner_ipv4_protocol_mask = twl_match->inner_ip_proto_mask;
  match_spec.inner_l4_ports_srcPort_start = twl_match->inner_l4_port_src_start;
  match_spec.inner_l4_ports_srcPort_end = twl_match->inner_l4_port_src_end;
  match_spec.inner_l4_ports_dstPort_start = twl_match->inner_l4_port_dst_start;
  match_spec.inner_l4_ports_dstPort_end = twl_match->inner_l4_port_dst_end;

  if (match_spec.inner_ipv4_srcAddr_mask != 0 ||
      match_spec.inner_ipv4_dstAddr_mask != 0 ||
      match_spec.inner_ipv4_protocol_mask != 0 ||
      match_spec.inner_l4_ports_srcPort_start != 0 ||
      match_spec.inner_l4_ports_srcPort_end != 0xFFFF ||
      match_spec.inner_l4_ports_dstPort_start != 0 ||
      match_spec.inner_l4_ports_dstPort_end != 0XFFFF) {
    match_spec.inner_ipv4_valid = 1;
    match_spec.inner_ipv4_valid_mask = 1;
  } else {
    match_spec.inner_ipv4_valid = 0;
    match_spec.inner_ipv4_valid_mask = 0;
  }
#endif  // P4_DTEL_WATCH_INNER_ENABLE

  if (watch && action_params->_postcard.flow_sample_percent == 0) {
    // no_watch is more accurate because of <= in data plane
    watch = false;
  }

  if (watch) {
    p4_pd_dc_postcard_watch_sample_action_spec_t action_spec;
    action_spec.action_suppress_enb =
        !action_params->_postcard.report_all_packets;
    action_spec.action_sample_index =
        action_params->_postcard.flow_sample_percent;  // sample_index i keeps
                                                       // percent i
    pd_status =
        p4_pd_dc_postcard_watchlist_table_add_with_postcard_watch_sample(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
            &action_spec,
            entry_hdl);
  } else {
    pd_status = p4_pd_dc_postcard_watchlist_table_add_with_postcard_not_watch(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard watchlist add success on device %d handle 0x%x\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard watchlist add failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_watchlist_entry_update(
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
#ifdef P4_POSTCARD_ENABLE
  if (watch && action_params->_postcard.flow_sample_percent == 0) {
    // no_watch is more accurate because of <= in data plane
    watch = false;
  }

  if (watch) {
    p4_pd_dc_postcard_watch_sample_action_spec_t action_spec;
    action_spec.action_suppress_enb =
        !action_params->_postcard.report_all_packets;
    action_spec.action_sample_index =
        action_params->_postcard.flow_sample_percent;  // sample_index i keeps
                                                       // percent i
    pd_status =
        p4_pd_dc_postcard_watchlist_table_modify_with_postcard_watch_sample(
            switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  } else {
    pd_status =
        p4_pd_dc_postcard_watchlist_table_modify_with_postcard_not_watch(
            switch_cfg_sess_hdl, device, entry_hdl);
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Postcard watchlist update success on device %d handle 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Postcard watchlist update failed on device %d : %s (pd: 0x%x) handle "
        "0x%x",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_watchlist_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_POSTCARD_ENABLE
  pd_status = p4_pd_dc_postcard_watchlist_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_POSTCARD_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "postcard watchlist delete success on device %d handle 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard watchlist delete failed on device %d : %s (pd: 0x%x) handle "
        "0x%x",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_dtel_postcard_set_sample(switch_device_t device,
                                                   uint16_t index,
                                                   uint8_t percent) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(percent);

#ifdef SWITCH_PD

#ifdef P4_POSTCARD_ENABLE
#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  if (percent > 100) {
    percent = 100;
  }

  uint32_t hash_threshold = (uint32_t)(0xFFFFFFFF * (100.0 - percent) / 100.0);
  pd_status = p4_pd_dc_register_write_dtel_postcard_sample_rate(
      switch_cfg_sess_hdl, p4_pd_device, index, &hash_threshold);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel sample rate set sample rate register failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE
#endif  // P4_POSTCARD_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("postcard sample rate set success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "postcard sample rate set failed on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}
