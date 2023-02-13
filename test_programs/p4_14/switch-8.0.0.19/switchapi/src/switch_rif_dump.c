/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_rif_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const void *cli_ctx) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif dump failed on device %d rif handle 0x%lx: "
        "parameters invalid(%s)",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif dump failed on device %d rif handle 0x%lx: "
        "rif get failed(%s)",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\trif: 0x%lx\n", rif_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\trif type: %s\n",
               switch_rif_type_to_string(rif_info->api_rif_info.rif_type));
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle: 0x%lx\n", rif_info->api_rif_info.vrf_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\trmac handle: 0x%lx\n", rif_info->api_rif_info.rmac_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tintf handle: 0x%lx\n", rif_info->api_rif_info.intf_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tln handle: 0x%lx\n", rif_info->api_rif_info.ln_handle);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle: 0x%lx\n", rif_info->bd_handle);
  SWITCH_PRINT(cli_ctx, "\t\thostif handle: 0x%lx\n", rif_info->hostif_handle);
  SWITCH_PRINT(cli_ctx, "\t\tvlan: %d\n", rif_info->api_rif_info.vlan);
  SWITCH_PRINT(
      cli_ctx, "\t\tipv4 unicast: %d\n", rif_info->api_rif_info.ipv4_unicast);
  SWITCH_PRINT(
      cli_ctx, "\t\tipv6 unicast: %d\n", rif_info->api_rif_info.ipv6_unicast);
  SWITCH_PRINT(cli_ctx,
               "\t\tipv4 multicast: %d\n",
               rif_info->api_rif_info.ipv6_multicast);
  SWITCH_PRINT(cli_ctx,
               "\t\tipv6 multicast: %d\n",
               rif_info->api_rif_info.ipv6_multicast);

  return status;
}

switch_status_t switch_api_rif_handle_dump(const switch_device_t device,
                                           const switch_handle_t rif_handle,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_rif_handle_dump_internal(device, rif_handle, cli_ctx))
}

#ifdef __cplusplus
}
#endif
