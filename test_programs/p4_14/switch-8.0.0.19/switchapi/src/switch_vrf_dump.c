/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_vrf_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const void *cli_ctx) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vrf dump failed on device %d: "
        "parameters invalid(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_vrf_get_internal(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf dump failed on device %d: 0x%lx"
        "vrf get failed(%s)",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx, "\tvrf handle: 0x%lx\n", vrf_info->bd_vrf_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\tvrf id %d\n", vrf_info->vrf_id);
  SWITCH_PRINT(cli_ctx, "\t\trmac handle 0x%lx\n", vrf_info->rmac_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf internal handle 0x%lx\n", vrf_info->vrf_handle);
  SWITCH_PRINT(cli_ctx, "\t\tbd vrf handle 0x%lx\n", vrf_info->bd_vrf_handle);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle 0x%lx\n", vrf_info->bd_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tipv4 unicast: %d\n", vrf_info->ipv4_unicast_enabled);
  SWITCH_PRINT(
      cli_ctx, "\t\tipv6 unicast: %d\n", vrf_info->ipv6_unicast_enabled);
  SWITCH_PRINT(cli_ctx, "\n");

  return status;
}

switch_status_t switch_vrf_handle_dump_all(switch_device_t device) {
  UNUSED(device);
  return SWITCH_STATUS_SUCCESS;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_vrf_handle_dump(const switch_device_t device,
                                           const switch_handle_t vrf_handle,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_vrf_handle_dump_internal(device, vrf_handle, cli_ctx))
}
