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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_sflow_tables_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  switch_pd_hdl_t entry_hdl;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_sflow_ingress_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "sflow table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
  pd_status = p4_pd_dc_sflow_config_set_default_action_set_sflow_parameters(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "sflow table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "switch config params table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "switch config params table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_sflow_ingress_table_add(
    switch_device_t device,
    switch_sflow_match_key_t *match_key,
    switch_port_lag_index_t port_lag_index,
    switch_uint16_t priority,
    switch_uint32_t sample_rate,
    switch_sflow_info_t *sflow_info,
    switch_sflow_match_entry_t *match_entry) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(match_key);
  UNUSED(priority);
  UNUSED(sflow_info);
  UNUSED(match_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_sflow_ingress_match_spec_t match_spec;
  p4_pd_dc_sflow_ing_pkt_to_cpu_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

  action_spec.action_sflow_i2e_mirror_id =
      handle_to_id(sflow_info->mirror_handle);
  action_spec.action_session_id = sflow_info->session_id;

  match_spec.sflow_valid = 0;
  if (match_key->port != SWITCH_API_INVALID_HANDLE) {
    match_spec.ingress_metadata_port_lag_index = port_lag_index;
    match_spec.ingress_metadata_port_lag_index_mask = -1;
  }
  if (match_key->vlan) {
    // TBD
  }
  if (match_key->sip) {
    // TBD
  }
  if (match_key->dip) {
    // TBD
  }
  if (sample_rate == 0) {
    // use the sample_rate from the session
    sample_rate = sflow_info->api_info.sample_rate;
  }

  match_spec.sflow_metadata_take_sample_start = 0;
  match_spec.sflow_metadata_take_sample_end =
      (uint8_t)(((uint8_t)-1) / sample_rate);

  pd_status = p4_pd_dc_sflow_ingress_table_add_with_sflow_ing_pkt_to_cpu(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority,
      &action_spec,
      &match_entry->ingress_sflow_pd_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = match_entry->ingress_sflow_pd_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "sflow table ingress add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sflow table entry add success "
        "on device %d 0x%lx\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sflow table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_sflow_ingress_table_delete(
    switch_device_t device, switch_sflow_match_entry_t *match_entry) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(match_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  pd_status = p4_pd_dc_sflow_ingress_table_delete(
      switch_cfg_sess_hdl, device, match_entry->ingress_sflow_pd_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = match_entry->ingress_sflow_pd_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "sflow table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sflow table entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sflow table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mirror_table_sflow_add(
    switch_device_t device, switch_sflow_info_t *sflow_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_mirror_match_spec_t match_spec;
  p4_pd_dc_sflow_pkt_to_cpu_action_spec_t action_spec;

  if ((sflow_info->api_info.collector_type != SFLOW_COLLECTOR_TYPE_CPU) ||
      (sflow_info->api_info.sample_mode != SWITCH_SFLOW_SAMPLE_PKT)) {
    // if collector == REMOTE : set action as sflow_pkt_to_remote - TBD
    return status;
  }

  // if collector == CPU : set action as sflow_pkt_to_cpu
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  match_spec.i2e_metadata_mirror_session_id =
      handle_to_id(sflow_info->mirror_handle);
  action_spec.action_reason_code = SWITCH_HOSTIF_REASON_CODE_SFLOW_SAMPLE;
  status = p4_pd_dc_mirror_table_add_with_sflow_pkt_to_cpu(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &action_spec,
      &sflow_info->mirror_table_ent_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = sflow_info->mirror_table_ent_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mirror table sflow add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror table sflow entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror table sflow entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_sflow_session_create(
    switch_device_t device, switch_sflow_info_t *sflow_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(sflow_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  if ((sflow_info->api_info.collector_type == SFLOW_COLLECTOR_TYPE_CPU) &&
      (sflow_info->api_info.sample_mode == SWITCH_SFLOW_SAMPLE_PKT)) {
    // For sflow to cpu, program mirror table in egress pipeline to
    // send pkt to CPU
    pd_status = switch_pd_mirror_table_sflow_add(device, sflow_info);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "sflow session create table add failed "
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

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sflow session create table entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sflow session create table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_sflow_session_delete(
    switch_device_t device, switch_sflow_info_t *sflow_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(sflow_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SFLOW_ENABLE

  if (sflow_info->mirror_table_ent_hdl != SWITCH_PD_INVALID_HANDLE) {
    pd_status = p4_pd_dc_mirror_table_delete(
        switch_cfg_sess_hdl, device, sflow_info->mirror_table_ent_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "sflow session table entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    sflow_info->mirror_table_ent_hdl = SWITCH_PD_INVALID_HANDLE;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SFLOW_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sflow session table entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sflow session table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

#ifdef __cplusplus
}
#endif
