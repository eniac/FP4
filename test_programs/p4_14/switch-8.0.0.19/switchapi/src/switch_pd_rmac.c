
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

#define SWITCH_AVG_PKT_SIZE (256)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_inner_rmac_table_entry_add(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rmac_group);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) || !defined(P4_L2_MULTICAST_DISABLE) || \
    !defined(P4_L3_MULTICAST_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_rmac_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_rmac_match_spec_t));
  match_spec.l3_metadata_rmac_group = rmac_group;
  SWITCH_MEMCPY(match_spec.l2_metadata_lkp_mac_da, mac, ETH_LEN);

  pd_status = p4_pd_dc_rmac_table_add_with_rmac_hit(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry add failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_L2_DISABLE || !P4_L2_MULTICAST_DISABLE || \
          P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "inner rmac entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_inner_rmac_table_entry_update(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rmac_group);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) || !defined(P4_L2_MULTICAST_DISABLE) || \
    !defined(P4_L3_MULTICAST_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_rmac_table_modify_with_rmac_hit(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry modify failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_L2_DISABLE || !P4_L2_MULTICAST_DISABLE || \
          P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "inner rmac entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_inner_rmac_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE) || !defined(P4_L2_MULTICAST_DISABLE) || \
    !defined(P4_L3_MULTICAST_DISABLE)

  pd_status =
      p4_pd_dc_rmac_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry delete failed "
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

#endif /* !P4_L2_DISABLE || !P4_L2_MULTICAST_DISABLE || \
          P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "inner rmac entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "inner rmac entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_smac_rewrite_table_entry_add(
    switch_device_t device,
    switch_smac_entry_t *smac_entry,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(smac_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE
  p4_pd_dc_smac_rewrite_match_spec_t match_spec;
  p4_pd_dc_rewrite_smac_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_smac_rewrite_match_spec_t));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(p4_pd_dc_rewrite_smac_action_spec_t));
  match_spec.egress_metadata_smac_idx = smac_entry->smac_index;
  SWITCH_MEMCPY(action_spec.action_smac, &smac_entry->mac, ETH_LEN);

  pd_status = p4_pd_dc_smac_rewrite_table_add_with_rewrite_smac(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "smac rewrite entry add failed "
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
        "smac rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "smac rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_smac_rewrite_table_entry_update(
    switch_device_t device,
    switch_smac_entry_t *smac_entry,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(smac_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE
  p4_pd_dc_rewrite_smac_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  SWITCH_MEMCPY(action_spec.action_smac, &smac_entry->mac, ETH_LEN);

  pd_status = p4_pd_dc_smac_rewrite_table_modify_with_rewrite_smac(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "smac rewrite entry update failed "
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
        "smac rewrite entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "smac rewrite entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_smac_rewrite_table_entry_delete(
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

  pd_status = p4_pd_dc_smac_rewrite_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "smac rewrite entry delete failed "
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
        "lag group entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_inner_rmac_table_default_entry_add(
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

#if !defined(P4_L2_DISABLE) || !defined(P4_L2_MULTICAST_DISABLE) || \
    !defined(P4_L3_MULTICAST_DISABLE)
  pd_status = p4_pd_dc_rmac_set_default_action_rmac_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rmac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_L2_DISABLE || !P4_L2_MULTICAST_DISABLE || \
          P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rmac table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rmac table entry default add failed "
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
