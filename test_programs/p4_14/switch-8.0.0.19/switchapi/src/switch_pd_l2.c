
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

switch_status_t switch_pd_dmac_table_entry_add(
    switch_device_t device,
    switch_handle_type_t handle_type,
    switch_bd_t bd,
    switch_mac_info_t *mac_info,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_t nhop_index,
    switch_mgid_t mgid_index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(handle_type);
  UNUSED(mac_info);
  UNUSED(ifindex);
  UNUSED(nhop_index);
  UNUSED(mgid_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  p4_pd_dc_dmac_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (!mac_info || !entry_hdl) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_PD_LOG_ERROR(
        "dmac entry add failed"
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
    return status;
  }

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_dmac_match_spec_t));

  match_spec.ingress_metadata_bd = bd;
  SWITCH_MEMCPY(match_spec.l2_metadata_lkp_mac_da,
                &mac_info->mac_entry.mac,
                SWITCH_MAC_LENGTH);

  if (mac_info->mac_action == SWITCH_MAC_ACTION_FORWARD) {
    switch (handle_type) {
      case SWITCH_HANDLE_TYPE_PORT:
      case SWITCH_HANDLE_TYPE_LAG:
      case SWITCH_HANDLE_TYPE_INTERFACE: {
        p4_pd_dc_dmac_hit_action_spec_t action_spec;
        SWITCH_MEMSET(
            &action_spec, 0x0, sizeof(p4_pd_dc_dmac_hit_action_spec_t));
        action_spec.action_ifindex = ifindex;
        action_spec.action_port_lag_index = port_lag_index;

        pd_status = p4_pd_dc_dmac_table_add_with_dmac_hit(switch_cfg_sess_hdl,
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
      case SWITCH_HANDLE_TYPE_MGID: {
        p4_pd_dc_dmac_multicast_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec,
                      0x0,
                      sizeof(p4_pd_dc_dmac_multicast_hit_action_spec_t));
        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_dmac_table_add_with_dmac_multicast_hit(switch_cfg_sess_hdl,
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
      case SWITCH_HANDLE_TYPE_NHOP: {
        p4_pd_dc_dmac_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec,
                      0x0,
                      sizeof(p4_pd_dc_dmac_redirect_nexthop_action_spec_t));
        action_spec.action_nexthop_index = nhop_index;

        pd_status = p4_pd_dc_dmac_table_add_with_dmac_redirect_nexthop(
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
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "dmac table entry add failed "
              "on device %d: %s\n",
              device,
              switch_error_to_string(status));
          return status;
        }
    }
  } else {
    pd_status = p4_pd_dc_dmac_table_add_with_dmac_drop(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
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
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dmac entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dmac entry add failed"
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_dmac_table_entry_update(
    switch_device_t device,
    switch_handle_type_t handle_type,
    switch_bd_t bd,
    switch_mac_info_t *mac_info,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_t nhop_index,
    switch_mgid_t mgid_index,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(handle_type);
  UNUSED(mac_info);
  UNUSED(ifindex);
  UNUSED(nhop_index);
  UNUSED(mgid_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  if (!mac_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_PD_LOG_ERROR(
        "dmac entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
    return status;
  }

  if (mac_info->mac_action == SWITCH_MAC_ACTION_FORWARD) {
    switch (handle_type) {
      case SWITCH_HANDLE_TYPE_PORT:
      case SWITCH_HANDLE_TYPE_LAG:
      case SWITCH_HANDLE_TYPE_INTERFACE: {
        p4_pd_dc_dmac_hit_action_spec_t action_spec;
        SWITCH_MEMSET(
            &action_spec, 0x0, sizeof(p4_pd_dc_dmac_hit_action_spec_t));
        action_spec.action_ifindex = ifindex;
        action_spec.action_port_lag_index = port_lag_index;

        pd_status = p4_pd_dc_dmac_table_modify_with_dmac_hit(
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
      case SWITCH_HANDLE_TYPE_MGID: {
        p4_pd_dc_dmac_multicast_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec,
                      0x0,
                      sizeof(p4_pd_dc_dmac_multicast_hit_action_spec_t));
        action_spec.action_mc_index = mgid_index;

        pd_status = p4_pd_dc_dmac_table_modify_with_dmac_multicast_hit(
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
      case SWITCH_HANDLE_TYPE_NHOP: {
        p4_pd_dc_dmac_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec,
                      0x0,
                      sizeof(p4_pd_dc_dmac_redirect_nexthop_action_spec_t));
        action_spec.action_nexthop_index = nhop_index;

        pd_status = p4_pd_dc_dmac_table_modify_with_dmac_redirect_nexthop(
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
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "dmac table entry update failed "
              "on device %d: %s\n",
              device,
              switch_error_to_string(status));
          return status;
        }
    }
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dmac entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dmac entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_dmac_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  pd_status =
      p4_pd_dc_dmac_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dmac entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dmac entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_mac_entry_aging_time_set(
    switch_device_t device,
    switch_pd_hdl_t entry_hdl,
    switch_uint32_t aging_interval) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = p4_pd_dc_smac_set_ttl(
      switch_cfg_sess_hdl, device, entry_hdl, aging_interval);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "smac entry aging set success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "smac entry aging set failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_smac_table_entry_add(switch_device_t device,
                                               switch_bd_t bd,
                                               switch_mac_info_t *mac_info,
                                               switch_ifindex_t ifindex,
                                               switch_uint32_t aging_time,
                                               switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mac_info);
  UNUSED(ifindex);
  UNUSED(aging_time);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  if (!mac_info || !entry_hdl) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_PD_LOG_ERROR(
        "smac entry add failed"
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
    return status;
  }

  p4_pd_dc_smac_match_spec_t match_spec;
  p4_pd_dc_smac_hit_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_smac_match_spec_t));
  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_smac_hit_action_spec_t));

  match_spec.ingress_metadata_bd = bd;
  SWITCH_MEMCPY(match_spec.l2_metadata_lkp_mac_sa,
                &mac_info->mac_entry.mac,
                SWITCH_MAC_LENGTH);

  action_spec.action_ifindex = ifindex;

  pd_status = p4_pd_dc_smac_table_add_with_smac_hit(switch_cfg_sess_hdl,
                                                    p4_pd_device,
                                                    &match_spec,
                                                    &action_spec,
                                                    aging_time,
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
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "smac entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "smac entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_smac_table_entry_update(switch_device_t device,
                                                  switch_bd_t bd,
                                                  switch_mac_info_t *mac_info,
                                                  switch_ifindex_t ifindex,
                                                  switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mac_info);
  UNUSED(ifindex);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE
  p4_pd_dc_smac_hit_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_smac_hit_action_spec_t));
  action_spec.action_ifindex = ifindex;

  pd_status = p4_pd_dc_smac_table_modify_with_smac_hit(
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

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "smac entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "smac entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_smac_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  pd_status =
      p4_pd_dc_smac_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dmac entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dmac entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE
switch_pd_status_t switch_pd_mac_learn_notify_cb(
    switch_pd_sess_hdl_t sess_hdl,
    p4_pd_dc_mac_learn_digest_digest_msg_t *msg,
    void *client_data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = 0;

  p4_pd_dc_mac_learn_digest_digest_entry_t *learn_entry = NULL;
  switch_pd_mac_info_t *pd_mac_entries = NULL;
  switch_pd_mac_info_t *pd_mac_entry = NULL;
  switch_device_t device;
  switch_uint16_t index = 0;

  SWITCH_ASSERT(msg != NULL);
  SWITCH_ASSERT(client_data != NULL);

  if (!msg || !client_data || msg->num_entries == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mac pd learn notify failed :%s\n",
                     switch_error_to_string(status));
    pd_status = switch_status_to_pd_status(status);
    p4_pd_dc_mac_learn_digest_notify_ack(sess_hdl, msg);
    return pd_status;
  }

  device = *((switch_device_t *)(client_data));
  pd_mac_entries =
      SWITCH_MALLOC(device, sizeof(switch_pd_mac_info_t), msg->num_entries);

  SWITCH_MEMSET(
      pd_mac_entries, 0x0, sizeof(switch_pd_mac_info_t) * msg->num_entries);

  for (index = 0; index < msg->num_entries; index++) {
    learn_entry = &(msg->entries[index]);
    pd_mac_entry = &pd_mac_entries[index];
    pd_mac_entry->bd = learn_entry->ingress_metadata_bd;
    pd_mac_entry->ifindex = learn_entry->ingress_metadata_ifindex;
    SWITCH_MEMCPY(&pd_mac_entry->mac,
                  learn_entry->l2_metadata_lkp_mac_sa,
                  sizeof(switch_mac_addr_t));
  }

  status = switch_mac_learn_notify(device, pd_mac_entries, msg->num_entries);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac pd learn notify failed "
        "on device %d : %s\n",
        device,
        switch_error_to_string(status));
    pd_status = switch_status_to_pd_status(status);
    p4_pd_dc_mac_learn_digest_notify_ack(sess_hdl, msg);
    return pd_status;
  }

  p4_pd_dc_mac_learn_digest_notify_ack(sess_hdl, msg);

  return pd_status;
}
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

switch_status_t switch_pd_mac_learn_callback_register(switch_device_t device,
                                                      void *client_data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = 0;

  UNUSED(device);
  UNUSED(client_data);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  SWITCH_ASSERT(client_data != NULL);

  SWITCH_MEMCPY(client_data, &device, sizeof(switch_device_t));

  pd_status = p4_pd_dc_mac_learn_digest_register(switch_cfg_sess_hdl,
                                                 device,
                                                 switch_pd_mac_learn_notify_cb,
                                                 (void *)client_data);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac learn callback register success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac learn callback register failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mac_learn_callback_deregister(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  pd_status = p4_pd_dc_mac_learn_digest_deregister(switch_cfg_sess_hdl, device);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac learn callback register success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac learn callback register failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE
void
#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
switch_pd_mac_aging_notify_cb(
        switch_pd_hdl_t pd_hdl,
        void *client_data)
{

  if (client_data == NULL) {
    SWITCH_PD_LOG_ERROR("mac pd aging notify failed : %s\n");
    return;
  }
  switch_device_t device = *((switch_device_t *)client_data);
#else
switch_pd_mac_aging_notify_cb(
        switch_int32_t device,
        switch_pd_hdl_t pd_hdl,
        void *client_data)
{
#endif /* __TARGET_TOFINO__ */
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_mac_aging_notify(device, pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("mac pd aging notify failed : %s\n",
                        switch_error_to_string(status));
  }
  return;
}
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

switch_status_t switch_pd_mac_aging_callback_register(
    switch_device_t device,
    switch_uint32_t min_aging_time,
    switch_uint32_t max_aging_time,
    switch_uint32_t query_interval,
    void *client_data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(min_aging_time);
  UNUSED(max_aging_time);
  UNUSED(query_interval);
  UNUSED(client_data);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  SWITCH_MEMCPY(client_data, &device, sizeof(switch_device_t));

#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
  pd_status = p4_pd_dc_smac_enable_entry_timeout(switch_cfg_sess_hdl,
                                                 switch_pd_mac_aging_notify_cb,
                                                 max_aging_time,
                                                 client_data);
#else
  p4_pd_idle_time_params_t idle_time_params;
  idle_time_params.mode = PD_NOTIFY_MODE;
  idle_time_params.params.notify.ttl_query_interval = query_interval;
  idle_time_params.params.notify.max_ttl = max_aging_time;
  idle_time_params.params.notify.min_ttl = min_aging_time;
  idle_time_params.params.notify.callback_fn = switch_pd_mac_aging_notify_cb;
  idle_time_params.params.notify.cookie = client_data;
  p4_pd_dc_smac_idle_tmo_enable(switch_cfg_sess_hdl, device, idle_time_params);
#endif /* __TARGET_TOFINO__ */

  status = switch_pd_status_to_status(pd_status);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac aging  callback register success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac aging callback register failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_smac_hit_state_get(switch_device_t device,
                                             switch_pd_hdl_t pd_hdl,
                                             bool *is_hit) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_hdl);
  UNUSED(is_hit);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
  p4_pd_hit_state_t hit_state = 0;
#else
  p4_pd_idle_time_hit_state_e hit_state = 0;
#endif /* __TARGET_TOFINO__ */

  *is_hit = TRUE;
#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
  pd_status =
      p4_pd_dc_smac_get_hit_state(switch_cfg_sess_hdl, pd_hdl, &hit_state);
#else
  pd_status = p4_pd_dc_smac_get_hit_state(
      switch_cfg_sess_hdl, device, pd_hdl, &hit_state);
#endif /* __TARGET_TOFINO__ */

  status = switch_pd_status_to_status(pd_status);
  if (status == SWITCH_STATUS_SUCCESS) {
#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
    if (hit_state == ENTRY_IDLE) {
#else
    if (hit_state == PD_ENTRY_IDLE) {
#endif /* __TARGET_TOFINO__ */
      *is_hit = FALSE;
    }
  }
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dmac hit state get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dmac hit state get failed "
        "on device %d : %s (pd: 0x%x)"
        "for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_hdl);
  }

  return status;
}

switch_status_t switch_pd_spanning_tree_table_entry_add(
    switch_device_t device,
    switch_stp_group_t stp_group,
    switch_ifindex_t ifindex,
    switch_stp_state_t stp_state,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(stp_group);
  UNUSED(ifindex);
  UNUSED(stp_state);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) && !defined(P4_STP_DISABLE)

  p4_pd_dc_spanning_tree_match_spec_t match_spec;
  p4_pd_dc_set_stp_state_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_spanning_tree_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_stp_state_action_spec_t));

  match_spec.ingress_metadata_ifindex = ifindex;
  match_spec.l2_metadata_stp_group = stp_group;
  action_spec.action_stp_state = stp_state;

  pd_status = p4_pd_dc_spanning_tree_table_add_with_set_stp_state(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);

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
        "stp table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE && P4_STP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "stp table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "stp table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_spanning_tree_table_entry_update(
    switch_device_t device,
    switch_stp_group_t stp_group,
    switch_ifindex_t ifindex,
    switch_stp_state_t stp_state,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(stp_group);
  UNUSED(ifindex);
  UNUSED(stp_state);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) && !defined(P4_STP_DISABLE)

  p4_pd_dc_set_stp_state_action_spec_t action_spec;

  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_stp_state_action_spec_t));
  action_spec.action_stp_state = stp_state;

  pd_status = p4_pd_dc_spanning_tree_table_modify_with_set_stp_state(
      switch_cfg_sess_hdl, device, entry_hdl, &action_spec);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE && P4_STP_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_spanning_tree_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) && !defined(P4_STP_DISABLE)

  pd_status = p4_pd_dc_spanning_tree_table_delete(
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
        "stp table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE && P4_STP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "stp table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "stp table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_validate_outer_ethernet_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_malformed_outer_ethernet_packet_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  action_spec.action_drop_reason = DROP_OUTER_ETHERNET_MISS;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_set_default_action_malformed_outer_ethernet_packet(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate outer ethernet table default add failed "
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
        "validate outer ethernet table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "validate outer ethernet table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_validate_packet_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  int priority = 100;
  int i;
  p4_pd_dc_validate_packet_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_validate_packet_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  /* src is multicast */
  p4_pd_dc_set_malformed_packet_action_spec_t action_spec;
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l2_metadata_lkp_mac_sa[0] = 0x01;
  match_spec.l2_metadata_lkp_mac_sa_mask[0] = 0x01;
  action_spec.action_drop_reason = DROP_SRC_MAC_MULTICAST;

  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* src is zero */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l2_metadata_lkp_mac_sa_mask[0] = 0xff;
  match_spec.l2_metadata_lkp_mac_sa_mask[1] = 0xff;
  match_spec.l2_metadata_lkp_mac_sa_mask[2] = 0xff;
  match_spec.l2_metadata_lkp_mac_sa_mask[3] = 0xff;
  match_spec.l2_metadata_lkp_mac_sa_mask[4] = 0xff;
  match_spec.l2_metadata_lkp_mac_sa_mask[5] = 0xff;
  action_spec.action_drop_reason = DROP_SRC_MAC_ZERO;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* dst is zero */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l2_metadata_lkp_mac_da_mask[0] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[1] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[2] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[3] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[4] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[5] = 0xff;
  action_spec.action_drop_reason = DROP_DST_MAC_ZERO;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

/* IPv4 IHL is invalid */
#ifndef P4_TUNNEL_DISABLE
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.tunnel_metadata_tunnel_terminate = true;
  match_spec.tunnel_metadata_tunnel_terminate_mask = 0xff;
  match_spec.inner_ipv4_ihl = 0;
  match_spec.inner_ipv4_ihl_mask = 0xfc;
  action_spec.action_drop_reason = DROP_IP_IHL_INVALID;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.inner_ipv4_ihl = 4;
  match_spec.inner_ipv4_ihl_mask = 0xff;
  match_spec.tunnel_metadata_tunnel_terminate = true;
  match_spec.tunnel_metadata_tunnel_terminate_mask = 0xff;
  action_spec.action_drop_reason = DROP_IP_IHL_INVALID;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_TUNNEL_DISABLE */
  /* IPv4 TTL is zero */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_ttl = 0;
  match_spec.l3_metadata_lkp_ip_ttl_mask = 0xff;
  action_spec.action_drop_reason = DROP_IP_TTL_ZERO;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* IPv6 TTL is zero */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.l3_metadata_lkp_ip_ttl = 0;
  match_spec.l3_metadata_lkp_ip_ttl_mask = 0xff;
  action_spec.action_drop_reason = DROP_IP_TTL_ZERO;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv4 src is loopback */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.ipv4_metadata_lkp_ipv4_sa = 0x7f000000;
  match_spec.ipv4_metadata_lkp_ipv4_sa_mask = 0xff000000;
  action_spec.action_drop_reason = DROP_IP_SRC_LOOPBACK;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv4 src is multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.ipv4_metadata_lkp_ipv4_sa = 0xe0000000;
  match_spec.ipv4_metadata_lkp_ipv4_sa_mask = 0xf0000000;
  action_spec.action_drop_reason = DROP_IP_SRC_MULTICAST;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv4 version invalid */
  for (i = 0; i < 16; i++) {
    if (i == SWITCH_IPV4_VERSION) {
      continue;
    }
    SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv4;
    match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
    match_spec.l3_metadata_lkp_ip_version = i;
    match_spec.l3_metadata_lkp_ip_version_mask = 0xff;
    action_spec.action_drop_reason = DROP_IP_VERSION_INVALID;
    pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        priority++,
        &action_spec,
        &entry_hdl);
    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
      pd_entry.match_spec = (switch_uint8_t *)&match_spec;
      pd_entry.match_spec_size = sizeof(match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&action_spec;
      pd_entry.action_spec_size = sizeof(action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "validate packet table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

#ifndef IPV6_DISABLE
  /* ipv6 version invalid */
  for (i = 0; i < 16; i++) {
    if (i == SWITCH_IPV6_VERSION) {
      continue;
    }
    SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
    match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
    match_spec.l3_metadata_lkp_ip_version = i;
    match_spec.l3_metadata_lkp_ip_version_mask = 0xff;
    action_spec.action_drop_reason = DROP_IP_VERSION_INVALID;
    pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        priority++,
        &action_spec,
        &entry_hdl);
    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
      pd_entry.match_spec = (switch_uint8_t *)&match_spec;
      pd_entry.match_spec_size = sizeof(match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&action_spec;
      pd_entry.action_spec_size = sizeof(action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "validate packet table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
  }

  /* ipv6 src is multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.ipv6_metadata_lkp_ipv6_sa[0] = 0xff;
  match_spec.ipv6_metadata_lkp_ipv6_sa_mask[0] = 0xff;
  action_spec.action_drop_reason = DROP_IP_SRC_MULTICAST;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_malformed_packet(
      switch_cfg_sess_hdl,
      p4_pd_device,
      &match_spec,
      priority++,
      &action_spec,
      &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* IPV6_DISABLE */

  /* broadcast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.l2_metadata_lkp_mac_da[0] = 0xff;
  match_spec.l2_metadata_lkp_mac_da[1] = 0xff;
  match_spec.l2_metadata_lkp_mac_da[2] = 0xff;
  match_spec.l2_metadata_lkp_mac_da[3] = 0xff;
  match_spec.l2_metadata_lkp_mac_da[4] = 0xff;
  match_spec.l2_metadata_lkp_mac_da[5] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[0] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[1] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[2] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[3] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[4] = 0xff;
  match_spec.l2_metadata_lkp_mac_da_mask[5] = 0xff;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_broadcast(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority++, &entry_hdl);
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
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef IPV6_DISABLE
  /* multicast, source is ipv6 link local */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.l2_metadata_lkp_mac_da[0] = 0x01;
  match_spec.l2_metadata_lkp_mac_da_mask[0] = 0x01;
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.ipv6_metadata_lkp_ipv6_sa[0] = 0xfe;
  match_spec.ipv6_metadata_lkp_ipv6_sa[1] = 0x80;
  match_spec.ipv6_metadata_lkp_ipv6_sa_mask[0] = 0xff;
  match_spec.ipv6_metadata_lkp_ipv6_sa_mask[1] = 0xff;
  pd_status =
      p4_pd_dc_validate_packet_table_add_with_set_multicast_and_ipv6_src_is_link_local(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority++,
          &entry_hdl);
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
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* IPV6_DISABLE */

  /* multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.l2_metadata_lkp_mac_da[0] = 0x01;
  match_spec.l2_metadata_lkp_mac_da_mask[0] = 0x01;
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_multicast(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority++, &entry_hdl);
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
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef IPV6_DISABLE
  /* unicast, source is ipv6 link local */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.l3_metadata_lkp_ip_type = SWITCH_IP_TYPE_IPv6;
  match_spec.l3_metadata_lkp_ip_type_mask = 0xff;
  match_spec.ipv6_metadata_lkp_ipv6_sa[0] = 0xfe;
  match_spec.ipv6_metadata_lkp_ipv6_sa[1] = 0x80;
  match_spec.ipv6_metadata_lkp_ipv6_sa_mask[0] = 0xfe;
  match_spec.ipv6_metadata_lkp_ipv6_sa_mask[1] = 0x80;
  pd_status =
      p4_pd_dc_validate_packet_table_add_with_set_unicast_and_ipv6_src_is_link_local(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority++,
          &entry_hdl);
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
        "validate packet table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* IPV6_DISABLE */

  /* unicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  pd_status = p4_pd_dc_validate_packet_table_add_with_set_unicast(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority++, &entry_hdl);
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
        "validate packet table add failed "
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
        "validate packet table entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "validate packet table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mac_table_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_smac_set_default_action_smac_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "smac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_dmac_set_default_action_dmac_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "dmac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_egress_bd_map_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_bd_map_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress bd table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress bd table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress bd table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_learn_notify_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef L2_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_learn_notify_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  // add default entry
  pd_status = p4_pd_dc_learn_notify_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == none and l2 src miss
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_miss = 1;
  match_spec.l2_metadata_l2_src_miss_mask = 1;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_NONE;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 900, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == disabled and l2 src miss
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_miss = 1;
  match_spec.l2_metadata_l2_src_miss_mask = 1;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_DISABLED;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 901, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == learning and l2 src miss
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_miss = 1;
  match_spec.l2_metadata_l2_src_miss_mask = 1;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_LEARNING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 902, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == forwarding and l2 src miss
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_miss = 1;
  match_spec.l2_metadata_l2_src_miss_mask = 1;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_FORWARDING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 903, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == none and l2 src move
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_move = 0;
  match_spec.l2_metadata_l2_src_move_mask = 0xFFFF;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_NONE;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1000, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_NONE;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1001, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == disabled and l2 src move
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_move = 0;
  match_spec.l2_metadata_l2_src_move_mask = 0xFFFF;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_DISABLED;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1002, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_DISABLED;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1003, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == learning and l2 src move
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_move = 0;
  match_spec.l2_metadata_l2_src_move_mask = 0xFFFF;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_LEARNING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1004, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_LEARNING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1005, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // stp_state == forwarding and l2 src move
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_l2_src_move = 0;
  match_spec.l2_metadata_l2_src_move_mask = 0xFFFF;
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_FORWARDING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1006, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.l2_metadata_stp_state = SWITCH_PORT_STP_STATE_FORWARDING;
  match_spec.l2_metadata_stp_state_mask = 0xF;
  pd_status = p4_pd_dc_learn_notify_table_add_with_generate_learn_notify(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1007, &entry_hdl);
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
        "learn notify table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "learn notify table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "learn notify table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_validate_outer_ethernet_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_validate_outer_ethernet_match_spec_t match_spec;
  p4_pd_dc_malformed_outer_ethernet_packet_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  /* mac sa is zeros */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ethernet_srcAddr_mask[0] = 0xff;
  match_spec.ethernet_srcAddr_mask[1] = 0xff;
  match_spec.ethernet_srcAddr_mask[2] = 0xff;
  match_spec.ethernet_srcAddr_mask[3] = 0xff;
  match_spec.ethernet_srcAddr_mask[4] = 0xff;
  match_spec.ethernet_srcAddr_mask[5] = 0xff;
  action_spec.action_drop_reason = DROP_OUTER_SRC_MAC_ZERO;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_malformed_outer_ethernet_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          10,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* mac sa is multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ethernet_srcAddr[0] = 0x01;
  match_spec.ethernet_srcAddr_mask[0] = 0x01;
  action_spec.action_drop_reason = DROP_OUTER_SRC_MAC_MULTICAST;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_malformed_outer_ethernet_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          11,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* mac da is zeros */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ethernet_dstAddr_mask[0] = 0xff;
  match_spec.ethernet_dstAddr_mask[1] = 0xff;
  match_spec.ethernet_dstAddr_mask[2] = 0xff;
  match_spec.ethernet_dstAddr_mask[3] = 0xff;
  match_spec.ethernet_dstAddr_mask[4] = 0xff;
  match_spec.ethernet_dstAddr_mask[5] = 0xff;
  action_spec.action_drop_reason = DROP_OUTER_DST_MAC_ZERO;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_malformed_outer_ethernet_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          12,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  /* double tagged broadcast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ethernet_dstAddr[0] = 0xff;
  match_spec.ethernet_dstAddr[1] = 0xff;
  match_spec.ethernet_dstAddr[2] = 0xff;
  match_spec.ethernet_dstAddr[3] = 0xff;
  match_spec.ethernet_dstAddr[4] = 0xff;
  match_spec.ethernet_dstAddr[5] = 0xff;
  match_spec.ethernet_dstAddr_mask[0] = 0xff;
  match_spec.ethernet_dstAddr_mask[1] = 0xff;
  match_spec.ethernet_dstAddr_mask[2] = 0xff;
  match_spec.ethernet_dstAddr_mask[3] = 0xff;
  match_spec.ethernet_dstAddr_mask[4] = 0xff;
  match_spec.ethernet_dstAddr_mask[5] = 0xff;
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__1__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
  match_spec.vlan_tag__1__valid_mask = 1;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_broadcast_packet_double_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1000, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* double tagged multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ethernet_dstAddr[0] = 0x01;
  match_spec.ethernet_dstAddr_mask[0] = 0x01;
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__1__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
  match_spec.vlan_tag__1__valid_mask = 1;
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_multicast_packet_double_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1001, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* double tagged unicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__1__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
  match_spec.vlan_tag__1__valid_mask = 1;

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_unicast_packet_double_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1002, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  /* single tagged broadcast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ethernet_dstAddr[0] = 0xff;
  match_spec.ethernet_dstAddr[1] = 0xff;
  match_spec.ethernet_dstAddr[2] = 0xff;
  match_spec.ethernet_dstAddr[3] = 0xff;
  match_spec.ethernet_dstAddr[4] = 0xff;
  match_spec.ethernet_dstAddr[5] = 0xff;
  match_spec.ethernet_dstAddr_mask[0] = 0xff;
  match_spec.ethernet_dstAddr_mask[1] = 0xff;
  match_spec.ethernet_dstAddr_mask[2] = 0xff;
  match_spec.ethernet_dstAddr_mask[3] = 0xff;
  match_spec.ethernet_dstAddr_mask[4] = 0xff;
  match_spec.ethernet_dstAddr_mask[5] = 0xff;
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */
  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_broadcast_packet_single_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 2000, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* single tagged multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ethernet_dstAddr[0] = 0x01;
  match_spec.ethernet_dstAddr_mask[0] = 0x01;
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_multicast_packet_single_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 2001, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* single tagged unicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.vlan_tag__0__valid = 1;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_unicast_packet_single_tagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 2002, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* untagged packet broadcast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(
      match_spec.ethernet_dstAddr, 0xff, sizeof(match_spec.ethernet_dstAddr));
  SWITCH_MEMSET(match_spec.ethernet_dstAddr_mask,
                0xff,
                sizeof(match_spec.ethernet_dstAddr_mask));
  match_spec.vlan_tag__0__valid = 0;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_broadcast_packet_untagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 3000, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* untagged packet multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ethernet_dstAddr[0] = 0x01;
  match_spec.ethernet_dstAddr_mask[0] = 0x01;
  match_spec.vlan_tag__0__valid = 0;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_multicast_packet_untagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 3001, &entry_hdl);
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
        "validate outer ethernet table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* untagged packet unicast */
  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_validate_outer_ethernet_match_spec_t));
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.vlan_tag__0__valid = 0;
  match_spec.vlan_tag__0__valid_mask = 1;
#if !defined(P4_DOUBLE_TAGGED_DISABLE)
  match_spec.vlan_tag__1__valid = 0;
  match_spec.vlan_tag__1__valid_mask = 1;
#endif /* !P4_DOUBLE_TAGGED_DISABLE */

  pd_status =
      p4_pd_dc_validate_outer_ethernet_table_add_with_set_valid_outer_unicast_packet_untagged(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 3002, &entry_hdl);
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
        "validate outer ethernet table init failed "
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
        "validate outer etherent table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "validate outer etherent table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_l2_mac_learning_set(switch_device_t device,
                                              bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L2_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (enable) {
    pd_status = p4_pd_dc_smac_set_default_action_smac_miss(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  } else {
    pd_status = p4_pd_dc_smac_set_default_action_nop(
        switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "smac table learning set failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac table entry default add failed "
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
