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
#define __MODULE__ SWITCH_API_TYPE_PACKET_DRIVER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pktdriver_rx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t filter_handle,
    const void *cli_ctx) {
  switch_pktdriver_rx_filter_info_t *rx_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PKTDRIVER_RX_FILTER_HANDLE(filter_handle));
  if (!SWITCH_PKTDRIVER_RX_FILTER_HANDLE(filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rx filter handle dump failed on device %d handle 0x%lx: "
        "rx filter handle invalid:(%s)\n",
        device,
        filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_rx_filter_get(device, filter_handle, &rx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rx filter handle dump failed on device %d handle 0x%lx: "
        "rx filter get failed:(%s)\n",
        device,
        filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\trx filter handle 0x%lx\n", filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice %d: \n", device);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tnum packets: 0x%ld\n", rx_info->num_packets);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\trx key:\n");
  SWITCH_PRINT(cli_ctx, "\t\t\t\tdev port: %d\n", rx_info->rx_key.dev_port);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tifindex: 0x%x\n", rx_info->rx_key.ifindex);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tbd: %d\n", rx_info->rx_key.bd);
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\treason code: 0x%x\n", rx_info->rx_key.reason_code);
  SWITCH_PRINT(cli_ctx,
               "\t\t\t\treason code mask: 0x%x\n",
               rx_info->rx_key.reason_code_mask);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\trx action:\n");
  SWITCH_PRINT(cli_ctx, "\t\t\t\tfd %d", rx_info->rx_action.fd);
  SWITCH_PRINT(
      cli_ctx,
      "\t\t\t\tvlan action: %s\n",
      switch_pktdriver_vlan_action_to_string(rx_info->rx_action.vlan_action));

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\tpriority: %d\n", rx_info->priority);
  SWITCH_PRINT(cli_ctx, "\t\t\tflags: %d\n", rx_info->flags);

  SWITCH_LOG_DEBUG(
      "pktdriver rx filter handle dump on device %d hostif handle %lx\n",
      device,
      filter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_pktdriver_tx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t filter_handle,
    const void *cli_ctx) {
  switch_pktdriver_tx_filter_info_t *tx_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PKTDRIVER_TX_FILTER_HANDLE(filter_handle));
  if (!SWITCH_PKTDRIVER_TX_FILTER_HANDLE(filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tx filter handle dump failed on device %d handle 0x%lx: "
        "tx filter handle invalid:(%s)\n",
        device,
        filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_tx_filter_get(device, filter_handle, &tx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tx filter handle dump failed on device %d handle 0x%lx: "
        "tx filter get failed:(%s)\n",
        device,
        filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\ttx filter handle 0x%lx\n", filter_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice %d: \n", device);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tnum packets: 0x%ld\n", tx_info->num_packets);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\ttx key:\n");
  SWITCH_PRINT(cli_ctx, "\t\t\t\tfd: %d\n", tx_info->tx_key.hostif_fd);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\ttx action:\n");
  SWITCH_PRINT(
      cli_ctx, "\t\t\t\tbypass flags: 0x%x\n", tx_info->tx_action.bypass_flags);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tbd: %d\n", tx_info->tx_action.bd);
  SWITCH_PRINT(cli_ctx, "\t\t\t\tdev port: %d\n", tx_info->tx_action.dev_port);

  SWITCH_PRINT(cli_ctx, "\n\n");
  SWITCH_PRINT(cli_ctx, "\t\t\tpriority: %d\n", tx_info->priority);
  SWITCH_PRINT(cli_ctx, "\t\t\tflags: %d\n", tx_info->flags);

  SWITCH_LOG_DEBUG(
      "pktdriver tx filter handle dump on device %d hostif handle %lx\n",
      device,
      filter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_pktdriver_rx_rc_counters_dump(
    const switch_device_t device, const void *cli_ctx) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_PRINT(cli_ctx, "\n\t\treason : bytes : packets\n");
  pktdriver_ctx = switch_config_packet_driver_context_get();
  for (index = 0; index < SWITCH_HOSTIF_REASON_CODE_MAX; index++) {
    if (SWITCH_HOSTIF_REASON_CODE_VALID(index)) {
      SWITCH_PRINT(cli_ctx,
                   "\t\t%s : %ld : %ld\n",
                   switch_hostif_code_to_string(index),
                   pktdriver_ctx->rx_rc_counters[index].num_bytes,
                   pktdriver_ctx->rx_rc_counters[index].num_packets);
    }
  }

  SWITCH_PRINT(cli_ctx, "\n\n");

  return status;
}

switch_status_t switch_pktdriver_rx_port_counters_dump(
    const switch_device_t device, const void *cli_ctx) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_PRINT(cli_ctx, "\n\t\tport : bytes : packets\n");
  pktdriver_ctx = switch_config_packet_driver_context_get();
  for (index = 0; index < SWITCH_MAX_PORTS; index++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\t%d : %ld : %ld\n",
                 index,
                 pktdriver_ctx->rx_port_counters[index].num_bytes,
                 pktdriver_ctx->rx_port_counters[index].num_packets);
  }

  SWITCH_PRINT(cli_ctx, "\n\n");
  return status;
}

switch_status_t switch_pktdriver_rx_total_counters_dump(
    const switch_device_t device, const void *cli_ctx) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_PRINT(cli_ctx, "\n\t\tcounter : packets\n");
  pktdriver_ctx = switch_config_packet_driver_context_get();
  SWITCH_PRINT(
      cli_ctx, "\t\ttotal rx packets : %ld\n", pktdriver_ctx->num_rx_packets);
  SWITCH_PRINT(
      cli_ctx, "\t\ttotal tx packets : %ld\n", pktdriver_ctx->num_tx_packets);
  SWITCH_PRINT(cli_ctx,
               "\t\ttotal rx netdev packets : %ld\n",
               pktdriver_ctx->num_rx_netdev_packets);
  SWITCH_PRINT(cli_ctx,
               "\t\ttotal tx netdev packets : %ld\n",
               pktdriver_ctx->num_tx_netdev_packets);
  SWITCH_PRINT(cli_ctx,
               "\t\ttotal rx cb packets : %ld\n",
               pktdriver_ctx->num_rx_cb_packets);
  SWITCH_PRINT(cli_ctx,
               "\t\ttotal tx cb packets : %ld\n",
               pktdriver_ctx->num_tx_cb_packets);

  SWITCH_PRINT(cli_ctx, "\n\n");
  return status;
}

switch_status_t switch_pktdriver_bd_mapping_dump(const switch_device_t device,
                                                 const void *cli_ctx) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  SWITCH_PRINT(cli_ctx, "\n\t\tvlan : bd\n");
  for (index = 1; index < SWITCH_MAX_VLANS; index++) {
    if (pktdriver_ctx->bd_mapping[index]) {
      SWITCH_PRINT(
          cli_ctx, "\t\t%d    : %d\n", index, pktdriver_ctx->bd_mapping[index]);
    }
  }

  SWITCH_PRINT(cli_ctx, "\n\n");
  return status;
}

switch_status_t switch_pktdriver_rx_tx_debug_enable(
    const switch_device_t device,
    const bool rx,
    const bool enable,
    const void *cli_ctx) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  if (rx) {
    pktdriver_ctx->rx_pkt_trace_enable = enable;
  } else {
    pktdriver_ctx->tx_pkt_trace_enable = enable;
  }

  return status;
}

#ifdef __cplusplus
}
#endif
