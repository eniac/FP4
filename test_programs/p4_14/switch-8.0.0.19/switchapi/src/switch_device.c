/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_DEVICE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_device_context_get(
    switch_device_t device, switch_device_context_t **device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_config_device_context_get(device, device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device context get failed on device %d: "
        "device config context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_device_api_init(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device api init failed on device %d: "
        "device context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d "
        "for api type %s: (%s)",
        device,
        switch_api_type_to_string(api_type),
        switch_error_to_string(status));
    goto cleanup;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    if (device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_DEVICE:
        break;

      case SWITCH_API_TYPE_PORT:
        status = switch_port_init(device);
        break;

      case SWITCH_API_TYPE_L2:
        status = switch_l2_init(device);
        break;

      case SWITCH_API_TYPE_BD:
        status = switch_bd_init(device);
        break;

      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_init(device);
        break;

      case SWITCH_API_TYPE_L3:
        status = switch_l3_init(device);
        break;

      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_init(device);
        break;

      case SWITCH_API_TYPE_INTERFACE:
        status = switch_interface_init(device);
        break;

      case SWITCH_API_TYPE_RIF:
        status = switch_rif_init(device);
        break;

      case SWITCH_API_TYPE_LAG:
        status = switch_lag_init(device);
        break;

      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_init(device);
        break;

      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_init(device);
        break;

      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_init(device);
        break;

      case SWITCH_API_TYPE_MCAST:
        status = switch_mcast_init(device);
        break;

      case SWITCH_API_TYPE_ACL:
        status = switch_acl_init(device);
        break;

      case SWITCH_API_TYPE_MIRROR:
        status = switch_mirror_init(device);
        break;

      case SWITCH_API_TYPE_METER:
        status = switch_meter_init(device);
        break;

      case SWITCH_API_TYPE_SFLOW:
        status = switch_sflow_init(device);
        break;

      case SWITCH_API_TYPE_HOSTIF:
        status = switch_hostif_init(device);
        break;

      case SWITCH_API_TYPE_VLAN:
        status = switch_vlan_init(device);
        break;

      case SWITCH_API_TYPE_QOS:
        status = switch_qos_init(device);
        break;

      case SWITCH_API_TYPE_QUEUE:
        status = switch_queue_init(device);
        break;

      case SWITCH_API_TYPE_LOGICAL_NETWORK:
        status = switch_ln_init(device);
        break;

      case SWITCH_API_TYPE_NAT:
        status = switch_nat_init(device);
        break;

      case SWITCH_API_TYPE_BUFFER:
        status = switch_buffer_init(device);
        break;

      case SWITCH_API_TYPE_BFD:
        status = switch_bfd_init(device);
        break;

      case SWITCH_API_TYPE_HASH:
        break;

      case SWITCH_API_TYPE_WRED:
        status = switch_wred_init(device);
        break;

      case SWITCH_API_TYPE_ILA:
        status = switch_ila_init(device);
        break;

      case SWITCH_API_TYPE_LABEL:
        status = switch_label_init(device);
        break;

      case SWITCH_API_TYPE_STP:
        status = switch_stp_init(device);
        break;

      case SWITCH_API_TYPE_FAILOVER:
        status = switch_failover_init(device);
        break;

      case SWITCH_API_TYPE_RPF:
        status = switch_rpf_init(device);
        break;

      case SWITCH_API_TYPE_DTEL:
        status = switch_dtel_init(device);
        break;

      case SWITCH_API_TYPE_PACKET_DRIVER:
        break;

      case SWITCH_API_TYPE_SCHEDULER:
        status = switch_scheduler_init(device);
        break;

      case SWITCH_API_TYPE_MPLS:
        status = switch_mpls_init(device);
        break;

      default:
        /* Internal error */
        status = SWITCH_STATUS_FAILURE;
        break;
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device init failed on device %d "
          "for api type %d %s: (%s)",
          device,
          api_type,
          switch_api_type_to_string(api_type),
          switch_error_to_string(status));
      goto cleanup;
    }

    device_ctx->api_inited[api_type] = true;
  }

  device_ctx->l2_miss_action[SWITCH_PACKET_TYPE_UNICAST] =
      SWITCH_ACL_ACTION_PERMIT;
  device_ctx->l2_miss_action[SWITCH_PACKET_TYPE_MULTICAST] =
      SWITCH_ACL_ACTION_PERMIT;
  device_ctx->l2_miss_action[SWITCH_PACKET_TYPE_BROADCAST] =
      SWITCH_ACL_ACTION_PERMIT;

  SWITCH_LOG_DEBUG("device api init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  tmp_status = switch_device_api_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_device_api_free(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device init failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    if (!device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_DEVICE:
        break;

      case SWITCH_API_TYPE_PORT:
        status = switch_port_free(device);
        break;

      case SWITCH_API_TYPE_L2:
        status = switch_l2_free(device);
        break;

      case SWITCH_API_TYPE_BD:
        status = switch_bd_free(device);
        break;

      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_free(device);
        break;

      case SWITCH_API_TYPE_L3:
        status = switch_l3_free(device);
        break;

      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_free(device);
        break;

      case SWITCH_API_TYPE_INTERFACE:
        status = switch_interface_free(device);
        break;

      case SWITCH_API_TYPE_RIF:
        status = switch_rif_free(device);
        break;

      case SWITCH_API_TYPE_LAG:
        status = switch_lag_free(device);
        break;

      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_free(device);
        break;

      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_free(device);
        break;

      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_free(device);
        break;

      case SWITCH_API_TYPE_MCAST:
        status = switch_mcast_free(device);
        break;

      case SWITCH_API_TYPE_HOSTIF:
        status = switch_hostif_free(device);
        break;

      case SWITCH_API_TYPE_ACL:
        status = switch_acl_free(device);
        break;

      case SWITCH_API_TYPE_MIRROR:
        status = switch_mirror_free(device);
        break;

      case SWITCH_API_TYPE_METER:
        status = switch_meter_free(device);
        break;

      case SWITCH_API_TYPE_SFLOW:
        status = switch_sflow_free(device);
        break;

      case SWITCH_API_TYPE_VLAN:
        status = switch_vlan_free(device);
        break;

      case SWITCH_API_TYPE_QOS:
        status = switch_qos_free(device);
        break;

      case SWITCH_API_TYPE_QUEUE:
        status = switch_queue_free(device);
        break;

      case SWITCH_API_TYPE_LOGICAL_NETWORK:
        status = switch_ln_free(device);
        break;

      case SWITCH_API_TYPE_NAT:
        status = switch_nat_free(device);
        break;

      case SWITCH_API_TYPE_BUFFER:
        status = switch_buffer_free(device);
        break;

      case SWITCH_API_TYPE_BFD:
        status = switch_bfd_free(device);
        break;

      case SWITCH_API_TYPE_HASH:
        break;

      case SWITCH_API_TYPE_WRED:
        status = switch_wred_free(device);
        break;

      case SWITCH_API_TYPE_ILA:
        status = switch_ila_free(device);
        break;

      case SWITCH_API_TYPE_LABEL:
        status = switch_label_free(device);
        break;

      case SWITCH_API_TYPE_STP:
        status = switch_stp_free(device);
        break;

      case SWITCH_API_TYPE_FAILOVER:
        status = switch_failover_free(device);
        break;

      case SWITCH_API_TYPE_RPF:
        status = switch_rpf_free(device);
        break;

      case SWITCH_API_TYPE_DTEL:
        status = switch_dtel_free(device);
        break;

      case SWITCH_API_TYPE_PACKET_DRIVER:
        break;

      case SWITCH_API_TYPE_SCHEDULER:
        status = switch_scheduler_free(device);
        break;

      case SWITCH_API_TYPE_MPLS:
        status = switch_mpls_free(device);
        break;

      default:
        /* Internal error */
        status = SWITCH_STATUS_FAILURE;
        break;
    }

    device_ctx->api_inited[api_type] = false;

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device free failed on device %d "
          "for api type %s: (%s)",
          device,
          switch_api_type_to_string(api_type),
          switch_error_to_string(status));
      continue;
    }
  }

  SWITCH_LOG_DEBUG("device api free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_device_default_entries_add(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device default entries add failed on device %d: "
        "device context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_api_batch_begin();

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    status = SWITCH_STATUS_SUCCESS;
    if (!device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_PORT:
        status = switch_port_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_L2:
        status = switch_l2_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_BD:
        status = switch_bd_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_L3:
        status = switch_l3_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_INTERFACE:
        status = switch_interface_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_LAG:
        status = switch_lag_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_MCAST:
        status = switch_mcast_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_HOSTIF:
        status = switch_hostif_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_ACL:
        status = switch_acl_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_MIRROR:
        status = switch_mirror_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_METER:
        status = switch_meter_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_SFLOW:
        status = switch_sflow_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_VLAN:
        status = switch_vlan_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_QOS:
        status = switch_qos_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_QUEUE:
        status = switch_queue_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_LOGICAL_NETWORK:
        status = switch_ln_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_NAT:
        status = switch_nat_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_BUFFER:
        status = switch_buffer_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_HASH:
        status = switch_hash_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_WRED:
        status = switch_wred_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_DTEL:
        status = switch_dtel_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_FAILOVER:
        status = switch_failover_default_entry_add(device);
        break;

      case SWITCH_API_TYPE_MPLS:
        status = switch_mpls_default_entries_add(device);
        break;

      case SWITCH_API_TYPE_PACKET_DRIVER:
      case SWITCH_API_TYPE_STP:
        status = SWITCH_STATUS_SUCCESS;
        break;

      default:
        break;
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device default entries add failed on device %d "
          "for api type %s: (%s)",
          device,
          switch_error_to_string(status),
          switch_api_type_to_string(api_type));
      continue;
    }
  }

  status = switch_pd_fwd_result_table_default_entry_add(device);
  status = switch_pd_rewrite_table_default_entry_add(device);
  status = switch_pd_fwd_result_table_entry_init(device);
  status = switch_pd_fabric_header_table_entry_init(device);
  status = switch_pd_compute_hashes_entry_init(device);
  status = switch_pd_adjust_lkp_fields_table_default_entry_add(device);
  status = switch_pd_flowlet_default_entry_add(device);
  status = switch_pd_srv6_rewrite_table_entry_init(device);
  status = switch_pd_process_srh_len_table_entry_init(device);
  status = switch_pd_srv6_table_entry_init(device);

  switch_api_batch_end(TRUE);

  SWITCH_LOG_DEBUG("device default entries added on device %d\n", device);

  return status;
}

switch_status_t switch_device_default_entries_delete(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device default entries delete failed on device %d: "
        "device context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    if (!device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_PORT:
        status = switch_port_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_L2:
        status = switch_l2_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_BD:
        status = switch_bd_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_L3:
        status = switch_l3_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_INTERFACE:
        status = switch_interface_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_LAG:
        status = switch_lag_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_MCAST:
        status = switch_mcast_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_ACL:
        status = switch_acl_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_MIRROR:
        status = switch_mirror_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_METER:
        status = switch_meter_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_SFLOW:
        status = switch_sflow_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_HOSTIF:
        status = switch_hostif_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_VLAN:
        status = switch_vlan_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_QOS:
        status = switch_qos_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_QUEUE:
        status = switch_queue_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_LOGICAL_NETWORK:
        status = switch_ln_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_NAT:
        status = switch_nat_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_BUFFER:
        status = switch_buffer_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_HASH:
        status = switch_hash_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_WRED:
        status = switch_wred_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_DTEL:
        status = switch_dtel_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_FAILOVER:
        status = switch_failover_default_entry_delete(device);
        break;

      case SWITCH_API_TYPE_MPLS:
        status = switch_mpls_default_entries_delete(device);
        break;

      case SWITCH_API_TYPE_PACKET_DRIVER:
        break;

      default:
        break;
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device default entries delete failed on device %d: %s"
          "for api type %s",
          device,
          switch_error_to_string(status),
          switch_api_type_to_string(api_type));
      continue;
    }
  }

  SWITCH_LOG_DEBUG("device default entries deleted on device %d\n", device);

  return status;
}

switch_status_t switch_device_deinit(switch_device_t device) {
  return switch_pd_free(device);
}

switch_status_t switch_device_init(switch_device_t device,
                                   switch_size_t *table_sizes) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t i = 0;
  bool cut_through_mode = false;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "device context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_table_init(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "table init failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    device_ctx->api_inited[api_type] = false;
  }

  status = switch_api_id_allocator_new(
      device, SWITCH_IFINDEX_SIZE, FALSE, &device_ctx->ifindex_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "ifindex allocator init failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  device_ctx->cpu_port = SWITCH_CONFIG_PCIE() ? SWITCH_CPU_PORT_PCIE_DEFAULT
                                              : SWITCH_CPU_PORT_ETH_DEFAULT;
  device_ctx->device_info.eth_cpu_port = SWITCH_CPU_PORT_ETH_DEFAULT;
  device_ctx->device_info.pcie_cpu_port = SWITCH_CPU_PORT_PCIE_DEFAULT;

  status = switch_device_api_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "device api init failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_CONFIG_PORT_ADD()) {
    switch_port_t port = 0;
    switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
    switch_api_port_info_t api_port_info;
    SWITCH_MEMSET(&api_port_info, 0, sizeof(switch_api_port_info_t));
    for (port = 0; port < device_ctx->device_info.max_ports; port++) {
      api_port_info.port = port;
      api_port_info.port_speed = SWITCH_CONFIG_PORT_SPEED_DEFAULT;
      api_port_info.initial_admin_state = SWITCH_CONFIG_PORT_ENABLE();
      api_port_info.rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
      api_port_info.tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
      status = switch_api_port_add(device, &api_port_info, &port_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "device init failed on device %d: "
            "port add failed(%s)",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  status = switch_cpu_port_add(
      device, device_ctx->cpu_port, &device_ctx->cpu_port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "cpu port add failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (i = 0; i < device_ctx->max_recirc_ports; i++) {
    status = switch_recirc_port_add(device,
                                    device_ctx->recirc_port_list[i],
                                    &device_ctx->recirc_port_handles[i]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device init failed on device %d: "
          "recirc port add failed(%s)",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_api_router_mac_group_create(
      device, SWITCH_RMAC_TYPE_ALL, &device_ctx->device_info.rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "rmac group create failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(device_ctx->device_info.rmac_handle));

  status = switch_api_router_mac_add(device,
                                     device_ctx->device_info.rmac_handle,
                                     &device_ctx->device_info.mac);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "rmac group mac add failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device init failed on device %d: "
        "device default entries add failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vrf_create(device,
                                 device_ctx->device_info.default_vrf,
                                 &device_ctx->device_info.vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device init failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_create(device,
                                  device_ctx->device_info.default_vlan,
                                  &device_ctx->device_info.vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device init failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * Query the cut_through mode for one dev_port and store the
   * default switching mode.
   */
  status = switch_pd_port_cut_through_get(
      device, device_ctx->dp_list[0], &cut_through_mode);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  device_ctx->cut_through_mode = cut_through_mode;
  SWITCH_LOG_DEBUG("Default switching mode %s",
                   cut_through_mode ? "cut-through" : "store-fwd");

  SWITCH_LOG_DEBUG("device init done on device %d", device);

  return status;
}

switch_status_t switch_device_free(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_port_t port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "device context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_table_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "table free failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_default_entries_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "default entries delete failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_router_mac_delete(device,
                                        device_ctx->device_info.rmac_handle,
                                        &device_ctx->device_info.mac);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "router mac delete failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_router_mac_group_delete(
      device, device_ctx->device_info.rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "router mac group delete failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_CONFIG_PORT_ADD()) {
    for (port = 0; port < device_ctx->device_info.max_ports; port++) {
      status = switch_api_port_id_to_handle_get(device, port, &port_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      status = switch_api_port_delete(device, port_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "device free failed on device %d: "
            "port delete failed(%s)",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  status = switch_device_api_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "device api free failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_destroy(device, device_ctx->ifindex_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device free failed on device %d: "
        "ifindex allocator free failed(%s)",
        device,
        switch_error_to_string(status));
  }

  SWITCH_LOG_DEBUG("device free done on device %d", device);

  return status;
}

switch_status_t switch_device_table_get(switch_device_t device,
                                        switch_table_t **table_info) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(table_info != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *table_info = device_ctx->table_info;
  return status;
}

switch_status_t switch_device_api_context_set(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void *context) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  device_ctx->context[api_type] = context;

  return status;
}

switch_status_t switch_device_api_context_get(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void **context) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(context != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *context = device_ctx->context[api_type];
  return status;
}

void switch_device_stats_timer_cb(bf_sys_timer_t *timer, void *data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_t device = SWITCH_DEVICE_INTERNAL;

  if (data) {
    device = *((switch_device_t *)data);
    status = switch_pd_stats_update(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stats update failed on device %d: "
          "pd stats update failed(%s)\n",
          device,
          switch_error_to_string(status));
      return;
    }
  }
  return;
}

switch_status_t switch_api_device_add(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_size_t table_sizes[SWITCH_TABLE_MAX];
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  char cpuif_netdev_name[IFNAMSIZ] = "";
  switch_knet_info_t *knet_info;

  UNUSED(cpuif_netdev_name);
  UNUSED(knet_info);
  SWITCH_LOG_ENTER();

  if (SWITCH_CONFIG_DEVICE_INITED(device)) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("device add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  device_ctx = SWITCH_MALLOC(device, sizeof(switch_device_context_t), 0x1);
  if (!device_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("device add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(device_ctx, 0, sizeof(switch_device_context_t));

  status = switch_config_device_context_set(device, device_ctx);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(table_sizes, 0x0, sizeof(switch_size_t) * SWITCH_TABLE_MAX);

  status = switch_config_table_sizes_get(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  device_ctx->device_info.default_vrf = SWITCH_DEFAULT_VRF;
  device_ctx->device_info.default_vlan = SWITCH_DEFAULT_VLAN;
  device_ctx->device_info.max_ports = SWITCH_MAX_PORTS;
  device_ctx->device_info.max_vrf = SWITCH_MAX_VRF;
  device_ctx->device_info.max_lag_members = SWITCH_MAX_LAG_MEMBERS;
  device_ctx->device_info.max_ecmp_members = SWITCH_MAX_ECMP_MEMBERS;
  device_ctx->device_info.num_active_ports = 0;
  device_ctx->device_info.max_port_mtu = SWITCH_MAX_PORT_MTU;
  device_ctx->refresh_interval = SWITCH_COUNTER_REFRESH_INTERVAL_DEFAULT;
  device_ctx->device_info.aging_interval = SWITCH_MAC_TABLE_DEFAULT_AGING_TIME;
  device_ctx->device_info.mac_learning = TRUE;
  device_ctx->max_pipes = SWITCH_MAX_PIPES;
  device_ctx->device_id = device;
  device_ctx->warm_init = false;

  for (index = 0; index < SWITCH_MAX_PORTS; index++) {
    device_ctx->fp_list[index] = SWITCH_PORT_INVALID;
    device_ctx->dp_list[index] = SWITCH_PORT_INVALID;
  }

  status = switch_pd_max_ports_get(device, &device_ctx->device_info.max_ports);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(device_ctx->device_info.max_ports != 0);

  status = switch_pd_max_pipes_get(device, &device_ctx->max_pipes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_list_get(device,
                                   device_ctx->device_info.max_ports,
                                   device_ctx->fp_list,
                                   device_ctx->dp_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_recirc_port_list_get(device,
                                          &device_ctx->max_recirc_ports,
                                          device_ctx->recirc_port_list,
                                          device_ctx->recirc_dev_port_list);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed: recirc port list get on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_DEVICE_DEFAULT_MAC(device, device_ctx->device_info.mac);

  switch_int32_t mutex_status = bf_sys_rmutex_init(&device_ctx->mtx);
  if (mutex_status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device lock aqcuiring failed for device %d: %s",
                     device,
                     switch_error_to_string(mutex_status));
    return status;
  }

  status = switch_device_init(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  pktdriver_ctx->knet_pkt_driver[device] = false;

  if (switch_pktdriver_mode_is_kernel(device)) {
    status = switch_pktdriver_knet_device_add(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device add failed for device %d:"
          "knet device add failed: %s",
          device,
          switch_error_to_string(status));

      return SWITCH_STATUS_FAILURE;
    }
  }

  if (!switch_pd_platform_type_model(device)) {
    status = SWITCH_TIMER_CREATE(&device_ctx->stats_timer,
                                 device_ctx->refresh_interval,
                                 switch_device_stats_timer_cb,
                                 (void *)&device_ctx->device_id);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_TIMER_START(&device_ctx->stats_timer);
  } else {
    device_ctx->refresh_interval = 0;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_remove_internal(switch_device_t device) {
  switch_device_context_t *device_ctx = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(knet_info);
  SWITCH_LOG_ENTER();

  if (!SWITCH_CONFIG_DEVICE_INITED(device)) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("device add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MT_LOCK(device);

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
    status = switch_pktdriver_knet_device_delete(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device delete failed on device %d"
          "knet device delete failed: %s",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (!switch_pd_platform_type_model(device)) {
    SWITCH_TIMER_STOP(&device_ctx->stats_timer);
    status = SWITCH_TIMER_DELETE(&device_ctx->stats_timer);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_device_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    SWITCH_MT_UNLOCK(device);
    return status;
  }

  SWITCH_MT_UNLOCK(device);

  SWITCH_FREE(device, device_ctx);

  status = switch_config_device_context_set(device, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_vrf_max_get(switch_device_t device,
                                              switch_uint16_t *max_vrf) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *max_vrf = device_ctx->device_info.max_vrf;
  return status;
}

switch_status_t switch_api_device_default_vrf_get_internal(
    switch_device_t device, switch_vrf_t *vrf_id, switch_handle_t *vrf_handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *vrf_id = device_ctx->device_info.default_vrf;
  *vrf_handle = device_ctx->device_info.vrf_handle;

  return status;
}

switch_status_t switch_api_device_default_vlan_get_internal(
    switch_device_t device,
    switch_vlan_t *vlan_id,
    switch_handle_t *vlan_handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *vlan_id = device_ctx->device_info.default_vlan;
  *vlan_handle = device_ctx->device_info.vlan_handle;

  return status;
}

switch_status_t switch_api_device_max_ports_get(switch_device_t device,
                                                switch_uint32_t *max_ports) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *max_ports = device_ctx->device_info.max_ports + device_ctx->max_recirc_ports;

  return status;
}

switch_status_t switch_api_device_default_rmac_handle_get_internal(
    switch_device_t device, switch_handle_t *rmac_handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *rmac_handle = device_ctx->device_info.rmac_handle;

  return status;
}

switch_status_t switch_api_device_mac_address_set(switch_device_t device,
                                                  switch_mac_addr_t *mac) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (SWITCH_MEMCMP(
          mac, &device_ctx->device_info.mac, sizeof(switch_mac_addr_t) == 0)) {
    return status;
  }

  rmac_handle = device_ctx->device_info.rmac_handle;
  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));

  status = switch_api_router_mac_delete(
      device, rmac_handle, &device_ctx->device_info.mac);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device mac address set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&device_ctx->device_info.mac, mac, sizeof(switch_mac_addr_t));

  status = switch_api_router_mac_add(
      device, rmac_handle, &device_ctx->device_info.mac);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device mac address set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_device_mac_address_get(switch_device_t device,
                                                  switch_mac_addr_t *mac) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(mac, &device_ctx->device_info.mac, sizeof(switch_mac_addr_t));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_cpu_port_get_internal(switch_device_t device,
                                                        switch_port_t *port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(port != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port = device_ctx->cpu_port;
  return status;
}

switch_status_t switch_api_device_cpu_port_handle_get_internal(
    switch_device_t device, switch_handle_t *port_handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(port_handle != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port_handle = device_ctx->cpu_port_handle;
  return status;
}

switch_status_t switch_device_ifindex_allocate(switch_device_t device,
                                               switch_ifindex_t *ifindex) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(ifindex != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device ifindex allocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *ifindex = 0;

  status = switch_api_id_allocator_allocate(
      device, device_ctx->ifindex_allocator, ifindex);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device ifindex allocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_device_ifindex_deallocate(switch_device_t device,
                                                 switch_ifindex_t ifindex) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(ifindex != 0);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device ifindex deallocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_release(
      device, device_ctx->ifindex_allocator, ifindex);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device ifindex deallocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_device_port_list_all_get(
    switch_device_t device, switch_handle_list_t *port_list) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_list != NULL);
  if (!port_list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "port list parameter null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(port_list, 0x0, sizeof(switch_handle_list_t));

  status = switch_api_handles_get(device,
                                  SWITCH_HANDLE_TYPE_PORT,
                                  &port_list->num_handles,
                                  &port_list->handles);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "port handle get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_port_list_get(
    switch_device_t device, switch_handle_list_t *port_list) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_t *port_handles = NULL;
  switch_port_info_t *port_info = NULL;
  switch_size_t num_handles = 0;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_list != NULL);
  if (!port_list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "port list parameter null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_handles_get(
      device, SWITCH_HANDLE_TYPE_PORT, &num_handles, &port_handles);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "port handle get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  port_list->num_handles = 0;
  port_list->handles = NULL;

  if (num_handles == 0) {
    return status;
  }

  SWITCH_MEMSET(port_list, 0x0, sizeof(switch_handle_list_t));
  port_list->handles =
      SWITCH_MALLOC(device, sizeof(switch_handle_t), num_handles);
  SWITCH_MEMSET(port_list->handles, 0x0, sizeof(switch_handle_t) * num_handles);

  for (index = 0; index < num_handles; index++) {
    status = switch_port_get(device, port_handles[index], &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device port list get failed on device %d: "
          "port get failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    if (port_info->port_type != SWITCH_PORT_TYPE_NORMAL) {
      continue;
    }

    port_list->handles[port_list->num_handles++] = port_handles[index];
  }

  SWITCH_FREE(device, port_handles);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_device_cpu_eth_dev_port_set(switch_device_t device,
                                                   switch_dev_port_t dev_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device cpu eth dev port set failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  device_ctx->eth_cpu_dev_port = dev_port;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_device_cpu_eth_dev_port_get(
    switch_device_t device, switch_dev_port_t *dev_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device cpu eth dev port get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *dev_port = device_ctx->eth_cpu_dev_port;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_device_cpu_pcie_dev_port_set(
    switch_device_t device, switch_dev_port_t dev_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device cpu pcie dev port set failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  device_ctx->pcie_cpu_dev_port = dev_port;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_cpu_eth_port_get_internal(
    switch_device_t device, switch_port_t *cpu_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *cpu_port = device_ctx->device_info.eth_cpu_port;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_cpu_pcie_port_get_internal(
    switch_device_t device, switch_port_t *cpu_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device port list get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *cpu_port = device_ctx->device_info.pcie_cpu_port;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_device_max_pipes_get(switch_device_t device,
                                            switch_int32_t *max_pipes) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device max pipes get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *max_pipes = device_ctx->max_pipes;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_counter_refresh_interval_set_internal(
    switch_device_t device, switch_uint32_t refresh_interval) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device counter refresh interval set failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (!switch_pd_platform_type_model(device)) {
    if (device_ctx->refresh_interval) {
      SWITCH_TIMER_STOP(&device_ctx->stats_timer);
      status = SWITCH_TIMER_DELETE(&device_ctx->stats_timer);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    }

    if (refresh_interval) {
      status = SWITCH_TIMER_CREATE(&device_ctx->stats_timer,
                                   refresh_interval,
                                   switch_device_stats_timer_cb,
                                   (void *)&device_ctx->device_id);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      SWITCH_TIMER_START(&device_ctx->stats_timer);
    }
  }

  device_ctx->refresh_interval = refresh_interval;
  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_counter_refresh_interval_get_internal(
    switch_device_t device, switch_uint32_t *refresh_interval) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device counter refresh interval get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *refresh_interval = device_ctx->refresh_interval;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_attribute_get_internal(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info) {
  switch_device_context_t *device_ctx = NULL;
  switch_api_device_info_t *device_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device ifindex deallocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(api_device_info != NULL);
  if (!api_device_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("device ifindex deallocate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  device_info = &device_ctx->device_info;
  SWITCH_MEMSET(api_device_info, 0x0, sizeof(switch_api_device_info_t));

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_VRF) {
    api_device_info->default_vrf = device_info->default_vrf;
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_VRF_HANDLE) {
    api_device_info->vrf_handle = device_info->vrf_handle;
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_VLAN) {
    api_device_info->default_vlan = device_info->default_vlan;
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_VLAN_HANDLE) {
    api_device_info->vlan_handle = device_info->vlan_handle;
  }

  if (flags & SWITCH_DEVICE_ATTR_PORT_LIST) {
    status =
        switch_api_device_port_list_get(device, &api_device_info->port_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device attribute get failed on device %d: "
          "port handle list get failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_DEVICE_ATTR_MAX_PORTS) {
    api_device_info->max_ports =
        device_info->max_ports + device_ctx->max_recirc_ports;
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_MAC) {
    SWITCH_MEMCPY(
        &api_device_info->mac, &device_info->mac, sizeof(switch_mac_addr_t));
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_MAC_HANDLE) {
    api_device_info->rmac_handle = device_info->rmac_handle;
  }

  if (flags & SWITCH_DEVICE_ATTR_COUNTER_REFRESH_INTERVAL) {
    api_device_info->refresh_interval = device_info->refresh_interval;
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_AGING_TIME) {
    api_device_info->aging_interval = device_info->aging_interval;
  }

  if (flags & SWITCH_DEVICE_ATTR_MAX_VRFS) {
    api_device_info->max_vrf = device_info->max_vrf;
  }

  if (flags & SWITCH_DEVICE_ATTR_MAX_LAG_MEMBERS) {
    api_device_info->max_lag_members = device_info->max_lag_members;
  }

  if (flags & SWITCH_DEVICE_ATTR_MAX_ECMP_MEMBERS) {
    api_device_info->max_ecmp_members = device_info->max_ecmp_members;
  }

  if (flags & SWITCH_DEVICE_ATTR_ETH_CPU_PORT) {
    api_device_info->eth_cpu_port = device_info->eth_cpu_port;
  }

  if (flags & SWITCH_DEVICE_ATTR_PCIE_CPU_PORT) {
    api_device_info->pcie_cpu_port = device_info->pcie_cpu_port;
  }

  if (flags & SWITCH_DEVICE_ATTR_TUNNEL_DMAC) {
    SWITCH_MEMCPY(&api_device_info->tunnel_dmac,
                  &device_info->tunnel_dmac,
                  sizeof(switch_mac_addr_t));
  }

  if (flags & SWITCH_DEVICE_ATTR_MAC_LEARNING) {
    api_device_info->mac_learning = device_info->mac_learning;
  }

  if (flags & SWITCH_DEVICE_ATTR_MAX_PORT_MTU) {
    api_device_info->max_port_mtu = device_info->max_port_mtu;
  }

  if (flags & SWITCH_DEVICE_ATTR_ACTIVE_PORTS) {
    api_device_info->num_active_ports = device_info->num_active_ports;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_device_attribute_set_internal(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device dev port get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_MAC) {
    status = switch_api_device_mac_address_set(device, &api_device_info->mac);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device attribute set failed on device %d: "
          "mac address set failed(%s)",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_DEVICE_ATTR_DEFAULT_AGING_TIME) {
    device_ctx->device_info.aging_interval = api_device_info->aging_interval;
    status = switch_api_device_mac_aging_interval_set(
        device, api_device_info->aging_interval);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device attribute set failed on device %d: "
          "aging interval set failed(%s)",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_DEVICE_ATTR_TUNNEL_DMAC) {
    status = switch_api_device_tunnel_dmac_set(device,
                                               &api_device_info->tunnel_dmac);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device attribute set failed on device %d: "
          "tunnel dmac set failed(%s)",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_DEVICE_ATTR_MAC_LEARNING) {
    status = switch_api_device_mac_learning_set(device,
                                                &api_device_info->mac_learning);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device attribute set failed on device %d: "
          "mac learning set failed(%s)",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

bool switch_device_recirc_port(switch_device_t device, switch_port_t port) {
  switch_uint32_t i = 0;

  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device recirc dev port failed on device %d port %d: "
        "device context get failed(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return FALSE;
  }

  for (i = 0; i < device_ctx->max_recirc_ports; i++) {
    if (port == device_ctx->recirc_port_list[i]) {
      return TRUE;
    }
  }
  return FALSE;
}

switch_status_t switch_device_recirc_dev_port_get(switch_device_t device,
                                                  switch_port_t port,
                                                  switch_dev_port_t *dev_port) {
  switch_uint32_t i = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device recirc dev port get failed on device %d port %d: "
        "device context get failed(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  for (i = 0; i < device_ctx->max_recirc_ports; i++) {
    if (port == device_ctx->recirc_port_list[i]) {
      *dev_port = device_ctx->recirc_dev_port_list[i];
    }
  }
  return status;
}

switch_status_t switch_device_dev_port_get(switch_device_t device,
                                           switch_port_t port,
                                           switch_dev_port_t *dev_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device dev port get failed on device %d port %d: "
        "device context get failed(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  if (port == SWITCH_CPU_PORT_ETH_DEFAULT) {
    *dev_port = device_ctx->eth_cpu_dev_port;
  } else if (port == SWITCH_CPU_PORT_PCIE_DEFAULT) {
    *dev_port = device_ctx->pcie_cpu_dev_port;
  } else if (switch_device_recirc_port(device, port)) {
    switch_device_recirc_dev_port_get(device, port, dev_port);
  } else {
    SWITCH_ASSERT(device_ctx->fp_list[port] == port);
    *dev_port = device_ctx->dp_list[port];
  }

  return status;
}

switch_status_t switch_device_front_port_get(switch_device_t device,
                                             switch_dev_port_t dev_port,
                                             switch_port_t *fp_port) {
  switch_device_context_t *device_ctx = NULL;
  switch_port_t port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *fp_port = SWITCH_PORT_INVALID;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device front port get failed on device %d port %d: "
        "device context get failed(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  for (port = 0; port < device_ctx->device_info.max_ports; port++) {
    if (device_ctx->dp_list[port] == dev_port) {
      *fp_port = port;
      return status;
    }
  }

  if (dev_port == device_ctx->eth_cpu_dev_port) {
    *fp_port = SWITCH_CPU_PORT_ETH_DEFAULT;
  } else if (dev_port == device_ctx->pcie_cpu_dev_port) {
    *fp_port = SWITCH_CPU_PORT_PCIE_DEFAULT;
  } else {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
  }

  return status;
}

switch_status_t switch_api_device_recirc_port_get_internal(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_handle_t *port_handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (pipe_id >= SWITCH_MAX_PIPES) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "device recirc port get failed on device %d, pipe_id %d: "
        "invalid pipe-id(%s)\n",
        device,
        pipe_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device recirc port get failed on device %d pipe_id %d: "
        "device context get failed(%s)\n",
        device,
        pipe_id,
        switch_error_to_string(status));
    return status;
  }

  *port_handle = SWITCH_API_INVALID_HANDLE;

  if (!SWITCH_PORT_HANDLE(device_ctx->recirc_port_handles[pipe_id])) {
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  *port_handle = device_ctx->recirc_port_handles[pipe_id];
  return status;
}

switch_status_t switch_api_device_max_recirc_ports_get_internal(
    switch_device_t device, switch_uint16_t *num_ports) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device recirc max ports get failed on device %d: "
        "device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *num_ports = device_ctx->max_recirc_ports;

  return status;
}

switch_status_t switch_api_device_dmac_miss_packet_action_get_internal(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t *action_type) {
  switch_device_context_t *dev_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &dev_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device dmac miss packet action set failed for device %d: "
        "device context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  *action_type = dev_ctx->l2_miss_action[pkt_type];
  return status;
}

switch_status_t switch_api_device_dmac_miss_packet_action_set_internal(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t action_type) {
  switch_device_context_t *dev_ctx = NULL;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t priority = SWITCH_API_L2_FDB_MISS_ACL_PRIORITY;
  switch_acl_action_params_t action_params;
  switch_acl_opt_action_params_t opt_action_params;
  switch_acl_system_key_value_pair_t ing_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
  switch_uint16_t kvp_count = 0;
  switch_api_meter_t api_meter_info;
  switch_handle_t meter_handle = SWITCH_API_INVALID_HANDLE;
  switch_meter_info_t *meter_info = NULL;
  bool attach_meter = false;

  status = switch_device_context_get(device, &dev_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device dmac miss packet action set failed for device %d: "
        "device context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (dev_ctx->acl_pkt_type_handle[pkt_type] == SWITCH_API_INVALID_HANDLE) {
    // Create acl_handle for the packet type.
    status = switch_api_acl_list_create(device,
                                        SWITCH_API_DIRECTION_INGRESS,
                                        SWITCH_ACL_TYPE_SYSTEM,
                                        SWITCH_HANDLE_TYPE_NONE,
                                        &acl_handle);
    dev_ctx->acl_pkt_type_handle[pkt_type] = acl_handle;
    SWITCH_ASSERT(dev_ctx->acl_pkt_type_handle[pkt_type] !=
                  SWITCH_API_INVALID_HANDLE)

    ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
    ing_acl_kvp[kvp_count].value.reason_code = 0;
    ing_acl_kvp[kvp_count].mask.u.mask = 0xFFFF;
    kvp_count++;

    ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_L2_DST_MISS;
    ing_acl_kvp[kvp_count].value.l2_dst_miss = 1;
    ing_acl_kvp[kvp_count].mask.u.mask = 1;
    kvp_count++;

    ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE;
    ing_acl_kvp[kvp_count].value.packet_type = pkt_type;
    ing_acl_kvp[kvp_count].mask.u.mask = 0x7;
    kvp_count++;

    ing_acl_kvp[kvp_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
    ing_acl_kvp[kvp_count].value.routed = 0;
    ing_acl_kvp[kvp_count].mask.u.mask = 1;
    kvp_count++;
  }

  SWITCH_MEMSET(&action_params, 0, sizeof(action_params));
  if (action_type == SWITCH_ACL_ACTION_DROP) {
    if (pkt_type == SWITCH_PACKET_TYPE_UNICAST) {
      action_params.drop.reason_code = DROP_L2_MISS_UNICAST;
    } else if (pkt_type == SWITCH_PACKET_TYPE_MULTICAST) {
      action_params.drop.reason_code = DROP_L2_MISS_MULTICAST;
    } else {
      action_params.drop.reason_code = DROP_L2_MISS_BROADCAST;
    }
  } else if ((action_type == SWITCH_ACL_ACTION_REDIRECT_TO_CPU) ||
             (action_type == SWITCH_ACL_ACTION_COPY_TO_CPU)) {
    if (pkt_type == SWITCH_PACKET_TYPE_UNICAST) {
      action_params.cpu_redirect.reason_code =
          SWITCH_HOSTIF_REASON_CODE_L2_MISS_UNICAST;
    } else if (pkt_type == SWITCH_PACKET_TYPE_MULTICAST) {
      action_params.cpu_redirect.reason_code =
          SWITCH_HOSTIF_REASON_CODE_L2_MISS_MULTICAST;
    } else {
      action_params.cpu_redirect.reason_code =
          SWITCH_HOSTIF_REASON_CODE_L2_MISS_BROADCAST;
    }
    attach_meter = true;
  } else if (action_type == SWITCH_ACL_ACTION_PERMIT) {
    action_type = SWITCH_ACL_ACTION_PERMIT;
  } else {
    SWITCH_LOG_ERROR(
        "Failed to update dmac miss action on device %d:"
        "invalid dmac miss action",
        device);
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  if (attach_meter &&
      dev_ctx->meter_pkt_type_handle[pkt_type] == SWITCH_API_INVALID_HANDLE) {
    SWITCH_MEMSET(&api_meter_info, 0, sizeof(api_meter_info));
    api_meter_info.cbs = api_meter_info.pbs = SWITCH_API_L2_MISS_BURST_SIZE;
    api_meter_info.cir = api_meter_info.pir = SWITCH_API_L2_MISS_RATE_BPS;
    api_meter_info.meter_type = SWITCH_METER_TYPE_BYTES;
    api_meter_info.meter_mode = SWITCH_METER_MODE_TWO_RATE_THREE_COLOR;
    api_meter_info.color_source = SWITCH_METER_COLOR_SOURCE_BLIND;
    api_meter_info.action[SWITCH_METER_COUNTER_GREEN] =
        SWITCH_ACL_ACTION_PERMIT;
    api_meter_info.action[SWITCH_METER_COUNTER_YELLOW] =
        SWITCH_ACL_ACTION_PERMIT;
    api_meter_info.action[SWITCH_METER_COUNTER_RED] = SWITCH_ACL_ACTION_PERMIT;
    status = switch_api_meter_create(device, &api_meter_info, &meter_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_ASSERT(meter_handle != SWITCH_API_INVALID_HANDLE);
    dev_ctx->meter_pkt_type_handle[pkt_type] = meter_handle;

    status = switch_meter_get(device, meter_handle, &meter_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    status = switch_pd_hostif_meter_set(device, meter_handle, meter_info, true);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }
  SWITCH_MEMSET(&opt_action_params, 0, sizeof(opt_action_params));

  if (attach_meter) {
    opt_action_params.meter_handle = dev_ctx->meter_pkt_type_handle[pkt_type];
  }

  if (dev_ctx->ace_pkt_type_handle[pkt_type] == SWITCH_API_INVALID_HANDLE) {
    status = switch_api_acl_rule_create(device,
                                        acl_handle,
                                        priority,
                                        kvp_count,
                                        ing_acl_kvp,
                                        action_type,
                                        &action_params,
                                        &opt_action_params,
                                        &ace_handle);
    dev_ctx->ace_pkt_type_handle[pkt_type] = ace_handle;

  } else {
    status =
        switch_api_acl_entry_action_set(device,
                                        dev_ctx->ace_pkt_type_handle[pkt_type],
                                        priority,
                                        action_type,
                                        &action_params,
                                        &opt_action_params);
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to update dmac miss packet action on device %d: acl rule "
        "action "
        "set failed for acl handle 0x%lx: %s",
        device,
        acl_handle,
        switch_error_to_string(status));
    return status;
  }
  dev_ctx->l2_miss_action[pkt_type] = action_type;

  return status;
}

switch_status_t switch_api_device_mac_aging_interval_set_internal(
    const switch_device_t device, const switch_int32_t aging_interval) {
  switch_device_context_t *device_ctx = NULL;
  switch_l2_context_t *l2_ctx = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_handle_t *mac_handle = NULL;
  switch_pd_hdl_t pd_hdl = 0;
  switch_int32_t tmp_aging_interval = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(aging_interval >= 0);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging interval set failed on device %d: "
        "device context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  device_ctx->device_info.aging_interval = aging_interval;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging interval set failed on device %d mac handle 0x%lx: "
        "l2 context get failed:(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_ARRAY(
      pd_hdl, l2_ctx->smac_pd_hdl_array, switch_handle_t, mac_handle) {
    status = switch_mac_get(device, (switch_handle_t)mac_handle, &mac_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac aging interval set failed on device %d mac handle 0x%lx: "
          "mac get failed:(%s)\n",
          device,
          mac_handle,
          switch_error_to_string(status));
      continue;
    }

    status = switch_bd_aging_interval_get(
        device, mac_info->mac_entry.bd_handle, &tmp_aging_interval);

    if (tmp_aging_interval == aging_interval) continue;
    if (tmp_aging_interval != SWITCH_AGING_INTERVAL_INVALID) continue;

    status = switch_mac_entry_aging_hw_update(
        device, (switch_handle_t)mac_handle, aging_interval);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac aging interval set failed on device %d "
          "mac handle: 0x%lx aging interval %d: "
          "mac entry aging hw update failed:(%s)\n",
          device,
          mac_handle,
          aging_interval,
          switch_error_to_string(status));
      continue;
    }
  }
  FOR_EACH_IN_ARRAY_END();

  return status;
}

switch_status_t switch_api_device_mac_aging_interval_get_internal(
    const switch_device_t device, switch_int32_t *aging_interval) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging interval get failed on device %d: "
        "device context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *aging_interval = device_ctx->device_info.aging_interval;
  return status;
}

switch_status_t switch_api_device_cut_through_mode_set_internal(
    switch_device_t device, bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device cut through mode set failed:"
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (device_ctx->cut_through_mode == enable) {
    SWITCH_LOG_DEBUG("cut through mode is already set");
    return status;
  }
  /*
   * Enable/Disable cut-through only for front panel ports.
   */
  for (switch_uint32_t index = 0; index < device_ctx->device_info.max_ports;
       index++) {
    if (device_ctx->dp_list[index] != SWITCH_PORT_INVALID &&
        index != SWITCH_CPU_PORT_ETH_DEFAULT &&
        index != SWITCH_CPU_PORT_PCIE_DEFAULT &&
        !switch_device_recirc_port(device, index)) {
      status = switch_pd_port_cut_through_set(
          device, device_ctx->dp_list[index], enable);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "cut through mode set failed on device %d, dev_port %d: %s",
            device,
            device_ctx->dp_list[index],
            switch_error_to_string(status));
        return status;
      }
    }
  }
  device_ctx->cut_through_mode = enable;
  return status;
}

switch_status_t switch_api_device_cut_through_mode_get_internal(
    switch_device_t device, bool *enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device cut through mode get failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  *enable = device_ctx->cut_through_mode;
  return status;
}

switch_status_t switch_api_device_tunnel_dmac_set_internal(
    switch_device_t device, switch_mac_addr_t *mac_addr) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device tunnel dmac set failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&device_ctx->device_info.tunnel_dmac,
                mac_addr,
                sizeof(switch_mac_addr_t));

  return status;
}

switch_status_t switch_api_device_tunnel_dmac_get_internal(
    switch_device_t device, switch_mac_addr_t *mac_addr) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device tunnel dmac get failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(mac_addr,
                &device_ctx->device_info.tunnel_dmac,
                sizeof(switch_mac_addr_t));

  return status;
}

switch_status_t switch_api_device_active_ports_get_internal(
    switch_device_t device, switch_uint16_t *num_active_ports) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device active ports get failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  *num_active_ports = device_ctx->device_info.num_active_ports;

  return status;
}

switch_status_t switch_device_active_ports_increment(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device active ports increment failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  device_ctx->device_info.num_active_ports++;
  return status;
}

switch_status_t switch_device_active_ports_decrement(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device active ports decrement failed: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  device_ctx->device_info.num_active_ports--;
  return status;
}

switch_status_t switch_api_device_mac_learning_set_internal(
    switch_device_t device, bool enable) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device mac learning set failed on device: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_l2_mac_learning_set(device, enable);
  device_ctx->device_info.mac_learning = enable;

  return status;
}

switch_status_t switch_api_device_mac_learning_get_internal(
    switch_device_t device, bool *enable) {
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device mac learning get failed on device: "
        "device context get failed on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  *enable = device_ctx->device_info.mac_learning;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_device_attribute_set(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info) {
  SWITCH_MT_WRAP(
      switch_api_device_attribute_set_internal(device, flags, api_device_info))
}

switch_status_t switch_api_device_default_rmac_handle_get(
    switch_device_t device, switch_handle_t *rmac_handle) {
  SWITCH_MT_WRAP(
      switch_api_device_default_rmac_handle_get_internal(device, rmac_handle))
}

switch_status_t switch_api_device_default_vrf_get(switch_device_t device,
                                                  switch_vrf_t *vrf_id,
                                                  switch_handle_t *vrf_handle) {
  SWITCH_MT_WRAP(
      switch_api_device_default_vrf_get_internal(device, vrf_id, vrf_handle))
}

switch_status_t switch_api_device_cpu_pcie_port_get(switch_device_t device,
                                                    switch_port_t *cpu_port) {
  SWITCH_MT_WRAP(switch_api_device_cpu_pcie_port_get_internal(device, cpu_port))
}

switch_status_t switch_api_device_cpu_port_handle_get(
    switch_device_t device, switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(
      switch_api_device_cpu_port_handle_get_internal(device, port_handle))
}

switch_status_t switch_api_device_counter_refresh_interval_set(
    switch_device_t device, switch_uint32_t refresh_interval) {
  SWITCH_MT_WRAP(switch_api_device_counter_refresh_interval_set_internal(
      device, refresh_interval))
}

switch_status_t switch_api_device_cpu_eth_port_get(switch_device_t device,
                                                   switch_port_t *cpu_port) {
  SWITCH_MT_WRAP(switch_api_device_cpu_eth_port_get_internal(device, cpu_port))
}

switch_status_t switch_api_device_counter_refresh_interval_get(
    switch_device_t device, switch_uint32_t *refresh_interval) {
  SWITCH_MT_WRAP(switch_api_device_counter_refresh_interval_get_internal(
      device, refresh_interval))
}

switch_status_t switch_api_device_attribute_get(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info) {
  SWITCH_MT_WRAP(
      switch_api_device_attribute_get_internal(device, flags, api_device_info))
}

switch_status_t switch_api_device_remove(switch_device_t device) {
  return switch_api_device_remove_internal(device);
}

switch_status_t switch_api_device_default_vlan_get(
    switch_device_t device,
    switch_vlan_t *vlan_id,
    switch_handle_t *vlan_handle) {
  SWITCH_MT_WRAP(
      switch_api_device_default_vlan_get_internal(device, vlan_id, vlan_handle))
}

switch_status_t switch_api_device_cpu_port_get(switch_device_t device,
                                               switch_port_t *port) {
  SWITCH_MT_WRAP(switch_api_device_cpu_port_get_internal(device, port))
}

switch_status_t switch_api_device_recirc_port_get(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(
      switch_api_device_recirc_port_get_internal(device, pipe_id, port_handle))
}

switch_status_t switch_api_device_dmac_miss_packet_action_set(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t action_type) {
  SWITCH_MT_WRAP(switch_api_device_dmac_miss_packet_action_set_internal(
      device, pkt_type, action_type))
}

switch_status_t switch_api_device_dmac_miss_packet_action_get(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t *action_type) {
  SWITCH_MT_WRAP(switch_api_device_dmac_miss_packet_action_get_internal(
      device, pkt_type, action_type))
}

switch_status_t switch_api_device_mac_aging_interval_set(
    const switch_device_t device, const switch_int32_t aging_time) {
  SWITCH_MT_WRAP(
      switch_api_device_mac_aging_interval_set_internal(device, aging_time));
}

switch_status_t switch_api_device_mac_aging_interval_get(
    const switch_device_t device, switch_int32_t *aging_time) {
  SWITCH_MT_WRAP(
      switch_api_device_mac_aging_interval_get_internal(device, aging_time));
}

switch_status_t switch_api_device_cut_through_mode_set(switch_device_t device,
                                                       bool enable) {
  SWITCH_MT_WRAP(
      switch_api_device_cut_through_mode_set_internal(device, enable))
}

switch_status_t switch_api_device_cut_through_mode_get(switch_device_t device,
                                                       bool *enable) {
  SWITCH_MT_WRAP(
      switch_api_device_cut_through_mode_get_internal(device, enable))
}

switch_status_t switch_api_device_max_recirc_ports_get(
    switch_device_t device, switch_uint16_t *num_ports) {
  SWITCH_MT_WRAP(
      switch_api_device_max_recirc_ports_get_internal(device, num_ports));
}

switch_status_t switch_api_device_tunnel_dmac_set(switch_device_t device,
                                                  switch_mac_addr_t *mac_addr) {
  SWITCH_MT_WRAP(switch_api_device_tunnel_dmac_set_internal(device, mac_addr));
}

switch_status_t switch_api_device_tunnel_dmac_get(switch_device_t device,
                                                  switch_mac_addr_t *mac_addr) {
  SWITCH_MT_WRAP(switch_api_device_tunnel_dmac_get_internal(device, mac_addr));
}

switch_status_t switch_api_device_active_ports_get(
    switch_device_t device, switch_uint16_t *num_active_ports) {
  SWITCH_MT_WRAP(
      switch_api_device_active_ports_get_internal(device, num_active_ports));
}

switch_status_t switch_api_device_mac_learning_set(switch_device_t device,
                                                   bool enable) {
  SWITCH_MT_WRAP(switch_api_device_mac_learning_set_internal(device, enable));
}

switch_status_t switch_api_device_mac_learning_get(switch_device_t device,
                                                   bool *enable) {
  SWITCH_MT_WRAP(switch_api_device_mac_learning_get_internal(device, enable));
}
