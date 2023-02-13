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

#include "switchapi/switch_neighbor.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_NHOP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_neighbor_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t neighbor_handle,
    const void *cli_ctx) {
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NEIGHBOR_HANDLE(neighbor_handle));
  if (!SWITCH_NEIGHBOR_HANDLE(neighbor_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "neighbor dump failed on device %d "
        "neighbor handle %lx: parameters invalid(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_get(device, neighbor_handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor dump failed on device %d "
        "neighbor handle %lx: neighbor get failed(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tneighbor handle: 0x%lx\n", neighbor_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tneighbor type: %s\n",
               switch_neighbor_type_to_string(
                   neighbor_info->api_neighbor_info.neighbor_type));
  SWITCH_PRINT(cli_ctx,
               "\t\t\trewrite type: %s\n",
               switch_neighbor_rewrite_type_to_string(
                   neighbor_info->api_neighbor_info.rw_type));
  SWITCH_PRINT(cli_ctx,
               "\t\t\tmac: %02x:%02x:%02x:%02x:%02x:%02x\n",
               neighbor_info->mac.mac_addr[0],
               neighbor_info->mac.mac_addr[1],
               neighbor_info->mac.mac_addr[2],
               neighbor_info->mac.mac_addr[3],
               neighbor_info->mac.mac_addr[4],
               neighbor_info->mac.mac_addr[5]);
  SWITCH_PRINT(
      cli_ctx, "\t\t\trewrite entry: 0x%lx\n", neighbor_info->rewrite_pd_hdl);

  return status;
}

switch_status_t switch_api_neighbor_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor context dump failed on device %d: "
        "neighbor context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tNeighbor Context:\n");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, neighbor_ctx->neighbor_dmac_hashtable, "Neighbor Dmac");
  SWITCH_CLI_HASHTABLE_PRINT(cli_ctx,
                             neighbor_ctx->tunnel_dmac_rewrite_hashtable,
                             "Neighbor Tunnel Dmac");

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_neighbor_handle_dump(
    const switch_device_t device,
    const switch_handle_t nieighbor_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_neighbor_handle_dump_internal(
      device, nieighbor_handle, cli_ctx))
}

switch_status_t switch_api_neighbor_context_dump(const switch_device_t device,
                                                 const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_neighbor_context_dump_internal(device, cli_ctx));
}
