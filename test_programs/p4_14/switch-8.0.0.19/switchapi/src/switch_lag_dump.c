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

#include "switchapi/switch_lag.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_LAG

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_lag_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t lag_handle,
    const void *cli_ctx) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "lag dump failed on device %d "
        "lag handle %lx: parameters invalid(%s)\n",
        device,
        lag_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag dump failed on device %d "
        "lag handle %lx: lag get failed(%s)\n",
        device,
        lag_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\tlag handle: 0x%lx\n", lag_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tport_lag_index 0x%x\n", lag_info->port_lag_index);
  SWITCH_PRINT(cli_ctx, "\t\t\tyid %d\n", lag_info->yid);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tingress acl group handle 0x%lx\n",
               lag_info->ingress_acl_group_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tegress acl group handle 0x%lx\n",
               lag_info->egress_acl_group_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\thw entry 0x%x\n", lag_info->hw_entry);
  SWITCH_PRINT(cli_ctx, "\t\t\tpd group entry 0x%x\n", lag_info->pd_group_hdl);

  SWITCH_PRINT(cli_ctx, "\t\t\tmembers: %d\n", lag_info->members.num_entries);
  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    SWITCH_PRINT(
        cli_ctx, "\t\t\t\tport handle 0x%lx\n", lag_member->port_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\t\tmember handle 0x%lx\n",
                 lag_member->lag_member_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\t\thw entry 0x%lx\n", lag_member->mbr_hdl);
    SWITCH_PRINT(cli_ctx, "\n");
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_PRINT(cli_ctx, "\n");

  SWITCH_LOG_DEBUG(
      "lag handle dump on device %d lag handle %lx\n", device, lag_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_lag_member_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t lag_member_handle,
    const void *cli_ctx) {
  switch_lag_member_t *lag_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_MEMBER_HANDLE(lag_member_handle));
  if (!SWITCH_LAG_MEMBER_HANDLE(lag_member_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "lag member dump failed on device %d "
        "lag member handle 0x%lx: parameters invalid(%s)\n",
        device,
        lag_member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_lag_member_get(device, lag_member_handle, &lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag member dump failed on device %d "
        "lag member handle 0x%lx: lag member get failed(%s)\n",
        device,
        lag_member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\tlag member handle: 0x%lx\n", lag_member_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\t\tport handle: 0x%lx\n", lag_member->port_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tlag handle: 0x%lx\n", lag_member->lag_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdirection: %d\n", lag_member->direction);
  SWITCH_PRINT(cli_ctx, "\t\t\tpd handle: 0x%lx\n", lag_member->mbr_hdl);
  SWITCH_PRINT(cli_ctx, "\t\t\tactive: %d\n", lag_member->active);

  SWITCH_PRINT(cli_ctx, "\n");

  SWITCH_LOG_DEBUG(
      "lag member handle dump on device %d lag member handle 0x%lx\n",
      device,
      lag_member_handle);

  SWITCH_LOG_EXIT();

  return status;
}
switch_status_t switch_api_lag_handle_dump(const switch_device_t device,
                                           const switch_handle_t lag_handle,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_lag_handle_dump_internal(device, lag_handle, cli_ctx));
}

switch_status_t switch_api_lag_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t lag_member_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_lag_member_handle_dump_internal(
      device, lag_member_handle, cli_ctx));
}

#ifdef __cplusplus
}
#endif
