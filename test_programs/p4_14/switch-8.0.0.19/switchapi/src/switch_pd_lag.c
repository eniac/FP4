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

switch_status_t switch_pd_lag_group_create(switch_device_t device,
                                           switch_pd_grp_hdl_t *pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t pd_device;

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_lag_action_profile_create_group(
      switch_cfg_sess_hdl, pd_device, MAX_LAG_GROUP_SIZE, pd_grp_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag group create failed "
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
    pd_entry.pd_grp_hdl = *pd_grp_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag group create success "
        "on device %d 0x%lx\n",
        device,
        *pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group create failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_group_delete(switch_device_t device,
                                           switch_pd_grp_hdl_t pd_grp_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(pd_grp_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
  pd_status = p4_pd_dc_lag_action_profile_del_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag group delete failed "
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
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag group delete success "
        "on device %d 0x%lx\n",
        device,
        pd_grp_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_grp_hdl);
  }
  return status;
}

switch_status_t switch_pd_lag_group_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_index_t port_lag_index,
    switch_pd_mbr_hdl_t *pd_mbr_hdl,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dev_port);
  UNUSED(port_lag_index);
  UNUSED(pd_mbr_hdl);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_lag_group_match_spec_t match_spec;
  p4_pd_dev_target_t pd_device;
  p4_pd_dc_set_lag_port_action_spec_t action_spec;

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_lag_group_match_spec_t));
  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_set_lag_port_action_spec_t));

#ifdef P4_FAST_FAILOVER_ENABLE
  action_spec.action_fallback_check = 0;
#endif /* P4_FAST_FAILOVER_ENABLE */

  action_spec.action_port = dev_port;
  match_spec.ingress_metadata_egress_port_lag_index = port_lag_index;

  pd_status = p4_pd_dc_lag_action_profile_add_member_with_set_lag_port(
      switch_cfg_sess_hdl, pd_device, &action_spec, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry add failed "
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

  pd_status = p4_pd_dc_lag_group_add_entry(
      switch_cfg_sess_hdl, pd_device, &match_spec, *pd_mbr_hdl, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag group entry add failed "
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
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag group entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_group_table_with_selector_add(
    switch_device_t device,
    switch_port_lag_index_t port_lag_index,
    switch_pd_grp_hdl_t pd_grp_hdl,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port_lag_index);
  UNUSED(pd_grp_hdl);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dc_lag_group_match_spec_t match_spec;
  p4_pd_dev_target_t pd_device;

  pd_device.device_id = device;
  pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_lag_group_match_spec_t));

  match_spec.ingress_metadata_egress_port_lag_index = port_lag_index;

  pd_status = p4_pd_dc_lag_group_add_entry_with_selector(
      switch_cfg_sess_hdl, pd_device, &match_spec, pd_grp_hdl, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag group entry add failed "
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
        "lag group entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_member_add(switch_device_t device,
                                         switch_pd_grp_hdl_t pd_grp_hdl,
                                         switch_dev_port_t dev_port,
                                         switch_pd_mbr_hdl_t *pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);
  UNUSED(dev_port);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_set_lag_port_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_set_lag_port_action_spec_t));

  action_spec.action_port = dev_port;
  pd_status = p4_pd_dc_lag_action_profile_add_member_with_set_lag_port(
      switch_cfg_sess_hdl, p4_pd_device, &action_spec, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry add failed "
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

  pd_status = p4_pd_dc_lag_action_profile_add_member_to_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, *pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry add failed "
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
    pd_entry.pd_mbr_hdl = *pd_mbr_hdl;
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag member entry add success "
        "on device %d 0x%lx\n",
        device,
        *pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag member entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_member_update(switch_device_t device,
                                            switch_pd_grp_hdl_t pd_grp_hdl,
                                            switch_dev_port_t dev_port,
                                            switch_pd_mbr_hdl_t pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pd_grp_hdl);
  UNUSED(dev_port);
  UNUSED(pd_mbr_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_set_lag_port_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  action_spec.action_port = dev_port;
  pd_status = p4_pd_dc_lag_action_profile_modify_member_with_set_lag_port(
      switch_cfg_sess_hdl, p4_pd_device.device_id, pd_mbr_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry update failed "
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
        "lag member entry update success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag member entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_member_delete(switch_device_t device,
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

  pd_status = p4_pd_dc_lag_action_profile_del_member_from_group(
      switch_cfg_sess_hdl, device, pd_grp_hdl, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry delete failed "
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
    pd_entry.pd_grp_hdl = pd_grp_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  pd_status = p4_pd_dc_lag_action_profile_del_member(
      switch_cfg_sess_hdl, device, pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag member entry delete failed "
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
        "lag member entry delete success "
        "on device %d 0x%lx\n",
        device,
        pd_mbr_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag member entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        pd_mbr_hdl);
  }
  return status;
}

switch_status_t switch_pd_lag_group_table_entry_delete(
    switch_device_t device,
    bool port,
    switch_pd_hdl_t entry_hdl,
    switch_pd_mbr_hdl_t pd_mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status =
      p4_pd_dc_lag_group_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag group entry delete failed "
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

  if (port) {
    pd_status = p4_pd_dc_lag_action_profile_del_member(
        switch_cfg_sess_hdl, device, pd_mbr_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "lag member entry delete failed "
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

switch_status_t switch_pd_lag_member_deactivate(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_group_hdl,
    switch_pd_mbr_hdl_t mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = p4_pd_dc_lag_action_profile_group_member_state_set(
      switch_cfg_sess_hdl,
      device,
      pd_group_hdl,
      mbr_hdl,
      P4_PD_GRP_MBR_STATE_INACTIVE);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_lag_member_activate(switch_device_t device,
                                              switch_pd_grp_hdl_t pd_group_hdl,
                                              switch_pd_mbr_hdl_t mbr_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = p4_pd_dc_lag_action_profile_group_member_state_set(
      switch_cfg_sess_hdl,
      device,
      pd_group_hdl,
      mbr_hdl,
      P4_PD_GRP_MBR_STATE_ACTIVE);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_lag_table_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch_pd_mbr_hdl_t pd_mbr_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_lag_action_profile_add_member_with_set_lag_miss(
      switch_cfg_sess_hdl, p4_pd_device, &pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_lag_group_set_default_entry(
      switch_cfg_sess_hdl, p4_pd_device, pd_mbr_hdl, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "lag table default add failed "
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
        "lag table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag table entry default add failed "
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
