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

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_ACL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_acl_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t acl_handle,
    const void *cli_ctx) {
  switch_acl_info_t *acl_info = NULL;
  switch_node_t *node = NULL;
  switch_ace_info_t *ace_info = NULL;
  switch_acl_ref_group_t *ref_group = NULL;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_HANDLE(acl_handle));
  if (!SWITCH_ACL_HANDLE(acl_handle)) {
    SWITCH_LOG_ERROR(
        "acl handle dump failed on device %d acl handle %lx: "
        "acl handle invalid:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl handle dump failed on device %d acl handle %lx: "
        "acl get failed:(%s)\n",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\t\tacl handle: 0x%lx\n", acl_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tdirection: %s\n",
               acl_info->direction == SWITCH_API_DIRECTION_INGRESS ? "ingress"
                                                                   : "egress");
  SWITCH_PRINT(cli_ctx,
               "\t\t\tacl type: %s\n",
               switch_acl_type_to_string(acl_info->type));
  SWITCH_PRINT(cli_ctx,
               "\t\t\tbindpoint type: %s\n",
               switch_acl_bp_type_to_string(acl_info->bp_type));
  SWITCH_PRINT(
      cli_ctx, "\t\t\tdefault group: 0x%lx\n", acl_info->default_group);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tdefault group member: 0x%lx\n",
               acl_info->default_group_member);

  SWITCH_PRINT(cli_ctx,
               "\n\t\t\tacl groups: %d\n",
               SWITCH_LIST_COUNT(&acl_info->group_list));
  FOR_EACH_IN_LIST(acl_info->group_list, node) {
    ref_group = (switch_acl_ref_group_t *)node->data;
    SWITCH_PRINT(cli_ctx, "\t\t\tgroup: 0x%lx\n", ref_group->acl_group_handle);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_PRINT(cli_ctx,
               "\n\t\t\tace handles: %d\n",
               SWITCH_ARRAY_COUNT(&acl_info->rules));
  FOR_EACH_IN_ARRAY(ace_handle, acl_info->rules, switch_ace_info_t, ace_info) {
    UNUSED(ace_info);
    SWITCH_PRINT(cli_ctx, "\t\t\tace handle: 0x%lx\n", ace_handle);
  }
  FOR_EACH_IN_ARRAY_END();

  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

switch_status_t switch_api_ace_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t ace_handle,
    const void *cli_ctx) {
  switch_ace_info_t *ace_info = NULL;
  switch_acl_info_t *acl_info = NULL;
  char buffer[SWITCH_LOG_BUFFER_SIZE];
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACE_HANDLE(ace_handle));
  if (!SWITCH_ACE_HANDLE(ace_handle)) {
    SWITCH_LOG_ERROR(
        "ace handle dump failed on device %d acl handle %lx: "
        "ace handle invalid:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ace_get(device, ace_handle, &ace_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ace handle dump failed on device %d ace handle %lx: "
        "ace get failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_get(device, ace_info->acl_handle, &acl_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ace handle dump failed on device %d ace handle %lx: "
        "acl get failed:(%s)\n",
        device,
        ace_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\t\tace handle: 0x%lx\n", ace_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);

  SWITCH_PRINT(cli_ctx, "\t\t\tacl handle: 0x%lx\n", ace_info->acl_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tacl action: %s\n",
               switch_acl_action_to_string(ace_info->action));

  if (ace_info->field_count) {
    status = switch_acl_print_kvp(acl_info->type,
                                  ace_info->fields,
                                  ace_info->field_count,
                                  buffer,
                                  SWITCH_LOG_BUFFER_SIZE);
    SWITCH_PRINT(cli_ctx, "\n%s\n", buffer);
  }

  SWITCH_PRINT(cli_ctx, "\n\t\t\taction parameters\n");
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tredirect handle: 0x%lx\n",
               ace_info->action_params.redirect.handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\treason code: %s\n",
               switch_hostif_code_to_string(
                   ace_info->action_params.cpu_redirect.reason_code));
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tdrop reason code: %d\n",
               ace_info->action_params.drop.reason_code);

  SWITCH_PRINT(cli_ctx, "\n\t\t\toptional action parameters\n");
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tcopy to cpu: %d\n",
               ace_info->opt_action_params.copy_to_cpu);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tmirror handle: 0x%lx\n",
               ace_info->opt_action_params.mirror_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tmeter handle: 0x%lx\n",
               ace_info->opt_action_params.meter_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tcounter handle: 0x%lx\n",
               ace_info->opt_action_params.counter_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tswitch id: %d\n",
               ace_info->opt_action_params.switch_id);
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\ttraffic class: %d\n", ace_info->opt_action_params.tc);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tcolor: %s\n",
               switch_color_to_string(ace_info->opt_action_params.tc));
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\tingress cos: %d\n",
               ace_info->opt_action_params.ingress_cos);
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tqueue id: %d\n", ace_info->opt_action_params.queue_id);

  SWITCH_PRINT(cli_ctx, "\n\n");
  return status;
}

switch_status_t switch_api_acl_group_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t acl_group_handle,
    const void *cli_ctx) {
  switch_acl_group_info_t *acl_group_info = NULL;
  switch_acl_handle_t *handle_info = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_GROUP_HANDLE(acl_group_handle));
  if (!SWITCH_ACL_GROUP_HANDLE(acl_group_handle)) {
    SWITCH_LOG_ERROR(
        "acl group handle dump failed on device %d acl group handle %lx: "
        "acl group handle invalid:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_get(device, acl_group_handle, &acl_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group handle dump failed on device %d acl group handle %lx: "
        "acl group get failed:(%s)\n",
        device,
        acl_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\t\tacl group handle: 0x%lx\n", acl_group_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tdirection: %s\n",
               acl_group_info->direction == SWITCH_API_DIRECTION_INGRESS
                   ? "ingress"
                   : "egress");
  SWITCH_PRINT(cli_ctx,
               "\t\t\tbindpoint type: %s\n",
               switch_acl_bp_type_to_string(acl_group_info->bp_type));

  SWITCH_PRINT(cli_ctx,
               "\n\t\t\tbindpoints: %d\n",
               SWITCH_LIST_COUNT(&acl_group_info->handle_list));
  FOR_EACH_IN_LIST(acl_group_info->handle_list, node) {
    handle_info = (switch_acl_handle_t *)node->data;
    SWITCH_PRINT(cli_ctx, "\t\t\tbp handle: 0x%lx\n", handle_info->handle);
  }
  FOR_EACH_IN_LIST_END();
  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

switch_status_t switch_api_acl_group_member_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t acl_group_member_handle,
    const void *cli_ctx) {
  switch_acl_group_member_info_t *acl_group_member_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ACL_GROUP_MEMBER_HANDLE(acl_group_member_handle));
  if (!SWITCH_ACL_GROUP_MEMBER_HANDLE(acl_group_member_handle)) {
    SWITCH_LOG_ERROR(
        "acl group member handle dump failed on device %d "
        "acl group member handle %lx: "
        "acl group member get failed:(%s)\n",
        device,
        acl_group_member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_acl_group_member_get(
      device, acl_group_member_handle, &acl_group_member_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl group member handle dump failed on device %d "
        "acl group member handle %lx: "
        "acl group member get failed:(%s)\n",
        device,
        acl_group_member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx,
               "\n\n\t\tacl group member handle: 0x%lx\n",
               acl_group_member_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tacl group handle: 0x%lx\n",
               acl_group_member_info->acl_group_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tacl handle: 0x%lx\n", acl_group_member_info->acl_handle);
  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

switch_status_t switch_api_acl_range_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t range_handle,
    const void *cli_ctx) {
  switch_range_info_t *range_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RANGE_HANDLE(range_handle));
  if (!SWITCH_RANGE_HANDLE(range_handle)) {
    SWITCH_LOG_ERROR(
        "range handle dump failed on device %d range handle %lx: "
        "range handle invalid:(%s)\n",
        device,
        range_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_range_get(device, range_handle, &range_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "range handle dump failed on device %d range handle %lx: "
        "range handle invalid:(%s)\n",
        device,
        range_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\t\trange handle: 0x%lx\n", range_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\trange type: %s\n",
               switch_range_type_to_string(range_info->range_type));
  SWITCH_PRINT(cli_ctx,
               "\t\t\tdirection: %s\n",
               range_info->direction == SWITCH_API_DIRECTION_INGRESS
                   ? "ingress"
                   : "egress");
  SWITCH_PRINT(cli_ctx, "\t\t\trange min: %d\n", range_info->range.start_value);
  SWITCH_PRINT(cli_ctx, "\t\t\trange max: %d\n", range_info->range.end_value);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tingress hw entry: %d\n", range_info->hw_entry[0]);
  SWITCH_PRINT(cli_ctx, "\t\t\tegress hw entry: %d\n", range_info->hw_entry[1]);
  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

switch_status_t switch_acl_drop_stats_dump(const switch_device_t device,
                                           const void *cli_ctx) {
  switch_int32_t num_counters = 0;
  switch_int32_t index = 0;
  switch_uint64_t *counters = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_api_drop_stats_get(device, &num_counters, &counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "acl drop stats get failed on device %d: "
        "drop stats get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\tACL drop stats:\n");
  for (index = 0; index < num_counters; index++) {
    if (SWITCH_DROP_REASON_VALID(index)) {
      SWITCH_PRINT(cli_ctx,
                   "\t\t\t%s : %ld\n",
                   switch_acl_drop_stats_id_to_string(index),
                   counters[index]);
    }
  }
  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_acl_handle_dump(const switch_device_t device,
                                           const switch_handle_t acl_handle,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_acl_handle_dump_internal(device, acl_handle, cli_ctx))
}

switch_status_t switch_api_ace_handle_dump(const switch_device_t device,
                                           const switch_handle_t ace_handle,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_ace_handle_dump_internal(device, ace_handle, cli_ctx))
}

switch_status_t switch_api_acl_group_handle_dump(
    const switch_device_t device,
    const switch_handle_t acl_group_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_acl_group_handle_dump_internal(
      device, acl_group_handle, cli_ctx))
}

switch_status_t switch_api_acl_group_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t acl_group_member_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_acl_group_member_handle_dump_internal(
      device, acl_group_member_handle, cli_ctx))
}

switch_status_t switch_api_acl_range_handle_dump(
    const switch_device_t device,
    const switch_handle_t range_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_acl_range_handle_dump_internal(device, range_handle, cli_ctx))
}
