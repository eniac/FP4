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

switch_status_t switch_pd_tunnel_rewrite_table_srv6_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_id_t sip_index,
    switch_id_t dip_index,
    switch_id_t smac_index,
    uint8_t first_seg,
    switch_srv6_segment_t *seg_list,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(sip_index);
  UNUSED(dip_index);
  UNUSED(entry_hdl);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_tunnel_rewrite_match_spec_t));
  match_spec.tunnel_metadata_tunnel_index = tunnel_index;

  switch (first_seg) {
    case 0: {
      p4_pd_dc_set_srv6_rewrite_segments1_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      SWITCH_MEMCPY(
          &action_spec.action_sid0, &seg_list[0].sid, SWITCH_SRV6_SID_LENGTH);
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_srv6_rewrite_segments1(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
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
    } break;
    case 1: {
      p4_pd_dc_set_srv6_rewrite_segments2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      SWITCH_MEMCPY(
          &action_spec.action_sid0, &seg_list[0].sid, SWITCH_SRV6_SID_LENGTH);
      SWITCH_MEMCPY(
          &action_spec.action_sid1, &seg_list[1].sid, SWITCH_SRV6_SID_LENGTH);
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_srv6_rewrite_segments2(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
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
    } break;
    case 2: {
      p4_pd_dc_set_srv6_rewrite_segments3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      SWITCH_MEMCPY(
          &action_spec.action_sid0, &seg_list[0].sid, SWITCH_SRV6_SID_LENGTH);
      SWITCH_MEMCPY(
          &action_spec.action_sid1, &seg_list[1].sid, SWITCH_SRV6_SID_LENGTH);
      SWITCH_MEMCPY(
          &action_spec.action_sid2, &seg_list[2].sid, SWITCH_SRV6_SID_LENGTH);
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_srv6_rewrite_segments3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
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
    } break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_srv6_sid_table_default_entry_add(
    switch_device_t device) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_srv6_sid_set_default_action_transit(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sr default entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ila entry update failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_process_srh_len_table_entry_init(
    switch_device_t device) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE
  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_process_srh_len_match_spec_t match_spec;
  p4_pd_dc_calculate_srh_total_len_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

  for (int i = 0; i < 256; i++) {
    match_spec.ipv6_srh_valid = TRUE;
    match_spec.ipv6_srh_hdrExtLen = i;         // Header lenth in 8-octet units
    action_spec.action_total_len = i * 8 + 8;  // Total header lenth in bytes
    pd_status = p4_pd_dc_process_srh_len_table_add_with_calculate_srh_total_len(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

  pd_status = p4_pd_dc_process_srh_len_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_srv6_rewrite_table_entry_init(
    switch_device_t device) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dc_srv6_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  match_spec.sr_metadata_endpoint_hit = 1;
  match_spec.ipv6_srh_valid = TRUE;

  // SegLeft = 0
  match_spec.ipv6_srh_segLeft = 0x00;
  match_spec.ipv6_srh_segLeft_mask = 0xff;
  pd_status = p4_pd_dc_srv6_rewrite_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 0, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // SegLeft = 1
  match_spec.ipv6_srh_segLeft = 0x01;
  match_spec.ipv6_srh_segLeft_mask = 0xff;
  pd_status = p4_pd_dc_srv6_rewrite_table_add_with_rewrite_ipv6_and_remove_srh(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // SegLeft > 1
  match_spec.ipv6_srh_segLeft = 0x00;
  match_spec.ipv6_srh_segLeft_mask = 0x00;
  pd_status = p4_pd_dc_srv6_rewrite_table_add_with_rewrite_ipv6_srh(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 3, &entry_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_srv6_table_entry_init(switch_device_t device) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  switch_pd_hdl_t entry_hdl;
  p4_pd_dc_srv6_sid_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_srv6_sid_match_spec_t));
  match_spec.ipv6_srh_segLeft = 0;
  match_spec.ipv6_srh_segLeft_mask = 0xff;
  match_spec.ipv6_srh_valid = TRUE;

  pd_status = p4_pd_dc_srv6_sid_table_add_with_transit(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1, &entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sr entry init success "
        "on device %d",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sr entry init failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_srv6_table_entry_add(
    switch_device_t device,
    switch_interface_ip_addr_t *ip_addr_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ip_addr_info);
  UNUSED(entry_hdl);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_srv6_sid_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_srv6_sid_match_spec_t));
  SWITCH_MEMCPY(
      &match_spec.ipv6_dstAddr, ip_addr_info->ip_address.ip.v6addr.u.addr8, 16);
  match_spec.l3_metadata_vrf = ip_addr_info->vrf_handle;
  match_spec.l3_metadata_vrf_mask = 0xffff;
  match_spec.ipv6_srh_segLeft = 0;
  match_spec.ipv6_srh_segLeft_mask = 0;
  match_spec.ipv6_srh_valid = TRUE;

  pd_status = p4_pd_dc_srv6_sid_table_add_with_endpoint(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1, entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sr entry add success "
        "on device %d",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sr entry add failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_srv6_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl) {
  switch_pd_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_SRV6_ENABLE
  pd_status =
      p4_pd_dc_srv6_sid_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_SRV6_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "sr entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "sr entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}
