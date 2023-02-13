/*
Copyright 2013-present Barefoot Networks, Inc.
*/
#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
void switch_l2_dump_mac_info(void *cli_ctx, void *node) {
  if (!cli_ctx || !node) return;

  switch_mac_info_t *mac_info = (switch_mac_info_t *)node;

  SWITCH_CLI_START_ENTRY_STR_PRINT(cli_ctx);
  SWITCH_PRINT(
      cli_ctx, "\t\tbd handle: 0x%lx\n", mac_info->mac_entry.bd_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tmac: %s\n",
               switch_macaddress_to_string(&mac_info->mac_entry.mac));
  SWITCH_PRINT(cli_ctx, "\t\thandle: 0x%lx\n", mac_info->handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tmac type: %s\n",
               switch_mac_entry_type_to_string(mac_info->entry_type));
  SWITCH_PRINT(
      cli_ctx, "\t\taging interval(ms): %d\n", mac_info->aging_interval);

  SWITCH_PRINT(cli_ctx, "\n\t\tpd handles:\n");
  SWITCH_PRINT(cli_ctx, "\t\tdmac entry: 0x%lx\n", mac_info->dmac_entry);
  SWITCH_PRINT(cli_ctx, "\t\tsmac entry: 0x%lx\n", mac_info->smac_entry);
  SWITCH_PRINT(cli_ctx, "\t\thw flags: 0x%lx\n", mac_info->hw_flags);
  SWITCH_CLI_END_ENTRY_STR_PRINT(cli_ctx);
  return;
}

switch_status_t switch_api_mac_entry_dump_internal(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    void *cli_ctx) {
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_api_mac_entry_handle_get(device, api_mac_entry, &mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac entry dump failed on device %d: "
        "mac entry handle get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_mac_entry_handle_dump(device, mac_handle, cli_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac entry dump failed on device %d: "
        "mac entry handle get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_mac_entry_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t mac_handle,
    void *cli_ctx) {
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    SWITCH_LOG_ERROR(
        "mac entry handle dump failed on device %d "
        "mac handle 0x%lx: mac handle invalid(%s)",
        device,
        mac_handle,
        switch_error_to_string(status));
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac entry handle dump failed on device %d "
        "mac handle 0x%lx: mac get failed(%s)",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_PRINT(cli_ctx, "\tmac handle: 0x%lx\n", mac_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  switch_l2_dump_mac_info(cli_ctx, (void *)mac_info);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_l2_mac_entry_interfce_handle_dump(
    switch_device_t device, switch_handle_t intf_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_l2_mac_entry_vlan_id_dump(switch_device_t device,
                                                 switch_vlan_t vlan_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_l2_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 context dump failed on device %d: "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tMAC hashtable:\n");
  SWITCH_CLI_HASHTABLE_PRINT(cli_ctx, l2_ctx->mac_hashtable, "MAC");

  SWITCH_PRINT(cli_ctx, "\t\tMAC event list:\n");
  for (index = 0; index < SWITCH_MAC_EVENT_REGISTRATION_MAX; index++) {
    if (l2_ctx->mac_event_list[index].valid) {
      SWITCH_PRINT(cli_ctx,
                   "\n\t\t\tapp id %d: \n",
                   l2_ctx->mac_event_list[index].app_id);
      SWITCH_PRINT(cli_ctx,
                   "\n\t\t\tflags 0x%x: \n",
                   l2_ctx->mac_event_list[index].mac_event_flags);
      SWITCH_PRINT(cli_ctx,
                   "\n\t\t\tcallback: 0x%x: \n",
                   l2_ctx->mac_event_list[index].cb_fn);
    }
  }
  return status;
}

switch_status_t switch_l2_hashtable_dump_internal(
    const switch_device_t device,
    const switch_hashtable_type_t type,
    void *cli_ctx) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 context dump failed on device %d: "
        "l2 hashtable dump failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch (type) {
    case SWITCH_HASHTABLE_TYPE_MAC:
      SWITCH_HASHTABLE_ITERATOR(
          &l2_ctx->mac_hashtable.table, switch_l2_dump_mac_info, cli_ctx);
      break;
    default:
      break;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_mac_entry_handle_dump(
    const switch_device_t device,
    const switch_handle_t mac_handle,
    void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_mac_entry_handle_dump_internal(device, mac_handle, cli_ctx))
}

switch_status_t switch_api_mac_entry_dump(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_mac_entry_dump_internal(device, api_mac_entry, cli_ctx))
}

switch_status_t switch_api_l2_context_dump(const switch_device_t device,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_l2_context_dump_internal(device, cli_ctx));
}

switch_status_t switch_l2_hashtable_dump(const switch_device_t device,
                                         const switch_hashtable_type_t type,
                                         void *cli_ctx) {
  SWITCH_MT_WRAP(switch_l2_hashtable_dump_internal(device, type, cli_ctx));
}
