/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_interface_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const void *cli_ctx) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface dump failed on device %d: "
        "parameters invalid(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface dump failed on device %d: "
        "interface get failed(%s)",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n\tInterface 0x%lx:\n", intf_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\ttype: %s\n",
               switch_interface_type_to_string(intf_info->api_intf_info.type));
  SWITCH_PRINT(cli_ctx, "\t\thandle: 0x%lx\n", intf_info->api_intf_info.handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tport lag index: 0x%x\n", intf_info->port_lag_index);
  SWITCH_PRINT(cli_ctx, "\t\tifindex: 0x%x\n", intf_info->ifindex);
  SWITCH_PRINT(cli_ctx, "\t\thostif handle: 0x%lx\n", intf_info->hostif_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tnv handle: 0x%lx\n", intf_info->native_vlan_handle);
  SWITCH_PRINT(cli_ctx, "\t\tln handle: 0x%lx\n", intf_info->ln_handle);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle: 0x%lx\n", intf_info->bd_handle);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_interface_handle_dump(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_interface_handle_dump_internal(device, intf_handle, cli_ctx))
}
