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
#include "switch_pd.h"

switch_status_t switch_pd_wred_early_drop_set(switch_device_t device,
                                              switch_handle_t wred_handle,
                                              switch_wred_info_t *wred_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(wred_handle);
  UNUSED(wred_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_wred_spec_t wred_spec;
  switch_api_wred_info_t *api_wred_info = &wred_info->api_wred_info;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&wred_spec, 0, sizeof(p4_pd_wred_spec_t));
  wred_spec.time_constant = api_wred_info->time_constant;
  wred_spec.red_min_threshold = api_wred_info->min_threshold;
  wred_spec.red_max_threshold = api_wred_info->max_threshold;
  wred_spec.max_probability = api_wred_info->max_probability;

  pd_status = p4_pd_dc_wred_set_wred_early_drop(
      switch_cfg_sess_hdl, p4_pd_device, handle_to_id(wred_handle), &wred_spec);

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_index_table_entry_delete(
    switch_device_t device, p4_pd_entry_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  pd_status =
      p4_pd_dc_wred_index_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_index_table_entry_update(
    switch_device_t device,
    switch_handle_t wred_handle,
    p4_pd_entry_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(wred_handle);
  UNUSED(entry_hdl);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_dc_wred_set_index_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_wred_set_index_action_spec_t));
  action_spec.action_index = handle_to_id(wred_handle);

  pd_status = p4_pd_dc_wred_index_table_modify_with_wred_set_index(
      switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_index_table_entry_add(
    switch_device_t device,
    switch_wred_queue_entry_t *queue_entry,
    switch_handle_t wred_handle,
    switch_handle_t wred_stats_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(queue_entry);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_dc_wred_index_match_spec_t match_spec;
  p4_pd_dc_wred_set_index_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_wred_index_match_spec_t));
  match_spec.ig_intr_md_for_tm_qid = queue_entry->id;
  match_spec.eg_intr_md_egress_port = queue_entry->port;
  match_spec.meter_metadata_packet_color = queue_entry->packet_color;

  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_wred_set_index_action_spec_t));
  action_spec.action_index = handle_to_id(wred_handle);
  action_spec.action_stats_index = handle_to_id(wred_stats_handle);

  pd_status =
      p4_pd_dc_wred_index_table_add_with_wred_set_index(switch_cfg_sess_hdl,
                                                        p4_pd_device,
                                                        &match_spec,
                                                        &action_spec,
                                                        &queue_entry->ent_hdl);

  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_action_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_entry_hdl_t entry_hdl;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  pd_status = p4_pd_dc_wred_action_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_action_table_entry_add(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_wred_info_t *wred_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(wred_handle);
  UNUSED(wred_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_wred_action_match_spec_t match_spec;
  switch_api_wred_info_t *api_wred_info = &wred_info->api_wred_info;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_wred_action_match_spec_t));

  for (int i = 0; i < SWITCH_PD_WRED_ENT_HDLS; i++) {
    wred_info->ent_hdls[i] = SWITCH_API_INVALID_HANDLE;
  }

  /* drop_flag is false */
  match_spec.wred_metadata_index = handle_to_id(wred_handle);
  match_spec.wred_metadata_drop_flag = false;

  pd_status = p4_pd_dc_wred_action_table_add_with_nop(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      2,
                                                      &wred_info->ent_hdls[0]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  /* drop_flag is true, drop mode */
  if (api_wred_info->enable) {
#ifdef P4_WRED_DEBUG
    match_spec.wred_metadata_drop_flag = false;
#else
    match_spec.wred_metadata_drop_flag = true;
#endif

#ifdef P4_WRED_DROP_ENABLE
    pd_status =
        p4_pd_dc_wred_action_table_add_with_wred_drop(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      1,
                                                      &wred_info->ent_hdls[1]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      goto cleanup;
    }
#else
    if (!api_wred_info->ecn_mark) {
      // Only ECN-marking is supported.
      return SWITCH_STATUS_NOT_SUPPORTED;
    }
#endif /* P4_WRED_DROP_ENABLE */

    if (api_wred_info->ecn_mark) {
      /* drop_flag is true, ecn mark mode */
      match_spec.ipv4_valid_mask = 0x1;
      match_spec.ipv6_valid_mask = 0x1;

      /* Ipv4 */
      match_spec.ipv4_valid = 0x1;
      match_spec.ipv6_valid = 0x0;
#ifndef P4_IPV6_DISABLE
      match_spec.ipv6_trafficClass_mask = 0x0;
      match_spec.ipv6_trafficClass = 0x0;
#endif
      match_spec.ipv4_diffserv = 0x00;
      match_spec.ipv4_diffserv_mask = 0x03;

#ifdef P4_WRED_DROP_ENABLE
      pd_status = p4_pd_dc_wred_action_table_add_with_wred_drop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &wred_info->ent_hdls[2]);
#else
      pd_status =
          p4_pd_dc_wred_action_table_add_with_nop(switch_cfg_sess_hdl,
                                                  p4_pd_device,
                                                  &match_spec,
                                                  0,
                                                  &wred_info->ent_hdls[2]);
#endif
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }

      match_spec.ipv4_diffserv_mask = 0x00;
      pd_status = p4_pd_dc_wred_action_table_add_with_set_ipv4_ecn_bits(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &wred_info->ent_hdls[3]);
      if (status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }

#ifndef P4_IPV6_DISABLE
      /* Ipv6 */
      match_spec.ipv4_valid = 0x0;
      match_spec.ipv6_valid = 0x1;
      match_spec.ipv4_diffserv_mask = 0x00;
      match_spec.ipv4_diffserv = 0x00;
      match_spec.ipv6_trafficClass = 0x00;
      match_spec.ipv6_trafficClass_mask = 0x03;
#ifdef P4_WRED_DROP_ENABLE
      pd_status = p4_pd_dc_wred_action_table_add_with_wred_drop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &wred_info->ent_hdls[4]);
#else
      pd_status =
          p4_pd_dc_wred_action_table_add_with_nop(switch_cfg_sess_hdl,
                                                  p4_pd_device,
                                                  &match_spec,
                                                  0,
                                                  &wred_info->ent_hdls[4]);
#endif
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }

      match_spec.ipv6_trafficClass_mask = 0x00;
      pd_status = p4_pd_dc_wred_action_table_add_with_set_ipv6_ecn_bits(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &wred_info->ent_hdls[5]);
#endif
    }
  }

cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_action_table_entry_update(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_wred_info_t *wred_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(wred_handle);
  UNUSED(wred_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_entry_hdl_t ent_hdls[SWITCH_PD_WRED_ENT_HDLS];

  /* Store a copy of the old handles */
  for (int i = 0; i < SWITCH_PD_WRED_ENT_HDLS; i++) {
    ent_hdls[i] = wred_info->ent_hdls[i];
  }

  status =
      switch_pd_wred_action_table_entry_add(device, wred_handle, wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  /* Delete the old entries */
  for (int i = 0; i < SWITCH_PD_WRED_ENT_HDLS; i++) {
    if (ent_hdls[i] != SWITCH_API_INVALID_HANDLE) {
      pd_status = p4_pd_dc_wred_action_table_delete(
          switch_cfg_sess_hdl, device, ent_hdls[i]);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }
    }
  }

cleanup:
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_action_table_entry_delete(
    switch_device_t device, switch_wred_info_t *wred_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(wred_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_entry_hdl_t entry_hdl;

  for (int i = 0; i < SWITCH_PD_WRED_ENT_HDLS; i++) {
    entry_hdl = wred_info->ent_hdls[i];
    if (entry_hdl != SWITCH_API_INVALID_HANDLE) {
      pd_status = p4_pd_dc_wred_action_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        return status;
      }
      wred_info->ent_hdls[i] = SWITCH_API_INVALID_HANDLE;
    }
  }
  p4_pd_complete_operations(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_stats_get(switch_device_t device,
                                         switch_wred_counter_t counter_id,
                                         switch_handle_t stats_hdl,
                                         switch_counter_t *counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_counter_value_t pd_counter;
  pd_counter.bytes = 0;
  pd_counter.packets = 0;

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (counter_id) {
    case SWITCH_WRED_STATS_GREEN_ECN_MARKED:
    case SWITCH_WRED_STATS_YELLOW_ECN_MARKED:
    case SWITCH_WRED_STATS_RED_ECN_MARKED:
      pd_status =
          p4_pd_dc_counter_read_wred_mark(switch_cfg_sess_hdl,
                                          p4_pd_device,
                                          handle_to_id(stats_hdl),
                                          switch_pd_counter_read_flags(device),
                                          &pd_counter);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR("wred stats get failed on device : %d : table %s\n",
                            device,
                            switch_pd_table_id_to_string(0));
        goto cleanup;
      }
      break;
    case SWITCH_WRED_STATS_GREEN_DROPPED:
    case SWITCH_WRED_STATS_YELLOW_DROPPED:
    case SWITCH_WRED_STATS_RED_DROPPED:
      return SWITCH_STATUS_NOT_SUPPORTED;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  counter->num_packets = pd_counter.packets;
  counter->num_bytes = pd_counter.bytes;

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wred_stats_clear(switch_device_t device,
                                           switch_wred_counter_t counter_id,
                                           switch_handle_t stats_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  p4_pd_counter_value_t pd_counter;
  pd_counter.bytes = 0;
  pd_counter.packets = 0;

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (counter_id) {
    case SWITCH_WRED_STATS_GREEN_ECN_MARKED:
    case SWITCH_WRED_STATS_YELLOW_ECN_MARKED:
    case SWITCH_WRED_STATS_RED_ECN_MARKED:
      pd_status = p4_pd_dc_counter_write_wred_mark(switch_cfg_sess_hdl,
                                                   p4_pd_device,
                                                   handle_to_id(stats_hdl),
                                                   pd_counter);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR("wred stats get failed on device : %d : table %s\n",
                            device,
                            switch_pd_table_id_to_string(0));
        goto cleanup;
      }
      break;
    case SWITCH_WRED_STATS_GREEN_DROPPED:
    case SWITCH_WRED_STATS_YELLOW_DROPPED:
    case SWITCH_WRED_STATS_RED_DROPPED:
      return SWITCH_STATUS_NOT_SUPPORTED;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  return status;
}
