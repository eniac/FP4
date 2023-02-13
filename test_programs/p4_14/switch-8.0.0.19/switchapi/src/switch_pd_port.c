
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

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
bf_port_speed_t switch_port_speed_to_pd_port_speed(
    switch_port_speed_t port_speed) {
  switch (port_speed) {
    case SWITCH_PORT_SPEED_NONE:
      return BF_SPEED_NONE;
    case SWITCH_PORT_SPEED_10G:
      return BF_SPEED_10G;
    case SWITCH_PORT_SPEED_25G:
      return BF_SPEED_25G;
    case SWITCH_PORT_SPEED_40G:
      return BF_SPEED_40G;
    case SWITCH_PORT_SPEED_50G:
      return BF_SPEED_50G;
    case SWITCH_PORT_SPEED_100G:
      return BF_SPEED_100G;
    default:
      return BF_SPEED_NONE;
  }
}

bf_rmon_counter_t switch_port_counter_to_pd_port_counter(
    switch_port_counter_id_t counter_id) {
  switch (counter_id) {
    case SWITCH_PORT_STAT_IN_GOOD_OCTETS:
      return bf_mac_stat_OctetsReceivedinGoodFrames;
    case SWITCH_PORT_STAT_IN_ALL_OCTETS:
      return bf_mac_stat_OctetsReceived;
    case SWITCH_PORT_STAT_IN_GOOD_PKTS:
      return bf_mac_stat_FramesReceivedOK;
    case SWITCH_PORT_STAT_IN_ALL_PKTS:
      return bf_mac_stat_FramesReceivedAll;
    case SWITCH_PORT_STAT_IN_VLAN_PKTS:
      return bf_mac_stat_RxVLANFramesGood;
    case SWITCH_PORT_STAT_IN_UCAST_PKTS:
      return bf_mac_stat_FramesReceivedwithUnicastAddresses;
    case SWITCH_PORT_STAT_IN_MCAST_PKTS:
      return bf_mac_stat_FramesReceivedwithMulticastAddresses;
    case SWITCH_PORT_STAT_IN_BCAST_PKTS:
      return bf_mac_stat_FramesReceivedwithBroadcastAddresses;
    case SWITCH_PORT_STAT_IN_FCS_ERRORS:
      return bf_mac_stat_FramesReceivedwithFCSError;
    case SWITCH_PORT_STAT_IN_ERROR_PKTS:
      return bf_mac_stat_FrameswithanyError;
    case SWITCH_PORT_STAT_IN_CRC_ERRORS:
      return bf_mac_stat_CRCErrorStomped;
    case SWITCH_PORT_STAT_IN_BUFFER_FULL:
      return bf_mac_stat_FramesDroppedBufferFull;
    case SWITCH_PORT_STAT_IN_FRAGMENTS:
      return bf_mac_stat_FragmentsReceived;
    case SWITCH_PORT_STAT_IN_JABBERS:
      return bf_mac_stat_JabberReceived;
    case SWITCH_PORT_STAT_OUT_GOOD_OCTETS:
      return bf_mac_stat_OctetsTransmittedwithouterror;
    case SWITCH_PORT_STAT_OUT_ALL_OCTETS:
      return bf_mac_stat_OctetsTransmittedTotal;
    case SWITCH_PORT_STAT_OUT_GOOD_PKTS:
      return bf_mac_stat_FramesTransmittedOK;
    case SWITCH_PORT_STAT_OUT_ALL_PKTS:
      return bf_mac_stat_FramesTransmittedAll;
    case SWITCH_PORT_STAT_OUT_VLAN_PKTS:
      return bf_mac_stat_FramesTransmittedVLAN;
    case SWITCH_PORT_STAT_OUT_UCAST_PKTS:
      return bf_mac_stat_FramesTransmittedUnicast;
    case SWITCH_PORT_STAT_OUT_MCAST_PKTS:
      return bf_mac_stat_FramesTransmittedMulticast;
    case SWITCH_PORT_STAT_OUT_BCAST_PKTS:
      return bf_mac_stat_FramesTransmittedBroadcast;
    case SWITCH_PORT_STAT_OUT_ERROR_PKTS:
      return bf_mac_stat_FramesTransmittedwithError;
    case SWITCH_PORT_STAT_IN_PKTS_LT_64:
      return bf_mac_stat_FramesReceivedLength_lt_64;
    case SWITCH_PORT_STAT_IN_PKTS_EQ_64:
      return bf_mac_stat_FramesReceivedLength_eq_64;
    case SWITCH_PORT_STAT_IN_PKTS_65_TO_127:
      return bf_mac_stat_FramesReceivedLength_65_127;
    case SWITCH_PORT_STAT_IN_PKTS_128_TO_255:
      return bf_mac_stat_FramesReceivedLength_128_255;
    case SWITCH_PORT_STAT_IN_PKTS_256_TO_511:
      return bf_mac_stat_FramesReceivedLength_256_511;
    case SWITCH_PORT_STAT_IN_PKTS_512_TO_1023:
      return bf_mac_stat_FramesReceivedLength_512_1023;
    case SWITCH_PORT_STAT_IN_PKTS_1024_TO_1518:
      return bf_mac_stat_FramesReceivedLength_1024_1518;
    case SWITCH_PORT_STAT_IN_PKTS_1519_TO_2047:
      return bf_mac_stat_FramesReceivedLength_2048_4095;
    case SWITCH_PORT_STAT_IN_PKTS_2048_TO_4095:
      return bf_mac_stat_FramesReceivedLength_2048_4095;
    case SWITCH_PORT_STAT_IN_PKTS_4096_TO_8191:
      return bf_mac_stat_FramesReceivedLength_4096_8191;
    case SWITCH_PORT_STAT_IN_PKTS_8192_TO_9215:
      return bf_mac_stat_FramesReceivedLength_8192_9215;
    case SWITCH_PORT_STAT_IN_PKTS_9216:
      return bf_mac_stat_FramesReceivedLength_9216;
    case SWITCH_PORT_STAT_OUT_PKTS_LT_64:
      return bf_mac_stat_FramesTransmittedLength_lt_64;
    case SWITCH_PORT_STAT_OUT_PKTS_EQ_64:
      return bf_mac_stat_FramesTransmittedLength_eq_64;
    case SWITCH_PORT_STAT_OUT_PKTS_65_TO_127:
      return bf_mac_stat_FramesTransmittedLength_65_127;
    case SWITCH_PORT_STAT_OUT_PKTS_128_TO_255:
      return bf_mac_stat_FramesTransmittedLength_128_255;
    case SWITCH_PORT_STAT_OUT_PKTS_256_TO_511:
      return bf_mac_stat_FramesTransmittedLength_256_511;
    case SWITCH_PORT_STAT_OUT_PKTS_512_TO_1023:
      return bf_mac_stat_FramesTransmittedLength_512_1023;
    case SWITCH_PORT_STAT_OUT_PKTS_1024_TO_1518:
      return bf_mac_stat_FramesTransmittedLength_1024_1518;
    case SWITCH_PORT_STAT_OUT_PKTS_1519_TO_2047:
      return bf_mac_stat_FramesTransmittedLength_1519_2047;
    case SWITCH_PORT_STAT_OUT_PKTS_2048_TO_4095:
      return bf_mac_stat_FramesTransmittedLength_2048_4095;
    case SWITCH_PORT_STAT_OUT_PKTS_4096_TO_8191:
      return bf_mac_stat_FramesTransmittedLength_4096_8191;
    case SWITCH_PORT_STAT_OUT_PKTS_8192_TO_9215:
      return bf_mac_stat_FramesTransmittedLength_8192_9215;
    case SWITCH_PORT_STAT_OUT_PKTS_9216:
      return bf_mac_stat_FramesTransmittedLength_9216;
    case SWITCH_PORT_STAT_IN_PFC_0_PKTS:
      return bf_mac_stat_Pri0FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_1_PKTS:
      return bf_mac_stat_Pri1FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_2_PKTS:
      return bf_mac_stat_Pri2FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_3_PKTS:
      return bf_mac_stat_Pri3FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_4_PKTS:
      return bf_mac_stat_Pri4FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_5_PKTS:
      return bf_mac_stat_Pri5FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_6_PKTS:
      return bf_mac_stat_Pri6FramesReceived;
    case SWITCH_PORT_STAT_IN_PFC_7_PKTS:
      return bf_mac_stat_Pri7FramesReceived;
    case SWITCH_PORT_STAT_IN_OVER_SIZED_PKTS:
      return bf_mac_stat_FramesReceivedOversized;
    case SWITCH_PORT_STAT_OUT_PFC_0_PKTS:
      return bf_mac_stat_Pri0FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_1_PKTS:
      return bf_mac_stat_Pri1FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_2_PKTS:
      return bf_mac_stat_Pri2FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_3_PKTS:
      return bf_mac_stat_Pri3FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_4_PKTS:
      return bf_mac_stat_Pri4FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_5_PKTS:
      return bf_mac_stat_Pri5FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_6_PKTS:
      return bf_mac_stat_Pri6FramesTransmitted;
    case SWITCH_PORT_STAT_OUT_PFC_7_PKTS:
      return bf_mac_stat_Pri7FramesTransmitted;
    case SWITCH_PORT_STAT_IN_UNDER_SIZED_PKTS:
      return bf_mac_stat_FramesReceivedUndersized;
    case SWITCH_PORT_STAT_IN_FRAMES_TOO_LONG:
      return bf_mac_stat_FrameTooLong;
    default:
      return 0;
  }
}

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

switch_status_t switch_pd_port_add(switch_device_t device,
                                   switch_dev_port_t dev_port,
                                   switch_port_speed_t port_speed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(port_speed);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  bf_port_speed_t pd_port_speed = BF_SPEED_NONE;
  pd_port_speed = switch_port_speed_to_pd_port_speed(port_speed);
  pd_status = bf_pal_port_add(device, dev_port, pd_port_speed, 0x0);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  /*
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  cleanup:

  */
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_delete(switch_device_t device,
                                      switch_dev_port_t dev_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_del(device, dev_port);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  /*
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  cleanup:
  */

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_enable(switch_device_t device,
                                      switch_dev_port_t dev_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_enable(device, dev_port);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port enable failed "
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

  return status;
}

switch_status_t switch_pd_port_disable(switch_device_t device,
                                       switch_dev_port_t dev_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_disable(device, dev_port);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port disable failed "
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

  return status;
}

switch_status_t switch_pd_port_stats_get(switch_device_t device,
                                         switch_dev_port_t dev_port,
                                         switch_uint16_t num_entries,
                                         switch_port_counter_id_t *counter_ids,
                                         switch_uint64_t *counters) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(num_entries);
  UNUSED(counter_ids);
  UNUSED(counters);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  uint64_t rmon_counters[BF_NUM_RMON_COUNTERS];
  bf_rmon_counter_t rmon_counter_id = 0;
  switch_uint16_t index = 0;
  SWITCH_MEMSET(rmon_counters, 0x0, sizeof(rmon_counters));

  pd_status = bf_pal_port_all_stats_get(device, dev_port, rmon_counters);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port stats get failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  for (index = 0; index < num_entries; index++) {
    rmon_counter_id =
        switch_port_counter_to_pd_port_counter(counter_ids[index]);
    counters[index] = rmon_counters[rmon_counter_id];
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && !BMV2TOFINO */
#endif /* SWITCH_ID */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "port stats get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "port stats get failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_port_stats_counter_id_clear(
    const switch_device_t device,
    const switch_dev_port_t dev_port,
    const switch_uint16_t num_counters,
    const switch_port_counter_id_t *counter_ids) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(num_counters);
  UNUSED(counter_ids);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  switch_uint16_t index = 0;
  bf_rmon_counter_t rmon_counter_id = 0;

  for (index = 0; index < num_counters; index++) {
    rmon_counter_id =
        switch_port_counter_to_pd_port_counter(counter_ids[index]);
    pd_status = bf_pal_port_this_stat_clear(device, dev_port, rmon_counter_id);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "port stats clear all failed"
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

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_stats_clear_all(switch_device_t device,
                                               switch_dev_port_t dev_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_all_stats_clear(device, dev_port);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port stats clear all failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ingress_port_mapping_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_index_t port_lag_index,
    switch_port_type_t port_type,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(port_lag_index);
  UNUSED(port_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_ingress_port_mapping_match_spec_t match_spec = {0};
  p4_pd_dc_set_port_lag_index_action_spec_t action_spec = {0};
  p4_pd_dev_target_t p4_pd_device = {0};

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_ingress_port_mapping_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_port_lag_index_action_spec_t));

  match_spec.ig_intr_md_ingress_port = dev_port;
  action_spec.action_port_lag_index = port_lag_index;
  action_spec.action_port_type = port_type;

  pd_status = p4_pd_dc_ingress_port_mapping_table_add_with_set_port_lag_index(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port mapping entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ingress_port_mapping_table_entry_update(
    switch_device_t device,
    switch_port_t port_id,
    switch_port_lag_index_t port_lag_index,
    switch_port_type_t port_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port_id);
  UNUSED(port_lag_index);
  UNUSED(port_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_set_port_lag_index_action_spec_t action_spec = {0};

  action_spec.action_port_lag_index = port_lag_index;
  action_spec.action_port_type = port_type;

  pd_status =
      p4_pd_dc_ingress_port_mapping_table_modify_with_set_port_lag_index(
          switch_cfg_sess_hdl, device, entry_hdl, &action_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port mapping entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ingress_port_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_ingress_port_mapping_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port mapping entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port mapping entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ingress_port_properties_table_entry_add(
    switch_device_t device,
    switch_yid_t yid,
    switch_port_info_t *port_info,
    switch_port_lag_label_t port_lag_label,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(yid);
  UNUSED(port_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_ingress_port_properties_match_spec_t match_spec;
  p4_pd_dc_set_ingress_port_properties_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_ingress_port_mapping_match_spec_t));
  SWITCH_MEMSET(&action_spec,
                0x0,
                sizeof(p4_pd_dc_set_ingress_port_properties_action_spec_t));

  match_spec.ig_intr_md_ingress_port = port_info->dev_port;
  action_spec.action_port_lag_label = port_lag_label;
  action_spec.action_exclusion_id = yid;
  action_spec.action_qos_group = port_info->ingress_qos_group;
#ifndef SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
  action_spec.action_tc_qos_group = port_info->tc_qos_group;
#endif
  action_spec.action_trust_dscp = port_info->trust_dscp;
  action_spec.action_trust_pcp = port_info->trust_pcp;
  action_spec.action_learning_enabled = port_info->learning_enabled;

  pd_status =
      p4_pd_dc_ingress_port_properties_table_add_with_set_ingress_port_properties(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port properties entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ingress_port_properties_table_entry_update(
    switch_device_t device,
    switch_yid_t yid,
    switch_port_info_t *port_info,
    switch_port_lag_label_t port_lag_label,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(yid);
  UNUSED(port_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_set_ingress_port_properties_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec,
                0x0,
                sizeof(p4_pd_dc_set_ingress_port_properties_action_spec_t));

  action_spec.action_port_lag_label = port_lag_label;
  action_spec.action_exclusion_id = yid;
  action_spec.action_qos_group = port_info->ingress_qos_group;
#ifndef SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
  action_spec.action_tc_qos_group = port_info->tc_qos_group;
#endif
  action_spec.action_trust_dscp = port_info->trust_dscp;
  action_spec.action_trust_pcp = port_info->trust_pcp;
  action_spec.action_learning_enabled = port_info->learning_enabled;

  pd_status =
      p4_pd_dc_ingress_port_properties_table_modify_with_set_ingress_port_properties(
          switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port properties entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ingress_port_properties_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_ingress_port_properties_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ingress port properties entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress port properties entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_port_mapping_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_label_t port_lag_label,
    switch_port_type_t port_type,
    switch_qos_group_t qos_group,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(port_lag_label);
  UNUSED(port_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_egress_port_mapping_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_egress_port_mapping_match_spec_t));
  match_spec.eg_intr_md_egress_port = dev_port;
  if (port_type == SWITCH_PORT_TYPE_NORMAL) {
    p4_pd_dc_egress_port_type_normal_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_qos_group = qos_group,
    action_spec.action_port_lag_label = port_lag_label;

    pd_status =
        p4_pd_dc_egress_port_mapping_table_add_with_egress_port_type_normal(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#ifdef P4_FABRIC_ENABLE
  } else if (port_type == SWITCH_PORT_TYPE_FABRIC) {
    pd_status =
        p4_pd_dc_egress_port_mapping_table_add_with_egress_port_type_fabric(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
      pd_entry.match_spec = (switch_uint8_t *)&match_spec;
      pd_entry.match_spec_size = sizeof(match_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_FABRIC_ENABLE */
  } else if (port_type == SWITCH_PORT_TYPE_CPU) {
    pd_status =
        p4_pd_dc_egress_port_mapping_table_add_with_egress_port_type_cpu(
            switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
      pd_entry.match_spec = (switch_uint8_t *)&match_spec;
      pd_entry.match_spec_size = sizeof(match_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress port mapping entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_port_mapping_table_entry_update(
    switch_device_t device,
    switch_port_t port_id,
    switch_port_lag_label_t port_lag_label,
    switch_port_type_t port_type,
    switch_qos_group_t qos_group,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (port_type == SWITCH_PORT_TYPE_NORMAL) {
    p4_pd_dc_egress_port_type_normal_action_spec_t action_spec = {0};
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_qos_group = qos_group;
    action_spec.action_port_lag_label = port_lag_label;
    pd_status =
        p4_pd_dc_egress_port_mapping_table_modify_with_egress_port_type_normal(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl,
            &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#ifdef P4_FABRIC_ENABLE
  } else if (port_type == SWITCH_PORT_TYPE_FABRIC) {
    pd_status =
        p4_pd_dc_egress_port_mapping_table_modify_with_egress_port_type_fabric(
            switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
      pd_entry.match_spec_size = 0;
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_FABRIC_ENABLE */
  } else if (port_type == SWITCH_PORT_TYPE_CPU) {
    pd_status =
        p4_pd_dc_egress_port_mapping_table_modify_with_egress_port_type_cpu(
            switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "egress port mapping entry update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
      pd_entry.match_spec_size = 0;
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress port mapping entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_port_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_egress_port_mapping_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress port mapping entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_port_mapping_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_egress_port_type_normal_action_spec_t action_spec;
  memset(&action_spec, 0, sizeof(action_spec));
  pd_status =
      p4_pd_dc_egress_port_mapping_set_default_action_egress_port_type_normal(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping table init failed "
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
        "egress port mapping table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress port mapping table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_int32_t switch_pd_port_state_change(switch_int32_t device,
                                           switch_dev_port_t dev_port,
                                           bool up,
                                           void *cookie) {
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_port_oper_status_t oper_status = SWITCH_PORT_OPER_STATUS_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(up);
  UNUSED(cookie);

  status = switch_port_dev_port_to_handle_get(device, dev_port, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port state change notification failed on device %d "
        "dev port %d: dev port to handle get failed:(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  oper_status = up ? SWITCH_PORT_OPER_STATUS_UP : SWITCH_PORT_OPER_STATUS_DOWN;

  status = switch_port_state_change(device, port_handle, oper_status, cookie);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port state change notification failed on device %d "
        "dev port %d: state change notify failed:(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_pd_port_state_change_notification_register(
    switch_device_t device, void *cookie) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(cookie);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_status_notif_reg(switch_pd_port_state_change, cookie);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port state change notification registration failed "
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

  return status;
}

switch_status_t switch_pd_port_loopback_mode_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_loopback_mode_t lb_mode) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(lb_mode);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  bf_loopback_mode_e bf_lb_mode = switch_lb_mode_to_pd_lb_mode(lb_mode);
  pd_status = bf_pal_port_loopback_mode_set(device, dev_port, bf_lb_mode);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port loopback set failed "
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

  return status;
}

switch_status_t switch_pd_port_pfc_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_uint32_t rx_pfc_map,
                                       switch_uint32_t tx_pfc_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(rx_pfc_map);
  UNUSED(tx_pfc_map);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status = bf_pal_port_flow_control_pfc_set(
      device, dev_port, tx_pfc_map, rx_pfc_map);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port pfc set failed on device %d: dev_port %d", device, dev_port);
  }
  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_link_pause_set(switch_device_t device,
                                              switch_dev_port_t dev_port,
                                              bool rx_pause_en,
                                              bool tx_pause_en) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(rx_pause_en);
  UNUSED(tx_pause_en);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status = bf_pal_port_flow_control_link_pause_set(
      device, dev_port, tx_pause_en, rx_pause_en);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port link flow-control set failed on device %d: dev_port %d",
        device,
        dev_port);
  }
  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_mtu_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_uint32_t tx_mtu,
                                       switch_uint32_t rx_mtu) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(rx_mtu);
  UNUSED(tx_mtu);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_mtu_set(device, dev_port, tx_mtu, rx_mtu);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port mtu set failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
  }
  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_fec_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_port_fec_mode_t switch_fec_mode) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  bf_fec_type_t bf_fec_type = BF_FEC_TYP_NONE;

  bf_fec_type = switch_fec_mode_to_bf_fec_type(switch_fec_mode);
  pd_status = bf_pal_port_fec_set(device, dev_port, bf_fec_type);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port FEC type set failed "
        "on device %d devport %d\n",
        device,
        dev_port);
  }

  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_cut_through_set(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(enable);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  if (enable) {
    pd_status = bf_pal_port_cut_through_enable(device, dev_port);
  } else {
    pd_status = bf_pal_port_cut_through_disable(device, dev_port);
  }
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port cut-through mode set failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
  }

  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_cut_through_get(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               bool *enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(enable);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status =
      bf_pal_port_cut_through_enable_status_get(device, dev_port, enable);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port cut-through mode get failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
  }

  status = switch_pd_status_to_status(pd_status);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ingress_port_mirror_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
#ifdef SWITCH_PD
#if defined(P4_INGRESS_PORT_MIRROR_ENABLE)
  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_ingress_port_mirror_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mirror initialization failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);

#endif
#endif
  return status;
}

switch_status_t switch_pd_port_ingress_mirror_delete(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(entry_hdl);
  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(pd_status);
#ifdef SWITCH_PD
#if defined(P4_INGRESS_PORT_MIRROR_ENABLE)
  pd_status = p4_pd_dc_ingress_port_mirror_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mirror delete failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);
#endif
#endif
  return status;
}

switch_status_t switch_pd_port_ingress_mirror_set(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_handle_t mirror_handle,
                                                  switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(mirror_handle);
  UNUSED(pd_status);
  UNUSED(status);

#ifdef SWITCH_PD
#if defined(P4_INGRESS_PORT_MIRROR_ENABLE)
  p4_pd_dc_ingress_port_mirror_match_spec_t match_spec;
  p4_pd_dc_set_ingress_port_mirror_index_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_ingress_port_mirror_match_spec_t));
  SWITCH_MEMSET(&action_spec,
                0,
                sizeof(p4_pd_dc_set_ingress_port_mirror_index_action_spec_t));

  match_spec.ig_intr_md_ingress_port = dev_port;
  action_spec.action_session_id = handle_to_id(mirror_handle);

  pd_status =
      p4_pd_dc_ingress_port_mirror_table_add_with_set_ingress_port_mirror_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress port mirror set failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);

#endif
#endif
  return status;
}

switch_status_t switch_pd_egress_port_mirror_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);
#ifdef SWITCH_PD
#if defined(P4_EGRESS_PORT_MIRROR_ENABLE)
  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_port_mirror_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress port mirror initialization failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);

#endif
#endif
  return status;
}

switch_status_t switch_pd_port_egress_mirror_delete(switch_device_t device,
                                                    switch_dev_port_t dev_port,
                                                    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(entry_hdl);
  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(pd_status);
#ifdef SWITCH_PD
#if defined(P4_EGRESS_PORT_MIRROR_ENABLE)
  pd_status = p4_pd_dc_egress_port_mirror_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress port mirror delete failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);
#endif
#endif
  return status;
}

switch_status_t switch_pd_port_egress_mirror_set(switch_device_t device,
                                                 switch_dev_port_t dev_port,
                                                 switch_handle_t mirror_handle,
                                                 switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(mirror_handle);
  UNUSED(pd_status);
  UNUSED(status);

#ifdef SWITCH_PD
#if defined(P4_EGRESS_PORT_MIRROR_ENABLE)
  p4_pd_dc_egress_port_mirror_match_spec_t match_spec;
  p4_pd_dc_set_egress_port_mirror_index_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_egress_port_mirror_match_spec_t));
  SWITCH_MEMSET(&action_spec,
                0,
                sizeof(p4_pd_dc_set_egress_port_mirror_index_action_spec_t));

  match_spec.eg_intr_md_egress_port = dev_port;
  action_spec.action_session_id = handle_to_id(mirror_handle);

  pd_status =
      p4_pd_dc_egress_port_mirror_table_add_with_set_egress_port_mirror_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress port mirror set failed"
        "on device %d \n",
        device);
  }
  status = switch_pd_status_to_status(pd_status);

#endif
#endif
  return status;
}

switch_status_t switch_pd_port_auto_neg_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_auto_neg_mode_t an_mode) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(an_mode);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = bf_pal_port_autoneg_policy_set(device, dev_port, an_mode);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port auto neg set failed "
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

  return status;
}

switch_status_t switch_pd_port_tm_drop_get(switch_device_t device,
                                           switch_dev_port_t dev_port,
                                           uint64_t *idrop_count,
                                           uint64_t *edrop_count) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status =
      p4_pd_tm_port_drop_get(device, 0x0, dev_port, idrop_count, edrop_count);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port tm drop count get failed "
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

  return status;
}

#ifdef __cplusplus
}
#endif
