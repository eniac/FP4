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

#include "switchapi/switch_stp.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_STP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_stp_handle_dump(const switch_device_t device,
                                           const switch_handle_t stp_handle,
                                           const void *cli_ctx) {
  switch_stp_info_t *stp_info = NULL;
  switch_node_t *node = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_stp_intf_entry_t *intf_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp dump failed on device %d "
        "stp handle 0x%lx: parameters invalid(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp dump failed on device %d "
        "stp handle 0x%lx: stp get failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\tstp handle: 0x%lx\n", stp_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);

  SWITCH_PRINT(cli_ctx, "\n\t\tnetwork handles:\n");
  FOR_EACH_IN_LIST(stp_info->network_list, node) {
    network_entry = (switch_stp_network_entry_t *)node->data;
    SWITCH_PRINT(
        cli_ctx, "\t\t\tnetwork handle: 0x%lx\n", network_entry->handle);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_PRINT(cli_ctx, "\n\t\tinterface handles:\n");
  FOR_EACH_IN_LIST(stp_info->intf_list, node) {
    intf_entry = (switch_stp_intf_entry_t *)node->data;
    SWITCH_PRINT(
        cli_ctx, "\t\t\tintf handle: 0x%lx\n", intf_entry->intf_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tstp state: %s\n",
                 switch_stp_state_to_string(intf_entry->stp_state));
    SWITCH_PRINT(cli_ctx, "\t\t\tpd handle: 0x%lx\n", intf_entry->hw_entry);
    SWITCH_PRINT(cli_ctx, "\t\t\thw flags: 0x%lx\n", intf_entry->hw_flags);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DEBUG(
      "stp handle dump on device %d stp handle 0x%lx\n", device, stp_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif
