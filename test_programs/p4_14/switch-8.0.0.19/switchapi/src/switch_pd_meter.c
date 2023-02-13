
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
#include <math.h>

switch_status_t switch_pd_meter_counters_get(switch_device_t device,
                                             switch_meter_info_t *meter_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE) && !defined(P4_STATS_DISABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;
  switch_uint32_t index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  for (index = 0; index < SWITCH_COLOR_MAX; index++) {
    pd_status =
        p4_pd_dc_counter_read_meter_stats(switch_cfg_sess_hdl,
                                          p4_pd_device,
                                          meter_info->action_pd_hdl[index],
                                          switch_pd_counter_read_flags(device),
                                          &counter);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Reading meter stats failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      status = switch_pd_status_to_status(pd_status);
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return status;
    }
    meter_info->counters[index].num_packets = counter.packets;
    meter_info->counters[index].num_bytes = counter.bytes;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE && P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("meter stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_storm_control_stats_get(
    switch_device_t device, switch_meter_info_t *meter_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE) && !defined(P4_STATS_DISABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;
  switch_uint32_t index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  for (index = 0; index < SWITCH_METER_COUNTER_MAX; index++) {
    pd_status = p4_pd_dc_counter_read_storm_control_stats(
        switch_cfg_sess_hdl,
        p4_pd_device,
        meter_info->action_pd_hdl[index],
        switch_pd_counter_read_flags(device),
        &counter);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Reading storm control stats failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      status = switch_pd_status_to_status(pd_status);
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return status;
    }
    meter_info->counters[index].num_packets = counter.packets;
    meter_info->counters[index].num_bytes = counter.bytes;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STORM_CONTROL_DISABLE && P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("storm control stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_storm_control_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  switch_pd_hdl_t entry_hdl;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_storm_control_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "storm control table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef __TARGET_TOFINO__
#ifndef BMV2

  pd_status = p4_pd_dc_storm_control_stats_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "storm control stats table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* BMV2 */
#endif /* __TARGET_TOFINO__ */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STORM_CONTROL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "storm control table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "strom control table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_storm_control_meter_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_idx);
  UNUSED(meter_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  switch_api_meter_t *api_meter_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  api_meter_info = &meter_info->api_meter_info;
  if (api_meter_info->meter_type == SWITCH_METER_TYPE_BYTES) {
    p4_pd_bytes_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_bytes_meter_spec_t));
    meter_spec.cir_kbps = api_meter_info->cir;
    meter_spec.cburst_kbits = api_meter_info->cbs;
    meter_spec.pir_kbps = api_meter_info->pir;
    meter_spec.pburst_kbits = api_meter_info->pbs;
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
    pd_status = p4_pd_dc_meter_set_storm_control_meter(
        switch_cfg_sess_hdl, p4_pd_device, meter_idx, &meter_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "storm control meter add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  } else {
    p4_pd_packets_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_packets_meter_spec_t));
    meter_spec.cir_pps = api_meter_info->cir;
    meter_spec.cburst_pkts = api_meter_info->cbs;
    meter_spec.pir_pps = api_meter_info->pir;
    meter_spec.pburst_pkts = api_meter_info->pbs;
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
    (void)meter_spec;
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "storm control meter add failed "
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

#endif /* P4_STORM_CONTROL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "storm control meter entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "strom control meter entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_storm_control_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_uint16_t priority,
    switch_packet_type_t pkt_type,
    switch_meter_id_t meter_idx,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(priority);
  UNUSED(pkt_type);
  UNUSED(meter_idx);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE)

  p4_pd_dc_storm_control_match_spec_t match_spec;
  p4_pd_dc_set_storm_control_meter_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_storm_control_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_storm_control_meter_action_spec_t));

  match_spec.l2_metadata_lkp_pkt_type = pkt_type;
  match_spec.l2_metadata_lkp_pkt_type_mask = 0xFF;
  match_spec.ig_intr_md_ingress_port = dev_port;

  action_spec.action_meter_idx = meter_idx;

  pd_status = p4_pd_dc_storm_control_table_add_with_set_storm_control_meter(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority,
      &action_spec,
      entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "strom control table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STORM_CONTROL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "storm control table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "storm control table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_storm_control_table_entry_update(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t priority,
    switch_packet_type_t pkt_type,
    switch_meter_id_t meter_idx,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);
  UNUSED(priority);
  UNUSED(pkt_type);
  UNUSED(meter_idx);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE)

  p4_pd_dc_set_storm_control_meter_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_storm_control_meter_action_spec_t));

  action_spec.action_meter_idx = meter_idx;

  pd_status = p4_pd_dc_storm_control_table_modify_with_set_storm_control_meter(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "strom control table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STORM_CONTROL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "storm control table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "storm control table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_storm_control_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STORM_CONTROL_DISABLE)

  pd_status = p4_pd_dc_storm_control_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "storm control table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STORM_CONTROL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "storm control table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "storm control table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_meter_index_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_bytes_meter_spec_t meter_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_bytes_meter_spec_t));
  meter_spec.meter_type = PD_METER_TYPE_COLOR_UNAWARE;
  pd_status = p4_pd_dc_meter_index_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &meter_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter index table entry default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry default add failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_meter_index_table_default_entry_delete(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  UNUSED(p4_pd_device);

  /*
  pd_status = p4_pd_dc_meter_index_reset_default_entry(
                           switch_cfg_sess_hdl,
                           p4_pd_device);
  */
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter index table entry default delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry default delete success "
        "on device %d 0x%lx\n",
        device);
  } else {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry default delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_uint32_t switch_meter_bytes_to_kbps(switch_uint64_t bytes) {
  return ceil(((double)(bytes * 8)) / 1000);
}

switch_status_t switch_pd_meter_index_table_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_idx);
  UNUSED(meter_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dc_meter_index_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;
  switch_api_meter_t *api_meter_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_meter_index_match_spec_t));

  match_spec.meter_metadata_meter_index = meter_idx;

  api_meter_info = &meter_info->api_meter_info;
  if (api_meter_info->meter_type == SWITCH_METER_TYPE_BYTES) {
    p4_pd_bytes_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_bytes_meter_spec_t));
    meter_spec.cburst_kbits = api_meter_info->cbs;
    /*
     * convert cir and pir from bytes to kbps.
     */
    meter_spec.cir_kbps = switch_meter_bytes_to_kbps(api_meter_info->cir);
    meter_spec.pir_kbps = switch_meter_bytes_to_kbps(api_meter_info->pir);
    meter_spec.pburst_kbits = switch_meter_bytes_to_kbps(api_meter_info->pbs);
    meter_spec.cburst_kbits = switch_meter_bytes_to_kbps(api_meter_info->cbs);
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
    pd_status = p4_pd_dc_meter_index_table_add_with_nop(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, &meter_spec, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "meter index table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  } else {
    p4_pd_packets_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_packets_meter_spec_t));
    /*
     * convert cir and pir from bytes to kbps.
     */
    meter_spec.cir_pps = api_meter_info->cir;
    meter_spec.cburst_pkts = api_meter_info->cbs;
    meter_spec.pir_pps = api_meter_info->pir;
    meter_spec.pburst_pkts = api_meter_info->pbs;
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
    (void)meter_spec;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "meter index table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_meter_index_table_entry_update(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_idx);
  UNUSED(meter_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  switch_api_meter_t *api_meter_info = NULL;

  api_meter_info = &meter_info->api_meter_info;
  if (api_meter_info->meter_type == SWITCH_METER_TYPE_BYTES) {
    p4_pd_bytes_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_bytes_meter_spec_t));
    meter_spec.cir_kbps = switch_meter_bytes_to_kbps(api_meter_info->cir);
    meter_spec.pir_kbps = switch_meter_bytes_to_kbps(api_meter_info->pir);
    meter_spec.cburst_kbits = switch_meter_bytes_to_kbps(api_meter_info->cbs);
    meter_spec.pburst_kbits = switch_meter_bytes_to_kbps(api_meter_info->pbs);
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;

    pd_status = p4_pd_dc_meter_index_table_modify_with_nop(
        switch_cfg_sess_hdl, device, entry_hdl, &meter_spec);
  } else {
    p4_pd_packets_meter_spec_t meter_spec;
    SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_bytes_meter_spec_t));
    meter_spec.cir_pps = api_meter_info->cir;
    meter_spec.cburst_pkts = api_meter_info->cbs;
    meter_spec.pir_pps = api_meter_info->pir;
    meter_spec.pburst_pkts = api_meter_info->pbs;
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
    /*
    pd_status = p4_pd_dc_meter_index_table_modify_with_nop(
                         switch_cfg_sess_hdl,
                         device,
                         entry_hdl,
                         &meter_spec);
    */
    (void)meter_spec;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_meter_index_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  pd_status =
      p4_pd_dc_meter_index_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter index table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter index table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "meter index table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_meter_action_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_meter_action_set_default_action_meter_permit(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter action table entry default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry default add failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_meter_action_table_default_entry_delete(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  UNUSED(p4_pd_device);

  /*
  pd_status = p4_pd_dc_meter_action_reset_default_entry(
                           switch_cfg_sess_hdl,
                           p4_pd_device);
  */
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter action table entry default delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry default delete success "
        "on device %d 0x%lx\n",
        device);
  } else {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry default delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_meter_action_table_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_idx);
  UNUSED(meter_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  p4_pd_dc_meter_action_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;
  switch_api_meter_t *api_meter_info = NULL;
  switch_color_t color;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  api_meter_info = &meter_info->api_meter_info;

  for (color = 0; color < SWITCH_COLOR_MAX; color++) {
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_meter_index_match_spec_t));
    match_spec.meter_metadata_meter_index = meter_idx;
    match_spec.meter_metadata_packet_color = color;

#ifdef __TARGET_TOFINO__
    /*
     * Tofino expects the value of RED to be 3.
     */
    if (color == SWITCH_COLOR_RED) {
      match_spec.meter_metadata_packet_color = 3;
    }
#endif

    switch (api_meter_info->action[color]) {
      case SWITCH_ACL_ACTION_PERMIT:
        pd_status = p4_pd_dc_meter_action_table_add_with_meter_permit(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl[color]);
        break;
      case SWITCH_ACL_ACTION_DROP:
        pd_status = p4_pd_dc_meter_action_table_add_with_meter_deny(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl[color]);
        break;
      default:
        return SWITCH_STATUS_INVALID_PARAMETER;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
      pd_entry.match_spec = (switch_uint8_t *)&match_spec;
      pd_entry.match_spec_size = sizeof(match_spec);
      pd_entry.action_spec_size = 0;
      pd_entry.pd_hdl = entry_hdl[color];
      switch_pd_entry_dump(device, &pd_entry);
    }

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "meter action table add failed "
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

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "meter action table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_meter_action_table_entry_update(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(meter_idx);
  UNUSED(meter_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  switch_api_meter_t *api_meter_info = NULL;
  switch_color_t color;

  api_meter_info = &meter_info->api_meter_info;

  for (color = 0; color < SWITCH_COLOR_MAX; color++) {
    switch (api_meter_info->action[color]) {
      case SWITCH_ACL_ACTION_PERMIT:
        pd_status = p4_pd_dc_meter_action_table_modify_with_meter_permit(
            switch_cfg_sess_hdl, device, entry_hdl[color]);
        break;
      case SWITCH_ACL_ACTION_DROP:
        pd_status = p4_pd_dc_meter_action_table_modify_with_meter_deny(
            switch_cfg_sess_hdl, device, entry_hdl[color]);
        break;
      default:
        return SWITCH_STATUS_INVALID_PARAMETER;
    }
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_meter_action_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_METERING_ENABLE)

  switch_color_t color;
  for (color = 0; color < SWITCH_COLOR_MAX; color++) {
    pd_status = p4_pd_dc_meter_action_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl[color]);

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
      pd_entry.match_spec_size = 0;
      pd_entry.action_spec_size = 0;
      pd_entry.pd_hdl = entry_hdl[color];
      switch_pd_entry_dump(device, &pd_entry);
    }

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "meter action table entry delete failed "
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

#endif /* P4_QOS_METERING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "meter action table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "meter action table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_switch_config_params_update(
    switch_device_t device, switch_config_params_t *config_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_set_config_parameters_action_spec_t cfg_action;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&cfg_action, 0, sizeof(cfg_action));
  cfg_action.action_switch_id = config_params->switch_id;

#ifdef P4_FLOWLET_ENABLE
  cfg_action.action_enable_flowlet = config_params->inactivity_timeout;
#endif /* P4_FLOWLET_ENABLE */

  pd_status =
      p4_pd_dc_switch_config_params_set_default_action_set_config_parameters(
          switch_cfg_sess_hdl, p4_pd_device, &cfg_action, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "switch config params table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "switch config params table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
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

switch_status_t switch_pd_hostif_meter_set(switch_device_t device,
                                           switch_meter_id_t meter_id,
                                           switch_meter_info_t *meter_info,
                                           bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef __TARGET_TOFINO__
#ifndef P4_COPP_METER_DISABLE

  p4_pd_packets_meter_spec_t meter_spec;
  p4_pd_dev_target_t p4_pd_device;
  switch_api_meter_t *api_meter_info;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  api_meter_info = &(meter_info->api_meter_info);
  SWITCH_MEMSET(&meter_spec, 0, sizeof(p4_pd_packets_meter_spec_t));
  if (enable) {
    meter_spec.cir_pps = api_meter_info->cir;
    meter_spec.pir_pps = api_meter_info->pir;
    meter_spec.cburst_pkts = api_meter_info->cbs;
    meter_spec.pburst_pkts = api_meter_info->pbs;
    meter_spec.meter_type =
        api_meter_info->color_source == SWITCH_METER_COLOR_SOURCE_BLIND
            ? PD_METER_TYPE_COLOR_UNAWARE
            : PD_METER_TYPE_COLOR_AWARE;
  }
  pd_status = p4_pd_dc_meter_set_copp(
      switch_cfg_sess_hdl, p4_pd_device, meter_id, &meter_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "hostif meter set failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_COPP_METER_DISABLE */
#endif /* __TARGET_TOFINO__ */
#endif /* SWITCH_PD */

  return status;
}
