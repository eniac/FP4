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

switch_status_t switch_pd_fwd_result_table_default_entry_add(
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

  pd_status = p4_pd_dc_fwd_result_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd resultc table default add failed "
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
        "fwd result table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fwd result table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_filter_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef EGRESS_FILTER

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_filter_set_default_action_egress_filter_check(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress filter table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status =
      p4_pd_dc_egress_filter_drop_set_default_action_set_egress_filter_drop(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress filter table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* EGRESS_FILTER */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress filter table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress filter table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_fwd_result_table_entry_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_fwd_result_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;
  uint16_t prio = 1000;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.acl_metadata_acl_redirect = 1;
  match_spec.acl_metadata_acl_redirect_mask = 1;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_acl_redirect(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // Non-IP packets sent to router mac
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l3_metadata_rmac_hit = 1;
  match_spec.l3_metadata_rmac_hit_mask = 1;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_NONE;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_rmac_non_ip_drop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_NAT_DISABLE
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l3_metadata_rmac_hit = 1;
  match_spec.l3_metadata_rmac_hit_mask = 1;
  match_spec.nat_metadata_nat_hit = 1;
  match_spec.nat_metadata_nat_hit_mask = 1;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_nat_redirect(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_NAT_DISABLE */

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l3_metadata_fib_hit = 1;
  match_spec.l3_metadata_fib_hit_mask = 1;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_fib_redirect(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l2_metadata_l2_redirect = 1;
  match_spec.l2_metadata_l2_redirect_mask = 1;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_l2_redirect(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);

  switch_interface_info_t *cpu_intf_info = NULL;
  status = switch_api_hostif_cpu_intf_info_get(device, &cpu_intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_FIB_MISS_CPU_REDIRECT
  p4_pd_dc_set_cpu_redirect_action_action_spec_t action_spec;
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l3_metadata_rmac_hit = 1;
  match_spec.l3_metadata_rmac_hit_mask = 1;
  match_spec.l3_metadata_fib_hit = 0;
  match_spec.l3_metadata_fib_hit_mask = 1;
  action_spec.action_cpu_ifindex = cpu_intf_info->port_lag_index;
  pd_status =
      p4_pd_dc_fwd_result_table_add_with_set_cpu_redirect(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &match_spec,
                                                          prio++,
                                                          &action_spec,
                                                          &entry_hdl);
#endif

#ifndef P4_MULTICAST_DISABLE
  prio = 2000;
  // mroute = hit, bridge = x, rpf = pass, mode = SM
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_rpf_group = 0;
  match_spec.multicast_metadata_mcast_rpf_group_mask = 0xFFFF;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_SM;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_route(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // mroute = hit, bridge = x, rpf = pass, mode = BIDIR
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_rpf_group = 0xFFFF;
  match_spec.multicast_metadata_mcast_rpf_group_mask = 0xFFFF;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_BIDIR;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_route(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // mroute = hit, bridge = hit, rpf = fail
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_bridge_hit = 1;
  match_spec.multicast_metadata_mcast_bridge_hit_mask = 1;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_SM;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_rpf_fail_bridge(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);

  // mroute = hit, bridge = miss, rpf = fail, igmp snooping enabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_bridge_hit = 0;
  match_spec.multicast_metadata_mcast_bridge_hit_mask = 1;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_SM;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.multicast_metadata_igmp_snooping_enabled = 1;
  match_spec.multicast_metadata_igmp_snooping_enabled_mask = 1;
  pd_status =
      p4_pd_dc_fwd_result_table_add_with_set_multicast_rpf_fail_flood_to_mrouters(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // mroute = hit, bridge = miss, rpf = fail, mld snooping enabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_bridge_hit = 0;
  match_spec.multicast_metadata_mcast_bridge_hit_mask = 1;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_SM;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.multicast_metadata_mld_snooping_enabled = 1;
  match_spec.multicast_metadata_mld_snooping_enabled_mask = 1;
  pd_status =
      p4_pd_dc_fwd_result_table_add_with_set_multicast_rpf_fail_flood_to_mrouters(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // mroute = hit, bridge = miss, rpf = fail, igmp/mld snooping disabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_route_hit = 1;
  match_spec.multicast_metadata_mcast_route_hit_mask = 1;
  match_spec.multicast_metadata_mcast_bridge_hit = 0;
  match_spec.multicast_metadata_mcast_bridge_hit_mask = 1;
  match_spec.multicast_metadata_mcast_mode = SWITCH_API_MCAST_IPMC_PIM_SM;
  match_spec.multicast_metadata_mcast_mode_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_miss_flood(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);

  // bridge = hit
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.multicast_metadata_mcast_bridge_hit = 1;
  match_spec.multicast_metadata_mcast_bridge_hit_mask = 1;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_bridge(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // bridge = miss, pkt_type = ipv4 multicast, igmp snooping enabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l2_metadata_lkp_pkt_type = SWITCH_BD_FLOOD_UMC;
  match_spec.l2_metadata_lkp_pkt_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.multicast_metadata_igmp_snooping_enabled = 1;
  match_spec.multicast_metadata_igmp_snooping_enabled_mask = 1;
  pd_status =
      p4_pd_dc_fwd_result_table_add_with_set_multicast_miss_flood_to_mrouters(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // bridge = miss, pkt_type = ipv6 multicast, mld snooping enabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l2_metadata_lkp_pkt_type = SWITCH_BD_FLOOD_UMC;
  match_spec.l2_metadata_lkp_pkt_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.multicast_metadata_mld_snooping_enabled = 1;
  match_spec.multicast_metadata_mld_snooping_enabled_mask = 1;
  pd_status =
      p4_pd_dc_fwd_result_table_add_with_set_multicast_miss_flood_to_mrouters(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // bridge = miss, pkt_type = ipv4 multicast, igmp snooping disabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l2_metadata_lkp_pkt_type = SWITCH_BD_FLOOD_UMC;
  match_spec.l2_metadata_lkp_pkt_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_miss_flood(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // bridge = miss, pkt_type = ipv6 multicast, mld snooping disabled
  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_fwd_result_match_spec_t));
  match_spec.l2_metadata_lkp_pkt_type = SWITCH_BD_FLOOD_UMC;
  match_spec.l2_metadata_lkp_pkt_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  pd_status = p4_pd_dc_fwd_result_table_add_with_set_multicast_miss_flood(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, prio++, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fwd result table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_MULTICAST_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fwd result table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fwd result  table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_drop_stats_get(switch_device_t device,
                                         switch_uint32_t num_counters,
                                         switch_uint64_t *counters) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(num_counters);
  UNUSED(counters);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;
  switch_uint32_t index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  for (index = 0; index < num_counters; index++) {
    pd_status =
        p4_pd_dc_counter_read_drop_stats(switch_cfg_sess_hdl,
                                         p4_pd_device,
                                         index,
                                         switch_pd_counter_read_flags(device),
                                         &counter);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Reading drop stats failed "
          "on device %d : table %s idx %d action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          index,
          switch_pd_action_id_to_string(0));
      status = switch_pd_status_to_status(pd_status);
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return status;
    }
    *(counters + index) = counter.packets;
    pd_status =
        p4_pd_dc_counter_read_drop_stats_2(switch_cfg_sess_hdl,
                                           p4_pd_device,
                                           index,
                                           switch_pd_counter_read_flags(device),
                                           &counter);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Reading drop stats_2 failed "
          "on device %d : table %s idx %d action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          index,
          switch_pd_action_id_to_string(0));
      status = switch_pd_status_to_status(pd_status);
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return status;
    }
    *(counters + index) += counter.packets;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("drop stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_flowlet_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifdef P4_FLOWLET_ENABLE
#if !defined(__TARGET_TOFINO__)

  p4_pd_dev_target_t p4_pd_device = {0};
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_flowlet_set_default_action_flowlet_lookup(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  pd_status = p4_pd_dc_new_flowlet_set_default_action_update_flowlet_id(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(__TARGET_TOFINO__) */
#endif /* P4_FLOWLET_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

void switch_pd_stats_update_cb(int device, void *cookie) { return; }

switch_status_t switch_pd_stats_update(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

  if (switch_pd_platform_type_model(device)) {
    return status;
  }

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

#ifndef P4_STORM_CONTROL_DISABLE
  pd_status = p4_pd_dc_counter_hw_sync_storm_control_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "storm control stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_STORM_CONTROL_DISABLE */

  pd_status = p4_pd_dc_counter_hw_sync_acl_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_RACL_STATS_ENABLE
  pd_status = p4_pd_dc_counter_hw_sync_racl_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_RACL_STATS_ENABLE */

#ifdef P4_EGRESS_ACL_STATS_ENABLE
  pd_status = p4_pd_dc_counter_hw_sync_egress_acl_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_EGRESS_ACL_STATS_ENABLE */

  pd_status = p4_pd_dc_counter_hw_sync_egress_bd_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_counter_hw_sync_ingress_bd_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress bd stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if defined(P4_QOS_METERING_ENABLE)
  pd_status = p4_pd_dc_counter_hw_sync_meter_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "meter stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_QOS_METERING_ENABLE */

  pd_status = p4_pd_dc_counter_hw_sync_drop_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "drop stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_counter_hw_sync_drop_stats_2(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "drop stats 2 update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_L3_MULTICAST_DISABLE
#ifndef P4_IPV4_DISABLE
  pd_status = p4_pd_dc_counter_hw_sync_ipv4_multicast_route_s_g_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 multicast sg stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_counter_hw_sync_ipv4_multicast_route_star_g_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 multicast star g stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_counter_hw_sync_ipv6_multicast_route_s_g_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 multicast sg stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_counter_hw_sync_ipv6_multicast_route_star_g_stats(
      switch_cfg_sess_hdl, p4_pd_device, &switch_pd_stats_update_cb, NULL);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 multicast star g stats update failed"
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */
#endif /* P4_L3_MULTICAST_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "stats table update success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "stats table update failed "
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
