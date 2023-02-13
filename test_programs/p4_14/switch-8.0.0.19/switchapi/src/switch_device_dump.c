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
#define __MODULE__ SWITCH_API_TYPE_DEVICE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_device_dump_internal(const switch_device_t device,
                                                const void *cli_ctx) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_t device_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device dump failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  device_handle = id_to_handle(SWITCH_HANDLE_TYPE_DEVICE, device);

  SWITCH_PRINT(cli_ctx, "\tdevice handle: 0x%x\n", device_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(
      cli_ctx, "\t\tsw-model: %d\n", switch_pd_platform_type_model(device));
  SWITCH_PRINT(
      cli_ctx, "\t\tdefault vrf: %d\n", device_ctx->device_info.default_vrf);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle: %lx\n", device_ctx->device_info.vrf_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tdefault vlan: %d\n", device_ctx->device_info.default_vlan);
  SWITCH_PRINT(
      cli_ctx, "\t\tvlan handle: %lx\n", device_ctx->device_info.vlan_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdefault mac: %s\n", "mac");
  SWITCH_PRINT(
      cli_ctx, "\t\trmac handle: %lx\n", device_ctx->device_info.rmac_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tmax lag groups: %d\n",
               device_ctx->device_info.max_lag_groups);
  SWITCH_PRINT(cli_ctx,
               "\t\tmax lag members: %d\n",
               device_ctx->device_info.max_lag_members);
  SWITCH_PRINT(cli_ctx,
               "\t\tmax ecmp groups: %d\n",
               device_ctx->device_info.max_ecmp_groups);
  SWITCH_PRINT(cli_ctx,
               "\t\tmax ecmp members: %d\n",
               device_ctx->device_info.max_ecmp_members);
  SWITCH_PRINT(cli_ctx, "\t\tmax vrf: %d\n", device_ctx->device_info.max_vrf);
  SWITCH_PRINT(
      cli_ctx, "\t\teth cpu port: %d\n", device_ctx->device_info.eth_cpu_port);
  SWITCH_PRINT(
      cli_ctx, "\t\teth cpu dev port %d\n", device_ctx->eth_cpu_dev_port);
  SWITCH_PRINT(cli_ctx,
               "\t\tpcie cpu port: %d\n",
               device_ctx->device_info.pcie_cpu_port);
  SWITCH_PRINT(
      cli_ctx, "\t\tpcie cpu dev port %d\n", device_ctx->pcie_cpu_dev_port);
  SWITCH_PRINT(
      cli_ctx, "\t\tcpu port handle: 0x%x\n", device_ctx->cpu_port_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tcounter refresh interval: %d\n",
               device_ctx->refresh_interval);
  SWITCH_PRINT(cli_ctx,
               "\t\taging interval: %d\n",
               device_ctx->device_info.aging_interval);
  SWITCH_PRINT(cli_ctx,
               "\t\tmac learning: %s\n",
               device_ctx->device_info.mac_learning ? "enabled" : "disabled");

  for (index = 0; index < device_ctx->max_recirc_ports; index++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\trecirc port %d handle 0x%lx - dev port %d\n",
                 index,
                 device_ctx->recirc_port_handles[index],
                 device_ctx->recirc_dev_port_list[index]);
  }

  SWITCH_PRINT(
      cli_ctx,
      "\t\ttunnel dmac: %s\n",
      switch_macaddress_to_string(&device_ctx->device_info.tunnel_dmac));

  SWITCH_PRINT(cli_ctx, "\n\t\tdevice context\n");
  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tapi: %s context: 0x%lx inited: %d\n",
                 switch_api_type_to_string(api_type),
                 device_ctx->context[api_type],
                 device_ctx->api_inited[api_type]);
  }

  status = switch_api_table_sizes_dump(device, cli_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("table dump failed on device %d\n",
                     device,
                     switch_error_to_string(status));
  }

  SWITCH_PRINT(cli_ctx, "\n\t\tmax pipes: %d\n", device_ctx->max_pipes);
  SWITCH_PRINT(
      cli_ctx, "\n\t\tmax ports: %d\n", device_ctx->device_info.max_ports);
  SWITCH_PRINT(cli_ctx,
               "\n\t\tactive ports: %d\n",
               device_ctx->device_info.num_active_ports);
  SWITCH_PRINT(
      cli_ctx,
      "\n\t\tswitching mode: %s\n",
      (device_ctx->cut_through_mode == true) ? "cut-through" : "store-fwd");
  for (index = 0; index < device_ctx->device_info.max_ports; index++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tfp port %d  - dev port %d\n",
                 device_ctx->fp_list[index],
                 device_ctx->dp_list[index]);
  }

  SWITCH_LOG_DEBUG("device dump on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_api_dump_internal(
    const switch_device_t device,
    const switch_api_type_t api_type,
    const void *cli_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch (api_type) {
    case SWITCH_API_TYPE_PORT:
      break;
    case SWITCH_API_TYPE_L2:
      status = switch_api_l2_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_BD:
      break;
    case SWITCH_API_TYPE_VRF:
      break;
    case SWITCH_API_TYPE_L3:
      status = switch_api_l3_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_RMAC:
      break;
    case SWITCH_API_TYPE_INTERFACE:
      break;
    case SWITCH_API_TYPE_LAG:
      break;
    case SWITCH_API_TYPE_NHOP:
      status = switch_api_nhop_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_NEIGHBOR:
      status = switch_api_neighbor_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_TUNNEL:
      status = switch_api_tunnel_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_MCAST:
      break;
    case SWITCH_API_TYPE_HOSTIF:
      break;
    case SWITCH_API_TYPE_ACL:
      break;
    case SWITCH_API_TYPE_MIRROR:
      break;
    case SWITCH_API_TYPE_METER:
      break;
    case SWITCH_API_TYPE_SFLOW:
      break;
    case SWITCH_API_TYPE_DTEL:
      break;
    case SWITCH_API_TYPE_STP:
      break;
    case SWITCH_API_TYPE_VLAN:
      break;
    case SWITCH_API_TYPE_QOS:
      break;
    case SWITCH_API_TYPE_QUEUE:
      break;
    case SWITCH_API_TYPE_LOGICAL_NETWORK:
      break;
    case SWITCH_API_TYPE_NAT:
      break;
    case SWITCH_API_TYPE_BUFFER:
      status = switch_api_buffer_context_dump(device, cli_ctx);
      break;
    case SWITCH_API_TYPE_BFD:
      break;
    case SWITCH_API_TYPE_HASH:
      break;
    case SWITCH_API_TYPE_WRED:
      break;
    case SWITCH_API_TYPE_ILA:
      break;
    case SWITCH_API_TYPE_FAILOVER:
      break;
    case SWITCH_API_TYPE_LABEL:
      break;
    case SWITCH_API_TYPE_RPF:
      break;
    case SWITCH_API_TYPE_DEVICE:
      break;
    case SWITCH_API_TYPE_RIF:
      break;
    case SWITCH_API_TYPE_PACKET_DRIVER:
      break;
    case SWITCH_API_TYPE_SCHEDULER:
      break;
    case SWITCH_API_TYPE_MPLS:
      break;
    default:
      break;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_device_dump(const switch_device_t device,
                                       const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_device_dump_internal(device, cli_ctx))
}

switch_status_t switch_api_device_api_dump(const switch_device_t device,
                                           const switch_api_type_t api_type,
                                           const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_device_api_dump_internal(device, api_type, cli_ctx))
}
