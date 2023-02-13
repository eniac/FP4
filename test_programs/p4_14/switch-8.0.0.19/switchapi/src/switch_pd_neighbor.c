
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_rewrite_table_unicast_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_rw_type_t rw_type,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(dmac);
  UNUSED(rw_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_rewrite_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_rewrite_match_spec_t));
  match_spec.l3_metadata_nexthop_index = nhop_index;

  if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2) {
    pd_status = p4_pd_dc_rewrite_table_add_with_set_l2_rewrite(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite unicast entry add failed "
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
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  } else {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0x0, sizeof(p4_pd_dc_set_l3_rewrite_action_spec_t));
    action_spec.action_bd = bd;
    SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);

    pd_status =
        p4_pd_dc_rewrite_table_add_with_set_l3_rewrite(switch_cfg_sess_hdl,
                                                       p4_pd_device,
                                                       &match_spec,
                                                       &action_spec,
                                                       entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite unicast entry add failed "
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
#endif /* P4_L3_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite unicast entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite unicast entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_tunnel_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_api_neighbor_info_t *api_neighbor_info,
    switch_tunnel_t tunnel_index,
    switch_vni_t tunnel_vni,
    switch_id_t tunnel_dst_index,
    switch_tunnel_type_egress_t tunnel_type,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(api_neighbor_info);
  UNUSED(tunnel_index);
  UNUSED(tunnel_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dc_rewrite_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_rewrite_match_spec_t));
  match_spec.l3_metadata_nexthop_index = nhop_index;

#ifndef P4_TUNNEL_DISABLE
  if (api_neighbor_info->rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2) {
    p4_pd_dc_set_l2_rewrite_with_tunnel_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec,
                  0x0,
                  sizeof(p4_pd_dc_set_l2_rewrite_with_tunnel_action_spec_t));

#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
    //#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
    //#endif
    pd_status = p4_pd_dc_rewrite_table_add_with_set_l2_rewrite_with_tunnel(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry add failed "
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
  } else if (api_neighbor_info->rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L3) {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_with_tunnel_and_ingress_vrf_action_spec_t
        action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(
            p4_pd_dc_set_l3_rewrite_with_tunnel_and_ingress_vrf_action_spec_t));
    SWITCH_MEMCPY(
        action_spec.action_dmac, &api_neighbor_info->mac_addr, ETH_LEN);
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
    //#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
    //#endif
    pd_status =
        p4_pd_dc_rewrite_table_add_with_set_l3_rewrite_with_tunnel_and_ingress_vrf(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry add failed "
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
#endif /* P4_L3_DISABLE */
  } else if (api_neighbor_info->rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L3_VNI) {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_with_tunnel_vnid_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(p4_pd_dc_set_l3_rewrite_with_tunnel_vnid_action_spec_t));
    SWITCH_MEMCPY(
        action_spec.action_dmac, &api_neighbor_info->mac_addr, ETH_LEN);
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
    action_spec.action_vnid = tunnel_vni;
    //#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
    //#endif
    pd_status = p4_pd_dc_rewrite_table_add_with_set_l3_rewrite_with_tunnel_vnid(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry add failed "
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
#endif /* P4_L3_DISABLE */
  }
#endif /* P4_TUNNEL_DISABLE */

#ifndef P4_MIRROR_NEXTHOP_DISABLE
  if (api_neighbor_info->rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2_MIRROR) {
    p4_pd_dc_set_l2_rewrite_with_tunnel_dst_index_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(p4_pd_dc_set_l2_rewrite_with_tunnel_dst_index_action_spec_t));

#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
    //#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
    action_spec.action_tunnel_dst_index = tunnel_dst_index;
    //#endif
    pd_status =
        p4_pd_dc_rewrite_table_add_with_set_l2_rewrite_with_tunnel_dst_index(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry add failed "
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
  }
#endif /* P4_MIRROR_NEXTHOP_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite tunnel entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite tunnel entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_unicast_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_rw_type_t rw_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(dmac);
  UNUSED(rw_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2) {
    pd_status = p4_pd_dc_rewrite_table_modify_with_set_l2_rewrite(
        switch_cfg_sess_hdl, device, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite unicast entry update failed "
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
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  } else {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0x0, sizeof(p4_pd_dc_set_l3_rewrite_action_spec_t));
    SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
    action_spec.action_bd = bd;
    pd_status = p4_pd_dc_rewrite_table_modify_with_set_l3_rewrite(
        switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite unicast entry update failed "
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
#endif /* P4_L3_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite unicast entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite unicast entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_tunnel_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_type_t neigh_type,
    switch_neighbor_rw_type_t rw_type,
    switch_tunnel_t tunnel_index,
    switch_vni_t tunnel_vni,
    switch_id_t tunnel_dst_index,
    switch_tunnel_type_egress_t tunnel_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(dmac);
  UNUSED(neigh_type);
  UNUSED(rw_type);
  UNUSED(tunnel_index);
  UNUSED(tunnel_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE
  if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2) {
    p4_pd_dc_set_l2_rewrite_with_tunnel_action_spec_t action_spec;
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
#endif
    pd_status = p4_pd_dc_rewrite_table_modify_with_set_l2_rewrite_with_tunnel(
        switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry update failed "
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
  } else if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L3) {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_with_tunnel_and_ingress_vrf_action_spec_t
        action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(
            p4_pd_dc_set_l3_rewrite_with_tunnel_and_ingress_vrf_action_spec_t));
    SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
#endif
    pd_status =
        p4_pd_dc_rewrite_table_modify_with_set_l3_rewrite_with_tunnel_and_ingress_vrf(
            switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry update failed "
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
#endif /* P4_L3_DISABLE */
  } else if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L3_VNI) {
#ifndef P4_L3_DISABLE
    p4_pd_dc_set_l3_rewrite_with_tunnel_vnid_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(p4_pd_dc_set_l3_rewrite_with_tunnel_vnid_action_spec_t));
    SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
#endif
    action_spec.action_vnid = tunnel_vni;
    pd_status =
        p4_pd_dc_rewrite_table_modify_with_set_l3_rewrite_with_tunnel_vnid(
            switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry update failed "
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
#endif /* P4_L3_DISABLE */
  }
#endif /* P4_TUNNEL_DISABLE */

#ifndef P4_MIRROR_NEXTHOP_DISABLE
  if (rw_type == SWITCH_API_NEIGHBOR_RW_TYPE_L2_MIRROR) {
    p4_pd_dc_set_l2_rewrite_with_tunnel_dst_index_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
#ifndef P4_TUNNEL_V4_VXLAN_ONLY
    action_spec.action_tunnel_type = tunnel_type;
#endif
#ifndef P4_TUNNEL_INDEX_BRIDGE_ENABLE
    action_spec.action_tunnel_index = tunnel_index;
#endif
    action_spec.action_tunnel_dst_index = tunnel_dst_index;
    pd_status =
        p4_pd_dc_rewrite_table_modify_with_set_l2_rewrite_with_tunnel_dst_index(
            switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rewrite tunnel entry update failed "
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
  }
#endif /* P4_MIRROR_NEXTHOP_DISABLE */

#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite tunnel entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite tunnel entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_entry_delete(
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
      p4_pd_dc_rewrite_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rewrite entry delete failed "
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
        "rewrite entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_default_entry_add(
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

  pd_status = p4_pd_dc_rewrite_set_default_action_set_l2_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rewrite table default add failed "
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
        "rewrite table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite table entry default add failed "
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
