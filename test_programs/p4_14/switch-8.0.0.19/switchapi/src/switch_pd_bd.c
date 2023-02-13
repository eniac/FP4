
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

switch_status_t switch_pd_bd_table_entry_add(switch_device_t device,
                                             switch_bd_t bd,
                                             switch_bd_info_t *bd_info,
                                             switch_pd_mbr_hdl_t *pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(bd_info);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_set_bd_properties_action_spec_t action_spec;
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_bd_properties_action_spec_t));

  action_spec.action_bd = bd;
  action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
  action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
  action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
  action_spec.action_bd_label = bd_info->ingress_bd_label;
  action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
  action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
  action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
  action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
  action_spec.action_igmp_snooping_enabled = bd_info->igmp_snooping;
  action_spec.action_mld_snooping_enabled = bd_info->mld_snooping;
#if !defined(P4_URPF_DISABLE)
  action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
  action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */
  action_spec.action_stp_group = handle_to_id(bd_info->stp_handle);
  action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);
  action_spec.action_learning_enabled = bd_info->learning;

#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
  if (bd_info->ipv4_multicast) {
    action_spec.action_ipv4_mcast_key_type = 1;
    action_spec.action_ipv4_mcast_key = handle_to_id(bd_info->vrf_handle);
  } else {
    action_spec.action_ipv4_mcast_key_type = 0;
    action_spec.action_ipv4_mcast_key = bd;
  }

  if (bd_info->ipv6_multicast) {
    action_spec.action_ipv6_mcast_key_type = 1;
    action_spec.action_ipv6_mcast_key = handle_to_id(bd_info->vrf_handle);
  } else {
    action_spec.action_ipv6_mcast_key_type = 0;
    action_spec.action_ipv6_mcast_key = bd;
  }
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */

  pd_status = p4_pd_dc_bd_action_profile_add_member_with_set_bd_properties(
      switch_cfg_sess_hdl, p4_pd_device, &action_spec, pd_mbr_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd entry add failed "
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
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  switch_interface_info_t *cpu_intf_info = NULL;
  status = switch_api_hostif_cpu_intf_info_get(device, &cpu_intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd entry add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  p4_pd_dc_cpu_packet_transform_match_spec_t cpu_match_spec;
  SWITCH_MEMSET(&cpu_match_spec, 0x0, sizeof(cpu_match_spec));
  cpu_match_spec.fabric_header_cpu_ingressBd = bd;
  pd_status = p4_pd_dc_cpu_packet_transform_add_entry(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &cpu_match_spec,
                                                      *pd_mbr_hdl,
                                                      &bd_info->cpu_tx_entry);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd cpu tx transform add failed "
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
    pd_entry.match_spec_size = sizeof(cpu_match_spec);
    pd_entry.match_spec = (switch_uint8_t *)&cpu_match_spec;
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    pd_entry.pd_hdl = bd_info->cpu_tx_entry;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd entry add success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_bd_flood_table_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_packet_type_t packet_type,
    bool flood_to_mrouters,
    switch_mgid_t mgid,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(packet_type);
  UNUSED(mgid);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  /* Unknown unicast flood */
  p4_pd_dc_bd_flood_match_spec_t flood_match_spec;
  p4_pd_dc_set_bd_flood_mc_index_action_spec_t flood_action_spec;
  SWITCH_MEMSET(&flood_match_spec, 0x0, sizeof(flood_match_spec));
  SWITCH_MEMSET(&flood_action_spec, 0x0, sizeof(flood_action_spec));

  flood_match_spec.ingress_metadata_bd = bd;
  flood_match_spec.l2_metadata_lkp_pkt_type = packet_type;
  flood_match_spec.multicast_metadata_flood_to_mrouters = flood_to_mrouters;
  flood_action_spec.action_mc_index = mgid;

  pd_status = p4_pd_dc_bd_flood_table_add_with_set_bd_flood_mc_index(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &flood_match_spec,
      &flood_action_spec,
      entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry add failed "
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
    pd_entry.match_spec = (switch_uint8_t *)&flood_match_spec;
    pd_entry.match_spec_size = sizeof(flood_match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&flood_action_spec;
    pd_entry.action_spec_size = sizeof(flood_action_spec);
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd flood entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_bd_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_bd_info_t *bd_info,
    switch_pd_mbr_hdl_t pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(bd_info);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_set_bd_properties_action_spec_t action_spec;
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_bd_properties_action_spec_t));
  action_spec.action_bd = bd;
  action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
  action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
  action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
  action_spec.action_bd_label = bd_info->ingress_bd_label;
  action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
  action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
  action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
  action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
  action_spec.action_igmp_snooping_enabled = bd_info->igmp_snooping;
  action_spec.action_mld_snooping_enabled = bd_info->mld_snooping;
#if !defined(P4_URPF_DISABLE)
  action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
  action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */
  action_spec.action_stp_group = handle_to_id(bd_info->stp_handle);
  action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);
  action_spec.action_learning_enabled = bd_info->learning;

#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
  if (bd_info->ipv4_multicast) {
    action_spec.action_ipv4_mcast_key_type = 1;
    action_spec.action_ipv4_mcast_key = handle_to_id(bd_info->vrf_handle);
  } else {
    action_spec.action_ipv4_mcast_key_type = 0;
    action_spec.action_ipv4_mcast_key = bd;
  }

  if (bd_info->ipv6_multicast) {
    action_spec.action_ipv6_mcast_key_type = 1;
    action_spec.action_ipv6_mcast_key = handle_to_id(bd_info->vrf_handle);
  } else {
    action_spec.action_ipv6_mcast_key_type = 0;
    action_spec.action_ipv6_mcast_key = bd;
  }
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */

  pd_status = p4_pd_dc_bd_action_profile_modify_member_with_set_bd_properties(
      switch_cfg_sess_hdl, device, pd_mbr_hdl, &action_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd entry update failed "
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
    pd_entry.pd_mbr_hdl = pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd entry update success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_mbr_hdl);
  }
  return status;
}

switch_status_t switch_pd_bd_flood_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_packet_type_t packet_type,
    bool flood_to_mrouters,
    switch_mgid_t mgid,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(packet_type);
  UNUSED(mgid);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  /* Unknown unicast flood */
  p4_pd_dc_set_bd_flood_mc_index_action_spec_t flood_action_spec;
  SWITCH_MEMSET(&flood_action_spec, 0x0, sizeof(flood_action_spec));
  flood_action_spec.action_mc_index = mgid;

  pd_status = p4_pd_dc_bd_flood_table_modify_with_set_bd_flood_mc_index(
      switch_cfg_sess_hdl, device, entry_hdl, &flood_action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry update failed "
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
    pd_entry.action_spec = (switch_uint8_t *)&flood_action_spec;
    pd_entry.action_spec_size = sizeof(flood_action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd flood entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_bd_table_entry_delete(switch_device_t device,
                                                switch_bd_info_t *bd_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(bd_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_cpu_packet_transform_table_delete(
      switch_cfg_sess_hdl, device, bd_info->cpu_tx_entry);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd cpu tx transform entry delete failed "
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
    pd_entry.pd_hdl = bd_info->cpu_tx_entry;
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_bd_action_profile_del_member(
      switch_cfg_sess_hdl, device, bd_info->bd_entry);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd entry delete failed "
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
    pd_entry.pd_mbr_hdl = bd_info->bd_entry;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd entry delete success "
        "on device %d 0x%lx\n",
        device,
        bd_info->bd_entry);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        bd_info->bd_entry);
  }
  return status;
}

switch_status_t switch_pd_bd_flood_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  pd_status =
      p4_pd_dc_bd_flood_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry delete failed "
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

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd flood entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd flood entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_table_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(bd_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_bd_map_match_spec_t match_spec;
  p4_pd_dc_set_egress_bd_properties_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  match_spec.egress_metadata_bd = bd;
  action_spec.action_smac_idx = bd_info->smac_index;
  action_spec.action_nat_mode = bd_info->nat_mode;
  action_spec.action_bd_label = bd_info->egress_bd_label;
  action_spec.action_mtu_index = handle_to_id(bd_info->mtu_handle);
  action_spec.action_nat_mode = bd_info->nat_mode;

  pd_status = p4_pd_dc_egress_bd_map_table_add_with_set_egress_bd_properties(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry add failed "
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

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd map entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(bd_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dc_set_egress_bd_properties_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  action_spec.action_smac_idx = bd_info->smac_index;
  action_spec.action_nat_mode = bd_info->nat_mode;
  action_spec.action_bd_label = bd_info->egress_bd_label;
  action_spec.action_mtu_index = handle_to_id(bd_info->mtu_handle);
  action_spec.action_nat_mode = bd_info->nat_mode;

  pd_status = p4_pd_dc_egress_bd_map_table_modify_with_set_egress_bd_properties(
      switch_cfg_sess_hdl, device, entry_hdl, &action_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry update failed "
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

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd map entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd map update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  pd_status = p4_pd_dc_egress_bd_map_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry delete failed "
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

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd map entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_stats_table_entry_add(
    switch_device_t device, switch_bd_t bd, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_bd_stats_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_egress_bd_stats_match_spec_t));

  match_spec.egress_metadata_bd = bd;
  match_spec.l2_metadata_lkp_pkt_type = 1;

  pd_status = p4_pd_dc_egress_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  match_spec.l2_metadata_lkp_pkt_type = 2;
  pd_status = p4_pd_dc_egress_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  match_spec.l2_metadata_lkp_pkt_type = 4;
  pd_status = p4_pd_dc_egress_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

#endif /* P4_STATS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd stats entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_stats_table_entry_update(
    switch_device_t device, switch_bd_t bd, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

#endif /* P4_STATS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd stats entry update success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_bd_stats_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  pd_status = p4_pd_dc_egress_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

#endif /* P4_STATS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd map entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd map entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_table_entry_add(
    switch_device_t device,
    switch_bd_t outer_bd,
    switch_bd_info_t *outer_bd_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(outer_bd);
  UNUSED(outer_bd_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_outer_bd_map_match_spec_t match_spec;
  p4_pd_dc_set_egress_outer_bd_properties_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  match_spec.egress_metadata_outer_bd = outer_bd;
  action_spec.action_smac_idx = outer_bd_info->smac_index;

  pd_status =
      p4_pd_dc_egress_outer_bd_map_table_add_with_set_egress_outer_bd_properties(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry add failed "
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

#endif /* !P4_L3_DISABLE && !P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd map entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_table_entry_update(
    switch_device_t device,
    switch_bd_t outer_bd,
    switch_bd_info_t *outer_bd_info,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(outer_bd);
  UNUSED(outer_bd_info);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  p4_pd_dc_set_egress_outer_bd_properties_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
  action_spec.action_smac_idx = outer_bd_info->smac_index;

  pd_status =
      p4_pd_dc_egress_outer_bd_map_table_modify_with_set_egress_outer_bd_properties(
          switch_cfg_sess_hdl, device, entry_hdl, &action_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry update failed "
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

#endif /* !P4_L3_DISABLE && !P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd map entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  pd_status = p4_pd_dc_egress_outer_bd_map_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry delete failed "
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

#endif /* !P4_L3_DISABLE && !P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd map entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_stats_table_entry_add(
    switch_device_t device, switch_bd_t outer_bd, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(outer_bd);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_EGRESS_OUTER_BD_STATS_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_outer_bd_stats_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_egress_outer_bd_stats_match_spec_t));

  match_spec.egress_metadata_outer_bd = outer_bd;
  match_spec.l2_metadata_lkp_pkt_type = 1;

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  match_spec.l2_metadata_lkp_pkt_type = 2;
  pd_status = p4_pd_dc_egress_outer_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  match_spec.l2_metadata_lkp_pkt_type = 4;
  pd_status = p4_pd_dc_egress_outer_bd_stats_table_add_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      &entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_OUTER_BD_STATS_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd stats entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_stats_table_entry_update(
    switch_device_t device, switch_bd_t outer_bd, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(outer_bd);
  UNUSED(entry_hdl);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_EGRESS_OUTER_BD_STATS_ENABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_modify_with_nop(
      switch_cfg_sess_hdl,
      p4_pd_device.device_id,
      entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry update failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_OUTER_BD_STATS_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd stats entry update success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_outer_bd_stats_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_EGRESS_OUTER_BD_STATS_ENABLE

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_UCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_MCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_egress_outer_bd_stats_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST]);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd stats entry delete failed "
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
    pd_entry.pd_hdl = entry_hdl[SWITCH_OUTER_BD_STATS_OUT_BCAST];
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_OUTER_BD_STATS_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress outer_bd map entry delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress outer_bd map entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_bd_stats_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE
#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status =
      p4_pd_dc_ingress_bd_stats_set_default_action_update_ingress_bd_stats(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress bd stats table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_egress_bd_stats_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd stats table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ || BMV2TOFINO */
#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd stats table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd stats table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_bd_flood_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef MULTICAST_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_bd_flood_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "bd flood table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "bd flood table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "bd flood table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_bd_stats_get(switch_device_t device,
                                       switch_bd_stats_t *bd_stats) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;
  int index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    switch (index) {
      case SWITCH_BD_STATS_IN_UCAST:
      case SWITCH_BD_STATS_IN_MCAST:
      case SWITCH_BD_STATS_IN_BCAST:
        pd_status = p4_pd_dc_counter_read_ingress_bd_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            bd_stats->stats_id[index],
            switch_pd_counter_read_flags(device),
            &counter);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "Reading ingress bd stats failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          status = switch_pd_status_to_status(pd_status);
          p4_pd_complete_operations(switch_cfg_sess_hdl);
          return status;
        }
        break;
      case SWITCH_BD_STATS_OUT_UCAST:
      case SWITCH_BD_STATS_OUT_MCAST:
      case SWITCH_BD_STATS_OUT_BCAST:
        pd_status = p4_pd_dc_counter_read_egress_bd_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            bd_stats->stats_pd_hdl[index],
            switch_pd_counter_read_flags(device),
            &counter);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "Reading egress bd stats failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          status = switch_pd_status_to_status(pd_status);
          p4_pd_complete_operations(switch_cfg_sess_hdl);
          return status;
        }
        break;
      default:
        counter.packets = 0;
        counter.bytes = 0;
    }
    bd_stats->counters[index].num_packets = counter.packets;
    bd_stats->counters[index].num_bytes = counter.bytes;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("bd stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_bd_stats_clear(switch_device_t device,
                                         switch_bd_stats_t *bd_stats) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_STATS_DISABLE

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;
  int index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  SWITCH_MEMSET(&counter, 0x0, sizeof(p4_pd_counter_value_t));

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    switch (index) {
      case SWITCH_BD_STATS_IN_UCAST:
      case SWITCH_BD_STATS_IN_MCAST:
      case SWITCH_BD_STATS_IN_BCAST:
        pd_status =
            p4_pd_dc_counter_write_ingress_bd_stats(switch_cfg_sess_hdl,
                                                    p4_pd_device,
                                                    bd_stats->stats_id[index],
                                                    counter);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "clearing ingress bd stats failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          status = switch_pd_status_to_status(pd_status);
          p4_pd_complete_operations(switch_cfg_sess_hdl);
          return status;
        }
        break;
      case SWITCH_BD_STATS_OUT_UCAST:
      case SWITCH_BD_STATS_OUT_MCAST:
      case SWITCH_BD_STATS_OUT_BCAST:
        pd_status = p4_pd_dc_counter_write_egress_bd_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            bd_stats->stats_pd_hdl[index],
            counter);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "clearing egress bd stats failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          status = switch_pd_status_to_status(pd_status);
          p4_pd_complete_operations(switch_cfg_sess_hdl);
          return status;
        }
        break;
      default:
        counter.packets = 0;
        counter.bytes = 0;
    }
    bd_stats->counters[index].num_packets = counter.packets;
    bd_stats->counters[index].num_bytes = counter.bytes;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("bd stats clear on device %d\n", device);

  return status;
}

#ifdef __cplusplus
}
#endif
