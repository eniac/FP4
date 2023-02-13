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

switch_status_t switch_pd_ila_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

#ifdef SWITCH_PD
#ifdef P4_ILA_ENABLE

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_ILA_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ila_table_entry_delete(switch_device_t device,
                                                 switch_ila_info_t *ila_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(ila_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_ILA_ENABLE

  pd_status = p4_pd_dc_ipv6_fib_table_delete(
      switch_cfg_sess_hdl, device, ila_info->hw_entry);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_ILA_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ila entry delete success "
        "on device %d 0x%lx\n",
        device,
        ila_info->hw_entry);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ila entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        ila_info->hw_entry);
  }

  return status;
}

switch_status_t switch_pd_ila_table_entry_update(switch_device_t device,
                                                 switch_ila_info_t *ila_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ila_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_ILA_ENABLE

  switch_api_ila_info_t *api_ila_info = &ila_info->api_ila_info;
  uint32_t nhop_index = handle_to_id(ila_info->nhop_handle);

  if (ila_info->ecmp) {
    p4_pd_dc_ila_hit_ecmp_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_ila_hit_ecmp_action_spec_t));
    action_spec.action_ecmp_index = nhop_index;
    SWITCH_MEMCPY(
        &action_spec.action_ila_addr, &api_ila_info->sir_addr.ip.v6addr, 16);
    pd_status = p4_pd_dc_ipv6_fib_table_modify_with_ila_hit_ecmp(
        switch_cfg_sess_hdl, device, ila_info->hw_entry, &action_spec);
  } else {
    p4_pd_dc_ila_hit_nexthop_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ila_hit_nexthop_action_spec_t));
    action_spec.action_nexthop_index = nhop_index;
    SWITCH_MEMCPY(
        &action_spec.action_ila_addr, &api_ila_info->sir_addr.ip.v6addr, 16);
    pd_status = p4_pd_dc_ipv6_fib_table_modify_with_ila_hit_nexthop(
        switch_cfg_sess_hdl, device, ila_info->hw_entry, &action_spec);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_ILA_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ila entry update success "
        "on device %d 0x%lx\n",
        device,
        ila_info->hw_entry);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ila entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        ila_info->hw_entry);
  }

  return status;
}

switch_status_t switch_pd_ila_table_entry_add(switch_device_t device,
                                              switch_ila_info_t *ila_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ila_info);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_ILA_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv6_fib_match_spec_t match_spec;
  switch_api_ila_info_t *api_ila_info = &ila_info->api_ila_info;
  uint32_t nhop_index = handle_to_id(ila_info->nhop_handle);

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv6_fib_match_spec_t));
  match_spec.l3_metadata_vrf = handle_to_id(api_ila_info->vrf_handle);
  SWITCH_MEMCPY(&match_spec.ipv6_metadata_lkp_ipv6_da,
                &api_ila_info->sir_addr.ip.v6addr,
                16);

  if (ila_info->ecmp) {
    p4_pd_dc_ila_hit_ecmp_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_ila_hit_ecmp_action_spec_t));
    action_spec.action_ecmp_index = nhop_index;
    SWITCH_MEMCPY(
        &action_spec.action_ila_addr, &ila_info->ila_addr.ip.v6addr, 16);
    pd_status =
        p4_pd_dc_ipv6_fib_table_add_with_ila_hit_ecmp(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      &action_spec,
                                                      &ila_info->hw_entry);
  } else {
    p4_pd_dc_ila_hit_nexthop_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ila_hit_nexthop_action_spec_t));
    action_spec.action_nexthop_index = nhop_index;
    SWITCH_MEMCPY(
        &action_spec.action_ila_addr, &ila_info->ila_addr.ip.v6addr, 16);
    pd_status =
        p4_pd_dc_ipv6_fib_table_add_with_ila_hit_nexthop(switch_cfg_sess_hdl,
                                                         p4_pd_device,
                                                         &match_spec,
                                                         &action_spec,
                                                         &ila_info->hw_entry);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_ILA_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ila entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ila entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}
