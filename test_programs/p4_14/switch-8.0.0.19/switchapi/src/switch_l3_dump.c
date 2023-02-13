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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_L3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
void switch_l3_dump_route_info(void *cli_ctx, void *node) {
  if (!cli_ctx || !node) return;

  switch_route_info_t *route_info = (switch_route_info_t *)node;
  SWITCH_CLI_START_ENTRY_STR_PRINT(cli_ctx);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle 0x%lx\n", route_info->route_entry.vrf_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tip address %s\n",
               switch_ipaddress_to_string(&route_info->route_entry.ip));
  SWITCH_PRINT(cli_ctx, "\t\tnhop handle 0x%lx\n", route_info->nhop_handle);
  SWITCH_PRINT(cli_ctx, "\t\troute handle 0x%lx\n", route_info->route_handle);

  SWITCH_PRINT(cli_ctx, "\n\t\tpd handles:\n");
  SWITCH_PRINT(cli_ctx, "\t\troute pd hdl 0x%lx\n", route_info->route_pd_hdl);
  SWITCH_PRINT(cli_ctx, "\t\turpf pd hdl 0x%lx\n", route_info->urpf_pd_hdl);
  SWITCH_PRINT(cli_ctx, "\t\thw flags 0x%lx\n", route_info->hw_flags);
  SWITCH_CLI_END_ENTRY_STR_PRINT(cli_ctx);

  return;
}

switch_status_t switch_api_l3_route_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t route_handle,
    void *cli_ctx) {
  switch_route_info_t *route_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route dump on device %d route handle %lx "
        "route handle invalid(%s)",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route dump on device %d route handle %lx "
        "route get failed(%s)",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  switch_l3_dump_route_info(cli_ctx, (void *)route_info);

  return status;
}

switch_status_t switch_api_l3_route_dump_internal(
    const switch_device_t device,
    const switch_api_route_entry_t *route_entry,
    void *cli_ctx) {
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route dump failed on device %d vrf handle %lx "
        "ip address %s: vrf handle invalid(%s)",
        device,
        route_entry->vrf_handle,
        switch_ipaddress_to_string(&route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_l3_route_handle_lookup(device, route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route dump failed on device %d vrf handle %lx "
        "ip address %s: vrf handle invalid(%s)",
        device,
        route_entry->vrf_handle,
        switch_ipaddress_to_string(&route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_l3_route_handle_dump(device, route_handle, cli_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route dump failed on device %d vrf handle %lx "
        "ip address %s: vrf handle invalid(%s)",
        device,
        route_entry->vrf_handle,
        switch_ipaddress_to_string(&route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 context dump failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tL3 Context:\n");
  SWITCH_CLI_HASHTABLE_PRINT(cli_ctx, l3_ctx->route_hashtable, "Route");

  return status;
}

switch_status_t switch_l3_hashtable_dump_internal(
    const switch_device_t device,
    const switch_hashtable_type_t type,
    void *cli_ctx) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 context dump failed on device %d: "
        "l3 hashtable dump failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch (type) {
    case SWITCH_HASHTABLE_TYPE_ROUTE:
      SWITCH_HASHTABLE_ITERATOR(
          &l3_ctx->route_hashtable.table, switch_l3_dump_route_info, cli_ctx);
      break;
    default:
      break;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_l3_route_handle_dump(
    const switch_device_t device,
    const switch_handle_t route_handle,
    void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_l3_route_handle_dump_internal(device, route_handle, cli_ctx))
}

switch_status_t switch_api_l3_route_dump(
    const switch_device_t device,
    const switch_api_route_entry_t *route_entry,
    void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_l3_route_dump_internal(device, route_entry, cli_ctx))
}

switch_status_t switch_api_l3_context_dump(const switch_device_t device,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_l3_context_dump_internal(device, cli_ctx));
}

switch_status_t switch_l3_hashtable_dump(const switch_device_t device,
                                         const switch_hashtable_type_t type,
                                         void *cli_ctx) {
  SWITCH_MT_WRAP(switch_l3_hashtable_dump_internal(device, type, cli_ctx));
}
