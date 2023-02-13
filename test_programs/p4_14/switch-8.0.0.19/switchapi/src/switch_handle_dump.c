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
switch_status_t switch_api_handle_dump_internal(const switch_device_t device,
                                                const switch_handle_t handle,
                                                void *cli_ctx) {
  switch_handle_type_t type = SWITCH_HANDLE_TYPE_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  type = switch_handle_type_get(handle);
  if (type <= SWITCH_HANDLE_TYPE_NONE || type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "handle dump failed on device %d "
        "handle %lx type %d: "
        "handle type invalid(%s)\n",
        device,
        handle,
        type,
        switch_error_to_string(status));
    return status;
  }

  switch (type) {
    case SWITCH_HANDLE_TYPE_PORT:
      status = switch_api_port_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_LAG:
      status = switch_api_lag_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_LAG_MEMBER:
      status = switch_api_lag_member_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_INTERFACE:
      status = switch_api_interface_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_VRF:
      status = switch_api_vrf_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_BD:
      status = switch_bd_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_NHOP:
      status = switch_api_nhop_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_NEIGHBOR:
      status = switch_api_neighbor_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_RMAC:
      status = switch_api_rmac_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_VLAN:
      status = switch_api_vlan_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_STP:
      status = switch_api_stp_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_MGID:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_ACL:
      status = switch_api_acl_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_MGID_ECMP:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_URPF:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_GROUP:
      status = switch_api_hostif_group_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF:
      status = switch_api_hostif_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_ACE:
      status = switch_api_ace_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_MIRROR:
      status = switch_api_mirror_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_METER:
      status = switch_api_meter_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_SFLOW:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_SFLOW_ACE:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_ACL_COUNTER:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_RACL_COUNTER:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_QOS_MAP:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_PRIORITY_GROUP:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_QUEUE:
      status = switch_api_queue_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_SCHEDULER:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_BUFFER_POOL:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_BUFFER_PROFILE:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_LABEL:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_BD_MEMBER:
      status = switch_bd_member_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_BFD:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_HASH:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_WRED:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_RANGE:
      status = switch_api_acl_range_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_ECMP_MEMBER:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_STP_PORT:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE:
      status = switch_api_hostif_rcode_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER:
      status = switch_api_hostif_rx_filter_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER:
      status = switch_api_hostif_tx_filter_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER:
      status = switch_pktdriver_rx_filter_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER:
      status = switch_pktdriver_tx_filter_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_RPF_GROUP:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;
    case SWITCH_HANDLE_TYPE_MAC:
      status = switch_api_mac_entry_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_ROUTE:
      status = switch_api_l3_route_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_ACL_GROUP:
      status = switch_api_acl_group_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER:
      status = switch_api_acl_group_member_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_RIF:
      status = switch_api_rif_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER:
      status = switch_api_tunnel_mapper_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY:
      status =
          switch_api_tunnel_mapper_entry_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL:
      status = switch_api_tunnel_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_TERM:
      status = switch_api_tunnel_term_handle_dump(device, handle, cli_ctx);
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_ENCAP:
      status = switch_api_tunnel_encap_handle_dump(device, handle, cli_ctx);
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "handle dump failed on device %d "
        "handle %lx type %d: "
        "handle dump failed(%s)\n",
        device,
        handle,
        type,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("handle dump on device %d handle %lx\n", device, handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_handle_dump_all_internal(
    const switch_device_t device,
    const switch_handle_type_t handle_type,
    void *cli_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;

  if (handle_type <= SWITCH_HANDLE_TYPE_NONE ||
      handle_type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "handle dump all failed on device %d: "
        "handle type invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_HANDLE_BEGIN(device, handle_type, handle) {
    if (handle != SWITCH_API_INVALID_HANDLE) {
      status = switch_api_handle_dump(device, handle, cli_ctx);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "handle dump all failed on device %d: "
            "handle dump failed(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  }
  FOR_EACH_HANDLE_END();

  return status;
}

switch_status_t switch_api_handle_info_dump_all(const switch_device_t device,
                                                void *cli_ctx) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // TODO: FIXME

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_handle_dump_all(
    const switch_device_t device,
    const switch_handle_type_t handle_type,
    void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_handle_dump_all_internal(device, handle_type, cli_ctx))
}

switch_status_t switch_api_handle_dump(const switch_device_t device,
                                       const switch_handle_t handle,
                                       void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_handle_dump_internal(device, handle, cli_ctx))
}
