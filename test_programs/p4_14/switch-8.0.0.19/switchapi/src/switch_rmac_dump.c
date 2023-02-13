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

#include "switchapi/switch_rmac.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_RMAC

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_rmac_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t rmac_handle,
    const void *cli_ctx) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rmac dump failed on device %d "
        "rmac handle 0x%lx: parameters invalid(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac dump failed on device %d "
        "rmac handle 0x%lx: rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\trmac handle: 0x%lx\n", rmac_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\trmac type: %s\n\n",
               switch_rmac_type_to_string(rmac_info->rmac_type));

  SWITCH_PRINT(
      cli_ctx, "\t\trmac members: %d\n", rmac_info->rmac_list.num_entries);
  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t *)node->data;
    SWITCH_PRINT(cli_ctx,
                 "\t\tmac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 rmac_entry->mac.mac_addr[0],
                 rmac_entry->mac.mac_addr[1],
                 rmac_entry->mac.mac_addr[2],
                 rmac_entry->mac.mac_addr[3],
                 rmac_entry->mac.mac_addr[4],
                 rmac_entry->mac.mac_addr[5]);
    SWITCH_PRINT(cli_ctx, "\t\tsmac index: %d\n", rmac_entry->smac_index);
    SWITCH_PRINT(
        cli_ctx, "\t\ttunnel smac index: %d\n", rmac_entry->tunnel_smac_index);
    SWITCH_PRINT(
        cli_ctx, "\t\touter rmac entry: 0x%lx\n", rmac_entry->outer_rmac_entry);
    SWITCH_PRINT(
        cli_ctx, "\t\tinner rmac entry: 0x%lx\n", rmac_entry->inner_rmac_entry);
    SWITCH_PRINT(cli_ctx, "\t\thw flags: 0x%lx\n", rmac_entry->hw_flags);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DEBUG(
      "rmac handle dump on device %d rmac handle 0x%lx\n", device, rmac_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_rmac_handle_dump(const switch_device_t device,
                                            const switch_handle_t rmac_handle,
                                            const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_rmac_handle_dump_internal(device, rmac_handle, cli_ctx))
}
