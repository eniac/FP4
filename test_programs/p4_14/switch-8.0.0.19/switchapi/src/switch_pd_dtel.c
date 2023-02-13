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

switch_uint32_t switch_build_dtel_report_flags(switch_uint8_t version,
                                               switch_uint8_t next_proto,
                                               bool dropped,
                                               bool congested,
                                               bool path_tracking_flow,
                                               switch_uint8_t reserved1,
                                               switch_uint16_t reserved2,
                                               switch_uint8_t hw_id) {
  switch_uint32_t res = 0;
  res |= hw_id & ((1 << 6) - 1);
  res |= (reserved2 & ((1 << 10) - 1)) << 6;
  res |= (reserved1 & ((1 << 5) - 1)) << 16;
  if (path_tracking_flow) {
    res |= 1 << 21;
  }
  if (congested) {
    res |= 1 << 22;
  }
  if (dropped) {
    res |= 1 << 23;
  }
  res |= (next_proto & ((1 << 4) - 1)) << 24;
  res |= (version & ((1 << 4) - 1)) << 28;
  return res;
}

switch_status_t switch_pd_dtel_tables_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#if defined(P4_DTEL_DROP_REPORT_ENABLE) ||                               \
    defined(P4_DTEL_QUEUE_REPORT_ENABLE) || defined(P4_INT_EP_ENABLE) || \
    defined(P4_POSTCARD_ENABLE)
  {
    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    pd_status =
        p4_pd_dc_dtel_record_egress_port_set_default_action_record_eg_port_invalid(
            switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_record_egress_port set default failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    p4_pd_dc_dtel_record_egress_port_match_spec_t match_spec;
    match_spec.eg_intr_md_deflection_flag = 0;
    match_spec.eg_intr_md_egress_rid = 0;
    match_spec.eg_intr_md_egress_rid_mask = 0;
    pd_status =
        p4_pd_dc_dtel_record_egress_port_table_add_with_record_eg_port_from_eg(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, 0, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_record_egress_port add dod=0 failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    match_spec.eg_intr_md_deflection_flag = 1;
    match_spec.eg_intr_md_egress_rid = 0;
    match_spec.eg_intr_md_egress_rid_mask = 0xFFFF;
    pd_status =
        p4_pd_dc_dtel_record_egress_port_table_add_with_record_eg_port_from_ig(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, 0, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_record_egress_port add dod=1 failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    pd_status = p4_pd_dc_dtel_ig_port_convert_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_ig_port_convert set default action failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    pd_status = p4_pd_dc_dtel_eg_port_convert_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_ig_port_convert set default action failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

#endif /* P4_DTEL_DROP_REPORT_ENABLE || P4_DTEL_QUEUE_REPORT_ENABLE || \
          P4_INT_EP_ENABLE || P4_POSTCARD_ENABLE */

#if defined(P4_DTEL_DROP_REPORT_ENABLE) || defined(P4_DTEL_QUEUE_REPORT_ENABLE)
  /* dod_control */
  {
    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    // allow Deflection_on_Drop for traffic w/o mcast or copy-to-cpu
    // default action is invalidate dod.
    pd_status = p4_pd_dc_dod_control_set_default_action_invalidate_dod(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dod_control_set_default_action_invalidate_dod failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    p4_pd_dc_dod_control_match_spec_t match_spec;
    match_spec.ig_intr_md_for_tm_mcast_grp_a = 0;
    match_spec.ig_intr_md_for_tm_mcast_grp_b = 0;
    match_spec.ig_intr_md_for_tm_copy_to_cpu = 0;
    pd_status = p4_pd_dc_dod_control_table_add_with_nop(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dod_control_table_add_with_nop failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }
#endif  // P4_DTEL_DROP_REPORT_ENABLE || P4_DTEL_QUEUE_REPORT_ENABLE

#if (defined(P4_DTEL_FLOW_STATE_TRACK_ENABLE) || \
     defined(P4_INT_DIGEST_ENABLE)) &&           \
    !defined(P4_DTEL_QUEUE_REPORT_ENABLE)
  {
    // if queue report is disabled but still need quantized latency
    // we need a table to do that instead of piggybacking on dtel_queue_alert
    // table

    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    p4_pd_dc_run_dtel_mask_latency_action_spec_t action_spec;
    action_spec.action_quantization_mask = (switch_uint32_t)(
        ~((1LL << DTEL_DEFAULT_LATENCY_QUANTIZATION_SHIFT) - 1));
    pd_status =
        p4_pd_dc_dtel_mask_latency_set_default_action_run_dtel_mask_latency(
            switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_mask_latency set default entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }
#endif /* (P4_DTEL_FLOW_STATE_TRACK_ENABLE || P4_INT_DIGEST_ENABLE) && \
          !P4_DTEL_QUEUE_REPORT_ENABLE */

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
  {
    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    pd_status =
        p4_pd_dc_dtel_make_local_digest_set_default_action_make_local_digest(
            switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel make_local_digest set default failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
    switch_pd_status_t (*dtel_eg_bfilter_set_default_actions[4])() = {
        p4_pd_dc_dtel_eg_bfilter_1_set_default_action_run_dtel_eg_bfilter_1,
        p4_pd_dc_dtel_eg_bfilter_2_set_default_action_run_dtel_eg_bfilter_2,
        p4_pd_dc_dtel_eg_bfilter_3_set_default_action_run_dtel_eg_bfilter_3,
        p4_pd_dc_dtel_eg_bfilter_4_set_default_action_run_dtel_eg_bfilter_4};

    for (int filter_id = 0; filter_id < 4; filter_id++) {
      pd_status = dtel_eg_bfilter_set_default_actions[filter_id](
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "DTel egress bfilter set default failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    }
  }
#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE

#if defined(P4_DTEL_REPORT_LB_ENABLE) || \
    defined(P4_DTEL_FLOW_STATE_TRACK_ENABLE)
  {
    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    // dtel_mirror_session table
    switch_pd_mbr_hdl_t mbr_hdl;
    pd_status = p4_pd_dc_dtel_selector_action_profile_add_member_with_nop(
        switch_cfg_sess_hdl, p4_pd_device, &mbr_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel selection action profile add nop member failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    pd_status = p4_pd_dc_dtel_mirror_session_set_default_entry(
        switch_cfg_sess_hdl, p4_pd_device, mbr_hdl, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_mirror_session set default failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

#endif  // P4_DTEL_REPORT_LB_ENABLE || P4_DTEL_FLOW_STATE_TRACK_ENABLE

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  {
    switch_pd_hdl_t entry_hdl;
    switch_pd_target_t p4_pd_device;

    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    // set to default to not think 0 quota and queue_alert = 0 is
    // a transition from alert to no alert
    p4_pd_dc_dtel_queue_report_quota_reg_value_t value;
    value.f0 = DTEL_QUEUE_REPORT_DEFAULT_QUOTA;
    value.f1 = DTEL_QUEUE_REPORT_DEFAULT_QUOTA;
    pd_status = p4_pd_dc_register_write_all_dtel_queue_report_quota_reg(
        switch_cfg_sess_hdl, p4_pd_device, &value);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_queue_report_quota_reg set default value failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    pd_status = p4_pd_dc_deflect_on_drop_queue_config_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "deflect_on_drop_queue_config set default failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    p4_pd_dc_dtel_queue_alert_update_match_spec_t match_spec;
    match_spec.dtel_md_queue_alert = 0;
    match_spec.dtel_md_queue_report_quota = 0;
    match_spec.dtel_md_queue_change = 0;
    pd_status =
        p4_pd_dc_dtel_queue_alert_update_table_add_with_dtel_set_queue_alert(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_queue_alert_update add default entries failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    match_spec.dtel_md_queue_alert = 1;
    match_spec.dtel_md_queue_report_quota = 0;
    match_spec.dtel_md_queue_change = 0;
    pd_status =
        p4_pd_dc_dtel_queue_alert_update_table_add_with_dtel_unset_queue_alert(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_queue_alert_update add default entries failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    pd_status = p4_pd_dc_dtel_queue_alert_update_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_queue_alert_update set default entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    {
      p4_pd_dc_run_dtel_mask_latency_action_spec_t action_spec;
      action_spec.action_quantization_mask = (switch_uint32_t)(
          ~((1LL << DTEL_DEFAULT_LATENCY_QUANTIZATION_SHIFT) - 1));
      pd_status =
          p4_pd_dc_dtel_queue_alert_set_default_action_run_dtel_mask_latency(
              switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dtel_queue_alert set default entry failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    }

    pd_status = p4_pd_dc_dtel_queue_report_dod_quota_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_queue_report_dod_quota set default entry failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#ifdef P4_DTEL_REPORT_ENABLE
  {
    switch_pd_target_t p4_pd_device;
    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

    pd_status = p4_pd_dc_register_reset_all_dtel_report_header_seqnum(
        switch_cfg_sess_hdl, p4_pd_device);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dtel_report_header_seqnum reset failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }
#endif  // P4_DTEL_REPORT_ENABLE

#if defined(P4_DTEL_FLOW_STATE_TRACK_ENABLE) ||                           \
    defined(P4_INT_DIGEST_ENABLE) || defined(P4_DTEL_REPORT_LB_ENABLE) || \
    defined(P4_DTEL_QUEUE_REPORT_ENABLE) ||                               \
    defined(P4_DTEL_DROP_REPORT_ENABLE) || defined(P4_INT_EP_ENABLE) ||   \
    defined(P4_POSTCARD_ENABLE) || defined(P4_DTEL_REPORT_ENABLE)

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel add default entries success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel add default entries failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_report_sequence_number_set(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t value) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(mirror_session_id);
  UNUSED(value);

#ifdef SWITCH_PD
#ifdef P4_DTEL_REPORT_ENABLE
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_register_write_dtel_report_header_seqnum(
      switch_cfg_sess_hdl, p4_pd_device, mirror_session_id, &value);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel report set sequence number failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_DTEL_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel report senquence number set success "
        "on device %d %d\n",
        device,
        mirror_session_id);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel report sequence number set failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_report_sequence_number_get(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t *values,
    switch_uint8_t *max_num) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(mirror_session_id);
  UNUSED(values);
  UNUSED(max_num);

#ifdef SWITCH_PD
#ifdef P4_DTEL_REPORT_ENABLE
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  switch_uint32_t values_[max_pipes];
  int read_values;
  pd_status =
      p4_pd_dc_register_read_dtel_report_header_seqnum(switch_cfg_sess_hdl,
                                                       p4_pd_device,
                                                       mirror_session_id,
                                                       REGISTER_READ_HW_SYNC,
                                                       values_,
                                                       &read_values);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel report get sequence number failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  if (*max_num > max_pipes) {
    *max_num = max_pipes;
  }
  if (*max_num > read_values) {
    *max_num = read_values;
  }
  for (int i = 0; i < *max_num; i++) {
    values[i] = values_[i];
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // P4_DTEL_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel report sequence number get success "
        "on device %d %d\n",
        device,
        mirror_session_id);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel report sequence number get failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_add_group(
    switch_device_t device, switch_pd_mbr_hdl_t *pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(pd_grp_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_dtel_selector_action_profile_create_group(
      switch_cfg_sess_hdl,
      p4_pd_device,
      DTEL_MAX_MIRROR_SESSION_PER_GROUP,
      pd_grp_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add group failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session add group success "
        "on device %d 0x%x\n",
        device,
        *pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add group failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_add_group_selector(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_grp_hdl,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(pd_grp_hdl);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  p4_pd_dc_dtel_mirror_session_match_spec_t match_spec;
  match_spec.ethernet_valid = 1;
  pd_status = p4_pd_dc_dtel_mirror_session_add_entry_with_selector(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, pd_grp_hdl, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add group selector failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session add group selector success "
        "on device %d group %x 0x%x\n",
        device,
        pd_grp_hdl,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add group selector failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  pd_status = p4_pd_dc_dtel_mirror_session_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session delete success "
        "on device %d 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_delete_group(
    switch_device_t device, switch_pd_grp_hdl_t pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(pd_grp_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  pd_status = p4_pd_dc_dtel_selector_action_profile_del_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete group failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session delete group success "
        "on device %d 0x%x\n",
        device,
        pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete group failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_add_member(
    switch_device_t device,
    switch_mirror_id_t mirror_id,
    switch_pd_mbr_hdl_t *pd_mbr_hdl,
    switch_pd_grp_hdl_t pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(mirror_id);
  UNUSED(pd_mbr_hdl);
  UNUSED(pd_grp_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  switch_pd_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_set_mirror_session_action_spec_t action_spec;
  action_spec.action_mirror_id = mirror_id;
  pd_status =
      p4_pd_dc_dtel_selector_action_profile_add_member_with_set_mirror_session(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session group member failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_dtel_selector_action_profile_add_member_to_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, *pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add member to group failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session add member to group success "
        "on device %d group0x%x 0x%x\n",
        device,
        pd_grp_hdl,
        *pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session add member to group failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_mirror_session_delete_member(
    switch_device_t device,
    switch_pd_mbr_hdl_t pd_mbr_hdl,
    switch_pd_grp_hdl_t pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(pd_mbr_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_REPORT_LB_ENABLE

  pd_status = p4_pd_dc_dtel_selector_action_profile_del_member_from_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete member from group failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_dtel_selector_action_profile_del_member(
      switch_cfg_sess_hdl, device, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session deleting a group member failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_REPORT_LB_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel mirror session delete a member from group success "
        "on device %d group0x%x 0x%x\n",
        device,
        pd_grp_hdl,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel mirror session delete a member from group failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

//  if P4_DTEL_QUEUE_REPORT_ENABLE, sets default in dtel_queue_alert table,
//  else sets the default entry in dtel_mask_latency table
switch_status_t switch_pd_dtel_quantize_latency_set(
    switch_device_t device,
    switch_uint8_t quant_shift,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(quant_shift);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD

#if defined(P4_DTEL_QUEUE_REPORT_ENABLE) || \
    defined(P4_DTEL_FLOW_STATE_TRACK_ENABLE) || defined(P4_INT_DIGEST_ENABLE)

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  if (quant_shift > 32) {
    quant_shift = 32;
  }

  p4_pd_dc_run_dtel_mask_latency_action_spec_t action_spec;
  action_spec.action_quantization_mask =
      (switch_uint32_t)(~((1LL << quant_shift) - 1));

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  pd_status =
      p4_pd_dc_dtel_queue_alert_set_default_action_run_dtel_mask_latency(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "dtel_queue_alert set default entry failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#else

  pd_status =
      p4_pd_dc_dtel_mask_latency_set_default_action_run_dtel_mask_latency(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "dtel_mask_latency set default entry failed "
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

#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE || P4_INT_DIGEST_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel latency quantization shift set success "
        "on device %d 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel latency quantization shift failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_bloom_filters_reset(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE

  static uint16_t filter_id = 0;
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

#ifdef P4_INT_ENABLE
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  switch_pd_status_t (*dtel_reset_ig_bfilters[4])() = {
      p4_pd_dc_register_reset_all_dtel_ig_bfilter_reg_1,
      p4_pd_dc_register_reset_all_dtel_ig_bfilter_reg_2,
      p4_pd_dc_register_reset_all_dtel_ig_bfilter_reg_3,
      p4_pd_dc_register_reset_all_dtel_ig_bfilter_reg_4};
#else
  switch_pd_status_t (*dtel_reset_ig_bfilters[4])() = {
      p4_pd_dc_register_reset_dtel_ig_bfilter_reg_1,
      p4_pd_dc_register_reset_dtel_ig_bfilter_reg_2,
      p4_pd_dc_register_reset_dtel_ig_bfilter_reg_3,
      p4_pd_dc_register_reset_dtel_ig_bfilter_reg_4};
#endif  // __TARGET_TOFINO__ && !BMV2TOFINO

  pd_status =
      dtel_reset_ig_bfilters[filter_id](switch_cfg_sess_hdl, p4_pd_device);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel ingress bloom filter reset all failure "
        "on device %d : bloom filter %d\n",
        device,
        filter_id);
    goto cleanup;
  }

#endif  // P4_INT_ENABLE

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  switch_pd_status_t (*dtel_reset_eg_bfilters[4])() = {
      p4_pd_dc_register_reset_all_dtel_eg_bfilter_reg_1,
      p4_pd_dc_register_reset_all_dtel_eg_bfilter_reg_2,
      p4_pd_dc_register_reset_all_dtel_eg_bfilter_reg_3,
      p4_pd_dc_register_reset_all_dtel_eg_bfilter_reg_4};
#else
  switch_pd_status_t (*dtel_reset_eg_bfilters[4])() = {
      p4_pd_dc_register_reset_dtel_eg_bfilter_reg_1,
      p4_pd_dc_register_reset_dtel_eg_bfilter_reg_2,
      p4_pd_dc_register_reset_dtel_eg_bfilter_reg_3,
      p4_pd_dc_register_reset_dtel_eg_bfilter_reg_4};
#endif  // __TARGET_TOFINO__ && !BMV2TOFINO

  pd_status =
      dtel_reset_eg_bfilters[filter_id](switch_cfg_sess_hdl, p4_pd_device);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel egress bloom filter reset all failure "
        "on device %d : bloom filter %d\n",
        device,
        filter_id);
    goto cleanup;
  }

  filter_id = (filter_id + 1) % 4;

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel reset bloom filters success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel reset bloom filters failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_bloom_filters_range_reset(
    switch_device_t device,
    switch_uint16_t range_number,
    switch_uint16_t total_ranges) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(range_number);
  UNUSED(total_ranges);

#ifdef SWITCH_PD

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  uint32_t start, next_start, size;
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  // range_number: [0, total_ranges-1]
  if (range_number >= total_ranges) {
    return status;
  }

  start = ((double)DTEL_BLOOM_FILTER_SIZE / total_ranges) * range_number;
  if (range_number == total_ranges - 1)
    next_start = DTEL_BLOOM_FILTER_SIZE;
  else
    next_start =
        ((double)DTEL_BLOOM_FILTER_SIZE / total_ranges) * (range_number + 1);
  size = next_start - start;

#ifdef P4_INT_ENABLE

  switch_pd_status_t (*dtel_range_reset_ig_bfilters[4])() = {
      p4_pd_dc_register_range_reset_dtel_ig_bfilter_reg_1,
      p4_pd_dc_register_range_reset_dtel_ig_bfilter_reg_2,
      p4_pd_dc_register_range_reset_dtel_ig_bfilter_reg_3,
      p4_pd_dc_register_range_reset_dtel_ig_bfilter_reg_4};

  for (int filter_id = 0; filter_id < 4; filter_id++) {
    pd_status = dtel_range_reset_ig_bfilters[filter_id](
        switch_cfg_sess_hdl, p4_pd_device, start, size);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel ingress bloom filter range reset failure "
          "on device %d : bloom filter %d\n",
          device,
          filter_id);
      goto cleanup;
    }
  }

#endif  // P4_INT_ENABLE

  switch_pd_status_t (*dtel_range_reset_eg_bfilters[4])() = {
      p4_pd_dc_register_range_reset_dtel_eg_bfilter_reg_1,
      p4_pd_dc_register_range_reset_dtel_eg_bfilter_reg_2,
      p4_pd_dc_register_range_reset_dtel_eg_bfilter_reg_3,
      p4_pd_dc_register_range_reset_dtel_eg_bfilter_reg_4};

  for (int filter_id = 0; filter_id < 4; filter_id++) {
    pd_status = dtel_range_reset_eg_bfilters[filter_id](
        switch_cfg_sess_hdl, p4_pd_device, start, size);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "DTel egress bloom filter range reset failure "
          "on device %d : bloom filter %d\n",
          device,
          filter_id);
      goto cleanup;
    }
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif  // __TARGET_TOFINO__ && !BMV2TOFINO

#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel range reset bloom filters success "
        "on device %d 0x%x\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel range reset bloom filters "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_alert_index_set(switch_device_t device,
                                                     switch_dev_port_t port,
                                                     switch_qid_t queue,
                                                     switch_uint16_t index,
                                                     switch_uint8_t quant_shift,
                                                     switch_pd_hdl_t *entry_hdl,
                                                     bool add) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(entry_hdl);
  UNUSED(port);
  UNUSED(queue);
  UNUSED(quant_shift);
  UNUSED(index);
  UNUSED(add);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_dtel_queue_alert_match_spec_t match_spec;
  match_spec.eg_intr_md_egress_port = port;
  match_spec.ig_intr_md_for_tm_qid = queue;

  p4_pd_dc_run_dtel_queue_alert_action_spec_t action_spec;
  action_spec.action_index = index;
  if (quant_shift > 32) {
    quant_shift = 32;
  }
  action_spec.action_quantization_mask =
      (switch_uint32_t)(~((1LL << quant_shift) - 1));

  if (add) {
    pd_status = p4_pd_dc_dtel_queue_alert_table_add_with_run_dtel_queue_alert(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
  } else {
    pd_status =
        p4_pd_dc_dtel_queue_alert_table_modify_with_run_dtel_queue_alert(
            switch_cfg_sess_hdl, device, *entry_hdl, &action_spec);
  }
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert add an alert failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue alert add success "
        "on device %d 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert add failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_alert_index_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  pd_status = p4_pd_dc_dtel_queue_alert_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert delete an alert index failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("DTel queue alert delete success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert delete failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_set_queue_alert_threshold(
    switch_device_t device,
    switch_uint16_t index,
    switch_uint32_t queue_depth,
    switch_uint32_t queue_latency) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(index);
  UNUSED(queue_depth);
  UNUSED(queue_latency);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_dtel_queue_alert_threshold_value_t value;
  value.f0 = queue_latency;
  value.f1 = queue_depth;
  pd_status = p4_pd_dc_register_write_dtel_queue_alert_threshold(
      switch_cfg_sess_hdl, p4_pd_device, index, &value);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert set queue alert threshold register failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue alert set threshold success "
        "on device %d index %d\n",
        index);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert set threshold failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_change_reset(switch_device_t device,
                                                  switch_uint16_t index) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(index);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  switch_uint32_t value = 0;
  pd_status = p4_pd_dc_register_write_dtel_queue_change_reg(
      switch_cfg_sess_hdl, p4_pd_device, index, &value);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue change reset failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue change reset success "
        "on device %d index %d\n",
        device,
        index);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue change reset failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_report_quota_set(switch_device_t device,
                                                      switch_uint16_t index,
                                                      switch_uint16_t quota) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(index);
  UNUSED(quota);
#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_dtel_queue_report_quota_reg_value_t value;
  value.f0 = quota;
  value.f1 = quota;
  pd_status = p4_pd_dc_register_write_dtel_queue_report_quota_reg(
      switch_cfg_sess_hdl, p4_pd_device, index, &value);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue report set quota register failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue change reset success "
        "on device %d index %d\n",
        device,
        index);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue change reset failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_remaining_report_quota_during_breach_get(
    switch_device_t device, switch_uint16_t index, switch_uint16_t *quota) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(index);
  UNUSED(quota);

#ifdef SWITCH_PD
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_dtel_queue_report_quota_reg_value_t _quotas[4];
  int read_values;
  pd_status =
      p4_pd_dc_register_read_dtel_queue_report_quota_reg(switch_cfg_sess_hdl,
                                                         p4_pd_device,
                                                         index,
                                                         REGISTER_READ_HW_SYNC,
                                                         _quotas,
                                                         &read_values);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue alert get quota failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  // return minimum remaining quota as that should be the pipe of that port
  *quota = 0xffff;
  for (int i = 0; i < read_values; i++) {
    if (*quota > (switch_uint16_t)_quotas[i].f1) {
      *quota = (switch_uint16_t)_quotas[i].f1;
    }
  }

cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("Queue report get quota success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Queue report get quota failed on device %d : %s (pd: "
        "0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_queue_report_dod_quota_add(
    switch_device_t device,
    switch_dev_port_t port,
    switch_qid_t queue,
    switch_uint16_t index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(entry_hdl);
  UNUSED(port);
  UNUSED(queue);
  UNUSED(index);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_dtel_queue_report_dod_quota_match_spec_t match_spec;
  match_spec.ig_intr_md_for_tm_ucast_egress_port = port;
  match_spec.ig_intr_md_for_tm_qid = queue;

  p4_pd_dc_dtel_update_dod_quota_action_spec_t action_spec;
  action_spec.action_index = index;

  pd_status =
      p4_pd_dc_dtel_queue_report_dod_quota_table_add_with_dtel_update_dod_quota(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue quota dod add an entry failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue quota dod add an entry success "
        "on device %d 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue quota dod add an entry failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_report_dod_quota_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  pd_status = p4_pd_dc_dtel_queue_report_dod_quota_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel queue quota dod delete an entry failed "
        "on device %d : table %s action %s 0x%x\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0),
        entry_hdl);
    goto cleanup;
  }

cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("Queue report quota dod success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Queue report quota dod failed on device %d : %s (pd: "
        "0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_deflect_on_drop_queue_config_add(
    switch_device_t device,
    switch_dev_port_t port,
    switch_qid_t queue,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(entry_hdl);
  UNUSED(port);
  UNUSED(queue);

#ifdef SWITCH_PD

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  p4_pd_dc_deflect_on_drop_queue_config_match_spec_t match_spec;
  match_spec.ig_intr_md_for_tm_ucast_egress_port = port;
  match_spec.ig_intr_md_for_tm_qid = queue;

  pd_status =
      p4_pd_dc_deflect_on_drop_queue_config_table_add_with_queue_dod_enb(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel dod queue add an entry failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel dod queue add an entry success "
        "on device %d 0x%x\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel dod queue add an entry failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_deflect_on_drop_queue_config_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  pd_status = p4_pd_dc_deflect_on_drop_queue_config_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
#endif  // SWITCH_PD
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "Deflect on Drop delete queue_config success on device %d\n", device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "Deflect on Drop delete queue_config failed on device %d : %s (pd: "
        "0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dtel_ig_port_convert_set(switch_device_t device,
                                                   switch_port_t in_port,
                                                   switch_port_t out_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(in_port);
  UNUSED(out_port);

#ifdef SWITCH_PD

#if defined(P4_INT_EP_ENABLE) || defined(P4_POSTCARD_ENABLE) || \
    defined(P4_DTEL_DROP_REPORT_ENABLE) ||                      \
    defined(P4_DTEL_QUEUE_REPORT_ENABLE)
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  switch_pd_hdl_t entry_hdl;

  p4_pd_dc_dtel_ig_port_convert_match_spec_t match_spec;
  p4_pd_dc_ig_port_convert_action_spec_t action_spec;
  match_spec.ingress_metadata_ingress_port = in_port;
  action_spec.action_port = out_port;
  pd_status = p4_pd_dc_dtel_ig_port_convert_table_add_with_ig_port_convert(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel ingress_port convert add entery failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif  // INT EP || MOD || POSTCARD || STATELESS

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("DTel ingress_port convert add success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel ingress_port add failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_eg_port_convert_set(switch_device_t device,
                                                   switch_port_t in_port,
                                                   switch_port_t out_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
  UNUSED(in_port);
  UNUSED(out_port);

#ifdef SWITCH_PD

#if defined(P4_INT_EP_ENABLE) || defined(P4_POSTCARD_ENABLE) || \
    defined(P4_DTEL_DROP_REPORT_ENABLE) ||                      \
    defined(P4_DTEL_QUEUE_REPORT_ENABLE)
  switch_pd_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  switch_pd_hdl_t entry_hdl;

  p4_pd_dc_dtel_eg_port_convert_match_spec_t match_spec;
  p4_pd_dc_eg_port_convert_action_spec_t action_spec;
  match_spec.egress_metadata_egress_port = in_port;
  action_spec.action_port = out_port;
  pd_status = p4_pd_dc_dtel_eg_port_convert_table_add_with_eg_port_convert(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "DTel egress_port convert add entery failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif  // INT EP || MOD || POSTCARD || STATELESS

#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("DTel egress_port convert add success on device %d\n",
                        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel egress_port add failure "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dtel_queue_latency_shift_set(switch_device_t device,
                                                       switch_uint8_t shift) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);
  UNUSED(device);
  UNUSED(shift);

#ifdef SWITCH_PD
#ifdef P4_DTEL_REPORT_ENABLE
  // max shift size is 14
  if (shift > 14) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  pd_status = p4_pd_tm_set_timestamp_shift(device, shift);
  status = switch_pd_status_to_status(pd_status);

cleanup:
#endif  // P4_DTEL_REPORT_ENABLE
#endif  // SWITCH_PD

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "DTel queue latency shift set succeed"
        "on device %d, shift value %u\n",
        device,
        shift);
  } else {
    SWITCH_PD_LOG_ERROR(
        "DTel queue latency shift failure "
        "on device %d shift value %u: %s (pd: 0x%x)\n",
        device,
        shift,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}
