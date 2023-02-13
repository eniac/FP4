
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

switch_status_t switch_pd_wcmp_group_create(switch_device_t device,
                                            switch_wcmp_t wcmp_index,
                                            switch_pd_mbr_hdl_t *mbr_hdl,
                                            switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = 0;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WCMP_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ecmp_group_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  pd_status = p4_pd_dc_ecmp_action_profile_add_member_with_set_wcmp(
      switch_cfg_sess_hdl, p4_pd_device, mbr_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  memset(&match_spec, 0, sizeof(p4_pd_dc_ecmp_group_match_spec_t));
  match_spec.l3_metadata_nexthop_index = wcmp_index;
  pd_status = p4_pd_dc_ecmp_group_add_entry(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, *mbr_hdl, entry_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* WCMP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "wcmp group create success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "wcmp group create failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_wcmp_group_delete(switch_device_t device,
                                            switch_pd_mbr_hdl_t mbr_hdl,
                                            switch_pd_hdl_t entry_hdl) {
  switch_status_t status = 0;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WCMP_ENABLE

  pd_status =
      p4_pd_dc_ecmp_group_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
  pd_status = p4_pd_dc_ecmp_action_profile_del_member(
      switch_cfg_sess_hdl, device, mbr_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* WCMP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "wcmp group delete success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "wcmp group delete failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_wcmp_member_add(switch_device_t device,
                                          switch_nhop_t nhop_index,
                                          switch_wcmp_t wcmp_index,
                                          switch_uint8_t start,
                                          switch_uint8_t end,
                                          switch_spath_info_t *spath_info,
                                          switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WCMP_ENABLE

  p4_pd_dev_target_t pd_device;
  p4_pd_dc_set_wcmp_nexthop_details_action_spec_t action_spec;
  p4_pd_dc_wcmp_group_match_spec_t match_spec;

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  memset(&match_spec, 0, sizeof(match_spec));
  memset(&action_spec, 0, sizeof(action_spec));

  action_spec.action_ifindex = spath_info->ifindex;
  action_spec.action_port_lag_index = spath_info->port_lag_index;
  action_spec.action_bd = handle_to_id(spath_info->bd_handle);
  action_spec.action_nhop_index = nhop_index;
  action_spec.action_tunnel = spath_info->tunnel;

  match_spec.l3_metadata_nexthop_index = wcmp_index;
  match_spec.hash_metadata_hash1_start = start;
  match_spec.hash_metadata_hash1_end = end;
  pd_status = p4_pd_dc_wcmp_group_table_add_with_set_wcmp_nexthop_details(
      switch_cfg_sess_hdl,
      pd_device,
      &match_spec,
      0x0,
      &action_spec,
      entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* WCMP_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_wcmp_member_delete(switch_device_t device,
                                             switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WCMP_ENABLE

  pd_status =
      p4_pd_dc_wcmp_group_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* WCMP_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mac_table_learning_timeout_set(switch_device_t device,
                                                         uint32_t timeout) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(timeout);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE
  pd_status =
      p4_pd_dc_set_learning_timeout(switch_cfg_sess_hdl, device, timeout);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac table learning timeout set success "
        "on device %d timeout %d\n",
        device,
        timeout);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac table learning timeout set failed "
        "on device %d timeout %d\n",
        device,
        timeout);
  }

  return status;
}

switch_status_t switch_pd_nexthop_table_entry_add(
    switch_device_t device,
    switch_nhop_t nhop_index,
    switch_bd_t bd,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_pd_action_t pd_action,
    switch_mgid_t mc_index,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(nhop_index);
  UNUSED(bd);
  UNUSED(ifindex);
  UNUSED(port_lag_index);
  UNUSED(mc_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_nexthop_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_nexthop_match_spec_t));
  match_spec.l3_metadata_nexthop_index = nhop_index;

  switch (pd_action) {
    case SWITCH_NHOP_PD_ACTION_FLOOD: {
      p4_pd_dc_set_nexthop_details_for_post_routed_flood_action_spec_t
          action_spec = {0};
      action_spec.action_bd = bd;
      action_spec.action_uuc_mc_index = mc_index;
      pd_status =
          p4_pd_dc_nexthop_table_add_with_set_nexthop_details_for_post_routed_flood(
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
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_GLEAN: {
      p4_pd_dc_set_nexthop_details_for_glean_action_spec_t action_spec = {0};
      action_spec.action_ifindex = ifindex;
      pd_status = p4_pd_dc_nexthop_table_add_with_set_nexthop_details_for_glean(
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
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_MGID_TUNNEL: {
      p4_pd_dc_set_nexthop_details_with_tunnel_action_spec_t action_spec = {0};
      action_spec.action_tunnel_dst_index = tunnel_index;
      pd_status =
          p4_pd_dc_nexthop_table_add_with_set_nexthop_details_with_tunnel(
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
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_NON_TUNNEL:
    case SWITCH_NHOP_PD_ACTION_TUNNEL: {
      p4_pd_dc_set_nexthop_details_action_spec_t action_spec = {0};
      action_spec.action_bd = bd;
      action_spec.action_ifindex = ifindex;
      action_spec.action_port_lag_index = port_lag_index;
      action_spec.action_tunnel =
          pd_action == SWITCH_NHOP_PD_ACTION_TUNNEL ? TRUE : FALSE;
      pd_status = p4_pd_dc_nexthop_table_add_with_set_nexthop_details(
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
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_DROP: {
      pd_status = p4_pd_dc_nexthop_table_add_with_set_nexthop_details_for_drop(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      break;
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "nexthop entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "nexthop entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_nexthop_table_entry_update(
    switch_device_t device,
    switch_nhop_t nhop_index,
    switch_bd_t bd,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_pd_action_t pd_action,
    switch_mgid_t mc_index,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(nhop_index);
  UNUSED(bd);
  UNUSED(ifindex);
  UNUSED(port_lag_index);
  UNUSED(mc_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch (pd_action) {
    case SWITCH_NHOP_PD_ACTION_FLOOD: {
      p4_pd_dc_set_nexthop_details_for_post_routed_flood_action_spec_t
          action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_uuc_mc_index = mc_index;
      pd_status =
          p4_pd_dc_nexthop_table_modify_with_set_nexthop_details_for_post_routed_flood(
              switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_MGID_TUNNEL: {
      p4_pd_dc_set_nexthop_details_with_tunnel_action_spec_t action_spec = {0};
      action_spec.action_tunnel_dst_index = tunnel_index;
      pd_status =
          p4_pd_dc_nexthop_table_modify_with_set_nexthop_details_with_tunnel(
              switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NHOP_PD_ACTION_TUNNEL:
    case SWITCH_NHOP_PD_ACTION_NON_TUNNEL: {
      p4_pd_dc_set_nexthop_details_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_ifindex = ifindex;
      action_spec.action_port_lag_index = port_lag_index;
      action_spec.action_tunnel =
          pd_action == SWITCH_NHOP_PD_ACTION_TUNNEL ? TRUE : FALSE;
      pd_status = p4_pd_dc_nexthop_table_modify_with_set_nexthop_details(
          switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      break;
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "nexthop entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "nexthop entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ecmp_group_create(switch_device_t device,
                                            switch_pd_grp_hdl_t *pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_ecmp_action_profile_create_group(
      switch_cfg_sess_hdl, p4_pd_device, MAX_ECMP_GROUP_SIZE, pd_grp_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_grp_hdl = *pd_grp_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp group create success "
        "on device %d 0x%lx\n",
        device,
        pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp group create failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ecmp_group_delete(switch_device_t device,
                                            switch_pd_grp_hdl_t pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(pd_grp_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_ecmp_action_profile_del_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp group delete success "
        "on device %d 0x%lx\n",
        device,
        pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp group delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_grp_hdl);
  }

  return status;
}

switch_status_t switch_pd_ecmp_member_add(switch_device_t device,
                                          switch_pd_grp_hdl_t pd_grp_hdl,
                                          switch_nhop_t nhop_index,
                                          switch_spath_info_t *spath_info,
                                          switch_pd_mbr_hdl_t *pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);
  UNUSED(nhop_index);
  UNUSED(spath_info);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t pd_device;
  p4_pd_dc_set_ecmp_nexthop_details_action_spec_t action_spec;

  if (!pd_mbr_hdl || !spath_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_PD_LOG_ERROR(
        "ecmp member add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
    return status;
  }

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec,
                0x0,
                sizeof(p4_pd_dc_set_ecmp_nexthop_details_action_spec_t));

  action_spec.action_ifindex = spath_info->ifindex;
  action_spec.action_port_lag_index = spath_info->port_lag_index;
#ifndef P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE
  action_spec.action_bd = handle_to_id(spath_info->bd_handle);
#endif /* P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE */
  action_spec.action_nhop_index = nhop_index;
  action_spec.action_tunnel = spath_info->tunnel;

  pd_status =
      p4_pd_dc_ecmp_action_profile_add_member_with_set_ecmp_nexthop_details(
          switch_cfg_sess_hdl, pd_device, &action_spec, pd_mbr_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp member add failed "
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
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_ecmp_action_profile_add_member_to_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, *pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp member add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp member add success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp member add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ecmp_member_update(switch_device_t device,
                                             switch_pd_grp_hdl_t pd_grp_hdl,
                                             switch_nhop_t nhop_index,
                                             switch_spath_info_t *spath_info,
                                             switch_pd_mbr_hdl_t pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);
  UNUSED(nhop_index);
  UNUSED(spath_info);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t pd_device;
  p4_pd_dc_set_ecmp_nexthop_details_action_spec_t action_spec;

  if (!spath_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_PD_LOG_ERROR(
        "ecmp member add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
    return status;
  }

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  action_spec.action_ifindex = spath_info->ifindex;
  action_spec.action_port_lag_index = spath_info->port_lag_index;
#ifndef P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE
  action_spec.action_bd = handle_to_id(spath_info->bd_handle);
#endif /* P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE */
  action_spec.action_nhop_index = nhop_index;
  action_spec.action_tunnel = spath_info->tunnel;

  pd_status =
      p4_pd_dc_ecmp_action_profile_modify_member_with_set_ecmp_nexthop_details(
          switch_cfg_sess_hdl, pd_device.device_id, pd_mbr_hdl, &action_spec);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp member update failed "
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
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.match_spec_size = 0;
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
        "ecmp member update success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp member update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ecmp_group_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status =
      p4_pd_dc_ecmp_group_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp group delete failed "
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
        "ecmp group entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp group entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_ecmp_member_delete(switch_device_t device,
                                             switch_pd_grp_hdl_t pd_grp_hdl,
                                             switch_pd_mbr_hdl_t pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(pd_grp_hdl);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
  pd_status = p4_pd_dc_ecmp_action_profile_del_member_from_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, pd_mbr_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp member delete failed "
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
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    pd_entry.pd_mbr_hdl = pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_ecmp_action_profile_del_member(
      switch_cfg_sess_hdl, device, pd_mbr_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp member delete failed "
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
    pd_entry.pd_mbr_hdl = pd_mbr_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp member delete success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp member delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_mbr_hdl);
  }

  return status;
}

switch_status_t switch_pd_ecmp_group_table_with_selector_add(
    switch_device_t device,
    switch_nhop_t nhop_index,
    switch_pd_grp_hdl_t pd_grp_hdl,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_ecmp_group_match_spec_t match_spec;
  p4_pd_dev_target_t pd_device;

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_ecmp_group_match_spec_t));
  match_spec.l3_metadata_nexthop_index = nhop_index;

  pd_status = p4_pd_dc_ecmp_group_add_entry_with_selector(
      switch_cfg_sess_hdl, pd_device, &match_spec, pd_grp_hdl, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecmp group add with selector failed "
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
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp group add with selector success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp group add with selector failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ecmp_member_activate(switch_device_t device,
                                               switch_pd_grp_hdl_t pd_group_hdl,
                                               switch_pd_mbr_hdl_t *mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = p4_pd_dc_ecmp_action_profile_group_member_state_set(
      switch_cfg_sess_hdl,
      device,
      pd_group_hdl,
      *mbr_hdl,
      P4_PD_GRP_MBR_STATE_ACTIVE);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ecmp_member_deactivate(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_group_hdl,
    switch_pd_mbr_hdl_t *mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = p4_pd_dc_ecmp_action_profile_group_member_state_set(
      switch_cfg_sess_hdl,
      device,
      pd_group_hdl,
      *mbr_hdl,
      P4_PD_GRP_MBR_STATE_INACTIVE);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nexthop_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status =
      p4_pd_dc_nexthop_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "nexthop entry delete failed "
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
        "nexthop entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "nexthop entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_nexthop_table_default_entry_add(
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

  pd_status = p4_pd_dc_nexthop_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "nexthop table default add failed "
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
        "nexthop table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "nexthop table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

#ifdef __cplusplus
}
#endif
