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
#include "switchapi/switch_acl.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"
#include "switch_log_int.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_ACL

switch_status_t switch_system_acl_default_entries_delete(
    switch_device_t device) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mod_ace_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  for (index = 0; index < SWITCH_ACL_DEFAULT_MAX; index++) {
    acl_handle = acl_ctx->acl_handle[index];
    ace_handle = acl_ctx->ace_handle[index];
    mod_ace_handle = acl_ctx->mod_ace_handle[index];

    if (acl_handle == SWITCH_API_INVALID_HANDLE ||
        ace_handle == SWITCH_API_INVALID_HANDLE) {
      SWITCH_LOG_DEBUG(
          "Acl(0x%lx)/Ace(0x%lx) handle is invalid, skip acl_delete for index: "
          "%d",
          acl_handle,
          ace_handle,
          index);
      continue;
    }

    status = switch_api_acl_rule_delete(device, acl_handle, ace_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl default entry delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
    }

    if (mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
      status = switch_api_acl_rule_delete(device, acl_handle, mod_ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl default MoD entry delete failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
      }
    }

    status = switch_api_acl_list_delete(device, acl_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl default entry delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
    }
    acl_ctx->acl_handle[index] = SWITCH_API_INVALID_HANDLE;
    acl_ctx->ace_handle[index] = SWITCH_API_INVALID_HANDLE;
    acl_ctx->mod_ace_handle[index] = SWITCH_API_INVALID_HANDLE;
  }

  return status;
}

static switch_uint16_t switch_system_acl_priority(switch_uint32_t acl_type,
                                                  bool mod_acl) {
  switch_uint16_t mod_acl_high_piority_base =
      SWITCH_DEFAULT_INERNAL_MOD_ACL_HIGH_PRIO_START;
  switch_uint16_t mod_acl_lo_priority_base =
      SWITCH_DEFAULT_INTERNAL_MOD_ACL_LOW_PRIO_START;
  switch_uint16_t acl_high_priority_base =
      SWITCH_DEFAULT_INTERNAL_ACL_HIGH_PRIO_START;
  switch_uint16_t acl_lo_priority_base =
      SWITCH_DEFAULT_INTERNAL_ACL_LOW_PRIO_START;
  bool high_prio = FALSE;
  switch_uint16_t final_priority = 0;

  switch (acl_type) {
    // LOW priority ACLs
    case SWITCH_ACL_DROP:
    case SWITCH_ACL_PV_MISS:
    case SWITCH_ACL_STP_BLOCKED_DROP:
    case SWITCH_ACL_STP_LEARN_DROP:
    case SWITCH_ACL_DENY_DROP:
    case SWITCH_ACL_RACL_DENY_DROP:
    case SWITCH_ACL_URPF_FAIL_DROP:
    case SWITCH_ACL_METER_DROP:
    case SWITCH_ACL_SAME_IF_CHECK_DROP:
    case SWITCH_ACL_TTL_1_TO_CPU:
    case SWITCH_ACL_TTL_1_REDIRECT_TO_CPU:
    case SWITCH_ACL_IPV6_LINK_LOCAL_TO_CPU:
    case SWITCH_ACL_IPV6_LINK_LOCAL_REDIRECT_TO_CPU:
    case SWITCH_ACL_GLEAN:
    case SWITCH_ACL_SAME_BD_CHECK:
    case SWITCH_ACL_L3_COPY_TO_CPU:
    case SWITCH_ACL_L3_MTU_CHECK:
    case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_YELLOW:
    case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_RED:
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_WATCHLIST:
    case SWITCH_ACL_EGRESS_DEFLECT_QUEUE_DOD:
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_DOD:
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_NODOD:
    case SWITCH_ACL_EGRESS_ACL_DENY:
    case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_YELLOW:
    case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_RED:
    case SWITCH_ACL_DROP_OTHERS:
    case SWITCH_ACL_EGRESS_DROP_OTHERS:
      high_prio = FALSE;
      break;

    default:
      high_prio = TRUE;
      break;
  }

  if (mod_acl) {
    final_priority = high_prio ? (mod_acl_high_piority_base + acl_type)
                               : (mod_acl_lo_priority_base + acl_type);
  } else {
    final_priority = high_prio ? (acl_high_priority_base + acl_type)
                               : (acl_lo_priority_base + acl_type);
  }
  SWITCH_LOG_DEBUG(
      "Final priority for internal ACL %d is %d", acl_type, final_priority);
  return final_priority;
}

switch_status_t switch_system_acl_default_entries_add(switch_device_t device) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_acl_system_key_value_pair_t ing_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
  switch_acl_egress_system_key_value_pair_t
      egr_acl_kvp[SWITCH_ACL_EGRESS_SYSTEM_FIELD_MAX];
  switch_acl_action_params_t action_params;
  switch_acl_action_params_t mod_action_params;
  switch_acl_opt_action_params_t opt_action_params;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_interface_info_t *cpu_intf_info = NULL;
  switch_ifindex_t cpu_ifindex = 0;
  void *acl_kvp = NULL;
  switch_uint16_t kvp_count = 0;
  switch_acl_type_t acl_type = 0;
  switch_acl_action_t acl_action = 0;
  switch_uint16_t index = 0;
  switch_direction_t direction = SWITCH_API_DIRECTION_INGRESS;
  switch_acl_default_info_t default_info = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t cpu_port_handle = SWITCH_API_INVALID_HANDLE;

  UNUSED(cpu_port_handle);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status = switch_api_hostif_cpu_intf_info_get(device, &cpu_intf_info);
  cpu_ifindex = cpu_intf_info->ifindex;
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_ACL_DEFAULT_MAX; index++) {
    acl_ctx->acl_handle[index] = SWITCH_API_INVALID_HANDLE;
    acl_ctx->ace_handle[index] = SWITCH_API_INVALID_HANDLE;
    acl_ctx->mod_ace_handle[index] = SWITCH_API_INVALID_HANDLE;
  }

  for (index = 1; index < SWITCH_ACL_DEFAULT_MAX; index++) {
    switch_acl_default_check(index, &default_info);
    if (!default_info.program_acl) {
      SWITCH_LOG_DEBUG("acl default entry add skipped on device %d: index %d\n",
                       device,
                       index);
      continue;
    }

    kvp_count = 0;
    SWITCH_MEMSET(ing_acl_kvp, 0x0, sizeof(ing_acl_kvp));
    SWITCH_MEMSET(egr_acl_kvp, 0x0, sizeof(egr_acl_kvp));
    SWITCH_MEMSET(&action_params, 0x0, sizeof(action_params));
    SWITCH_MEMSET(&mod_action_params, 0x0, sizeof(mod_action_params));
    SWITCH_MEMSET(&opt_action_params, 0x0, sizeof(opt_action_params));

    switch (index) {
      case SWITCH_ACL_DROP:
        // system acl for dropped packets
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_DROP;
        ing_acl_kvp[kvp_count].value.drop_flag = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_PV_MISS:
        // port vlan mapping miss, drop
        ing_acl_kvp[kvp_count].field =
            SWITCH_ACL_SYSTEM_FIELD_PORT_VLAN_MAPPING_MISS;
        ing_acl_kvp[kvp_count].value.port_vlan_mapping_miss = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_PORT_VLAN_MAPPING_MISS;
        mod_action_params.drop.reason_code = DROP_PORT_VLAN_MAPPING_MISS;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_STP_BLOCKED_DROP:
        // STP state == blocked, drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_STP_STATE;
        ing_acl_kvp[kvp_count].value.stp_state = SWITCH_PORT_STP_STATE_BLOCKING;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_STP_STATE_BLOCKING;
        mod_action_params.drop.reason_code = DROP_STP_STATE_BLOCKING;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_STP_LEARN_DROP:
        // STP state == learning, drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_STP_STATE;
        ing_acl_kvp[kvp_count].value.stp_state = SWITCH_PORT_STP_STATE_LEARNING;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_STP_STATE_LEARNING;
        mod_action_params.drop.reason_code = DROP_STP_STATE_LEARNING;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_DENY_DROP:
        // ACL deny, drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ACL_DENY;
        ing_acl_kvp[kvp_count].value.acl_deny = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_ACL_DENY;
        mod_action_params.drop.reason_code = DROP_ACL_DENY;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_URPF_FAIL_DROP:
        // URPF check fail, drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_URPF_CHECK;
        ing_acl_kvp[kvp_count].value.urpf_check_fail = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_URPF_CHECK_FAIL;
        mod_action_params.drop.reason_code = DROP_URPF_CHECK_FAIL;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_METER_DROP:
        // meter drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_METER_DROP;
        ing_acl_kvp[kvp_count].value.meter_drop = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_METER;
        mod_action_params.drop.reason_code = DROP_METER;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_SAME_IF_CHECK_DROP:
        // same if check fail, drop
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_IF_CHECK;
        ing_acl_kvp[kvp_count].value.if_check = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_BD_CHECK;
        ing_acl_kvp[kvp_count].value.bd_check = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_TUNNEL_IF_CHECK;
        ing_acl_kvp[kvp_count].value.tunnel_if_check = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_SAME_IFINDEX;
        mod_action_params.drop.reason_code = DROP_SAME_IFINDEX;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_TTL_1_TO_CPU:
        // routed, ttl == 1, egress_ifindex == cpu, permit
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_TTL;
        ing_acl_kvp[kvp_count].value.ttl = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX;
        ing_acl_kvp[kvp_count].value.out_ifindex = cpu_ifindex;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_PERMIT;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_TTL_1_REDIRECT_TO_CPU:
        // routed, ttl == 1, redirect to cpu
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_TTL;
        ing_acl_kvp[kvp_count].value.ttl = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_TTL_ERROR;
        mod_action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_TTL_ERROR;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_IPV6_LINK_LOCAL_TO_CPU:
        // routed, ipv6_src_is_link_local == 1, egress_ifindex == cpu, permit
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL;
        ing_acl_kvp[kvp_count].value.src_is_link_local = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX;
        ing_acl_kvp[kvp_count].value.out_ifindex = cpu_ifindex;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL;
        mod_action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_PERMIT;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_IPV6_LINK_LOCAL_REDIRECT_TO_CPU:
        // routed, ipv6_src_is_link_local == 1, redirect to cpu
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL;
        ing_acl_kvp[kvp_count].value.src_is_link_local = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL;
        mod_action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_GLEAN:
        // glean, redirect to cpu
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_NEXTHOP_GLEAN;
        ing_acl_kvp[kvp_count].value.nexthop_glean = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_GLEAN;
        mod_action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_GLEAN;
        break;

      case SWITCH_ACL_SAME_BD_CHECK:
        // routed, ingress bd == egress bd, copy to cpu
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
        ing_acl_kvp[kvp_count].value.routed = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_BD_CHECK;
        ing_acl_kvp[kvp_count].value.bd_check = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_ICMP_REDIRECT;
        mod_action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_ICMP_REDIRECT;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_COPY_TO_CPU;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_L3_COPY_TO_CPU:
        // l3_copy to cpu
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_L3_COPY;
        ing_acl_kvp[kvp_count].value.l3_copy = true;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_COPY_TO_CPU;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_L3_MTU_CHECK:
        // egress l3_mtu_check
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT;
        egr_acl_kvp[kvp_count].value.deflection_flag = 0;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK;
        egr_acl_kvp[kvp_count].value.l3_mtu_check = 0;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU;
        action_params.cpu_redirect.reason_code =
            SWITCH_HOSTIF_REASON_CODE_L3_MTU_ERROR;
        mod_action_params.drop.reason_code = DROP_MTU_CHECK_FAIL;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_YELLOW:
        ing_acl_kvp[kvp_count].field =
            SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR;
        ing_acl_kvp[kvp_count].value.storm_control_color = SWITCH_COLOR_YELLOW;
        ing_acl_kvp[kvp_count].mask.u.mask = 0x3;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
        ing_acl_kvp[kvp_count].value.reason_code = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;

        action_params.drop.reason_code = DROP_STORM_CONTROL_COLOR_YELLOW;
        mod_action_params.drop.reason_code = DROP_STORM_CONTROL_COLOR_YELLOW;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_RED:
        ing_acl_kvp[kvp_count].field =
            SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR;
        ing_acl_kvp[kvp_count].value.storm_control_color = SWITCH_COLOR_RED;
        ing_acl_kvp[kvp_count].mask.u.mask = 0x3;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
        ing_acl_kvp[kvp_count].value.reason_code = 0;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;

        action_params.drop.reason_code = DROP_STORM_CONTROL_COLOR_RED;
        mod_action_params.drop.reason_code = DROP_STORM_CONTROL_COLOR_RED;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_EGRESS_DEFLECT_MOD_WATCHLIST:
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT;
        egr_acl_kvp[kvp_count].value.deflection_flag = 1;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE;
        egr_acl_kvp[kvp_count].value.queue_dod_enable = 0;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 3;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x03;
        kvp_count++;
        action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        mod_action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_EGRESS_DEFLECT_QUEUE_DOD:
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT;
        egr_acl_kvp[kvp_count].value.deflection_flag = 1;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE;
        egr_acl_kvp[kvp_count].value.queue_dod_enable = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 0;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x02;
        kvp_count++;
        action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        mod_action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP_QALERT;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_DOD:
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT;
        egr_acl_kvp[kvp_count].value.deflection_flag = 1;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE;
        egr_acl_kvp[kvp_count].value.queue_dod_enable = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 3;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x03;
        kvp_count++;
        action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        mod_action_params.drop.reason_code = DROP_TRAFFIC_MANAGER;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP_QALERT;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_NODOD:
        // dod can happen because of queue but queue disables it if quota
        // finished
        // mod (of other other features) should not generate dod
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT;
        egr_acl_kvp[kvp_count].value.deflection_flag = 1;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE;
        egr_acl_kvp[kvp_count].value.queue_dod_enable = 0;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x03;
        kvp_count++;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_EGRESS_ACL_DENY:
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_ACL_DENY;
        egr_acl_kvp[kvp_count].value.acl_deny = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_EGRESS_ACL_DENY;
        mod_action_params.drop.reason_code = DROP_EGRESS_ACL_DENY;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP;
        break;

      case SWITCH_ACL_RACL_DENY_DROP:
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_RACL_DENY;
        ing_acl_kvp[kvp_count].value.racl_deny = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0xFF;
        kvp_count++;
        action_params.drop.reason_code = DROP_RACL_DENY;
        mod_action_params.drop.reason_code = DROP_RACL_DENY;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_YELLOW:
        switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_PACKET_COLOR;
        egr_acl_kvp[kvp_count].value.packet_color = SWITCH_COLOR_YELLOW;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x3;
        kvp_count++;
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT;
        egr_acl_kvp[kvp_count].value.egr_port = cpu_port_handle;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;

        action_params.drop.reason_code = DROP_CPU_COLOR_YELLOW;
        mod_action_params.drop.reason_code = DROP_CPU_COLOR_YELLOW;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_RED:
        switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_PACKET_COLOR;
        egr_acl_kvp[kvp_count].value.packet_color = SWITCH_COLOR_RED;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x3;
        kvp_count++;
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT;
        egr_acl_kvp[kvp_count].value.egr_port = cpu_port_handle;
        egr_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
        kvp_count++;

        action_params.drop.reason_code = DROP_CPU_COLOR_RED;
        mod_action_params.drop.reason_code = DROP_CPU_COLOR_RED;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      case SWITCH_ACL_DROP_OTHERS:
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_DROP_CTL;
        ing_acl_kvp[kvp_count].value.drop_ctl = 4;
        ing_acl_kvp[kvp_count].mask.u.mask = 0x4;
        kvp_count++;
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP;
        ing_acl_kvp[kvp_count].value.mirror_on_drop = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0x01;
        kvp_count++;
        action_params.drop.reason_code = DROP_OTHERS_INGRESS;
        mod_action_params.drop.reason_code = DROP_OTHERS_INGRESS;
        acl_type = SWITCH_ACL_TYPE_SYSTEM;
        acl_action = SWITCH_ACL_ACTION_MIRROR_AND_DROP;
        acl_kvp = (void *)ing_acl_kvp;
        direction = SWITCH_API_DIRECTION_INGRESS;
        break;

      case SWITCH_ACL_EGRESS_DROP_OTHERS:
        egr_acl_kvp[kvp_count].field = SWITCH_ACL_EGRESS_SYSTEM_FIELD_DROP_CTL;
        egr_acl_kvp[kvp_count].value.drop_ctl = 4;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x4;
        kvp_count++;
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x01;
        kvp_count++;
        action_params.drop.reason_code = DROP_OTHERS_EGRESS;
        mod_action_params.drop.reason_code = DROP_OTHERS_EGRESS;
        acl_type = SWITCH_ACL_TYPE_EGRESS_SYSTEM;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP;
        acl_kvp = (void *)egr_acl_kvp;
        direction = SWITCH_API_DIRECTION_EGRESS;
        break;

      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR("acl default entry add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
    }
    status = switch_api_acl_list_create(
        device, direction, acl_type, SWITCH_HANDLE_TYPE_NONE, &acl_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl default entry add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
    acl_ctx->acl_handle[index] = acl_handle;

    status =
        switch_api_acl_rule_create(device,
                                   acl_ctx->acl_handle[index],
                                   switch_system_acl_priority(index, false),
                                   kvp_count,
                                   acl_kvp,
                                   acl_action,
                                   &action_params,
                                   &opt_action_params,
                                   &acl_ctx->ace_handle[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl default entry add failed on device %d: %si : index=%d\n",
          device,
          switch_error_to_string(status),
          index);
      goto cleanup;
    }

    if (default_info.program_mod_acl) {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP;
        ing_acl_kvp[kvp_count].value.mirror_on_drop = 1;
        ing_acl_kvp[kvp_count].mask.u.mask = 0x01;
        acl_action = SWITCH_ACL_ACTION_MIRROR_AND_DROP;
        kvp_count++;
      } else {
        egr_acl_kvp[kvp_count].field =
            SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
        egr_acl_kvp[kvp_count].value.mirror_on_drop = 1;
        egr_acl_kvp[kvp_count].mask.u.mask = 0x01;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP;
        kvp_count++;
      }
      status =
          switch_api_acl_rule_create(device,
                                     acl_ctx->acl_handle[index],
                                     switch_system_acl_priority(index, true),
                                     kvp_count,
                                     acl_kvp,
                                     acl_action,
                                     &mod_action_params,
                                     &opt_action_params,
                                     &acl_ctx->mod_ace_handle[index]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl default entry add failed on device %d: %si : index=%d\n",
            device,
            switch_error_to_string(status),
            index);
        goto cleanup;
      }
    }
  }

  return status;

cleanup:
  tmp_status = switch_system_acl_default_entries_delete(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_acl_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_system_acl_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_acl_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_acl_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("egress acl default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_l4port_fields_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl l4port default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_acl_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_system_acl_default_entries_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl default entry delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_acl_table_size_get(switch_device_t device,
                                          switch_size_t *acl_table_size) {
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_table_id_t table_id = 0;

  SWITCH_ASSERT(acl_table_size != NULL);

  *acl_table_size = 0;

  for (table_id = SWITCH_TABLE_IPV4_ACL;
       table_id <= SWITCH_TABLE_EGRESS_SYSTEM_ACL;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *acl_table_size = 0;
      SWITCH_LOG_ERROR(
          "acl handle size get failed on device %d: %s"
          "for table %s\n",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }
    *acl_table_size += table_size;
  }
  return status;
}

/** \brief switch_acl_init:
 *   Allocates acl and ace handle array based on respective table sizes.
 *
 *   This function computes the number of handles needed by the acl tables.
 *   a. ipv4 acl
 *   b. ipv6 acl
 *   c. ipv4 racl
 *   d. ipv6 racl
 *   e. mac acl
 *   f. system acl
 *   g. egress acl
 *   h. ipv4 mirror acl
 *   i. ipv6 mirror acl
 *   j. l2 qos acl
 *   k. ipv4 qos acl
 *   l. ipv6 qos acl
 *   m. egress ipv4 acl
 *   n. egress ipv6 acl
 *   Ace handle array will be set to same value as acl handle array since
 *   there can be an ace for every acl.
 *
 *   \param device Device number
 *   \return switch_status_t The status of acl init
 */

switch_status_t switch_acl_init(switch_device_t device) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_size_t acl_table_size = 0;
  switch_size_t ace_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  acl_ctx = SWITCH_MALLOC(device, sizeof(switch_acl_context_t), 0x1);
  if (!acl_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl init failed on device %d: "
        "acl context malloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(acl_ctx, 0x0, sizeof(switch_acl_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_ACL, (void *)acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl context set failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Compute the acl handle array size
   */
  status = switch_acl_table_size_get(device, &acl_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl table size get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Set ace handle array size same as acl handle size
   * as there can an ace for every acl
   */
  ace_table_size = acl_table_size;

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_ACL
   */
  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_ACL, acl_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl handle type init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_ACE
   */
  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_ACE, ace_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "ace handle type init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
    return status;
  }

  status = switch_api_id_allocator_new(
      device, ace_table_size, FALSE, &acl_ctx->counter_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl counter handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACE);
    return status;
  }
  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_RANGE
   */
  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_RANGE, ace_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl range handle type init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACE);
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_GROUP
   */
  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_ACL_GROUP, ace_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl group handle type init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACE);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RANGE);
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_GROUP_MEMBER
   */
  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER, ace_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl init failed for device %d: "
        "acl group member handle type init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACE);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RANGE);
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL_GROUP);
    return status;
  }
  SWITCH_LOG_DEBUG("acl init done successfully for device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/** \brief switch_acl_free:
 *   Deallocate the acl and ace handle array
 *
 *   \param device Device number
 *   \return switch_status_t The status of acl init
 */
switch_status_t switch_acl_free(switch_device_t device) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_ACL
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "ace handle type free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_ACE
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl handle type free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, acl_ctx->counter_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl counter handle allocator free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RANGE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl range handle type free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL_GROUP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl group handle type free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl free failed for device %d: "
        "acl group member handle type free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, acl_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_ACL, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("acl free done successfully for device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/**
 * \brief switch_api_acl_list_create:
 * Create a access control list (acl) table
 *
 * This function creates a access control list (acl) handle for the acl type.
 * Rules (filters) can be added to the acl using the handle provided.
 * Based on the key, appropriate acl type can be used.
 *
 * \param device Device number
 * \param acl_type Type of acl (ipv4 acl, ipv6 acl, mac acl, ipv4 racl, ipv6
 * racl, system acl, egress acl, ipv4 mirror acl, ipv6 mirror acl).
 *   \return switch_handle_t acl list handle
 */
switch_status_t switch_api_acl_list_create_internal(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_type_t type,
    switch_handle_type_t bp_type,
    switch_handle_t *acl_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_handle_t acl_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  *acl_handle = SWITCH_API_INVALID_HANDLE;

  if (!SWITCH_BIND_POINT_SUPPORTED(bp_type)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "acl list create failed on device %d: "
        "bind point not supported:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Create an acl handle irrespective of acl type
   */
  handle = switch_acl_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl list create failed on device %d: "
        "acl list handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl list create failed on device %d: "
        "acl get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  acl_info->type = type;
  acl_info->direction = direction;
  acl_info->bp_type = bp_type;

  status = SWITCH_LIST_INIT(&acl_info->group_list);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (bp_type != SWITCH_HANDLE_TYPE_NONE) {
    status = switch_api_acl_list_group_create(
        device, direction, bp_type, &acl_group_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl list create failed on device %d: "
          "acl list group create failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_api_acl_group_member_create(
        device, acl_group_handle, handle, &acl_info->default_group_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl list create failed on device %d: "
          "acl group member create failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    acl_info->default_group = acl_group_handle;
  }

  *acl_handle = handle;

  SWITCH_LOG_DEBUG(
      "acl list created on device %d handle 0x%lx "
      "bind point %s type %s\n",
      device,
      handle,
      switch_acl_bp_type_to_string(bp_type),
      switch_acl_type_to_string(type));

  SWITCH_LOG_EXIT();

  return status;
}

/**
 * \brief switch_api_acl_list_delete:
 * Delete a access control list (acl) table
 *
 * This function deletes the access control list (acl).
 * Rules (filters) will be deleted implicitly when acl list
 * delete is called.
 *
 * \param device Device number
 * \param acl_handle opaque handle returned while creating acl list
 * \return switch_status_t status of acl list delete
 */
switch_status_t switch_api_acl_list_delete_internal(
    switch_device_t device, switch_handle_t acl_handle) {
  switch_node_t *node = NULL;
  switch_acl_info_t *acl_info = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_handle_t *ace_handles = NULL;
  switch_uint16_t num_rules = 0;
  switch_uint16_t index = 0;
  switch_handle_t ace_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  /*
   * Check for valid acl handle
   */
  if (!SWITCH_ACL_HANDLE(acl_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "acl list delete failed on device %d acl handle 0x%lx: "
        "acl handle invalid:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl list delete failed on device %d acl handle 0x%lx: "
        "acl get failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Delete all the ace rules referring to this acl list
   */
  num_rules = SWITCH_ARRAY_COUNT(&acl_info->rules);
  if (num_rules) {
    ace_handles = SWITCH_MALLOC(device, sizeof(switch_handle_t), num_rules);
    if (!ace_handles) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "acl list delete failed on device %d acl handle 0x%lx: "
          "acl rule list malloc failed:(%s)\n",
          device,
          acl_handle,
          switch_error_to_string(status));
      return status;
    }

    FOR_EACH_IN_ARRAY(
        ace_handle, acl_info->rules, switch_ace_info_t, ace_info) {
      UNUSED(ace_info);
      ace_handles[index++] = ace_handle;
    }
    FOR_EACH_IN_ARRAY_END();

    for (index = 0; index < num_rules; index++) {
      status =
          switch_api_acl_rule_delete(device, acl_handle, ace_handles[index]);
      /*
       * Log the error and continue deleting the rules
       */
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl list delete failed on device %d acl handle 0x%lx "
            "ace handle 0x%lx: "
            "acl rule delete failed:(%s)\n",
            device,
            acl_handle,
            ace_handles[index],
            switch_error_to_string(status));
      }
    }
    SWITCH_FREE(device, ace_handles);
  }

  if (acl_info->bp_type != SWITCH_HANDLE_TYPE_NONE) {
    status = switch_api_acl_group_member_delete(device,
                                                acl_info->default_group_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl list delete failed on device %d acl handle 0x%lx "
          "acl default group member 0x%lx: "
          "acl group member delete failed:(%s)\n",
          device,
          acl_handle,
          acl_info->default_group_member,
          switch_error_to_string(status));
      return status;
    }

    status = switch_api_acl_list_group_delete(device, acl_info->default_group);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl list delete failed on device %d acl handle 0x%lx "
          "acl default group 0x%lx: "
          "acl group delete failed:(%s)\n",
          device,
          acl_handle,
          acl_info->default_group,
          switch_error_to_string(status));
      return status;
    }
  } else {
    /*
     * Don't delete ACL handle if there is a group associated
     */
    FOR_EACH_IN_LIST(acl_info->group_list, node) {
      status = SWITCH_STATUS_RESOURCE_IN_USE;
      SWITCH_LOG_ERROR(
          "acl list delete failed on device %d acl handle 0x%lx: "
          "acl is still referenced:(%s)\n",
          device,
          acl_handle,
          switch_error_to_string(status));
      return status;
    }
    FOR_EACH_IN_LIST_END();
  }

  status = switch_acl_handle_delete(device, acl_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl list delete failed on device %d acl handle 0x%lx "
        "acl handle delete failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "acl list deleted on device %d acl handle 0x%lx\n", device, acl_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_handle_t switch_api_acl_list_group_create_internal(
    switch_device_t device,
    switch_direction_t direction,
    switch_handle_type_t bp_type,
    switch_handle_t *acl_group_handle) {
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_BIND_POINT_SUPPORTED(bp_type)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "acl list group create failed on device %d: "
        "bind point invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *acl_group_handle = SWITCH_API_INVALID_HANDLE;

  handle = switch_acl_group_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "acl list group create failed on device %d: "
        "acl group handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_get(device, handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl list group create failed on device %d: "
        "acl group get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  acl_group_info->direction = direction;
  acl_group_info->bp_type = bp_type;

  status = SWITCH_LIST_INIT(&(acl_group_info->handle_list));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_LIST_INIT(&(acl_group_info->acl_member_list));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("acl group created on device %d acl group handle 0x%lx\n",
                   device,
                   handle);

  *acl_group_handle = handle;

  return status;
}

switch_status_t switch_api_acl_list_group_delete_internal(
    switch_device_t device, switch_handle_t acl_group_handle) {
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_size_t count = 0;

  SWITCH_ASSERT(SWITCH_ACL_GROUP_HANDLE(acl_group_handle));
  if (!SWITCH_ACL_GROUP_HANDLE(acl_group_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "acl group delete failed on device %d acl group handle 0x%lx: "
        "acl group handle invalid:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_get(device, acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group delete failed on device %d acl group handle 0x%lx: "
        "acl group get failed:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  count = SWITCH_LIST_COUNT(&acl_group_info->handle_list);
  if (count != 0) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "acl group delete failed on device %d acl group handle 0x%lx: "
        "bind points still referenced:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  count = SWITCH_LIST_COUNT(&acl_group_info->acl_member_list);
  if (count != 0) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "acl group delete failed on device %d acl group handle 0x%lx: "
        "acl members still referenced:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_handle_delete(device, acl_group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group delete failed on device %d acl group handle 0x%lx: "
        "acl group handle delete failed:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "acl group handle deleted on device %d acl group handle 0x%lx\n",
      device,
      acl_group_handle);

  return status;
}

/* \brief switch_acl_ip_set_fields_actions:
 * This function programs the hardware for ipv4 acl
 */
static switch_status_t switch_acl_ip_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ip_key_value_pair_t *ip_acl = NULL;

  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_acl = (switch_acl_ip_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv4_acl_table_entry_add(device,
                                                rule->priority,
                                                field_count,
                                                ip_acl,
                                                rule->action,
                                                &(rule->action_params),
                                                &(rule->opt_action_params),
                                                entry);
  } else {
    status =
        switch_pd_egress_ipv4_acl_table_entry_add(device,
                                                  rule->priority,
                                                  field_count,
                                                  ip_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  }

  return status;
}

/* \brief switch_acl_ip_fields_actions_update:
 * This function updates the hardware for ipv4 acl actions
 */
static switch_status_t switch_acl_ip_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ip_key_value_pair_t *ip_acl = NULL;

  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_acl = (switch_acl_ip_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv4_acl_table_entry_update(device,
                                                   rule->priority,
                                                   field_count,
                                                   ip_acl,
                                                   rule->action,
                                                   &(rule->action_params),
                                                   &(rule->opt_action_params),
                                                   entry);
  } else {
    status =
        switch_pd_egress_ipv4_acl_table_entry_update(device,
                                                     rule->priority,
                                                     field_count,
                                                     ip_acl,
                                                     rule->action,
                                                     &(rule->action_params),
                                                     &(rule->opt_action_params),
                                                     entry);
  }

  return status;
}

/* \brief switch_acl_ipv6_set_fields_actions:
 * This function programs the hardware for ipv6 acl
 */
static switch_status_t switch_acl_ipv6_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ipv6_key_value_pair_t *ipv6_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_acl = (switch_acl_ipv6_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv6_acl_table_entry_add(device,
                                                rule->priority,
                                                field_count,
                                                ipv6_acl,
                                                rule->action,
                                                &(rule->action_params),
                                                &(rule->opt_action_params),
                                                entry);
  } else {
    status =
        switch_pd_egress_ipv6_acl_table_entry_add(device,
                                                  rule->priority,
                                                  field_count,
                                                  ipv6_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  }

  return status;
}

/* \brief switch_acl_ipv6_fields_actions_update:
 * This function updates the hardware for ipv6 acl
 */
static switch_status_t switch_acl_ipv6_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ipv6_key_value_pair_t *ipv6_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_acl = (switch_acl_ipv6_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv6_acl_table_entry_update(device,
                                                   rule->priority,
                                                   field_count,
                                                   ipv6_acl,
                                                   rule->action,
                                                   &(rule->action_params),
                                                   &(rule->opt_action_params),
                                                   entry);
  } else {
    status =
        switch_pd_egress_ipv6_acl_table_entry_update(device,
                                                     rule->priority,
                                                     field_count,
                                                     ipv6_acl,
                                                     rule->action,
                                                     &(rule->action_params),
                                                     &(rule->opt_action_params),
                                                     entry);
  }

  return status;
}

/* \brief switch_acl_mac_set_fields_actions:
 * This function programs the hardware for mac acl
 */
static switch_status_t switch_acl_mac_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_mac_key_value_pair_t *mac_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  mac_acl = (switch_acl_mac_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_mac_acl_table_entry_add(device,
                                               rule->priority,
                                               field_count,
                                               mac_acl,
                                               rule->action,
                                               &(rule->action_params),
                                               &(rule->opt_action_params),
                                               entry);
  } else {
    status =
        switch_pd_egress_mac_acl_table_entry_add(device,
                                                 rule->priority,
                                                 field_count,
                                                 mac_acl,
                                                 rule->action,
                                                 &(rule->action_params),
                                                 &(rule->opt_action_params),
                                                 entry);
  }

  return status;
}

/* \brief switch_acl_mac_fields_actions_update:
 * This function programs the hardware for mac acl
 */
static switch_status_t switch_acl_mac_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_mac_key_value_pair_t *mac_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  mac_acl = (switch_acl_mac_key_value_pair_t *)payload;

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_mac_acl_table_entry_update(device,
                                                  rule->priority,
                                                  field_count,
                                                  mac_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  } else {
    status =
        switch_pd_egress_mac_acl_table_entry_update(device,
                                                    rule->priority,
                                                    field_count,
                                                    mac_acl,
                                                    rule->action,
                                                    &(rule->action_params),
                                                    &(rule->opt_action_params),
                                                    entry);
  }

  return status;
}

/* \brief switch_acl_ip_racl_set_fields_actions:
 * This function programs the hardware for ipv4 racl
 */
static switch_status_t switch_acl_ip_racl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ip_racl_key_value_pair_t *ip_racl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_racl = (switch_acl_ip_racl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv4_racl_table_entry_add(device,
                                                 rule->priority,
                                                 field_count,
                                                 ip_racl,
                                                 rule->action,
                                                 &(rule->action_params),
                                                 &(rule->opt_action_params),
                                                 entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv4 racl set field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ip_racl_fields_actions_update:
 * This function programs the hardware for ipv4 racl
 */
static switch_status_t switch_acl_ip_racl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ip_racl_key_value_pair_t *ip_racl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_racl = (switch_acl_ip_racl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv4_racl_table_entry_update(device,
                                                    rule->priority,
                                                    field_count,
                                                    ip_racl,
                                                    rule->action,
                                                    &(rule->action_params),
                                                    &(rule->opt_action_params),
                                                    entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR(
        "ipv4 racl update field actions failed for device %d  %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ipv6_racl_set_fields_actions:
 * This function programs the hardware for ipv6 racl
 */
static switch_status_t switch_acl_ipv6_racl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ipv6_racl_key_value_pair_t *ipv6_racl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_racl = (switch_acl_ipv6_racl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv6_racl_table_entry_add(device,
                                                 rule->priority,
                                                 field_count,
                                                 ipv6_racl,
                                                 rule->action,
                                                 &(rule->action_params),
                                                 &(rule->opt_action_params),
                                                 entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv6 racl set field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_ipv6_racl_fields_actions_update:
 * This function programs the hardware for ipv6 racl
 */
static switch_status_t switch_acl_ipv6_racl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ipv6_racl_key_value_pair_t *ipv6_racl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_racl = (switch_acl_ipv6_racl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv6_racl_table_entry_update(device,
                                                    rule->priority,
                                                    field_count,
                                                    ipv6_racl,
                                                    rule->action,
                                                    &(rule->action_params),
                                                    &(rule->opt_action_params),
                                                    entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv6 racl update field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_ip_mirror_acl_set_fields_actions:
 * This function programs the hardware for ipv4 mirror_acl
 */
static switch_status_t switch_acl_ip_mirror_acl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_mirror_acl = (switch_acl_ip_mirror_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv4_mirror_acl_table_entry_add(device,
                                                  rule->priority,
                                                  field_count,
                                                  ip_mirror_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ip_mirror_acl_fields_actions_update:
 * This function programs the hardware for ipv4 mirror_acl
 */
static switch_status_t switch_acl_ip_mirror_acl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_mirror_acl = (switch_acl_ip_mirror_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv4_mirror_acl_table_entry_update(device,
                                                     rule->priority,
                                                     field_count,
                                                     ip_mirror_acl,
                                                     rule->action,
                                                     &(rule->action_params),
                                                     &(rule->opt_action_params),
                                                     entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ipv6_mirror_acl_set_fields_actions:
 * This function programs the hardware for ipv6 mirror_acl
 */
static switch_status_t switch_acl_ipv6_mirror_acl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_mirror_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_mirror_acl = (switch_acl_ipv6_mirror_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv6_mirror_acl_table_entry_add(device,
                                                  rule->priority,
                                                  field_count,
                                                  ipv6_mirror_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("acl field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_ipv6_mirror_acl_fields_actions_update:
 * This function programs the hardware for ipv6 mirror_acl
 */
static switch_status_t switch_acl_ipv6_mirror_acl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_mirror_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_mirror_acl = (switch_acl_ipv6_mirror_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv6_mirror_acl_table_entry_update(device,
                                                     rule->priority,
                                                     field_count,
                                                     ipv6_mirror_acl,
                                                     rule->action,
                                                     &(rule->action_params),
                                                     &(rule->opt_action_params),
                                                     entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("acl field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_mac_qos_set_fields_actions:
 * This function programs the hardware for mac qos_acl
 */
static switch_status_t switch_acl_mac_qos_acl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_mac_qos_acl_key_value_pair_t *mac_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  mac_qos_acl = (switch_acl_mac_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_mac_qos_acl_table_entry_add(device,
                                                   rule->priority,
                                                   field_count,
                                                   mac_qos_acl,
                                                   rule->action,
                                                   &(rule->action_params),
                                                   &(rule->opt_action_params),
                                                   entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("mac qos acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_mac_qos_acl_fields_actions_update:
 * This function programs the hardware for mac qos_acl
 */
static switch_status_t switch_acl_mac_qos_acl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_mac_qos_acl_key_value_pair_t *mac_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  mac_qos_acl = (switch_acl_mac_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_mac_qos_acl_table_entry_update(device,
                                                 rule->priority,
                                                 field_count,
                                                 mac_qos_acl,
                                                 rule->action,
                                                 &(rule->action_params),
                                                 &(rule->opt_action_params),
                                                 entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("mac qos acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ip_qos_acl_set_fields_actions:
 * This function programs the hardware for ipv4 qos_acl
 */
static switch_status_t switch_acl_ip_qos_acl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_qos_acl = (switch_acl_ip_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv4_qos_acl_table_entry_add(device,
                                                    rule->priority,
                                                    field_count,
                                                    ip_qos_acl,
                                                    rule->action,
                                                    &(rule->action_params),
                                                    &(rule->opt_action_params),
                                                    entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv4 qos acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ip_qos_acl_fields_actions_update:
 * This function programs the hardware for ipv4 qos_acl
 */
static switch_status_t switch_acl_ip_qos_acl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ip_qos_acl = (switch_acl_ip_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv4_qos_acl_table_entry_update(device,
                                                  rule->priority,
                                                  field_count,
                                                  ip_qos_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv4 qos acl field actions failed for device %d  %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

/* \brief switch_acl_ipv6_qos_acl_set_fields_actions:
 * This function programs the hardware for ipv6 qos_acl
 */
static switch_status_t switch_acl_ipv6_qos_acl_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ipv6_qos_acl_key_value_pair_t *ipv6_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_qos_acl = (switch_acl_ipv6_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_ipv6_qos_acl_table_entry_add(device,
                                                    rule->priority,
                                                    field_count,
                                                    ipv6_qos_acl,
                                                    rule->action,
                                                    &(rule->action_params),
                                                    &(rule->opt_action_params),
                                                    entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv6 qos acl field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_ipv6_qos_acl_fields_actions_update:
 * This function programs the hardware for ipv6 qos_acl
 */
static switch_status_t switch_acl_ipv6_qos_acl_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ipv6_qos_acl_key_value_pair_t *ipv6_qos_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ipv6_qos_acl = (switch_acl_ipv6_qos_acl_key_value_pair_t *)payload;
  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status =
        switch_pd_ipv6_qos_acl_table_entry_update(device,
                                                  rule->priority,
                                                  field_count,
                                                  ipv6_qos_acl,
                                                  rule->action,
                                                  &(rule->action_params),
                                                  &(rule->opt_action_params),
                                                  entry);
  } else {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("ipv6 qos acl field actions failed for device %d %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_system_set_fields_actions:
 * This function programs the hardware for system acl
 */
static switch_status_t switch_acl_system_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_system_key_value_pair_t *system_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  system_acl = (switch_acl_system_key_value_pair_t *)payload;
  status = switch_pd_system_acl_table_entry_add(device,
                                                rule->priority,
                                                field_count,
                                                system_acl,
                                                rule->action,
                                                &rule->action_params,
                                                &rule->opt_action_params,
                                                entry);
  return status;
}

/* \brief switch_acl_system_fields_actions_update:
 * This function programs the hardware for system acl
 */
static switch_status_t switch_acl_system_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_system_key_value_pair_t *system_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  system_acl = (switch_acl_system_key_value_pair_t *)payload;
  status = switch_pd_system_acl_table_entry_update(device,
                                                   rule->priority,
                                                   field_count,
                                                   system_acl,
                                                   rule->action,
                                                   &rule->action_params,
                                                   &rule->opt_action_params,
                                                   entry);
  return status;
}

/* \brief switch_acl_egress_system_set_fields_actions:
 * This function programs the hardware for egress acl
 */
static switch_status_t switch_acl_egress_system_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_egress_system_key_value_pair_t *egr_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  egr_acl = (switch_acl_egress_system_key_value_pair_t *)payload;
  status = switch_pd_egress_acl_table_entry_add(device,
                                                rule->priority,
                                                field_count,
                                                egr_acl,
                                                rule->action,
                                                &rule->action_params,
                                                &rule->opt_action_params,
                                                entry);
  return status;
}

/* \brief switch_acl_egress_system_fields_actions_update:
 * This function programs the hardware for egress acl
 */
static switch_status_t switch_acl_egress_system_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_egress_system_key_value_pair_t *egr_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  egr_acl = (switch_acl_egress_system_key_value_pair_t *)payload;
  status = switch_pd_egress_acl_table_entry_update(device,
                                                   rule->priority,
                                                   field_count,
                                                   egr_acl,
                                                   rule->action,
                                                   &rule->action_params,
                                                   &rule->opt_action_params,
                                                   entry);
  return status;
}

/* \brief switch_acl_ecn_set_fields_actions:
 * This function programs the hardware for ecn acl
 */
static switch_status_t switch_acl_ecn_set_fields_actions(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t *entry) {
  switch_acl_ecn_key_value_pair_t *ecn_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ecn_acl = (switch_acl_ecn_key_value_pair_t *)payload;
  status = switch_pd_ecn_acl_table_entry_add(device,
                                             rule->priority,
                                             field_count,
                                             ecn_acl,
                                             rule->action,
                                             &rule->action_params,
                                             &rule->opt_action_params,
                                             entry);
  return status;
}

/* \brief switch_acl_ecn_fields_actions_update:
 * This function programs the hardware for ecn acl
 */
static switch_status_t switch_acl_ecn_fields_actions_update(
    switch_device_t device,
    switch_direction_t direction,
    switch_acl_rule_t *rule,
    void *payload,
    switch_int32_t field_count,
    switch_pd_hdl_t entry) {
  switch_acl_ecn_key_value_pair_t *ecn_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  ecn_acl = (switch_acl_ecn_key_value_pair_t *)payload;
  status = switch_pd_ecn_acl_table_entry_update(device,
                                                rule->priority,
                                                field_count,
                                                ecn_acl,
                                                rule->action,
                                                &rule->action_params,
                                                &rule->opt_action_params,
                                                entry);
  return status;
}

static switch_status_t switch_api_handle_acl_group_get(
    switch_device_t device,
    switch_handle_t handle,
    switch_direction_t direction,
    switch_handle_t *acl_group_handle) {
  switch_handle_type_t type = switch_handle_type_get(handle);

  switch (type) {
    case SWITCH_HANDLE_TYPE_NONE:
      *acl_group_handle = SWITCH_API_INVALID_HANDLE;
      return SWITCH_STATUS_SUCCESS;
    case SWITCH_HANDLE_TYPE_PORT:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_port_ingress_acl_group_get(
            device, handle, acl_group_handle);
      } else {
        return switch_api_port_egress_acl_group_get(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_LAG:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_lag_ingress_acl_group_get(
            device, handle, acl_group_handle);
      } else {
        return switch_api_lag_egress_acl_group_get(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_VLAN:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_vlan_ingress_acl_group_get(
            device, handle, acl_group_handle);
      } else {
        return switch_api_vlan_egress_acl_group_get(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_RIF:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_rif_ingress_acl_group_get(
            device, handle, acl_group_handle);
      } else {
        return switch_api_rif_egress_acl_group_get(
            device, handle, acl_group_handle);
      }
    default:

      SWITCH_LOG_ERROR(
          "Unexpected interface type! %u, handle = %lx\n", type, handle);
      return SWITCH_STATUS_INVALID_PARAMETER;
  }
}

static switch_status_t switch_api_handle_acl_group_set(
    switch_device_t device,
    switch_handle_t handle,
    switch_direction_t direction,
    switch_handle_t acl_group_handle) {
  switch_handle_type_t handle_type = switch_handle_type_get(handle);

  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      /* do nothing */
      return SWITCH_STATUS_SUCCESS;
    case SWITCH_HANDLE_TYPE_PORT:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_port_ingress_acl_group_set(
            device, handle, acl_group_handle);
      } else {
        return switch_api_port_egress_acl_group_set(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_LAG:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_lag_ingress_acl_group_set(
            device, handle, acl_group_handle);
      } else {
        return switch_api_lag_egress_acl_group_set(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_VLAN:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_vlan_ingress_acl_group_set(
            device, handle, acl_group_handle);
      } else {
        return switch_api_vlan_egress_acl_group_set(
            device, handle, acl_group_handle);
      }
    case SWITCH_HANDLE_TYPE_RIF:
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        return switch_api_rif_ingress_acl_group_set(
            device, handle, acl_group_handle);
      } else {
        return switch_api_rif_egress_acl_group_set(
            device, handle, acl_group_handle);
      }
    default:

      SWITCH_LOG_ERROR(
          "Unexpected interface type! %u, handle = %lx\n", handle_type, handle);
      return SWITCH_STATUS_INVALID_PARAMETER;
  }
}

static inline switch_int32_t switch_acl_field_size_get(
    switch_acl_info_t *acl_info, unsigned int acl_kvp_count) {
  switch_int32_t field_size = 0;

  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_IP:
    case SWITCH_ACL_TYPE_EGRESS_IP_ACL:
      field_size = sizeof(switch_acl_ip_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_SYSTEM:
      field_size = sizeof(switch_acl_system_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IPV6:
    case SWITCH_ACL_TYPE_EGRESS_IPV6_ACL:
      field_size = sizeof(switch_acl_ipv6_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_MAC:
      field_size = sizeof(switch_acl_mac_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      field_size =
          sizeof(switch_acl_egress_system_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IP_RACL:
      field_size = sizeof(switch_acl_ip_racl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IPV6_RACL:
      field_size =
          sizeof(switch_acl_ipv6_racl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      field_size =
          sizeof(switch_acl_ip_mirror_acl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      field_size =
          sizeof(switch_acl_ipv6_mirror_acl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_MAC_QOS:
      field_size =
          sizeof(switch_acl_mac_qos_acl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IP_QOS:
      field_size =
          sizeof(switch_acl_ip_qos_acl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_IPV6_QOS:
      field_size =
          sizeof(switch_acl_ipv6_qos_acl_key_value_pair_t) * acl_kvp_count;
      break;
    case SWITCH_ACL_TYPE_ECN:
      field_size = sizeof(switch_acl_ecn_key_value_pair_t) * acl_kvp_count;
      break;
    default:
      SWITCH_LOG_ERROR("acl field size get failed %d: %s\n",
                       acl_kvp_count,
                       switch_error_to_string(SWITCH_STATUS_INVALID_HANDLE));
  }

  return field_size;
}

static switch_status_t switch_acl_payload_get(switch_device_t device,
                                              switch_acl_info_t *acl_info,
                                              switch_acl_rule_t *rule,
                                              switch_acl_ref_group_t *ref_group,
                                              void **kvp_payload,
                                              switch_int32_t *kvp_count) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t field_size = 0;
  switch_int32_t count = 0;
  switch_port_lag_label_t port_lag_label = 0;
  switch_bd_label_t bd_label = 0;
  switch_uint16_t port_lag_mask = 0, bd_mask = 0;
  void *payload = NULL;

  field_size = switch_acl_field_size_get(acl_info, rule->field_count);

  if (field_size) {
    payload = SWITCH_MALLOC(device, field_size, 0x1);
    if (!payload) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "acl payload get failed on device %d size%d "
          "acl payload malloc failed:(%s)\n",
          device,
          field_size,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(payload, 0, field_size);
    SWITCH_MEMCPY(payload, rule->fields, field_size);
  }

  count = rule->field_count;

  /* If HANDLE_TYPE_NONE, all fields are set, return now */
  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    *kvp_payload = payload;
    *kvp_count = count;
    return status;
  }

  SWITCH_ASSERT(ref_group != NULL);

  SWITCH_ACL_LABEL_GET(
      acl_info->bp_type, ref_group->acl_group_handle, port_lag_label, bd_label);

  if (port_lag_label) {
    port_lag_mask = 0xFFFF;
  }

  if (bd_label) {
    bd_mask = 0xFFFF;
  }

  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_SYSTEM: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_system_key_value_pair_t,
                             SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_system_key_value_pair_t,
                             SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_system_key_value_pair_t) * 2));
      switch_acl_system_key_value_pair_t *system_acl =
          (switch_acl_system_key_value_pair_t *)payload;
      system_acl[count].field = SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL;
      system_acl[count].value.port_lag_label = port_lag_label;
      system_acl[count].mask.u.mask = port_lag_mask;
      count++;
      system_acl[count].field = SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL;
      system_acl[count].value.vlan_rif_label = bd_label;
      system_acl[count].mask.u.mask = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_EGRESS_IP_ACL:
    case SWITCH_ACL_TYPE_IP: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_key_value_pair_t,
                             SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_key_value_pair_t,
                             SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ip_key_value_pair_t) * 2));
      switch_acl_ip_key_value_pair_t *ip_acl =
          (switch_acl_ip_key_value_pair_t *)payload;
      ip_acl[count].field = SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL;
      ip_acl[count].value.port_lag_label = port_lag_label;
      ip_acl[count].mask.u.mask = port_lag_mask;
      count++;
      ip_acl[count].field = SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL;
      ip_acl[count].value.vlan_rif_label = bd_label;
      ip_acl[count].mask.u.mask = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_EGRESS_IPV6_ACL:
    case SWITCH_ACL_TYPE_IPV6: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_key_value_pair_t,
                             SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_key_value_pair_t,
                             SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ipv6_key_value_pair_t) * 2));
      switch_acl_ipv6_key_value_pair_t *ipv6_acl =
          (switch_acl_ipv6_key_value_pair_t *)payload;
      ipv6_acl[count].field = SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL;
      ipv6_acl[count].value.port_lag_label = port_lag_label;
      ipv6_acl[count].mask.u.mask16 = port_lag_mask;
      count++;
      ipv6_acl[count].field = SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL;
      ipv6_acl[count].value.vlan_rif_label = bd_label;
      ipv6_acl[count].mask.u.mask16 = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IP_RACL: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_racl_key_value_pair_t,
                             SWITCH_ACL_IP_RACL_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_racl_key_value_pair_t,
                             SWITCH_ACL_IP_RACL_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ip_racl_key_value_pair_t) * 2));
      switch_acl_ip_racl_key_value_pair_t *ip_racl =
          (switch_acl_ip_racl_key_value_pair_t *)payload;
      ip_racl[count].field = SWITCH_ACL_IP_RACL_FIELD_PORT_LAG_LABEL;
      ip_racl[count].value.port_lag_label = port_lag_label;
      ip_racl[count].mask.u.mask = port_lag_mask;
      count++;
      ip_racl[count].field = SWITCH_ACL_IP_RACL_FIELD_VLAN_RIF_LABEL;
      ip_racl[count].value.vlan_rif_label = bd_label;
      ip_racl[count].mask.u.mask = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IPV6_RACL: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_racl_key_value_pair_t,
                             SWITCH_ACL_IPV6_RACL_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_racl_key_value_pair_t,
                             SWITCH_ACL_IPV6_RACL_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ipv6_racl_key_value_pair_t) * 2));
      switch_acl_ipv6_racl_key_value_pair_t *ipv6_racl =
          (switch_acl_ipv6_racl_key_value_pair_t *)payload;
      ipv6_racl[count].field = SWITCH_ACL_IPV6_RACL_FIELD_PORT_LAG_LABEL;
      ipv6_racl[count].value.port_lag_label = port_lag_label;
      ipv6_racl[count].mask.u.mask16 = port_lag_mask;
      count++;
      ipv6_racl[count].field = SWITCH_ACL_IPV6_RACL_FIELD_VLAN_RIF_LABEL;
      ipv6_racl[count].value.vlan_rif_label = bd_label;
      ipv6_racl[count].mask.u.mask16 = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_mirror_acl_key_value_pair_t,
                             SWITCH_ACL_IP_MIRROR_ACL_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ip_mirror_acl_key_value_pair_t) * 1));
      switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror =
          (switch_acl_ip_mirror_acl_key_value_pair_t *)payload;
      ip_mirror[count].field = SWITCH_ACL_IP_MIRROR_ACL_FIELD_PORT_LAG_LABEL;
      ip_mirror[count].value.port_lag_label = port_lag_label;
      ip_mirror[count].mask.u.mask = port_lag_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_mirror_acl_key_value_pair_t,
                             SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size +
              (sizeof(switch_acl_ipv6_mirror_acl_key_value_pair_t) * 1));
      switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_mirror =
          (switch_acl_ipv6_mirror_acl_key_value_pair_t *)payload;
      ipv6_mirror[count].field =
          SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_PORT_LAG_LABEL;
      ipv6_mirror[count].value.port_lag_label = port_lag_label;
      ipv6_mirror[count].mask.u.mask16 = port_lag_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_MAC_QOS: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_mac_qos_acl_key_value_pair_t,
                             SWITCH_ACL_MAC_QOS_ACL_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_mac_qos_acl_key_value_pair_t) * 1));
      switch_acl_mac_qos_acl_key_value_pair_t *mac_qos =
          (switch_acl_mac_qos_acl_key_value_pair_t *)payload;
      mac_qos[count].field = SWITCH_ACL_MAC_QOS_ACL_FIELD_PORT_LAG_LABEL;
      mac_qos[count].value.port_lag_label = port_lag_label;
      mac_qos[count].mask.u.mask = port_lag_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IP_QOS: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ip_qos_acl_key_value_pair_t,
                             SWITCH_ACL_IP_QOS_ACL_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ip_qos_acl_key_value_pair_t) * 1));
      switch_acl_ip_qos_acl_key_value_pair_t *ip_qos =
          (switch_acl_ip_qos_acl_key_value_pair_t *)payload;
      ip_qos[count].field = SWITCH_ACL_IP_QOS_ACL_FIELD_PORT_LAG_LABEL;
      ip_qos[count].value.port_lag_label = port_lag_label;
      ip_qos[count].mask.u.mask = port_lag_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_IPV6_QOS: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ipv6_qos_acl_key_value_pair_t,
                             SWITCH_ACL_IPV6_QOS_ACL_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ipv6_qos_acl_key_value_pair_t) * 1));
      switch_acl_ipv6_qos_acl_key_value_pair_t *ipv6_qos =
          (switch_acl_ipv6_qos_acl_key_value_pair_t *)payload;
      ipv6_qos[count].field = SWITCH_ACL_IPV6_QOS_ACL_FIELD_PORT_LAG_LABEL;
      ipv6_qos[count].value.port_lag_label = port_lag_label;
      ipv6_qos[count].mask.u.mask16 = port_lag_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_MAC: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_mac_key_value_pair_t,
                             SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL,
                             count);
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_mac_key_value_pair_t,
                             SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_mac_key_value_pair_t) * 2));
      switch_acl_mac_key_value_pair_t *mac_acl =
          (switch_acl_mac_key_value_pair_t *)payload;
      mac_acl[count].field = SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL;
      mac_acl[count].value.port_lag_label = port_lag_label;
      mac_acl[count].mask.u.mask = port_lag_mask;
      count++;
      mac_acl[count].field = SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL;
      mac_acl[count].value.vlan_rif_label = bd_label;
      mac_acl[count].mask.u.mask = bd_mask;
      count++;
    } break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      break;
    case SWITCH_ACL_TYPE_ECN: {
      SWITCH_ACL_FIELD_CHECK(payload,
                             switch_acl_ecn_key_value_pair_t,
                             SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL,
                             count);
      payload = SWITCH_REALLOC(
          device,
          payload,
          field_size + (sizeof(switch_acl_ecn_key_value_pair_t) * 1));
      switch_acl_ecn_key_value_pair_t *ecn_acl =
          (switch_acl_ecn_key_value_pair_t *)payload;
      ecn_acl[count].field = SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL;
      ecn_acl[count].value.port_lag_label = port_lag_label;
      ecn_acl[count].mask.u.mask = port_lag_mask;
      count++;
    } break;
    default:
      status = SWITCH_STATUS_INVALID_HANDLE;
  }

  *kvp_payload = payload;
  *kvp_count = count;

  return status;
}

/* \brief switch_acl_hw_set:
 * This function programs the hw based on acl type
 */
static switch_status_t switch_acl_hw_set(switch_device_t device,
                                         switch_acl_info_t *acl_info,
                                         switch_handle_t ace_handle,
                                         switch_acl_rule_t *rule,
                                         switch_acl_ref_group_t *ref_group) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_hdl_t pd_hdl = 0;
  void *acl_payload = NULL;
  switch_int32_t kvp_count = 0;

  SWITCH_ASSERT(rule != NULL);
  SWITCH_ASSERT(acl_info != NULL);

  status = switch_acl_payload_get(
      device, acl_info, rule, ref_group, &acl_payload, &kvp_count);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl hw set failed on device %d ace handle 0x%lx: "
        "set field actions failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_SYSTEM:
      status = switch_acl_system_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP:
    case SWITCH_ACL_TYPE_EGRESS_IP_ACL:
      status = switch_acl_ip_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6:
    case SWITCH_ACL_TYPE_EGRESS_IPV6_ACL:
      status = switch_acl_ipv6_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_RACL:
      status = switch_acl_ip_racl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_RACL:
      status = switch_acl_ipv6_racl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      status = switch_acl_ip_mirror_acl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      status = switch_acl_ipv6_mirror_acl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC_QOS:
      status = switch_acl_mac_qos_acl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_QOS:
      status = switch_acl_ip_qos_acl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_QOS:
      status = switch_acl_ipv6_qos_acl_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC:
      status = switch_acl_mac_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      status = switch_acl_egress_system_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    case SWITCH_ACL_TYPE_ECN:
      status = switch_acl_ecn_set_fields_actions(
          device, acl_info->direction, rule, acl_payload, kvp_count, &pd_hdl);
      break;
    default:
      return SWITCH_STATUS_INVALID_HANDLE;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl hw set failed on device %d ace handle 0x%lx: "
        "set field actions failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    SWITCH_ARRAY_INSERT(&acl_info->pd_hdl_array,
                        ace_handle,
                        (void *)((switch_uint64_t)(pd_hdl + 1)));
  } else {
    SWITCH_ARRAY_INSERT(&ref_group->pd_hdl_array,
                        ace_handle,
                        (void *)((switch_uint64_t)(pd_hdl + 1)));
  }

  SWITCH_FREE(device, acl_payload);

  return status;
}

/* \brief switch_acl_hw_action_update:
 * This function updates the hw based on acl action and acl type
 */
static switch_status_t switch_acl_hw_action_update(
    switch_device_t device,
    switch_acl_info_t *acl_info,
    switch_handle_t ace_handle,
    switch_acl_rule_t *rule,
    switch_acl_ref_group_t *ref_group) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_hdl_t pd_hdl = 0;
  switch_uint64_t pd_hdl_tmp = 0;
  void *acl_payload = NULL;
  switch_int32_t kvp_count = 0;

  SWITCH_ASSERT(rule != NULL);
  SWITCH_ASSERT(acl_info != NULL);

  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    status = SWITCH_ARRAY_GET(
        &acl_info->pd_hdl_array, ace_handle, (void **)&pd_hdl_tmp);
  } else {
    status = SWITCH_ARRAY_GET(
        &ref_group->pd_hdl_array, ace_handle, (void **)&pd_hdl_tmp);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule action update failed: Get PD handle for ace_handle %lu: %s",
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  pd_hdl = pd_hdl_tmp - 1;

  status = switch_acl_payload_get(
      device, acl_info, rule, ref_group, &acl_payload, &kvp_count);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule action update failed: payload get for ace_handle %lu: %s",
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_SYSTEM:
      status = switch_acl_system_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP:
      status = switch_acl_ip_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6:
      status = switch_acl_ipv6_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_RACL:
      status = switch_acl_ip_racl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_RACL:
      status = switch_acl_ipv6_racl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      status = switch_acl_ip_mirror_acl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      status = switch_acl_ipv6_mirror_acl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC_QOS:
      status = switch_acl_mac_qos_acl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_QOS:
      status = switch_acl_ip_qos_acl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_QOS:
      status = switch_acl_ipv6_qos_acl_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC:
      status = switch_acl_mac_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      status = switch_acl_egress_system_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_ECN:
      status = switch_acl_ecn_fields_actions_update(
          device, acl_info->direction, rule, acl_payload, kvp_count, pd_hdl);
      break;
    default:
      return SWITCH_STATUS_INVALID_HANDLE;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl rule action update failed for acl group %lx: %s\n",
                     ref_group->acl_group_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/* \brief switch_acl_hw_delete:
 * This function deletes the hw based on acl type
 */
static switch_status_t switch_acl_hw_delete(switch_device_t device,
                                            switch_acl_info_t *acl_info,
                                            switch_handle_t ace_handle,
                                            switch_acl_rule_t *rule,
                                            switch_acl_ref_group_t *ref_group) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint64_t pd_hdl_tmp = 0;
  switch_pd_hdl_t pd_hdl = 0;

  SWITCH_ASSERT(rule != NULL);
  SWITCH_ASSERT(acl_info != NULL);

  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    status = SWITCH_ARRAY_GET(
        &acl_info->pd_hdl_array, ace_handle, (void **)&pd_hdl_tmp);
  } else {
    status = SWITCH_ARRAY_GET(
        &ref_group->pd_hdl_array, ace_handle, (void **)&pd_hdl_tmp);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl hw delete failed on device %d ace handle 0x%lx: "
        "pd handle array get failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  pd_hdl = pd_hdl_tmp - 1;

  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_SYSTEM:
      status = switch_pd_system_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP:
      status = switch_pd_ipv4_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6:
      status = switch_pd_ipv6_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_RACL:
      status = switch_pd_ipv4_racl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_RACL:
      status = switch_pd_ipv6_racl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      status = switch_pd_ipv4_mirror_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      status = switch_pd_ipv6_mirror_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC_QOS:
      status = switch_pd_mac_qos_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IP_QOS:
      status = switch_pd_ipv4_qos_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_IPV6_QOS:
      status = switch_pd_ipv6_qos_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_MAC:
      status = switch_pd_mac_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      status = switch_pd_egress_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    case SWITCH_ACL_TYPE_ECN:
      status = switch_pd_ecn_acl_table_entry_delete(
          device, acl_info->direction, pd_hdl);
      break;
    default:
      break;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl hw delete failed on device %d ace handle 0x%lx: "
        "acl table entry delete failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    status = SWITCH_ARRAY_DELETE(&acl_info->pd_hdl_array, ace_handle);
  } else {
    status = SWITCH_ARRAY_DELETE(&ref_group->pd_hdl_array, ace_handle);
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_handle_t switch_api_acl_group_member_create_internal(
    switch_device_t device,
    switch_handle_t acl_group_handle,
    switch_handle_t acl_handle,
    switch_handle_t *acl_group_member_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_acl_group_member_t *acl_group_member = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_node_t *node = NULL;
  switch_acl_group_member_info_t *acl_group_member_info = NULL;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_GROUP_HANDLE(acl_group_handle));
  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));

  status = switch_acl_group_get(device, acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl group get failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl get failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_info->direction != acl_group_info->direction) {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl table group direction mismatch:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_info->bp_type != acl_group_info->bp_type) {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl table group bind point mismatch:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(acl_group_info->acl_member_list, node) {
    acl_group_member = (switch_acl_group_member_t *)node->data;
    if (acl_group_member->acl_handle == acl_handle) {
      return SWITCH_STATUS_SUCCESS;
    }
  }
  FOR_EACH_IN_LIST_END();

  FOR_EACH_IN_LIST(acl_info->group_list, node) {
    ref_group = (switch_acl_ref_group_t *)node->data;
    if (ref_group->acl_group_handle == acl_group_handle) {
      return SWITCH_STATUS_SUCCESS;
    }
  }
  FOR_EACH_IN_LIST_END();

  handle = switch_acl_group_member_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl group member handle create failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_member_get(device, handle, &acl_group_member_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl group member get failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_group_member_info->acl_group_handle = acl_group_handle;
  acl_group_member_info->acl_handle = acl_handle;

  acl_group_member = SWITCH_MALLOC(device, sizeof(*acl_group_member), 1);
  if (acl_group_member == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "acl group member malloc failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_group_member->acl_handle = acl_handle;
  status = SWITCH_LIST_INSERT(&(acl_group_info->acl_member_list),
                              &(acl_group_member->node),
                              acl_group_member);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  ref_group = SWITCH_MALLOC(device, sizeof(*ref_group), 1);
  if (ref_group == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl group member create failed on device %d "
        "acl handle 0x%lx acl group handle 0x%lx: "
        "ref group malloc failed:(%s)\n",
        device,
        acl_handle,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  ref_group->acl_group_handle = acl_group_handle;

  status = SWITCH_LIST_INSERT(
      &(acl_info->group_list), &(ref_group->node), ref_group);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_ARRAY_INIT(&ref_group->pd_hdl_array);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  FOR_EACH_IN_ARRAY(ace_handle, acl_info->rules, switch_ace_info_t, ace_info) {
    status =
        switch_acl_hw_set(device, acl_info, ace_handle, ace_info, ref_group);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl group member create failed on device %d "
          "acl handle 0x%lx acl group handle 0x%lx: "
          "acl hw set failed:(%s)\n",
          device,
          acl_handle,
          acl_group_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  FOR_EACH_IN_ARRAY_END();

  SWITCH_LOG_DEBUG(
      "acl group member created on device %d acl handle 0%lx "
      "acl group handle 0x%lx acl member handle 0x%lx\n",
      device,
      acl_handle,
      acl_group_handle,
      handle);

  *acl_group_member_handle = handle;

  return status;
}

switch_status_t switch_api_acl_group_member_delete_internal(
    switch_device_t device, switch_handle_t acl_group_member_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_acl_group_member_info_t *acl_group_member_info = NULL;
  switch_node_t *node = NULL;
  switch_acl_group_member_t *group_member = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_GROUP_MEMBER_HANDLE(acl_group_member_handle));
  status = switch_acl_group_member_get(
      device, acl_group_member_handle, &acl_group_member_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member delete failed on device %d "
        "acl member handle 0x%lx: "
        "acl member handle invalid:(%s)\n",
        device,
        acl_group_member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_get(
      device, acl_group_member_info->acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member delete failed on device %d "
        "acl member handle 0x%lx: "
        "acl group get failed:(%s)\n",
        device,
        acl_group_member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_group_member_info->acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member delete failed on device %d "
        "acl member handle 0x%lx: "
        "acl get failed:(%s)\n",
        device,
        acl_group_member_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(acl_group_info->acl_member_list, node) {
    group_member = (switch_acl_group_member_t *)node->data;
    if (group_member->acl_handle == acl_group_member_info->acl_handle) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (node) {
    SWITCH_LIST_DELETE(&(acl_group_info->acl_member_list), node);
  }

  FOR_EACH_IN_LIST(acl_info->group_list, node) {
    ref_group = (switch_acl_ref_group_t *)node->data;
    if (ref_group->acl_group_handle ==
        acl_group_member_info->acl_group_handle) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (node) {
    SWITCH_LIST_DELETE(&(acl_info->group_list), node);
  }

  FOR_EACH_IN_ARRAY(ace_handle, acl_info->rules, switch_ace_info_t, ace_info) {
    status =
        switch_acl_hw_delete(device, acl_info, ace_handle, ace_info, ref_group);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl group member delete failed on device %d "
          "acl member handle 0x%lx ace handle 0x%lx: "
          "acl hw delete failed:(%s)\n",
          device,
          acl_group_member_handle,
          ace_handle,
          switch_error_to_string(status));
    }
  }
  FOR_EACH_IN_ARRAY_END();

  SWITCH_FREE(device, group_member);
  SWITCH_FREE(device, ref_group);

  status =
      switch_acl_group_member_handle_delete(device, acl_group_member_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "acl group member deleted on device %d "
      "acl group member handle 0x%lx\n",
      device,
      acl_group_member_handle);

  return status;
}

/* binds interface to acl/acl group */
switch_status_t switch_acl_reference(const switch_device_t device,
                                     const switch_handle_t acl_handle,
                                     const switch_direction_t direction,
                                     const switch_handle_t bp_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_acl_info_t *acl_info = NULL;
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_handle_t acl_handle_old = SWITCH_API_INVALID_HANDLE;
  switch_handle_t acl_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_acl_handle_t *acl_handle_info = NULL;

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle) ||
                SWITCH_ACL_GROUP_HANDLE(acl_handle));

  status = switch_api_handle_acl_group_get(
      device, bp_handle, direction, &acl_handle_old);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group bp get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_handle_old != SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: bind point still referenced:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_group_handle = acl_handle;
  if (SWITCH_ACL_HANDLE(acl_handle)) {
    status = switch_acl_get(device, acl_handle, &acl_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl reference failed on device %d acl handle 0x%lx "
          "bp handle 0x%lx: acl get failed:(%s)\n",
          device,
          acl_handle,
          bp_handle,
          switch_error_to_string(status));
      return status;
    }
    acl_group_handle = acl_info->default_group;
  }

  SWITCH_ASSERT(SWITCH_ACL_GROUP_HANDLE(acl_group_handle));

  status = switch_acl_group_get(device, acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_handle_info = SWITCH_MALLOC(device, sizeof(*acl_handle_info), 1);
  if (acl_handle_info == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_handle_info->handle = bp_handle;

  SWITCH_LIST_INSERT(&(acl_group_info->handle_list),
                     &(acl_handle_info->node),
                     acl_handle_info);

  status = switch_api_handle_acl_group_set(
      device, bp_handle, direction, acl_group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group bp set failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "acl reference on device %d acl handle 0x%lx "
      "acl group handle 0x%lx bp handle 0x%lx\n",
      device,
      acl_handle,
      acl_group_handle,
      bp_handle);

  return status;
}

switch_status_t switch_api_ingress_acl_reference_internal(
    const switch_device_t device,
    const switch_handle_t acl_handle,
    const switch_handle_t bp_handle) {
  return switch_acl_reference(
      device, acl_handle, SWITCH_API_DIRECTION_INGRESS, bp_handle);
}

switch_status_t switch_api_egress_acl_reference_internal(
    const switch_device_t device,
    const switch_handle_t acl_handle,
    const switch_handle_t bp_handle) {
  return switch_acl_reference(
      device, acl_handle, SWITCH_API_DIRECTION_EGRESS, bp_handle);
}

switch_status_t switch_api_acl_reference_internal(switch_device_t device,
                                                  switch_handle_t acl_handle,
                                                  switch_handle_t bp_handle) {
  switch_direction_t direction = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ACL_DIRECTION_GET(device, acl_handle, direction, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_acl_reference(device, acl_handle, direction, bp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl reference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group bp set failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_acl_dereference(const switch_device_t device,
                                       const switch_handle_t acl_handle,
                                       const switch_direction_t direction,
                                       const switch_handle_t bp_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_handle_t acl_group_handle_old = SWITCH_API_INVALID_HANDLE;
  switch_handle_t acl_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_acl_handle_t *acl_handle_info = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle) ||
                SWITCH_ACL_GROUP_HANDLE(acl_handle));

  acl_group_handle = acl_handle;
  if (SWITCH_ACL_HANDLE(acl_handle)) {
    status = switch_acl_get(device, acl_handle, &acl_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl dereference failed on device %d acl handle 0x%lx "
          "bp handle 0x%lx: acl get failed:(%s)\n",
          device,
          acl_handle,
          bp_handle,
          switch_error_to_string(status));
      return status;
    }
    acl_group_handle = acl_info->default_group;
  }

  status = switch_api_handle_acl_group_get(
      device, bp_handle, direction, &acl_group_handle_old);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl dereference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group bp get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_group_handle_old != acl_group_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "acl dereference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: bind point mismatch:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ACL_GROUP_HANDLE(acl_group_handle));

  status = switch_acl_group_get(device, acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl dereference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(acl_group_info->handle_list, node) {
    acl_handle_info = (switch_acl_handle_t *)node->data;
    if (acl_handle_info->handle == bp_handle) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END()

  if (node) {
    SWITCH_LIST_DELETE(&(acl_group_info->handle_list), node);
    SWITCH_FREE(device, acl_handle_info);
  }

  status = switch_api_handle_acl_group_set(
      device, bp_handle, direction, SWITCH_API_INVALID_HANDLE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl dereference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group get failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "acl dereference on device %d acl handle 0x%lx "
      "acl group handle 0x%lx bp handle 0x%lx\n",
      device,
      acl_handle,
      acl_group_handle,
      bp_handle);

  return status;
}

switch_status_t switch_api_ingress_acl_dereference_internal(
    const switch_device_t device,
    const switch_handle_t acl_handle,
    const switch_handle_t bp_handle) {
  return switch_acl_dereference(
      device, acl_handle, SWITCH_API_DIRECTION_INGRESS, bp_handle);
}

switch_status_t switch_api_egress_acl_dereference_internal(
    const switch_device_t device,
    const switch_handle_t acl_handle,
    const switch_handle_t bp_handle) {
  return switch_acl_dereference(
      device, acl_handle, SWITCH_API_DIRECTION_EGRESS, bp_handle);
}

switch_status_t switch_api_acl_dereference_internal(switch_device_t device,
                                                    switch_handle_t acl_handle,
                                                    switch_handle_t bp_handle) {
  switch_direction_t direction = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ACL_DIRECTION_GET(device, acl_handle, direction, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_acl_dereference(device, acl_handle, direction, bp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl dereference failed on device %d acl handle 0x%lx "
        "bp handle 0x%lx: acl group bp set failed:(%s)\n",
        device,
        acl_handle,
        bp_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

static switch_status_t switch_acl_ipv4_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_ip_key_value_pair_t *ip_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t tmp_length = 0;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl ip to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  ip_acl = (switch_acl_ip_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length += snprintf((buffer + *length),
                        (buffer_size - *length),
                        "\n\t\t\t\t%s: ",
                        switch_acl_ipv4_field_to_string(ip_acl[index].field));

    switch (ip_acl[index].field) {
      case SWITCH_ACL_IP_FIELD_IPV4_SRC:
      case SWITCH_ACL_IP_FIELD_IPV4_DEST:
        status = switch_ipv4_to_string(ip_acl[index].value.ipv4_source,
                                       (buffer + *length),
                                       (buffer_size - *length),
                                       &tmp_length);
        *length += tmp_length;
        break;
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%lx",
                            ip_acl[index].value.sport_range_handle);
        break;
      case SWITCH_ACL_IP_FIELD_IP_PROTO:
      case SWITCH_ACL_IP_FIELD_ICMP_TYPE:
      case SWITCH_ACL_IP_FIELD_ICMP_CODE:
      case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
      case SWITCH_ACL_IP_FIELD_TTL:
      case SWITCH_ACL_IP_FIELD_IP_FLAGS:
      case SWITCH_ACL_IP_FIELD_IP_FRAGMENT:
      case SWITCH_ACL_IP_FIELD_RMAC_HIT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            ip_acl[index].value.ip_proto);
        break;
      case SWITCH_ACL_IP_FIELD_ETH_TYPE:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            ip_acl[index].value.eth_type);
        break;
      case SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            ip_acl[index].value.port_lag_label);
        break;
      case SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            ip_acl[index].value.vlan_rif_label);
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR("acl ip to string failed: %s(0x%x)\n",
                         switch_error_to_string(status),
                         ip_acl[index].field);
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_acl_ipv6_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_ipv6_key_value_pair_t *ipv6_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t tmp_length = 0;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl ipv6 to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  ipv6_acl = (switch_acl_ipv6_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length += snprintf((buffer + *length),
                        (buffer_size - *length),
                        "\n\t\t\t\t%s: ",
                        switch_acl_ipv6_field_to_string(ipv6_acl[index].field));

    switch (ipv6_acl[index].field) {
      case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
      case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
        status = switch_ipv6_to_string(ipv6_acl[index].value.ipv6_source,
                                       (buffer + *length),
                                       (buffer_size - *length),
                                       &tmp_length);
        *length += tmp_length;
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT:
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%lx",
                            ipv6_acl[index].value.sport_range_handle);
        break;
      case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
      case SWITCH_ACL_IPV6_FIELD_ICMP_TYPE:
      case SWITCH_ACL_IPV6_FIELD_ICMP_CODE:
      case SWITCH_ACL_IPV6_FIELD_TCP_FLAGS:
      case SWITCH_ACL_IPV6_FIELD_TTL:
      case SWITCH_ACL_IPV6_FIELD_FLOW_LABEL:
      case SWITCH_ACL_IPV6_FIELD_RMAC_HIT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            ipv6_acl[index].value.ip_proto);
        break;
      case SWITCH_ACL_IPV6_FIELD_ETH_TYPE:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            ipv6_acl[index].value.eth_type);
        break;
      case SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            ipv6_acl[index].value.port_lag_label);
        break;
      case SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            ipv6_acl[index].value.vlan_rif_label);
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR("acl ipv6 to string failed: %s(0x%x)\n",
                         switch_error_to_string(status),
                         ipv6_acl[index].field);
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_acl_ecn_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_ecn_key_value_pair_t *ecn_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl ecn to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  ecn_acl = (switch_acl_ecn_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length +=
        snprintf((buffer + *length),
                 (buffer_size - *length),
                 "\n\t\t\t\t%s: ",
                 switch_acl_ecn_acl_field_to_string(ecn_acl[index].field));

    switch (ecn_acl[index].field) {
      case SWITCH_ACL_ECN_FIELD_DSCP:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d %d",
                            (switch_uint32_t)ecn_acl[index].value.dscp,
                            (switch_uint32_t)ecn_acl[index].mask.u.mask);
        break;
      case SWITCH_ACL_ECN_FIELD_ECN:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d %d",
                            (switch_uint32_t)ecn_acl[index].value.ecn,
                            (switch_uint32_t)ecn_acl[index].mask.u.mask);
        break;
      case SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL:
        *length +=
            snprintf((buffer + *length),
                     (buffer_size - *length),
                     "%d",
                     (switch_uint32_t)ecn_acl[index].value.port_lag_label);
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR("acl ecn to string failed: %s(0x%x)\n",
                         switch_error_to_string(status),
                         ecn_acl[index].field);
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_acl_mac_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_mac_key_value_pair_t *mac_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t tmp_length = 0;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl mac to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  mac_acl = (switch_acl_mac_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length += snprintf((buffer + *length),
                        (buffer_size - *length),
                        "\n\t\t\t\t%s: ",
                        switch_acl_mac_field_to_string(mac_acl[index].field));

    switch (mac_acl[index].field) {
      case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
      case SWITCH_ACL_MAC_FIELD_DEST_MAC:
        status = switch_mac_to_string(&mac_acl[index].value.source_mac,
                                      (buffer + *length),
                                      (buffer_size - *length),
                                      &tmp_length);
        *length += tmp_length;
        break;
      case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
      case SWITCH_ACL_MAC_FIELD_VLAN_PRI:
      case SWITCH_ACL_MAC_FIELD_VLAN_CFI:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            mac_acl[index].value.eth_type);
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            mac_acl[index].value.port_lag_label);
        break;
      case SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            mac_acl[index].value.vlan_rif_label);
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        *length = 0;
        SWITCH_LOG_ERROR("acl mac to string failed: %s(0x%x)\n",
                         switch_error_to_string(status),
                         mac_acl[index].field);
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_acl_system_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_system_key_value_pair_t *system_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t tmp_length = 0;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl system to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  system_acl = (switch_acl_system_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length +=
        snprintf((buffer + *length),
                 (buffer_size - *length),
                 "\n\t\t\t\t%s: ",
                 switch_acl_system_field_to_string(system_acl[index].field));

    switch (system_acl[index].field) {
      case SWITCH_ACL_SYSTEM_FIELD_SOURCE_MAC:
      case SWITCH_ACL_SYSTEM_FIELD_DEST_MAC:
        status = switch_mac_to_string(&system_acl[index].value.source_mac,
                                      buffer,
                                      (buffer_size - *length),
                                      &tmp_length);
        *length += tmp_length;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_PORT_VLAN_MAPPING_MISS:
      case SWITCH_ACL_SYSTEM_FIELD_IPSG_CHECK:
      case SWITCH_ACL_SYSTEM_FIELD_ACL_DENY:
      case SWITCH_ACL_SYSTEM_FIELD_RACL_DENY:
      case SWITCH_ACL_SYSTEM_FIELD_URPF_CHECK:
      case SWITCH_ACL_SYSTEM_FIELD_METER_DROP:
      case SWITCH_ACL_SYSTEM_FIELD_L3_COPY:
      case SWITCH_ACL_SYSTEM_FIELD_DROP:
      case SWITCH_ACL_SYSTEM_FIELD_ROUTED:
      case SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL:
      case SWITCH_ACL_SYSTEM_FIELD_NEXTHOP_GLEAN:
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_HIT:
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_S_G_HIT:
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_COPY_TO_CPU:
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_RPF_FAIL:
      case SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX:
      case SWITCH_ACL_SYSTEM_FIELD_CONTROL_FRAME:
      case SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED:
      case SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED:
      case SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT:
      case SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE:
      case SWITCH_ACL_SYSTEM_FIELD_IF_CHECK:
      case SWITCH_ACL_SYSTEM_FIELD_BD_CHECK:
      case SWITCH_ACL_SYSTEM_FIELD_STP_STATE:
      case SWITCH_ACL_SYSTEM_FIELD_TUNNEL_IF_CHECK:
      case SWITCH_ACL_SYSTEM_FIELD_TTL:
      case SWITCH_ACL_SYSTEM_FIELD_REASON_CODE:
      case SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP:
      case SWITCH_ACL_SYSTEM_FIELD_DROP_CTL:
      case SWITCH_ACL_SYSTEM_FIELD_L2_DST_MISS:
      case SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE:
      case SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR:
      case SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE:
      case SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP:
      case SWITCH_ACL_SYSTEM_FIELD_INGRESS_IFINDEX:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            system_acl[index].value.eth_type);
        break;
      case SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            system_acl[index].value.port_lag_label);
        break;
      case SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            system_acl[index].value.vlan_rif_label);
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        *length = 0;
        SWITCH_LOG_ERROR("acl system to string failed: %s(0x%x)\n",
                         switch_error_to_string(status),
                         system_acl[index].field);
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_acl_egress_acl_to_string(
    void *acl_kvp,
    switch_uint32_t acl_kvp_count,
    char *buffer,
    switch_int32_t buffer_size,
    switch_int32_t *length) {
  switch_uint32_t index = 0;
  switch_acl_egress_system_key_value_pair_t *egr_acl = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  if (!acl_kvp || !buffer) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl egress to string failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  egr_acl = (switch_acl_egress_system_key_value_pair_t *)acl_kvp;
  *length = 0;

  for (index = 0; index < acl_kvp_count; index++) {
    *length +=
        snprintf((buffer + *length),
                 (buffer_size - *length),
                 "\n\t\t\t\t%s: ",
                 switch_acl_egress_field_to_string(egr_acl[index].field));

    switch (egr_acl[index].field) {
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%lx",
                            egr_acl[index].value.egr_port);
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_SRC_PORT:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%lx",
                            egr_acl[index].value.ing_port);
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT:
        *length +=
            snprintf((buffer + *length),
                     (buffer_size - *length),
                     "%s",
                     egr_acl[index].value.deflection_flag ? "true" : "false");
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.l3_mtu_check);
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_ACL_DENY:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.acl_deny);
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_REASON_CODE:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.reason_code);
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.mirror_on_drop);
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.queue_dod_enable);
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_PACKET_COLOR:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%d",
                            egr_acl[index].value.packet_color);
        break;

      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DROP_CTL:
        *length += snprintf((buffer + *length),
                            (buffer_size - *length),
                            "%x",
                            egr_acl[index].value.drop_ctl);
        break;

      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        *length = 0;
        SWITCH_LOG_ERROR("acl egress to string failed: %s\n",
                         switch_error_to_string(status));
        return status;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_acl_print_kvp(switch_acl_type_t acl_type,
                                     void *acl_kvp,
                                     switch_uint32_t acl_kvp_count,
                                     char *buffer,
                                     switch_int32_t buffer_size) {
  switch_int32_t length = 0;
  switch_int32_t tmp_length = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(acl_kvp != NULL);
  SWITCH_ASSERT(buffer != NULL);

  length = snprintf(buffer, buffer_size, "\t\t\tACL Key:\n");
  length += snprintf(buffer + length,
                     buffer_size - length,
                     "\t\t\t\tacl type: %s\n",
                     switch_acl_type_to_string(acl_type));
  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP:
      status = switch_acl_ipv4_acl_to_string(acl_kvp,
                                             acl_kvp_count,
                                             (buffer + length),
                                             (buffer_size - length),
                                             &tmp_length);
      break;
    case SWITCH_ACL_TYPE_IPV6:
      status = switch_acl_ipv6_acl_to_string(acl_kvp,
                                             acl_kvp_count,
                                             (buffer + length),
                                             (buffer_size - length),
                                             &tmp_length);
      break;
    case SWITCH_ACL_TYPE_SYSTEM:
      status = switch_acl_system_acl_to_string(acl_kvp,
                                               acl_kvp_count,
                                               (buffer + length),
                                               (buffer_size - length),
                                               &tmp_length);
      break;
    case SWITCH_ACL_TYPE_MAC:
      status = switch_acl_mac_acl_to_string(acl_kvp,
                                            acl_kvp_count,
                                            (buffer + length),
                                            (buffer_size - length),
                                            &tmp_length);
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      status = switch_acl_egress_acl_to_string(acl_kvp,
                                               acl_kvp_count,
                                               (buffer + length),
                                               (buffer_size - length),
                                               &tmp_length);
      break;
    case SWITCH_ACL_TYPE_ECN:
      status = switch_acl_ecn_acl_to_string(acl_kvp,
                                            acl_kvp_count,
                                            (buffer + length),
                                            (buffer_size - length),
                                            &tmp_length);
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }
  length += tmp_length;
  return status;
}

bool ace_fields_include_specified_field(void *acl_kvp,
                                        unsigned int acl_kvp_count,
                                        switch_uint16_t field,
                                        switch_acl_type_t acl_type) {
  switch_uint16_t index = 0;
  if (acl_kvp == NULL || acl_kvp_count == 0) {
    return false;
  }
  switch (acl_type) {
    case SWITCH_ACL_TYPE_SYSTEM: {
      switch_acl_system_key_value_pair_t *ing_acl_kvp =
          (switch_acl_system_key_value_pair_t *)acl_kvp;
      switch_acl_system_field_t system_field = (switch_acl_system_field_t)field;

      for (index = 0; index < acl_kvp_count; index++) {
        if (ing_acl_kvp[index].field == system_field) {
          return true;
        }
      }
    } break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
      switch_acl_egress_system_key_value_pair_t *egr_acl_kvp =
          (switch_acl_egress_system_key_value_pair_t *)acl_kvp;
      switch_acl_egress_system_field_t egr_field =
          (switch_acl_egress_system_field_t)field;
      for (index = 0; index < acl_kvp_count; index++) {
        if (egr_acl_kvp[index].field == egr_field) {
          return true;
        }
      }
    } break;
    default:
      break;
  }
  return false;
}

/**
 * \brief switch_api_acl_rule_create:
 * Create a acl rule (filter)
 *
 * This function creates a access control entry using the key value pair
 * provided. key values are interpreted based on the acl list type.
 *
 * \param device Device number
 * \param acl_handle Acl list handle
 * \param priority Ace entry priority
 * \param acl_kvp_count Number of key value pairs
 * \param acl_kvp Key value pairs
 * \param action Acl action to perform on the entry
 * \param action_params Action parameters (reason code, port redirect)
 * \param opt_action_params Optional action parameters (mirror handle, meter
 *handle)
 * \return switch_status_t status of acl rule create
 */
switch_status_t switch_api_acl_rule_create_internal(
    switch_device_t device,
    switch_handle_t acl_handle,
    unsigned int priority,
    unsigned int acl_kvp_count,
    void *acl_kvp,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_handle_t *ace_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_node_t *node = NULL;
  switch_int32_t field_size = 0;
  switch_int32_t copy_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  if (priority == 0) {
    priority = SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY;
  }
  /*
   * Check for valid acl handle
   */
  if (!SWITCH_ACL_HANDLE(acl_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "acl rule create failed on device %d acl handle 0x%lx "
        "acl handle invalid:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule create failed on device %d acl handle 0x%lx "
        "acl get failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Create an ACE handle
   */
  *ace_handle = switch_ace_handle_create(device);
  if (*ace_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "acl rule create failed on device %d acl handle 0x%lx "
        "ace handle create failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ace_get(device, *ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule create failed on device %d acl handle 0x%lx "
        "ace get failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  ace_info->mod_ace_handle = SWITCH_API_INVALID_HANDLE;

  field_size = switch_acl_field_size_get(acl_info, acl_kvp_count);
  copy_size = field_size;

  switch_pd_feature_t *pd_feature = switch_pd_feature_get();
  switch (acl_info->type) {
    case SWITCH_ACL_TYPE_SYSTEM:
      if (pd_feature->mirror_on_drop && action == SWITCH_ACL_ACTION_DROP) {
        if (!ace_fields_include_specified_field(
                acl_kvp,
                acl_kvp_count,
                SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP,
                acl_info->type)) {
          field_size += sizeof(switch_acl_system_key_value_pair_t);
        }
      }
      break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
      switch_acl_egress_system_action_t egr_action =
          (switch_acl_egress_system_action_t)action;
      if (pd_feature->mirror_on_drop &&
          egr_action == SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP) {
        if (!ace_fields_include_specified_field(
                acl_kvp,
                acl_kvp_count,
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP,
                acl_info->type)) {
          field_size += sizeof(switch_acl_egress_system_key_value_pair_t);
        }
      }
    } break;
    default:
      break;
  }

  ace_info->field_count = acl_kvp_count;

  if (field_size) {
    ace_info->fields = SWITCH_MALLOC(device, field_size, 0x1);
    if (!ace_info->fields) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "acl rule create failed on device %d acl handle 0x%lx "
          "ace fields malloc failed:(%s)\n",
          device,
          acl_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_MEMSET(ace_info->fields, 0, field_size);
    SWITCH_MEMCPY(ace_info->fields, acl_kvp, copy_size);

    if (field_size != copy_size) {
      switch_handle_t mod_ace_handle;
      switch_acl_action_params_t mod_action_params;
      SWITCH_MEMSET(&mod_action_params, 0x0, sizeof(mod_action_params));
      switch (acl_info->type) {
        case SWITCH_ACL_TYPE_SYSTEM: {
          switch_acl_system_key_value_pair_t *ing_acl_fields =
              (switch_acl_system_key_value_pair_t *)ace_info->fields;
          /* first create mod rule with mirror_on_drop = 1
           * and action mirror_and_drop */
          ing_acl_fields[acl_kvp_count].field =
              SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP;
          ing_acl_fields[acl_kvp_count].value.mirror_on_drop = 1;
          ing_acl_fields[acl_kvp_count].mask.u.mask = 0x01;
          if (action_params != NULL && action_params->drop.reason_code != 0) {
            mod_action_params.drop.reason_code =
                action_params->drop.reason_code;
          } else if (!ace_fields_include_specified_field(
                         acl_kvp,
                         acl_kvp_count,
                         SWITCH_ACL_SYSTEM_FIELD_DROP,
                         acl_info->type)) {
            mod_action_params.drop.reason_code = DROP_OTHERS_INGRESS;
          }
          status = switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              acl_kvp_count + 1,
                                              ace_info->fields,
                                              SWITCH_ACL_ACTION_MIRROR_AND_DROP,
                                              &mod_action_params,
                                              opt_action_params,
                                              &mod_ace_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "acl rule create failed on device %d acl handle 0x%lx "
                "creating mod rule:(%s)\n",
                device,
                acl_handle,
                switch_error_to_string(status));
            goto cleanup;
          }
          ace_info->mod_ace_handle = mod_ace_handle;
          /* continue creating base rule with mirror_on_drop = 0 */
          ing_acl_fields[acl_kvp_count].value.mirror_on_drop = 0;
          ace_info->field_count++;
        } break;
        case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
          switch_acl_egress_system_key_value_pair_t *egr_acl_fields =
              (switch_acl_egress_system_key_value_pair_t *)ace_info->fields;
          /* first create mod rule with mirror_on_drop = 1
           * and action mirror_and_drop */
          egr_acl_fields[acl_kvp_count].field =
              SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
          egr_acl_fields[acl_kvp_count].value.mirror_on_drop = 1;
          egr_acl_fields[acl_kvp_count].mask.u.mask = 0x01;
          if (action_params != NULL && action_params->drop.reason_code != 0) {
            mod_action_params.drop.reason_code =
                action_params->drop.reason_code;
          } else {
            mod_action_params.drop.reason_code = DROP_OTHERS_EGRESS;
          }
          status = switch_api_acl_rule_create(
              device,
              acl_handle,
              priority,
              acl_kvp_count + 1,
              ace_info->fields,
              SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP,
              &mod_action_params,
              opt_action_params,
              &mod_ace_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "acl rule create failed on device %d acl handle 0x%lx "
                "creating mod rule:(%s)\n",
                device,
                acl_handle,
                switch_error_to_string(status));
            goto cleanup;
          }
          ace_info->mod_ace_handle = mod_ace_handle;
          /* continue creating base rule with mirror_on_drop = 0 */
          egr_acl_fields[acl_kvp_count].value.mirror_on_drop = 0;
          ace_info->field_count++;
        } break;
        default:
          status = SWITCH_STATUS_INVALID_PARAMETER;
          SWITCH_LOG_ERROR(
              "acl rule create failed for handle %lx: %s "
              "mismatch while adding hidden match fields\n",
              acl_handle,
              switch_error_to_string(status));
          goto cleanup;
      }
    }
  }

  ace_info->acl_handle = acl_handle;
  ace_info->action = action;
  ace_info->action_params = *action_params;
  ace_info->opt_action_params = *opt_action_params;
  ace_info->priority = priority;

  status = SWITCH_ARRAY_INSERT(&acl_info->rules, *ace_handle, (void *)ace_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    status = switch_acl_hw_set(device, acl_info, *ace_handle, ace_info, NULL);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl rule create failed on device %d acl handle 0x%lx "
          "acl hw set failed:(%s)\n",
          device,
          acl_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  } else {
    FOR_EACH_IN_LIST(acl_info->group_list, node) {
      ref_group = (switch_acl_ref_group_t *)node->data;
      status =
          switch_acl_hw_set(device, acl_info, *ace_handle, ace_info, ref_group);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl rule create failed on device %d acl handle 0x%lx "
            "acl hw set failed:(%s)\n",
            device,
            acl_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }
    FOR_EACH_IN_LIST_END();
  }

  if (field_size) {
    char buffer[SWITCH_LOG_BUFFER_SIZE];
    switch_acl_print_kvp(
        acl_info->type, acl_kvp, acl_kvp_count, buffer, SWITCH_LOG_BUFFER_SIZE);
    SWITCH_LOG_DETAIL(
        "acl rule created successfully for "
        "device %d acl handle %lx "
        "ace handle %lx rule %s "
        "priority %x action %s\n",
        device,
        acl_handle,
        *ace_handle,
        buffer,
        priority,
        switch_acl_action_to_string(action));
  }

  SWITCH_LOG_DEBUG(
      "acl rule created successfully for device %d"
      "acl handle %lx: ace handle %lx\n",
      device,
      acl_handle,
      *ace_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  if (ace_info && ace_info->mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
    tmp_status = switch_api_acl_rule_delete(
        device, acl_handle, ace_info->mod_ace_handle);
    SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  }
  if (ace_info && ace_info->fields) {
    SWITCH_FREE(device, ace_info->fields);
  }

  if (*ace_handle) {
    status = switch_ace_handle_delete(device, *ace_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }
  return status;
}

switch_status_t switch_api_acl_entry_action_set_internal(
    switch_device_t device,
    switch_handle_t ace_handle,
    unsigned int priority,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params) {
  switch_acl_info_t *acl_info = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_ace_info_t *temp_ace_info = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_handle_t acl_handle = 0;
  switch_node_t *node = NULL;
  bool reprogram = false;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));
  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule action update failed on device %d: for ace handle %lx : %s\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_handle = ace_info->acl_handle;
  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule action update failed on device %d, for acl handle %lx: %s\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  temp_ace_info =
      (switch_ace_info_t *)SWITCH_MALLOC(device, sizeof(switch_ace_info_t), 1);
  SWITCH_MEMSET(temp_ace_info, 0, sizeof(switch_ace_info_t));
  SWITCH_MEMCPY(temp_ace_info, ace_info, sizeof(switch_ace_info_t));

  if (action != SWITCH_ACL_ACTION_NOP) {
    temp_ace_info->action = action;
    switch_pd_feature_t *pd_feature = switch_pd_feature_get();
    if (pd_feature->mirror_on_drop) {
      switch_int32_t field_size = 0;
      switch_int32_t copy_size = 0;
      switch_uint16_t field_count = ace_info->field_count;
      switch_acl_action_params_t mod_action_params;
      SWITCH_MEMSET(&mod_action_params, 0x0, sizeof(mod_action_params));
      switch (acl_info->type) {
        case SWITCH_ACL_TYPE_SYSTEM: {
          switch_acl_system_key_value_pair_t *ing_acl_fields =
              (switch_acl_system_key_value_pair_t *)ace_info->fields;
          if (action == SWITCH_ACL_ACTION_DROP &&
              ace_info->mod_ace_handle == SWITCH_API_INVALID_HANDLE) {
            if (!ace_fields_include_specified_field(
                    ace_info->fields,
                    field_count,
                    SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP,
                    acl_info->type)) {
              /* need to add mod ace so that drops are mirrored */
              reprogram = true;
              copy_size =
                  sizeof(switch_acl_system_key_value_pair_t) * field_count;
              field_size =
                  copy_size + sizeof(switch_acl_system_key_value_pair_t);

              temp_ace_info->fields = SWITCH_MALLOC(device, field_size, 0x1);
              if (!temp_ace_info->fields) {
                status = SWITCH_STATUS_NO_MEMORY;
                SWITCH_LOG_ERROR(
                    "ace action set failed on device %d ace handle 0x%lx "
                    "ace fields malloc failed:(%s)\n",
                    device,
                    ace_handle,
                    switch_error_to_string(status));
                goto cleanup;
              }

              SWITCH_MEMSET(temp_ace_info->fields, 0, field_size);
              SWITCH_MEMCPY(temp_ace_info->fields, ace_info->fields, copy_size);
              switch_acl_system_key_value_pair_t *temp_ing_acl_fields =
                  (switch_acl_system_key_value_pair_t *)temp_ace_info->fields;

              /* first create mod rule with mirror_on_drop = 1
               * and action mirror_and_drop */
              temp_ing_acl_fields[field_count].field =
                  SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP;
              temp_ing_acl_fields[field_count].value.mirror_on_drop = 1;
              temp_ing_acl_fields[field_count].mask.u.mask = 0x01;
              if (action_params != NULL &&
                  action_params->drop.reason_code != 0) {
                mod_action_params.drop.reason_code =
                    action_params->drop.reason_code;
              } else if (!ace_fields_include_specified_field(
                             ace_info->fields,
                             field_count,
                             SWITCH_ACL_SYSTEM_FIELD_DROP,
                             acl_info->type)) {
                mod_action_params.drop.reason_code = DROP_OTHERS_INGRESS;
              }
              status =
                  switch_api_acl_rule_create(device,
                                             acl_handle,
                                             priority,
                                             field_count + 1,
                                             temp_ace_info->fields,
                                             SWITCH_ACL_ACTION_MIRROR_AND_DROP,
                                             &mod_action_params,
                                             opt_action_params,
                                             &temp_ace_info->mod_ace_handle);
              if (status != SWITCH_STATUS_SUCCESS) {
                SWITCH_LOG_ERROR(
                    "acl action set failed on device %d ace handle 0x%lx "
                    "creating mod rule:(%s)\n",
                    device,
                    ace_handle,
                    switch_error_to_string(status));
                goto cleanup;
              }
              /* continue creating base rule with mirror_on_drop = 0 */
              temp_ing_acl_fields[field_count].value.mirror_on_drop = 0;
              temp_ace_info->field_count++;
            }
          } else if (action == SWITCH_ACL_ACTION_DROP) {
            if (action_params != NULL && action_params->drop.reason_code != 0) {
              mod_action_params.drop.reason_code =
                  action_params->drop.reason_code;
            } else if (!ace_fields_include_specified_field(
                           ace_info->fields,
                           field_count,
                           SWITCH_ACL_SYSTEM_FIELD_DROP,
                           acl_info->type)) {
              mod_action_params.drop.reason_code = DROP_OTHERS_INGRESS;
            }
            status = switch_api_acl_entry_action_set(
                device,
                temp_ace_info->mod_ace_handle,
                priority,
                SWITCH_ACL_ACTION_MIRROR_AND_DROP,
                &mod_action_params,
                opt_action_params);
            if (status != SWITCH_STATUS_SUCCESS) {
              SWITCH_LOG_ERROR(
                  "acl action set failed on device %d ace handle 0x%lx "
                  "setting action in mod rule:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
          } else if (action != SWITCH_ACL_ACTION_DROP &&
                     ace_info->mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
            /* check that last field is mirror_on_drop */
            if (field_count == 0 ||
                ing_acl_fields[field_count - 1].field !=
                    SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP) {
              status = SWITCH_STATUS_INVALID_HANDLE;
              SWITCH_LOG_ERROR(
                  "acl entry action set failed for handle %lx: "
                  "%s mod ace exists but last field is not mod"
                  "\n",
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }

            /* need to remove mod ace */
            reprogram = true;
            temp_ace_info->mod_ace_handle = SWITCH_API_INVALID_HANDLE;

            temp_ace_info->field_count--;
            field_size = sizeof(switch_acl_system_key_value_pair_t) *
                         temp_ace_info->field_count;
            temp_ace_info->fields = SWITCH_MALLOC(device, field_size, 0x1);
            if (!temp_ace_info->fields) {
              status = SWITCH_STATUS_NO_MEMORY;
              SWITCH_LOG_ERROR(
                  "ace action set failed on device %d ace handle 0x%lx "
                  "ace fields malloc failed:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
            SWITCH_MEMCPY(temp_ace_info->fields, ace_info->fields, field_size);

            status = switch_api_acl_rule_delete(
                device, acl_handle, ace_info->mod_ace_handle);
            if (status != SWITCH_STATUS_SUCCESS) {
              SWITCH_LOG_ERROR(
                  "acl action set failed on device %d ace handle 0x%lx "
                  "deleting mod rule:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
            ace_info->mod_ace_handle = SWITCH_API_INVALID_HANDLE;
          }
        } break;
        case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
          switch_acl_egress_system_key_value_pair_t *egr_acl_fields =
              (switch_acl_egress_system_key_value_pair_t *)ace_info->fields;
          switch_acl_egress_system_action_t egr_action =
              (switch_acl_egress_system_action_t)action;
          if (egr_action == SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP &&
              ace_info->mod_ace_handle == SWITCH_API_INVALID_HANDLE) {
            if (!ace_fields_include_specified_field(
                    ace_info->fields,
                    field_count,
                    SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP,
                    acl_info->type)) {
              /* need to add mod ace so that drops are mirrored */
              reprogram = true;
              copy_size = sizeof(switch_acl_egress_system_key_value_pair_t) *
                          field_count;
              field_size =
                  copy_size + sizeof(switch_acl_egress_system_key_value_pair_t);

              temp_ace_info->fields = SWITCH_MALLOC(device, field_size, 0x1);
              if (!temp_ace_info->fields) {
                status = SWITCH_STATUS_NO_MEMORY;
                SWITCH_LOG_ERROR(
                    "ace action set failed on device %d ace handle 0x%lx "
                    "ace fields malloc failed:(%s)\n",
                    device,
                    ace_handle,
                    switch_error_to_string(status));
                goto cleanup;
              }

              SWITCH_MEMSET(temp_ace_info->fields, 0, field_size);
              SWITCH_MEMCPY(temp_ace_info->fields, ace_info->fields, copy_size);
              switch_acl_egress_system_key_value_pair_t *
                  temp_egress_system_acl_fields =
                      (switch_acl_egress_system_key_value_pair_t *)
                          temp_ace_info->fields;

              /* first create mod rule with mirror_on_drop = 1
               * and action mirror_and_drop */
              temp_egress_system_acl_fields[field_count].field =
                  SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP;
              temp_egress_system_acl_fields[field_count].value.mirror_on_drop =
                  1;
              temp_egress_system_acl_fields[field_count].mask.u.mask = 0x01;
              if (action_params != NULL &&
                  action_params->drop.reason_code != 0) {
                mod_action_params.drop.reason_code =
                    action_params->drop.reason_code;
              } else {
                mod_action_params.drop.reason_code = DROP_OTHERS_EGRESS;
              }
              status = switch_api_acl_rule_create(
                  device,
                  acl_handle,
                  priority,
                  field_count + 1,
                  temp_ace_info->fields,
                  SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP,
                  &mod_action_params,
                  opt_action_params,
                  &temp_ace_info->mod_ace_handle);
              if (status != SWITCH_STATUS_SUCCESS) {
                SWITCH_LOG_ERROR(
                    "acl action set failed on device %d ace handle 0x%lx "
                    "creating mod rule:(%s)\n",
                    device,
                    ace_handle,
                    switch_error_to_string(status));
                goto cleanup;
              }
              /* continue creating base rule with mirror_on_drop = 0 */
              temp_egress_system_acl_fields[field_count].value.mirror_on_drop =
                  0;
              temp_ace_info->field_count++;
            }
          } else if (egr_action == SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP) {
            if (action_params != NULL && action_params->drop.reason_code != 0) {
              mod_action_params.drop.reason_code =
                  action_params->drop.reason_code;
            } else {
              mod_action_params.drop.reason_code = DROP_OTHERS_EGRESS;
            }
            status = switch_api_acl_entry_action_set(
                device,
                temp_ace_info->mod_ace_handle,
                priority,
                SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP,
                &mod_action_params,
                opt_action_params);
            if (status != SWITCH_STATUS_SUCCESS) {
              SWITCH_LOG_ERROR(
                  "acl action set failed on device %d ace handle 0x%lx "
                  "setting action in egress mod rule:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
          } else if (egr_action != SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP &&
                     ace_info->mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
            /* check that last field is mirror_on_drop */
            if (field_count == 0 ||
                egr_acl_fields[field_count - 1].field !=
                    SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP) {
              status = SWITCH_STATUS_INVALID_HANDLE;
              SWITCH_LOG_ERROR(
                  "acl entry action set failed for handle %lx: "
                  "%s mod ace exists but last field is not mod"
                  "\n",
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }

            /* need to remove mod ace */
            reprogram = true;
            temp_ace_info->mod_ace_handle = SWITCH_API_INVALID_HANDLE;

            temp_ace_info->field_count--;
            field_size = sizeof(switch_acl_egress_system_key_value_pair_t) *
                         temp_ace_info->field_count;
            temp_ace_info->fields = SWITCH_MALLOC(device, field_size, 0x1);
            if (!temp_ace_info->fields) {
              status = SWITCH_STATUS_NO_MEMORY;
              SWITCH_LOG_ERROR(
                  "ace action set failed on device %d ace handle 0x%lx "
                  "ace fields malloc failed:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
            SWITCH_MEMCPY(temp_ace_info->fields, ace_info->fields, field_size);

            status = switch_api_acl_rule_delete(
                device, acl_handle, ace_info->mod_ace_handle);
            if (status != SWITCH_STATUS_SUCCESS) {
              SWITCH_LOG_ERROR(
                  "acl action set failed on device %d ace handle 0x%lx "
                  "deleting mod rule:(%s)\n",
                  device,
                  ace_handle,
                  switch_error_to_string(status));
              goto cleanup;
            }
            ace_info->mod_ace_handle = SWITCH_API_INVALID_HANDLE;
          }
        } break;
        default:
          break;
      }
    }
  }
  temp_ace_info->action_params = *action_params;
  SWITCH_MEMCPY(&temp_ace_info->opt_action_params,
                opt_action_params,
                sizeof(switch_acl_opt_action_params_t));

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  if (reprogram == false) {
    if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
      status = switch_acl_hw_action_update(
          device, acl_info, ace_handle, temp_ace_info, NULL);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl rule action update failed on device %d, for ace handle %lx : "
            "%s\n",
            device,
            ace_handle,
            switch_error_to_string(status));
        SWITCH_FREE(device, temp_ace_info);
        return status;
      }
    } else {
      FOR_EACH_IN_LIST(acl_info->group_list, node) {
        ref_group = (switch_acl_ref_group_t *)node->data;
        status = switch_acl_hw_action_update(
            device, acl_info, ace_handle, temp_ace_info, ref_group);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "acl rule action update failed on device %d, for ace handle %lx "
              ": %s\n",
              device,
              ace_handle,
              switch_error_to_string(status));
          SWITCH_FREE(device, temp_ace_info);
          goto cleanup;
        }
      }
      FOR_EACH_IN_LIST_END();
    }
  } else {
    if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
      status =
          switch_acl_hw_delete(device, acl_info, ace_handle, ace_info, NULL);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl rule set action failed on device %d, for ace handle %lx : "
            "during removal (%s)\n",
            device,
            ace_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      status =
          switch_acl_hw_set(device, acl_info, ace_handle, temp_ace_info, NULL);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl rule set action failed on device %d, for ace handle %lx : "
            "during reprogramming (%s)\n",
            device,
            ace_handle,
            switch_error_to_string(status));
        tmp_status =
            switch_acl_hw_set(device, acl_info, ace_handle, ace_info, NULL);
        if (tmp_status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "acl rule set action failed on device %d, for ace handle %lx : "
              "could not restore hw after reprogramming failure (%s)\n",
              device,
              ace_handle,
              switch_error_to_string(tmp_status));
        }
        goto cleanup;
      }
    } else {
      FOR_EACH_IN_LIST(acl_info->group_list, node) {
        ref_group = (switch_acl_ref_group_t *)node->data;
        status = switch_acl_hw_delete(
            device, acl_info, ace_handle, ace_info, ref_group);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "acl rule set action failed on device %d, for ace handle %lx : "
              "during removal (%s)\n",
              device,
              ace_handle,
              switch_error_to_string(status));
          goto cleanup;
        }

        status = switch_acl_hw_set(
            device, acl_info, ace_handle, temp_ace_info, ref_group);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "acl rule set action failed on device %d, for ace handle %lx : "
              "during reprogramming (%s)\n",
              device,
              ace_handle,
              switch_error_to_string(status));
          tmp_status = switch_acl_hw_set(
              device, acl_info, ace_handle, ace_info, ref_group);
          if (tmp_status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "acl rule set action failed on device %d, for ace handle %lx : "
                "could not restore hw after reprogramming failure (%s)\n",
                device,
                ace_handle,
                switch_error_to_string(tmp_status));
          }
          goto cleanup;
        }
      }
      FOR_EACH_IN_LIST_END();
    }
    SWITCH_FREE(device, ace_info->fields);
  }

  SWITCH_MEMCPY(ace_info, temp_ace_info, sizeof(switch_ace_info_t));
  if (action != SWITCH_ACL_ACTION_NOP) {
    ace_info->action = temp_ace_info->action;
  }
  SWITCH_FREE(device, temp_ace_info);
  return status;

cleanup:
  if (temp_ace_info && reprogram != false) {
    if (temp_ace_info->mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
      tmp_status = switch_api_acl_rule_delete(
          device, acl_handle, ace_info->mod_ace_handle);
      SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
    }
    if (temp_ace_info->fields) {
      SWITCH_FREE(device, temp_ace_info->fields);
    }
  }

  if (temp_ace_info) {
    SWITCH_FREE(device, temp_ace_info);
  }
  return status;
}

switch_status_t switch_api_acl_entry_action_get_internal(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_acl_action_t *action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params) {
  switch_ace_info_t *ace_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));

  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule action get failed on device %d, for handle %lx : %s\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  *action = ace_info->action;
  *action_params = ace_info->action_params;
  SWITCH_MEMCPY(opt_action_params,
                &ace_info->opt_action_params,
                sizeof(switch_acl_opt_action_params_t));
  return status;
}

switch_status_t switch_api_acl_entry_rules_count_get_internal(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_uint16_t *rules_count) {
  switch_ace_info_t *ace_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));

  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule count get failed on device %d, for handle %lx : %s\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }
  *rules_count = ace_info->field_count;
  return status;
}

switch_status_t switch_api_acl_entry_rules_get_internal(
    switch_device_t device, switch_handle_t ace_handle, void *kvp) {
  switch_ace_info_t *ace_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_acl_type_t acl_type;

  SWITCH_LOG_ENTER();
  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));

  if (!kvp) {
    SWITCH_LOG_ERROR(
        "acl rules get failed on device %d, for ace_handle %lx: null kvp : "
        "%s\n",
        device,
        ace_handle,
        switch_error_to_string(SWITCH_STATUS_INVALID_PARAMETER));
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rules get failed on device %d, for ace handle %lx : %s\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_acl_type_get(device, ace_info->acl_handle, &acl_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rules get failed for ace handle, on device %d: acl type get "
        "failed %lx: %s \n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_ip_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_MAC:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_mac_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IPV6:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_ipv6_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_SYSTEM:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_system_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IP_RACL:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_ip_racl_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IPV6_RACL:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_ipv6_racl_key_value_pair_t));
      break;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_ip_mirror_acl_key_value_pair_t));
      break;
    case SWITCH_ACL_TYPE_ECN:
      SWITCH_MEMCPY(
          kvp,
          ace_info->fields,
          ace_info->field_count * sizeof(switch_acl_ecn_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_ipv6_mirror_acl_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_MAC_QOS:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_mac_qos_acl_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IP_QOS:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_ip_qos_acl_key_value_pair_t));
      break;

    case SWITCH_ACL_TYPE_IPV6_QOS:
      SWITCH_MEMCPY(kvp,
                    ace_info->fields,
                    ace_info->field_count *
                        sizeof(switch_acl_ipv6_qos_acl_key_value_pair_t));
      break;

    default:
      break;
  }
  return status;
}

switch_status_t switch_api_acl_l4_port_delete(switch_device_t device,
                                              switch_acl_type_t acl_type,
                                              void *acl_kvp,
                                              switch_uint32_t acl_kvp_count) {
  switch_uint32_t index = 0;
  switch_acl_ip_key_value_pair_t *ip_acl = NULL;
  switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl = NULL;
  switch_acl_ipv6_key_value_pair_t *ipv6_acl = NULL;
  switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_mirror_acl = NULL;

  switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  for (index = 0; index < acl_kvp_count; index++) {
    range_handle = SWITCH_API_INVALID_HANDLE;
    if (acl_type == SWITCH_ACL_TYPE_IP) {
      ip_acl = (switch_acl_ip_key_value_pair_t *)acl_kvp;
      if (ip_acl[index].field == SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT) {
        range_handle = ip_acl[index].value.sport_range_handle;
      }
      if (ip_acl[index].field == SWITCH_ACL_IP_FIELD_L4_DEST_PORT) {
        range_handle = ip_acl[index].value.dport_range_handle;
      }
    } else if (acl_type == SWITCH_ACL_TYPE_IPV6) {
      ipv6_acl = (switch_acl_ipv6_key_value_pair_t *)acl_kvp;
      if (ipv6_acl[index].field == SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT) {
        range_handle = ipv6_acl[index].value.sport_range_handle;
      }
      if (ipv6_acl[index].field == SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT) {
        range_handle = ipv6_acl[index].value.dport_range_handle;
      }
    } else if (acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL) {
      ip_mirror_acl = (switch_acl_ip_mirror_acl_key_value_pair_t *)acl_kvp;
      if (ip_mirror_acl[index].field ==
          SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT) {
        range_handle = ip_mirror_acl[index].value.sport_range_handle;
      }
      if (ip_mirror_acl[index].field ==
          SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT) {
        range_handle = ip_mirror_acl[index].value.dport_range_handle;
      }
    } else {
      ipv6_mirror_acl = (switch_acl_ipv6_mirror_acl_key_value_pair_t *)acl_kvp;
      if (ipv6_mirror_acl[index].field ==
          SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT) {
        range_handle = ipv6_mirror_acl[index].value.sport_range_handle;
      }
      if (ipv6_mirror_acl[index].field ==
          SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT) {
        range_handle = ipv6_mirror_acl[index].value.dport_range_handle;
      }
    }
    if (range_handle != SWITCH_API_INVALID_HANDLE) {
      status = switch_api_acl_range_delete(device, range_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("Failed to remove range handle 0x%lx on device %d: %s",
                         range_handle,
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }
  }
  return status;
}

/**
 * \brief switch_api_acl_rule_delete:
 * Delete a acl rule (filter)
 *
 * This function deletes an access control entry
 *
 * \param device Device number
 * \param acl_handle Acl list handle
 * \param ace_handle Ace entry handle
 * \return switch_status_t status of acl rule delete
 */
switch_status_t switch_api_acl_rule_delete_internal(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_handle_t ace_handle) {
  switch_acl_info_t *acl_info = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));
  /*
   * Check for valid acl handle
   */
  if (!SWITCH_ACE_HANDLE(ace_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "acl rule delete failed on device %d ace handle 0x%lx "
        "ace handle invalid:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule delete failed on device %d ace handle 0x%lx "
        "ace handle invalid:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Get the acl handle from ace if acl handle is invalid
   */
  if (acl_handle == SWITCH_API_INVALID_HANDLE) {
    acl_handle = ace_info->acl_handle;
  }

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule delete failed on device %d ace handle 0x%lx "
        "acl handle 0x%lx: "
        "ace handle invalid:(%s)\n",
        device,
        ace_handle,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  if ((acl_info->type == SWITCH_ACL_TYPE_IP) ||
      (acl_info->type == SWITCH_ACL_TYPE_IPV6) ||
      (acl_info->type == SWITCH_ACL_TYPE_IP_MIRROR_ACL ||
       (acl_info->type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL))) {
    status = switch_api_acl_l4_port_delete(
        device, acl_info->type, ace_info->fields, ace_info->field_count);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl rule delete failed on device %d, ace_handle 0x%lx"
          "L4 port range handle delete failed: %s",
          device,
          ace_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  /*
   * There can be several groups that
   * will reference this access control entry. Delete them
   * all when the ace rule is deleted.
   */
  if (acl_info->bp_type == SWITCH_HANDLE_TYPE_NONE) {
    status = switch_acl_hw_delete(device, acl_info, ace_handle, ace_info, NULL);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl rule delete failed on device %d ace handle 0x%lx "
          "ace hw delete invalid:(%s)\n",
          device,
          ace_handle,
          switch_error_to_string(status));
    }
  } else {
    FOR_EACH_IN_LIST(acl_info->group_list, node) {
      ref_group = (switch_acl_ref_group_t *)node->data;
      status = switch_acl_hw_delete(
          device, acl_info, ace_handle, ace_info, ref_group);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl rule delete failed on device %d ace handle 0x%lx "
            "ace hw delete invalid:(%s)\n",
            device,
            ace_handle,
            switch_error_to_string(status));
      }
    }
    FOR_EACH_IN_LIST_END();
  }

  status = SWITCH_ARRAY_DELETE(&acl_info->rules, ace_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl rule delete failed on device %d ace handle 0x%lx "
        "acl rule array deleted failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  if (ace_info->mod_ace_handle != SWITCH_API_INVALID_HANDLE) {
    status = switch_api_acl_rule_delete(
        device, acl_handle, ace_info->mod_ace_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl rule delete failed on device %d ace handle 0x%lx "
          "mod rule delete failed with mod ace handle 0x%lx :(%s)\n",
          device,
          ace_handle,
          ace_info->mod_ace_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "acl rule removed successfully for device %d "
      "acl handle %lx: ace handle %lx\n",
      device,
      acl_handle,
      ace_handle);

  if (ace_info->field_count) {
    char buffer[SWITCH_LOG_BUFFER_SIZE];
    status = switch_acl_print_kvp(acl_info->type,
                                  ace_info->fields,
                                  ace_info->field_count,
                                  buffer,
                                  SWITCH_LOG_BUFFER_SIZE);

    SWITCH_LOG_DETAIL(
        "acl rule removed successfully for"
        "device %d acl handle %lx"
        "ace handle %lx rule %s"
        "priority %x action %s\n",
        device,
        acl_handle,
        ace_handle,
        buffer,
        ace_info->priority,
        switch_acl_action_to_string(ace_info->action));

    SWITCH_FREE(device, ace_info->fields);
  }

  status = switch_ace_handle_delete(device, ace_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_drop_stats_get_internal(switch_device_t device,
                                                   switch_int32_t *num_counters,
                                                   switch_uint64_t **counters) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *num_counters = 256;
  *counters = SWITCH_MALLOC(device, sizeof(switch_uint64_t), (*num_counters));

  if (!(*counters)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("acl drop stats get failed on device %s\n",
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(*counters, 0, sizeof(switch_uint64_t) * (*num_counters));

  status = switch_pd_drop_stats_get(device, *num_counters, *counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl drop stats get failed on device %s\n",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_acl_counter_create_internal(
    switch_device_t device, switch_handle_t *counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(counter_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter create failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_allocate(
      device, acl_ctx->counter_index, &counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter create failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *counter_handle = id_to_handle(SWITCH_HANDLE_TYPE_ACL_COUNTER, counter_id);
  return status;
}

switch_status_t switch_api_racl_counter_create_internal(
    switch_device_t device, switch_handle_t *counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(counter_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "racl counter create : api context get failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_allocate(
      device, acl_ctx->counter_index, &counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "racl counter create : api id allocator failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *counter_handle = id_to_handle(SWITCH_HANDLE_TYPE_RACL_COUNTER, counter_id);
  return status;
}

switch_status_t switch_api_egress_acl_counter_create_internal(
    switch_device_t device, switch_handle_t *counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(counter_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress acl counter create : api context get failed for device %d: "
        "%s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_allocate(
      device, acl_ctx->counter_index, &counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress acl counter create : api id allocator failed for device %d: "
        "%s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *counter_handle =
      id_to_handle(SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER, counter_id);
  return status;
}

switch_status_t switch_api_acl_counter_delete_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  counter_id = handle_to_id(counter_handle);
  status = switch_api_id_allocator_release(
      device, acl_ctx->counter_index, counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_racl_counter_delete_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("racl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_RACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("racl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  counter_id = handle_to_id(counter_handle);
  status = switch_api_id_allocator_release(
      device, acl_ctx->counter_index, counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("racl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_egress_acl_counter_delete_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_id_t counter_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("egress acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("egress acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  counter_id = handle_to_id(counter_handle);
  status = switch_api_id_allocator_release(
      device, acl_ctx->counter_index, counter_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter delete failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_acl_counter_clear_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter clear failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ACL_COUNTER_HANDLE(counter_handle));
  if (!SWITCH_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("acl counter clear failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_acl_stats_clear(device, handle_to_id(counter_handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl counter clear failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_acl_counter_get_internal(
    switch_device_t device,
    switch_handle_t counter_handle,
    switch_counter_t *counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!counter) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("acl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_acl_stats_get(device, handle_to_id(counter_handle), counter);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_racl_counter_clear_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "racl counter clear : api context get failed for handle %lx: %s\n",
        counter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RACL_COUNTER_HANDLE(counter_handle));
  if (!SWITCH_RACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("acl counter clear failed : invalid handle type %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_racl_stats_clear(device, handle_to_id(counter_handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("racl counter clear failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_racl_counter_get_internal(
    switch_device_t device,
    switch_handle_t counter_handle,
    switch_counter_t *counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!counter) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("racl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_RACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("racl stats get failed : invalid handle type %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_racl_stats_get(device, handle_to_id(counter_handle), counter);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("racl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_egress_acl_counter_clear_internal(
    switch_device_t device, switch_handle_t counter_handle) {
  switch_acl_context_t *acl_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_ACL, (void **)&acl_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress acl counter clear : api context get failed for handle %lx: "
        "%s\n",
        counter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle));
  if (!SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "egress acl counter clear failed : invalid handle type %lx: %s\n",
        counter_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_egress_acl_stats_clear(device, handle_to_id(counter_handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("egress acl counter clear failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_egress_acl_counter_get_internal(
    switch_device_t device,
    switch_handle_t counter_handle,
    switch_counter_t *counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!counter) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("egress acl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle));
  /*
   * Check for valid acl counter handle
   */
  if (!SWITCH_EGRESS_ACL_COUNTER_HANDLE(counter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "egress acl stats get failed : invalid handle type %lx: %s\n",
        counter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_acl_stats_get(
      device, handle_to_id(counter_handle), counter);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("egress_acl stats get failed for handle %lx: %s\n",
                     counter_handle,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_acl_range_create_internal(
    switch_device_t device,
    switch_direction_t direction,
    switch_range_type_t range_type,
    switch_range_t *range,
    switch_handle_t *range_handle) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t prev_handle;
  switch_handle_type_t type;

  SWITCH_ASSERT(range != NULL && range_handle != NULL);
  if (!range || !range_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *range_handle = SWITCH_API_INVALID_HANDLE;
  do {
    prev_handle = *range_handle;
    status = switch_api_handle_iterate(
        device, SWITCH_HANDLE_TYPE_RANGE, prev_handle, range_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl range create failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    if (*range_handle != SWITCH_API_INVALID_HANDLE) {
      status = switch_range_get(device, *range_handle, &range_info);
      type = switch_handle_type_get(*range_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "acl range create failed on device %d, %s handle get failed: %s\n",
            device,
            switch_handle_type_to_string(type),
            switch_error_to_string(status));
        return status;
      }
      if (range_info->range_type == range_type &&
          range_info->direction == direction &&
          range->start_value == range_info->range.start_value &&
          range->end_value == range_info->range.end_value) {
        range_info->ref_count++;
        return status;
      }
    }
  } while (*range_handle != SWITCH_API_INVALID_HANDLE);

  *range_handle = switch_range_handle_create(device);
  if (*range_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("acl range create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, *range_handle, &range_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (range_type == SWITCH_RANGE_TYPE_VLAN ||
      range_type == SWITCH_RANGE_TYPE_PACKET_LENGTH) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&range_info->range, range, sizeof(switch_range_t));

  range_info->range_type = range_type;
  range_info->direction = direction;
  range_info->ref_count++;

  if (direction == SWITCH_API_DIRECTION_BOTH ||
      direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_range_entry_add(device,
                                       SWITCH_API_DIRECTION_INGRESS,
                                       handle_to_id(*range_handle),
                                       range_type,
                                       range,
                                       &range_info->hw_entry[0]);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("acl range create failed on device %d: for dir %s: %s\n",
                       device,
                       "Ingress",
                       switch_error_to_string(status));
      return status;
    }
  }

  if (direction == SWITCH_API_DIRECTION_BOTH ||
      direction == SWITCH_API_DIRECTION_EGRESS) {
    status = switch_pd_range_entry_add(device,
                                       SWITCH_API_DIRECTION_EGRESS,
                                       handle_to_id(*range_handle),
                                       range_type,
                                       range,
                                       &range_info->hw_entry[1]);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("acl range create failed on device %d: for dir %s: %s\n",
                       device,
                       "Egress",
                       switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_acl_range_update_internal(
    switch_device_t device,
    switch_handle_t range_handle,
    switch_range_t *range) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RANGE_HANDLE(range_handle));
  if (!SWITCH_RANGE_HANDLE(range_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, range_handle, &range_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl range update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&range_info->range, range, sizeof(switch_range_t));

  if (range_info->direction == SWITCH_API_DIRECTION_INGRESS ||
      range_info->direction == SWITCH_API_DIRECTION_BOTH) {
    status = switch_pd_range_entry_update(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          handle_to_id(range_handle),
                                          range_info->range_type,
                                          range,
                                          range_info->hw_entry[0]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl range update failed on device %d: for direction %s: %s\n",
          device,
          "Ingress",
          switch_error_to_string(status));
      return status;
    }
  }

  if (range_info->direction == SWITCH_API_DIRECTION_EGRESS ||
      range_info->direction == SWITCH_API_DIRECTION_BOTH) {
    status = switch_pd_range_entry_update(device,
                                          SWITCH_API_DIRECTION_EGRESS,
                                          handle_to_id(range_handle),
                                          range_info->range_type,
                                          range,
                                          range_info->hw_entry[1]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "acl range update failed on device %d: for direction %s: %s\n",
          device,
          "Ingress",
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_acl_range_get_internal(switch_device_t device,
                                                  switch_handle_t range_handle,
                                                  switch_range_t *range) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(range != NULL);
  if (!range) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RANGE_HANDLE(range_handle));
  if (!SWITCH_RANGE_HANDLE(range_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, range_handle, &range_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl range get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(range, &range_info->range, sizeof(switch_range_t));

  return status;
}

switch_status_t switch_api_acl_range_delete_internal(
    switch_device_t device, switch_handle_t range_handle) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RANGE_HANDLE(range_handle));
  if (!SWITCH_RANGE_HANDLE(range_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, range_handle, &range_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (--(range_info->ref_count)) {
    SWITCH_LOG_DEBUG(
        "acl range delete skipped on device  %d: handle %lx refcount > 1\n",
        device,
        range_handle);
    return status;
  }
  if (range_info->direction == SWITCH_API_DIRECTION_INGRESS ||
      range_info->direction == SWITCH_API_DIRECTION_BOTH) {
    status = switch_pd_range_entry_delete(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          range_info->range_type,
                                          range_info->hw_entry[0]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  if (range_info->direction == SWITCH_API_DIRECTION_EGRESS ||
      range_info->direction == SWITCH_API_DIRECTION_BOTH) {
    status = switch_pd_range_entry_delete(device,
                                          SWITCH_API_DIRECTION_EGRESS,
                                          range_info->range_type,
                                          range_info->hw_entry[1]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }
  status = switch_range_handle_delete(device, range_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_acl_range_type_get_internal(
    switch_device_t device,
    switch_handle_t range_handle,
    switch_range_type_t *range_type) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RANGE_HANDLE(range_handle));
  if (!SWITCH_RANGE_HANDLE(range_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, range_handle, &range_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("acl range delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *range_type = range_info->range_type;

  return status;
}

switch_status_t switch_api_acl_type_get_internal(switch_device_t device,
                                                 switch_handle_t acl_handle,
                                                 switch_acl_type_t *acl_type) {
  switch_acl_info_t *acl_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  if (!SWITCH_ACL_HANDLE(acl_handle)) {
    SWITCH_LOG_ERROR(
        "acl type get fails on device %d acl handle %lx: "
        "acl handle invalid(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl type get fails on device %d acl handle %lx: "
        "acl get failed(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  *acl_type = acl_info->type;

  return status;
}

switch_status_t switch_api_acl_direction_get_internal(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_direction_t *direction) {
  switch_direction_t dir = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ACL_DIRECTION_GET(device, acl_handle, dir, status);
  *direction = dir;
  return status;
}

switch_status_t switch_api_acl_type_set_internal(switch_device_t device,
                                                 switch_handle_t acl_handle,
                                                 switch_acl_type_t acl_type) {
  switch_acl_info_t *acl_info = NULL;
  switch_uint16_t num_rules = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  if (!SWITCH_ACL_HANDLE(acl_handle)) {
    SWITCH_LOG_ERROR(
        "acl type set fails on device %d acl handle %lx: "
        "acl handle invalid(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl type set fails on device %d acl handle %lx: "
        "acl get failed(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  if (acl_info->type == acl_type) {
    return status;
  }

  num_rules = SWITCH_ARRAY_COUNT(&acl_info->rules);
  if (num_rules != 0) {
    SWITCH_LOG_ERROR(
        "acl type set fails on device %d acl handle %lx: "
        "num rules is non zero(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  acl_info->type = acl_type;

  return status;
}

switch_status_t switch_api_acl_entry_acl_table_get_internal(
    switch_device_t device,
    switch_handle_t acl_entry_handle,
    switch_handle_t *acl_table_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_ace_info_t *ace_info = NULL;

  status = switch_ace_get(device, acl_entry_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ace entry acl table get failed for handle %lx,: ace_info get: %s",
        acl_entry_handle,
        switch_error_to_string(status));
    return status;
  }
  *acl_table_handle = ace_info->acl_handle;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_acl_dereference(switch_device_t device,
                                           switch_handle_t acl_handle,
                                           switch_handle_t handle) {
  SWITCH_MT_WRAP(
      switch_api_acl_dereference_internal(device, acl_handle, handle))
}

switch_status_t switch_api_ingress_acl_dereference(switch_device_t device,
                                                   switch_handle_t acl_handle,
                                                   switch_handle_t handle) {
  SWITCH_MT_WRAP(
      switch_api_ingress_acl_dereference_internal(device, acl_handle, handle))
}

switch_status_t switch_api_egress_acl_dereference(switch_device_t device,
                                                  switch_handle_t acl_handle,
                                                  switch_handle_t handle) {
  SWITCH_MT_WRAP(
      switch_api_egress_acl_dereference_internal(device, acl_handle, handle))
}

switch_status_t switch_api_acl_range_delete(switch_device_t device,
                                            switch_handle_t range_handle) {
  SWITCH_MT_WRAP(switch_api_acl_range_delete_internal(device, range_handle))
}

switch_status_t switch_api_acl_list_create(switch_device_t device,
                                           switch_direction_t direction,
                                           switch_acl_type_t type,
                                           switch_handle_type_t bp_type,
                                           switch_handle_t *acl_handle) {
  SWITCH_MT_WRAP(switch_api_acl_list_create_internal(
      device, direction, type, bp_type, acl_handle))
}

switch_status_t switch_api_acl_range_create(switch_device_t device,
                                            switch_direction_t direction,
                                            switch_range_type_t range_type,
                                            switch_range_t *range,
                                            switch_handle_t *range_handle) {
  SWITCH_MT_WRAP(switch_api_acl_range_create_internal(
      device, direction, range_type, range, range_handle))
}

switch_status_t switch_api_acl_group_member_delete(
    switch_device_t device, switch_handle_t acl_group_member_handle) {
  SWITCH_MT_WRAP(switch_api_acl_group_member_delete_internal(
      device, acl_group_member_handle))
}

switch_status_t switch_api_acl_range_get(switch_device_t device,
                                         switch_handle_t range_handle,
                                         switch_range_t *range) {
  SWITCH_MT_WRAP(switch_api_acl_range_get_internal(device, range_handle, range))
}

switch_status_t switch_api_acl_counter_create(switch_device_t device,
                                              switch_handle_t *counter_handle) {
  SWITCH_MT_WRAP(switch_api_acl_counter_create_internal(device, counter_handle))
}

switch_status_t switch_api_racl_counter_create(
    switch_device_t device, switch_handle_t *counter_handle) {
  SWITCH_MT_WRAP(
      switch_api_racl_counter_create_internal(device, counter_handle))
}

switch_status_t switch_api_egress_acl_counter_create(
    switch_device_t device, switch_handle_t *counter_handle) {
  SWITCH_MT_WRAP(
      switch_api_egress_acl_counter_create_internal(device, counter_handle))
}

switch_handle_t switch_api_acl_list_group_create(
    switch_device_t device,
    switch_direction_t direction,
    switch_handle_type_t bp_type,
    switch_handle_t *acl_group_handle) {
  SWITCH_MT_WRAP(switch_api_acl_list_group_create_internal(
      device, direction, bp_type, acl_group_handle))
}

switch_status_t switch_api_acl_counter_get(switch_device_t device,
                                           switch_handle_t counter_handle,
                                           switch_counter_t *counter) {
  SWITCH_MT_WRAP(
      switch_api_acl_counter_get_internal(device, counter_handle, counter))
}

switch_status_t switch_api_racl_counter_get(switch_device_t device,
                                            switch_handle_t counter_handle,
                                            switch_counter_t *counter) {
  SWITCH_MT_WRAP(
      switch_api_racl_counter_get_internal(device, counter_handle, counter))
}

switch_status_t switch_api_egress_acl_counter_get(
    switch_device_t device,
    switch_handle_t counter_handle,
    switch_counter_t *counter) {
  SWITCH_MT_WRAP(switch_api_egress_acl_counter_get_internal(
      device, counter_handle, counter))
}

switch_status_t switch_api_acl_counter_delete(switch_device_t device,
                                              switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(switch_api_acl_counter_delete_internal(device, counter_handle))
}

switch_status_t switch_api_racl_counter_delete(switch_device_t device,
                                               switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(
      switch_api_racl_counter_delete_internal(device, counter_handle))
}

switch_status_t switch_api_egress_acl_counter_delete(
    switch_device_t device, switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(
      switch_api_egress_acl_counter_delete_internal(device, counter_handle))
}

switch_status_t switch_api_acl_range_update(switch_device_t device,
                                            switch_handle_t range_handle,
                                            switch_range_t *range) {
  SWITCH_MT_WRAP(
      switch_api_acl_range_update_internal(device, range_handle, range))
}

switch_status_t switch_api_acl_rule_create(
    switch_device_t device,
    switch_handle_t acl_handle,
    unsigned int priority,
    unsigned int key_value_count,
    void *acl_kvp,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_handle_t *ace_handle) {
  SWITCH_MT_WRAP(switch_api_acl_rule_create_internal(device,
                                                     acl_handle,
                                                     priority,
                                                     key_value_count,
                                                     acl_kvp,
                                                     action,
                                                     action_params,
                                                     opt_action_params,
                                                     ace_handle))
}

switch_handle_t switch_api_acl_group_member_create(
    switch_device_t device,
    switch_handle_t acl_group_handle,
    switch_handle_t acl_handle,
    switch_handle_t *acl_group_member_handle) {
  SWITCH_MT_WRAP(switch_api_acl_group_member_create_internal(
      device, acl_group_handle, acl_handle, acl_group_member_handle))
}

switch_status_t switch_api_acl_range_type_get(switch_device_t device,
                                              switch_handle_t range_handle,
                                              switch_range_type_t *range_type) {
  SWITCH_MT_WRAP(
      switch_api_acl_range_type_get_internal(device, range_handle, range_type))
}

switch_status_t switch_api_acl_counter_clear(switch_device_t device,
                                             switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(switch_api_acl_counter_clear_internal(device, counter_handle))
}

switch_status_t switch_api_racl_counter_clear(switch_device_t device,
                                              switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(switch_api_racl_counter_clear_internal(device, counter_handle))
}

switch_status_t switch_api_egress_acl_counter_clear(
    switch_device_t device, switch_handle_t counter_handle) {
  SWITCH_MT_WRAP(
      switch_api_egress_acl_counter_clear_internal(device, counter_handle))
}

switch_status_t switch_api_acl_list_group_delete(
    switch_device_t device, switch_handle_t acl_group_handle) {
  SWITCH_MT_WRAP(
      switch_api_acl_list_group_delete_internal(device, acl_group_handle))
}

switch_status_t switch_api_acl_rule_delete(switch_device_t device,
                                           switch_handle_t acl_handle,
                                           switch_handle_t ace_handle) {
  SWITCH_MT_WRAP(
      switch_api_acl_rule_delete_internal(device, acl_handle, ace_handle))
}

switch_status_t switch_api_acl_list_delete(switch_device_t device,
                                           switch_handle_t acl_handle) {
  SWITCH_MT_WRAP(switch_api_acl_list_delete_internal(device, acl_handle))
}

switch_status_t switch_api_drop_stats_get(switch_device_t device,
                                          int *num_counters,
                                          switch_uint64_t **counters) {
  SWITCH_MT_WRAP(
      switch_api_drop_stats_get_internal(device, num_counters, counters))
}

switch_status_t switch_api_acl_type_get(switch_device_t device,
                                        switch_handle_t acl_handle,
                                        switch_acl_type_t *acl_type) {
  SWITCH_MT_WRAP(switch_api_acl_type_get_internal(device, acl_handle, acl_type))
}

switch_status_t switch_api_acl_type_set(switch_device_t device,
                                        switch_handle_t acl_handle,
                                        switch_acl_type_t acl_type) {
  SWITCH_MT_WRAP(switch_api_acl_type_set_internal(device, acl_handle, acl_type))
}

switch_status_t switch_api_acl_reference(switch_device_t device,
                                         switch_handle_t acl_handle,
                                         switch_handle_t interface_handle) {
  SWITCH_MT_WRAP(
      switch_api_acl_reference_internal(device, acl_handle, interface_handle))
}

switch_status_t switch_api_ingress_acl_reference(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_handle_t interface_handle) {
  SWITCH_MT_WRAP(switch_api_ingress_acl_reference_internal(
      device, acl_handle, interface_handle))
}

switch_status_t switch_api_egress_acl_reference(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_handle_t interface_handle) {
  SWITCH_MT_WRAP(switch_api_egress_acl_reference_internal(
      device, acl_handle, interface_handle))
}

switch_status_t switch_api_acl_entry_action_set(
    switch_device_t device,
    switch_handle_t ace_handle,
    unsigned int priority,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params) {
  SWITCH_MT_WRAP(switch_api_acl_entry_action_set_internal(
      device, ace_handle, priority, action, action_params, opt_action_params))
}

switch_status_t switch_api_acl_entry_acl_table_get(
    switch_device_t device,
    switch_handle_t acl_entry_handle,
    switch_handle_t *acl_table_handle) {
  SWITCH_MT_WRAP(switch_api_acl_entry_acl_table_get_internal(
      device, acl_entry_handle, acl_table_handle))
}

switch_status_t switch_api_acl_entry_action_get(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_acl_action_t *action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params) {
  SWITCH_MT_WRAP(switch_api_acl_entry_action_get_internal(
      device, ace_handle, action, action_params, opt_action_params))
}

switch_status_t switch_api_acl_entry_rules_count_get(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_uint16_t *rules_count) {
  SWITCH_MT_WRAP(switch_api_acl_entry_rules_count_get_internal(
      device, ace_handle, rules_count))
}

switch_status_t switch_api_acl_entry_rules_get(switch_device_t device,
                                               switch_handle_t ace_handle,
                                               void *kvp) {
  SWITCH_MT_WRAP(
      switch_api_acl_entry_rules_get_internal(device, ace_handle, kvp))
}

switch_status_t switch_api_acl_direction_get(switch_device_t device,
                                             switch_handle_t acl_handle,
                                             switch_direction_t *direction) {
  SWITCH_MT_WRAP(
      switch_api_acl_direction_get_internal(device, acl_handle, direction))
}
