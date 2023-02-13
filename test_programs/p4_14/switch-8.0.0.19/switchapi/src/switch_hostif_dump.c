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

#include "switchapi/switch_hostif.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_HOSTIF

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_hostif_handle_dump(
    const switch_device_t device,
    const switch_handle_t hostif_handle,
    const void *cli_ctx) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(hostif_handle));
  if (!SWITCH_HOSTIF_HANDLE(hostif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif dump failed on device %d "
        "hostif  handle %lx: parameters invalid(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_get(device, hostif_handle, &hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif dump failed on device %d "
        "hostif handle %lx: hostif get failed(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\thostif handle: %lx\n", hostif_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\thandle: 0x%lx\n", hostif_info->hostif.handle);
  SWITCH_PRINT(cli_ctx, "\t\thostif fd: %d\n", hostif_info->hostif_fd);
  SWITCH_PRINT(cli_ctx, "\t\thostif name: %s\n", hostif_info->hostif.intf_name);
  SWITCH_PRINT(cli_ctx,
               "\t\toper status: %d\n",
               (hostif_info->hostif.operstatus == TRUE) ? "UP" : "DOWN");
  SWITCH_PRINT(cli_ctx,
               "\t\tadmin state: %d\n",
               (hostif_info->hostif.admin_state == TRUE) ? "UP" : "DOWN");

  if (hostif_info->flags & SWITCH_HOSTIF_ATTR_MAC_ADDRESS) {
    SWITCH_PRINT(cli_ctx,
                 "\t\tmac address: 0x%02x:%02x:%02x:%02x:%02x:%02x\n",
                 hostif_info->hostif.mac.mac_addr[0],
                 hostif_info->hostif.mac.mac_addr[1],
                 hostif_info->hostif.mac.mac_addr[2],
                 hostif_info->hostif.mac.mac_addr[3],
                 hostif_info->hostif.mac.mac_addr[4],
                 hostif_info->hostif.mac.mac_addr[5]);
  }

  if (hostif_info->flags & SWITCH_HOSTIF_ATTR_IPV4_ADDRESS) {
    SWITCH_PRINT(
        cli_ctx, "\t\tipv4 address: 0x%x\n", hostif_info->hostif.v4addr);
  }

  if (hostif_info->flags & SWITCH_HOSTIF_ATTR_IPV6_ADDRESS) {
  }

  if (hostif_info->flags & SWITCH_HOSTIF_ATTR_VLAN_ACTION) {
    SWITCH_PRINT(
        cli_ctx,
        "\t\tvlan action: %s\n",
        switch_hostif_vlan_action_to_string(hostif_info->hostif.vlan_action));
  }

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG("hostif handle dump on device %d hostif handle %lx\n",
                   device,
                   hostif_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_group_handle_dump(
    const switch_device_t device,
    const switch_handle_t hostif_group_handle,
    const void *cli_ctx) {
  switch_hostif_group_info_t *hostif_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_HOSTIF_GROUP_HANDLE(hostif_group_handle));
  if (!SWITCH_HOSTIF_GROUP_HANDLE(hostif_group_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif group dump failed on device %d "
        "hostif handle %lx: parameters invalid(%s)\n",
        device,
        hostif_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_hostif_group_get(device, hostif_group_handle, &hostif_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif group dump failed on device %d "
        "hostif group handle %lx: hostif group get failed(%s)\n",
        device,
        hostif_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\thostif group handle: %lx\n", hostif_group_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\tpolicer handle: 0x%lx\n",
               hostif_group_info->hif_group.policer_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tqueue handle: 0x%lx\n",
               hostif_group_info->hif_group.queue_handle);

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG(
      "hostif group handle dump on device %d "
      "hostif group handle 0x%lx\n",
      device,
      hostif_group_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_rcode_handle_dump(
    const switch_device_t device,
    const switch_handle_t rcode_handle,
    const void *cli_ctx) {
  switch_hostif_rcode_info_t *hostif_rcode_info = NULL;
  switch_counter_t counter;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle));
  if (!SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif rcode dump failed on device %d "
        "hostif rcode handle 0x%lx: parameters invalid(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rcode_get(device, rcode_handle, &hostif_rcode_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rcode dump failed on device %d "
        "hostif handle 0x%lx: hostif get failed(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\trcode handle: %lx\n", rcode_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(
      cli_ctx, "\t\tacl handle: 0x%lx\n", hostif_rcode_info->acl_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tsystem acl handle: 0x%lx\n",
               hostif_rcode_info->system_acl_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\treason code: %s\n",
               switch_hostif_code_to_string(
                   hostif_rcode_info->rcode_api_info.reason_code));
  SWITCH_PRINT(
      cli_ctx,
      "\t\taction: %s\n",
      switch_acl_action_to_string(hostif_rcode_info->rcode_api_info.action));
  SWITCH_PRINT(cli_ctx,
               "\t\tpriority: %d\n",
               hostif_rcode_info->rcode_api_info.priority);
  SWITCH_PRINT(cli_ctx,
               "\t\thostif group handle: 0x%lx\n",
               hostif_rcode_info->rcode_api_info.hostif_group_id);
  if (SWITCH_ACL_COUNTER_HANDLE(hostif_rcode_info->counter_handle)) {
    SWITCH_MEMSET(&counter, 0x0, sizeof(switch_counter_t));
    status = switch_api_acl_counter_get(
        device, hostif_rcode_info->counter_handle, &counter);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rcode dump failed on device %d "
          "hostif handle 0x%lx: counter get failed(%s)\n",
          device,
          rcode_handle,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_PRINT(cli_ctx, "\n\t\tacl counter:\n");
    SWITCH_PRINT(cli_ctx, "\t\t\tnum packets: %d\n", counter.num_packets);
    SWITCH_PRINT(cli_ctx, "\t\t\tnum bytes: %d\n", counter.num_bytes);
  }

  if (SWITCH_ACL_COUNTER_HANDLE(hostif_rcode_info->system_counter_handle)) {
    SWITCH_MEMSET(&counter, 0x0, sizeof(switch_counter_t));
    status = switch_api_acl_counter_get(
        device, hostif_rcode_info->system_counter_handle, &counter);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rcode dump failed on device %d "
          "hostif handle 0x%lx: counter get failed(%s)\n",
          device,
          rcode_handle,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_PRINT(cli_ctx, "\n\t\tsystem acl counter:\n");
    SWITCH_PRINT(cli_ctx, "\t\t\tnum packets: %d\n", counter.num_packets);
    SWITCH_PRINT(cli_ctx, "\t\t\tnum bytes: %d\n", counter.num_bytes);
  }

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG(
      "hostif rcode handle dump on device %d "
      "hostif rcode handle %lx\n",
      device,
      rcode_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_rx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t rx_filter_handle,
    const void *cli_ctx) {
  switch_hostif_rx_filter_info_t *rx_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_HOSTIF_RX_FILTER_HANDLE(rx_filter_handle));
  if (!SWITCH_HOSTIF_RX_FILTER_HANDLE(rx_filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif rx filter dump failed on device %d handle 0x%lx: "
        "rx filter handle invalid:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rx_filter_get(device, rx_filter_handle, &rx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx filter dump failed on device %d handle 0x%lx: "
        "hostif rx filter get failed:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\trx filter handle: 0x%lx\n", rx_filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);

  SWITCH_PRINT(cli_ctx, "\t\t\trx key:\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tport handle: 0x%lx\n", rx_info->rx_key.port_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tlag handle: 0x%lx\n", rx_info->rx_key.lag_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tintf handle: 0x%lx\n", rx_info->rx_key.intf_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\t\thandle: 0x%lx\n", rx_info->rx_key.handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\treason code: %s\n",
               switch_hostif_code_to_string(rx_info->rx_key.reason_code));

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\trx action:\n");
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\thostif handle: 0x%lx",
               rx_info->rx_action.hostif_handle);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\tpktdriver filter handle 0x%lx\n", rx_info->filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdev port %d\n", rx_info->dev_port);
  SWITCH_PRINT(cli_ctx, "\t\t\tifindex 0x%x\n", rx_info->ifindex);
  SWITCH_PRINT(cli_ctx, "\t\t\tbd %d\n", rx_info->bd);
  SWITCH_PRINT(cli_ctx,
               "\t\t\treason code: %s\n",
               switch_hostif_code_to_string(rx_info->reason_code));
  SWITCH_PRINT(cli_ctx, "\t\t\tflags 0x%x\n", rx_info->flags);

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG("rx filter handle dump on device %d handle 0x%lx\n",
                   device,
                   rx_filter_handle);

  return status;
}

switch_status_t switch_api_hostif_tx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t tx_filter_handle,
    const void *cli_ctx) {
  switch_hostif_tx_filter_info_t *tx_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_HOSTIF_TX_FILTER_HANDLE(tx_filter_handle));
  if (!SWITCH_HOSTIF_TX_FILTER_HANDLE(tx_filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif tx filter dump failed on device %d handle 0x%lx: "
        "tx filter handle invalid:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_tx_filter_get(device, tx_filter_handle, &tx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter dump failed on device %d handle 0x%lx: "
        "hostif tx filter get failed:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\ttx filter handle: 0x%lx\n", tx_filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);

  SWITCH_PRINT(cli_ctx, "\t\t\ttx key:\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\thostif handle: 0x%lx\n", tx_info->tx_key.hostif_handle);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\ttx action:\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tbypass flags 0x%x\n", tx_info->tx_action.bypass_flags);
  SWITCH_PRINT(cli_ctx, "\t\t\t\thandle 0x%lx\n", tx_info->tx_action.handle);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\tpktdriver filter handle 0x%lx\n", tx_info->filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\thostif fd %d\n", tx_info->hostif_fd);
  SWITCH_PRINT(cli_ctx, "\t\t\tbd %d\n", tx_info->bd);
  SWITCH_PRINT(cli_ctx, "\t\t\tdev port %d\n", tx_info->dev_port);
  SWITCH_PRINT(cli_ctx, "\t\t\tflags 0x%x\n", tx_info->flags);

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG("tx filter handle dump on device %d handle 0x%lx\n",
                   device,
                   tx_filter_handle);

  return status;
}

switch_status_t switch_api_hostif_by_name_dump(const switch_device_t device,
                                               const char *intf_name,
                                               const void *cli_ctx) {
  switch_handle_t hostif_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_api_hostif_handle_get(device, intf_name, &hostif_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif dump failed on device %d intf name %s: "
        "hostif handle get failed:(%s)\n",
        device,
        intf_name,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_hostif_handle_dump(device, hostif_handle, cli_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif dump failed on device %d intf name %s "
        "hostif handle 0x%lx: hostif dump failed:(%s)\n",
        device,
        intf_name,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG(
      "hostif handle dump on device %d handle 0x%lx\n", device, hostif_handle);

  return status;
}

#ifdef __cplusplus
}
#endif
