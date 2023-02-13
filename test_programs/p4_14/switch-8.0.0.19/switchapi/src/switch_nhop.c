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

#include "switchapi/switch_nhop.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_NHOP

static switch_status_t switch_api_nhop_mgid_tree_create(
    switch_device_t device, switch_nhop_info_t *nhop_info);

static switch_status_t switch_api_nhop_mgid_tree_delete(
    switch_device_t device, switch_nhop_info_t *nhop_info);

switch_status_t switch_nhop_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_nexthop_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_nhop_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_nhop_hash_key_init(void *args,
                                          switch_uint8_t *key,
                                          switch_uint32_t *len) {
  switch_nhop_key_t *nhop_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  nhop_key = (switch_nhop_key_t *)args;
  *len = 0;

  SWITCH_MEMCPY((key + *len), &nhop_key->handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY((key + *len), &nhop_key->ip_addr, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_ASSERT(*len == SWITCH_NHOP_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_nhop_hash_compare(const void *key1, const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_NHOP_HASH_KEY_SIZE);
}

switch_status_t switch_nhop_table_size_get(switch_device_t device,
                                           switch_size_t *nhop_table_size) {
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_table_id_t table_id = 0;

  SWITCH_ASSERT(nhop_table_size != NULL);

  *nhop_table_size = 0;

  for (table_id = SWITCH_TABLE_NHOP; table_id <= SWITCH_TABLE_ECMP_GROUP;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *nhop_table_size = 0;
      SWITCH_LOG_ERROR(
          "nhop handle size get failed on device %d: %s"
          "for table %s",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }
    *nhop_table_size += table_size;
  }
  return status;
}

switch_status_t switch_api_nhop_table_size_get_internal(
    switch_device_t device, switch_size_t *tbl_size) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop context get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }
  *tbl_size = nhop_ctx->nhop_hashtable.size;
  return status;
}

switch_status_t switch_ecmp_member_handle_init(switch_device_t device) {
  switch_size_t ecmp_select_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  /*
   * Compute the ecmp member  handle array size
   */
  status = switch_api_table_size_get(
      device, SWITCH_TABLE_ECMP_SELECT, &ecmp_select_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp selector table size get failed on device %d: %s"
        "for table %s\n",
        device,
        switch_error_to_string(status),
        switch_table_id_to_string(SWITCH_TABLE_ECMP_SELECT));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_ECMP_MEMBER, ecmp_select_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ecmp member handle init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_nhop_init(switch_device_t device) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_size_t nhop_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  nhop_ctx = SWITCH_MALLOC(device, sizeof(switch_nhop_context_t), 0x1);
  if (!nhop_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("nhop init failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_NHOP, (void *)nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop init failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * Compute the nhop handle array size
   */
  status = switch_nhop_table_size_get(device, &nhop_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  nhop_ctx->nhop_hashtable.size = nhop_table_size;
  nhop_ctx->nhop_hashtable.compare_func = switch_nhop_hash_compare;
  nhop_ctx->nhop_hashtable.key_func = switch_nhop_hash_key_init;
  nhop_ctx->nhop_hashtable.hash_seed = SWITCH_NHOP_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&nhop_ctx->nhop_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop init failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_NHOP, nhop_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_member_handle_init(device);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ecmp member init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_nhop_free(switch_device_t device) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop free failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DONE(&nhop_ctx->nhop_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop free failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_NHOP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop free failed for device %d:",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ECMP_MEMBER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop free failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, nhop_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_NHOP, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_api_nhop_handle_get_internal(
    const switch_device_t device,
    const switch_nhop_key_t *nhop_key,
    switch_handle_t *nhop_handle) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(nhop_key != NULL);
  SWITCH_ASSERT(nhop_handle != NULL);

  if (!nhop_key || !nhop_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("nhop key find failed : %s",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop_key find failed : %s",
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &nhop_ctx->nhop_hashtable, (void *)nhop_key, (void **)&spath_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    *nhop_handle = spath_info->nhop_handle;
  }

  return status;
}

switch_status_t switch_api_neighbor_handle_get_internal(
    const switch_device_t device,
    const switch_handle_t nhop_handle,
    switch_handle_t *neighbor_handle) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "neighbor handle get failed for"
        "device %d handle 0x%lx: %s",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "neighbor handle get failed for"
        "device %d handle 0x%lx: %s",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));
  *neighbor_handle = spath_info->neighbor_handle;

  return status;
}

switch_status_t switch_api_nhop_create_internal(
    const switch_device_t device,
    const switch_api_nhop_info_t *api_nhop_info,
    switch_handle_t *nhop_handle) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_api_rif_info_t *api_rif_info = NULL;
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_mpls_info_t *mpls_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_nhop_key_t nhop_key = {0};
  uint32_t mc_index = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_encap_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_nhop_pd_action_t pd_action = 0;
  switch_api_neighbor_info_t api_neighbor_info = {0};
  switch_uint16_t tunnel_dst_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d: "
        "nhop context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(api_nhop_info != NULL);
  SWITCH_ASSERT(nhop_handle != NULL);
  if (!api_nhop_info || !nhop_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d: "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *nhop_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_NHOP_KEY_GET(api_nhop_info, nhop_key);
  status = switch_api_nhop_handle_get(device, &nhop_key, &handle);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d: "
        "nhop get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "nhop create failed on device %d nhop handle 0x%lx: "
        "nhop already exists\n",
        device,
        handle);
    status = switch_nhop_get(device, handle, &nhop_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    nhop_info->nhop_ref_count++;
    *nhop_handle = handle;
    return status;
  }

  handle = switch_nhop_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d "
        "nhop handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d "
        "nhop get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /* Set mgid state for nhop */
  SET_NHOP_TUNNEL_MGID_STATE(nhop_info, switch_api_nhop_mgid_state_init);
  NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info) = (Pvoid_t)NULL;
  NHOP_TUNNEL_MGID_ROUTE_LIST(nhop_info) = (Pvoid_t)NULL;

  SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info) = 0;
  SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info) = (Pvoid_t)NULL;
  nhop_info->id_type = SWITCH_NHOP_ID_TYPE_ONE_PATH;
  nhop_info->nhop_handle = handle;
  nhop_info->nhop_ref_count = 1;

  spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));
  spath_info->urpf_pd_hdl = SWITCH_PD_INVALID_HANDLE;
  spath_info->nhop_handle = handle;
  spath_info->tunnel = FALSE;

  SWITCH_MEMCPY(&spath_info->api_nhop_info,
                api_nhop_info,
                sizeof(switch_api_nhop_info_t));
  SWITCH_MEMCPY(&spath_info->nhop_key, &nhop_key, sizeof(nhop_key));

  if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_IP) {
    SWITCH_ASSERT(SWITCH_RIF_HANDLE(api_nhop_info->rif_handle));
    rif_handle = api_nhop_info->rif_handle;
    status = switch_rif_get(device, api_nhop_info->rif_handle, &rif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop create failed on device %d "
          "rif get failed:(%s)\n",
          device,
          rif_handle,
          switch_error_to_string(status));
      return status;
    }

    api_rif_info = &rif_info->api_rif_info;
    bd_handle = rif_info->bd_handle;
    spath_info->bd_handle = bd_handle;

    if (SWITCH_RIF_TYPE(rif_info) == SWITCH_RIF_TYPE_INTF) {
      intf_handle = api_rif_info->intf_handle;
      status = switch_interface_get(device, intf_handle, &intf_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

      spath_info->ifindex = intf_info->ifindex;
      spath_info->port_lag_index = intf_info->port_lag_index;
      pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
    } else {
      status = switch_api_hostif_cpu_intf_info_get(device, &intf_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      spath_info->ifindex = intf_info->ifindex;
      spath_info->port_lag_index = intf_info->port_lag_index;
      pd_action = SWITCH_NHOP_PD_ACTION_GLEAN;
    }

    status = switch_pd_urpf_bd_table_entry_add(device,
                                               handle_to_id(handle),
                                               handle_to_id(bd_handle),
                                               &spath_info->urpf_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop create failed on device %d "
          "urpf bd entry add failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  } else if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_TUNNEL) {
    SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(api_nhop_info->tunnel_handle));
    SWITCH_NHOP_TUNNEL_BD_HANDLE_GET(api_nhop_info, bd_handle, status);

    spath_info->tunnel = TRUE;
    pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
    tunnel_handle = api_nhop_info->tunnel_handle;

    status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop create failed on device %d tunnel handle 0x%lx: "
          "tunnel get failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }

    if (SWITCH_TUNNEL_USING_MGID(tunnel_info)) {
      pd_action = SWITCH_NHOP_PD_ACTION_MGID_TUNNEL;
    }

    status = switch_api_tunnel_encap_create(
        device, handle, &nhop_info->tunnel_encap_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop create failed on device %d tunnel handle 0x%lx: "
          "tunnel encap create failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }

    tunnel_encap_handle = nhop_info->tunnel_encap_handle;
    status = switch_tunnel_encap_get(
        device, tunnel_encap_handle, &tunnel_encap_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    tunnel_dst_index = tunnel_encap_info->tunnel_dip_index;
    spath_info->tunnel_dst_index = tunnel_dst_index;

    intf_handle = tunnel_info->intf_handle;
    status = switch_interface_get(device, intf_handle, &intf_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    spath_info->ifindex = intf_info->ifindex;
    spath_info->port_lag_index = intf_info->port_lag_index;
    spath_info->bd_handle = bd_handle;

    SWITCH_MEMSET(&api_neighbor_info, 0x0, sizeof(api_neighbor_info));
    api_neighbor_info.neighbor_type = SWITCH_NEIGHBOR_TYPE_NHOP;
    api_neighbor_info.rw_type = SWITCH_API_NEIGHBOR_RW_TYPE_L2;
    if (api_nhop_info->rewrite_type == SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2) {
      api_neighbor_info.rw_type = SWITCH_API_NEIGHBOR_RW_TYPE_L2;
    } else if (api_nhop_info->rewrite_type ==
               SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR) {
      api_neighbor_info.rw_type = SWITCH_API_NEIGHBOR_RW_TYPE_L2_MIRROR;
    } else if (api_nhop_info->rewrite_type ==
               SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3) {
      api_neighbor_info.rw_type = SWITCH_API_NEIGHBOR_RW_TYPE_L3;
      SWITCH_MEMCPY(&api_neighbor_info.mac_addr,
                    &api_nhop_info->mac_addr,
                    sizeof(switch_mac_addr_t));
    } else if (api_nhop_info->rewrite_type ==
               SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3_VNI) {
      api_neighbor_info.rw_type = SWITCH_API_NEIGHBOR_RW_TYPE_L3_VNI;
      if (SWITCH_MAC_VALID(api_nhop_info->mac_addr)) {
        SWITCH_MEMCPY(&api_neighbor_info.mac_addr,
                      &api_nhop_info->mac_addr,
                      sizeof(switch_mac_addr_t));
      } else {
        status = switch_api_device_tunnel_dmac_get(device,
                                                   &api_neighbor_info.mac_addr);
        SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      }
    }

    api_neighbor_info.nhop_handle = handle;

    status = switch_api_neighbor_create(
        device, &api_neighbor_info, &spath_info->neighbor_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop create failed on device %d tunnel handle 0x%lx: "
          "tunnel neighbor create failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
  } else if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_NONE ||
             api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_GLEAN ||
             api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_DROP) {
    intf_handle = api_nhop_info->intf_handle;
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
    status = switch_interface_get(device, intf_handle, &intf_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    spath_info->ifindex = intf_info->ifindex;
    spath_info->port_lag_index = intf_info->port_lag_index;
    pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
    if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_GLEAN) {
      pd_action = SWITCH_NHOP_PD_ACTION_GLEAN;
    } else if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_DROP) {
      pd_action = SWITCH_NHOP_PD_ACTION_DROP;
    }
  } else if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_MPLS) {
    status = switch_mpls_get(device, api_nhop_info->mpls_handle, &mpls_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    intf_handle = mpls_info->api_mpls_info.intf_handle;
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
    status = switch_interface_get(device, intf_handle, &intf_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    spath_info->ifindex = intf_info->ifindex;
    spath_info->port_lag_index = intf_info->port_lag_index;
    pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
    if (api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_GLEAN) {
      pd_action = SWITCH_NHOP_PD_ACTION_GLEAN;
    }
  } else {
    SWITCH_ASSERT(0);
  }

  status = switch_pd_nexthop_table_entry_add(device,
                                             handle_to_id(handle),
                                             handle_to_id(bd_handle),
                                             spath_info->ifindex,
                                             spath_info->port_lag_index,
                                             pd_action,
                                             mc_index,
                                             tunnel_dst_index,
                                             &spath_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop create failed on device %d nhop handle 0x%lx: "
        "nexthop entry add failed:(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  PWord_t PValue;
  switch_handle_t port_handle;
  switch_port_info_t *port_info = NULL;

  if (SWITCH_RIF_HANDLE(api_nhop_info->rif_handle) &&
      rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_INTF) {
    switch_api_interface_handle_get(
        device, rif_info->api_rif_info.intf_handle, &port_handle);

    if (SWITCH_PORT_HANDLE(port_handle)) {
      status = switch_port_get(device, port_handle, &port_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop create failed on device %d port handle 0x%lx: "
            "port get failed:(%s)\n",
            device,
            port_handle,
            switch_error_to_string(status));
        return status;
      }

      JLI(PValue, port_info->PJLarr_nexthops, handle);
      if (PValue == PJERR) {
        SWITCH_LOG_ERROR(
            "nhop create failed on device %d nhop handle 0x%lx: "
            "judy insert failed:(%s)\n",
            device,
            handle,
            switch_error_to_string(status));
        return SWITCH_STATUS_FAILURE;
      }
    }
  }

  status = SWITCH_HASHTABLE_INSERT(&nhop_ctx->nhop_hashtable,
                                   &(spath_info->node),
                                   (void *)&nhop_key,
                                   (void *)spath_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  *nhop_handle = handle;

  SWITCH_LOG_DEBUG(
      "nhop created on device %d nhop handle 0x%lx\n", device, handle);

  return status;
}

switch_status_t switch_api_nhop_delete_internal(
    const switch_device_t device, const switch_handle_t nhop_handle) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_nhop_key_t nhop_key = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop device context get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop handle invalid:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(nhop_info->id_type == SWITCH_NHOP_ID_TYPE_ONE_PATH);
  if (nhop_info->id_type != SWITCH_NHOP_ID_TYPE_ONE_PATH) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop id type invalid:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (nhop_info->nhop_ref_count > 1) {
    nhop_info->nhop_ref_count--;
    return status;
  }

  spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));
  api_nhop_info = &spath_info->api_nhop_info;
  SWITCH_NHOP_KEY_GET(api_nhop_info, nhop_key);
  if (SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info) > 0) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop id type invalid:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_TUNNEL_ENCAP_HANDLE(nhop_info->tunnel_encap_handle)) {
    status =
        switch_api_tunnel_encap_delete(device, nhop_info->tunnel_encap_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop delete failed on device %d nhop handle 0x%lx: "
          "tunnel encap delete failed:(%s)\n",
          device,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_NHOP_TYPE(nhop_info) == SWITCH_NHOP_TYPE_TUNNEL) {
    status = switch_api_neighbor_delete(device, spath_info->neighbor_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop delete failed on device %d nhop handle 0x%lx: "
          "tunnel neighbor entry delete failed:(%s)\n",
          device,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  int Rc_int;
  switch_handle_t port_handle;
  switch_port_info_t *port_info = NULL;
  switch_rif_info_t *rif_info = NULL;

  if (SWITCH_RIF_HANDLE(spath_info->nhop_key.handle)) {
    status = switch_rif_get(device, spath_info->nhop_key.handle, &rif_info);
    if (rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_INTF) {
      switch_api_interface_handle_get(
          device, rif_info->api_rif_info.intf_handle, &port_handle);

      if (SWITCH_PORT_HANDLE(port_handle)) {
        status = switch_port_get(device, port_handle, &port_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "nhop delete failed on device %d port handle 0x%lx: "
              "port get failed:(%s)\n",
              device,
              port_handle,
              switch_error_to_string(status));
          return status;
        }

        JLD(Rc_int, port_info->PJLarr_nexthops, nhop_handle);
        if (Rc_int != 1) {
          return SWITCH_STATUS_FAILURE;
        }
      }
    }
  }

  status = switch_pd_nexthop_table_entry_delete(device, spath_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop entry delete failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_urpf_bd_table_entry_delete(device, spath_info->urpf_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop delete failed on device %d nhop handle 0x%lx: "
        "nhop urpf bd entry delete failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(
      &nhop_ctx->nhop_hashtable, (void *)(&nhop_key), (void **)&spath_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_nhop_handle_delete(device, nhop_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "nhop deleted on device %d nhop handle 0x%lx\n", device, nhop_handle);

  return status;
}

switch_status_t switch_nhop_ecmp_member_list_add(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t ecmp_mem_handle) {
  PWord_t PValue;

  JLI(PValue, SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info), ecmp_mem_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR(
        "nhop add ecmp member failed on device %d"
        "nhop info 0x%lx: , ecmp mem handle 0x%lx: ",
        device,
        nhop_info,
        ecmp_mem_handle);
    return SWITCH_STATUS_FAILURE;
  }

  SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info) += 1;
  SWITCH_LOG_DEBUG(
      "nhop add ecmp member success on device %d"
      "nhop info 0x%lx: , ecmp mem handle 0x%lx: ref_cnt: %d",
      device,
      nhop_info,
      ecmp_mem_handle,
      SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info));

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_nhop_ecmp_member_list_remove(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t ecmp_mem_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  int Rc_int;
  JLD(Rc_int, SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info), ecmp_mem_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR(
        "nhop remove ecmp mem failed on device %d"
        "nhop info 0x%lx: , ecmp mem handle 0x%lx: ",
        device,
        nhop_info,
        ecmp_mem_handle);
    return SWITCH_STATUS_FAILURE;
  }

  SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info) -= 1;
  SWITCH_LOG_DEBUG(
      "nhop remove ecmp mem success on device %d"
      "nhop info 0x%lx: , ecmp mem handle 0x%lx: ref_cnt: %d",
      device,
      nhop_info,
      ecmp_mem_handle,
      SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop_info));

  return status;
}

switch_status_t switch_nhop_ecmp_members_update(switch_device_t device,
                                                switch_nhop_info_t *nhop_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t ecmp_member_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ecmp_handle = SWITCH_API_INVALID_HANDLE;
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_spath_info_t *spath_info = NULL;

  /*
   * walk through the list of ecmp members and
   * update the ecmp member information.
   */
  JLF(PValue, SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info), ecmp_member_handle);
  while (PValue != NULL) {
    /*
     * Get ecmp group and memeber pd handle and trigger pd modify API.
     */
    status = switch_ecmp_member_get(device, ecmp_member_handle, &ecmp_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "failed to get ecmp member info, update failed, device %d",
          device,
          switch_error_to_string(status));
      return status;
    }

    ecmp_handle = ecmp_member->ecmp_handle;
    if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "failed to update ecmp member,  device %d: "
          " ecmp_handle: %s",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "failed to get ecmp info,"
          " update ecmp-member failed, device: %d: "
          " ecmp handle: 0x%lx reason: %s",
          device,
          ecmp_handle,
          switch_error_to_string(status));
      return status;
    }
    mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));
    spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));

    status =
        switch_pd_ecmp_member_update(device,
                                     mpath_info->pd_group_hdl,
                                     handle_to_id(ecmp_member->nhop_handle),
                                     spath_info,
                                     ecmp_member->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ecmp memeber pd update failed on device %d",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    SWITCH_LOG_DEBUG(
        " update ecmp-member successful on device: %d: "
        " ecmp-group handle: 0x%lx  ecmp_mem_handle: 0x%lx",
        device,
        ecmp_handle,
        ecmp_member_handle);
    JLN(PValue,
        SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info),
        ecmp_member_handle);
  }

  return status;
}

switch_status_t switch_nhop_ecmp_members_deactivate(
    switch_device_t device, switch_nhop_info_t *nhop_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t ecmp_member_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ecmp_handle = SWITCH_API_INVALID_HANDLE;
  switch_ecmp_member_t *ecmp_member = NULL;

  // walk through the list of ecmp members and deactivate corresponding ecmp
  // members.

  JLF(PValue, SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info), ecmp_member_handle);
  while (PValue != NULL) {
    // Get ecmp group and memeber pd handle and trigger pd modify API.
    status = switch_ecmp_member_get(device, ecmp_member_handle, &ecmp_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "failed to get ecmp member info, update failed, device %d",
          device,
          switch_error_to_string(status));
      return status;
    }

    ecmp_handle = ecmp_member->ecmp_handle;
    if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "failed to deactivate ecmp member,  device %d: "
          "ecmp_handle: %s",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_ecmp_member_activate(
        device, ecmp_handle, 1, &nhop_info->nhop_handle, FALSE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ecmp memeber pd update failed on device %d",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    SWITCH_LOG_DEBUG(
        "deactivate ecmp-member successful on device: %d: "
        "ecmp-group handle: 0x%lx  ecmp_mem_handle: 0x%lx",
        device,
        ecmp_handle,
        ecmp_member_handle);
    JLN(PValue,
        SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop_info),
        ecmp_member_handle);
  }

  return status;
}

switch_status_t switch_ecmp_member_get_from_nhop(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    const switch_handle_t nhop_handle,
    switch_ecmp_member_t **ecmp_member) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *tmp_ecmp_member = NULL;
  switch_node_t *node = NULL;
  bool member_found = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(ecmp_member != NULL);
  if (!ecmp_member) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to get ecmp member on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to get ecmp member on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to get ecmp member on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get ecmp member on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  FOR_EACH_IN_LIST(mpath_info->members, node) {
    tmp_ecmp_member = (switch_ecmp_member_t *)node->data;
    if (tmp_ecmp_member->nhop_handle == nhop_handle) {
      member_found = TRUE;
      *ecmp_member = tmp_ecmp_member;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!member_found) {
    *ecmp_member = NULL;
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
  }

  return status;
}

switch_status_t switch_api_ecmp_create_internal(const switch_device_t device,
                                                switch_handle_t *ecmp_handle) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  handle = switch_ecmp_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "ecmp create failed on device %d "
        "handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp create failed on device %d "
        "ecmp get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  ecmp_info->id_type = SWITCH_NHOP_ID_TYPE_ECMP;
  ecmp_info->nhop_handle = handle;

  SWITCH_NHOP_NUM_ECMP_MEMBER_REF(ecmp_info) = 0;
  SWITCH_NHOP_ECMP_MEMBER_REF_LIST(ecmp_info) = (Pvoid_t)NULL;

  /* Set mgid state for nhop */
  SET_NHOP_TUNNEL_MGID_STATE(ecmp_info, switch_api_nhop_mgid_state_init);
  NHOP_TUNNEL_MGID_TUNNEL_LIST(ecmp_info) = (Pvoid_t)NULL;
  NHOP_TUNNEL_MGID_ROUTE_LIST(ecmp_info) = (Pvoid_t)NULL;

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));
  SWITCH_LIST_INIT(&(mpath_info->members));

  status = switch_pd_ecmp_group_create(device, &(mpath_info->pd_group_hdl));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp create failed on device %d "
        "ecmp pd group create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ecmp_group_register_callback(device, (void *)handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp create failed on device %d "
        "ecmp pd register cb failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *ecmp_handle = handle;

  SWITCH_LOG_DEBUG("ecmp handle created on device %d ecmp handle 0x%lx\n",
                   device,
                   ecmp_handle);

  return status;
}

switch_status_t switch_api_ecmp_create_with_members_internal(
    const switch_device_t device,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles,
    switch_handle_t *ecmp_handle,
    switch_handle_t *member_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(num_nhops != 0);
  SWITCH_ASSERT(nhop_handles != NULL);
  SWITCH_ASSERT(ecmp_handle != NULL);

  if (num_nhops == 0 || !nhop_handles || !ecmp_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ecmp create members failed on device %d "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_ecmp_create(device, ecmp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp create members failed on device %d "
        "ecmp handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_ecmp_member_add(
      device, *ecmp_handle, num_nhops, nhop_handles, member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp create members failed on device %d "
        "ecmp member add failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("ecmp created on device %d handle 0x%lx num nhops %d\n",
                   device,
                   *ecmp_handle,
                   num_nhops);

  return status;
}

switch_status_t switch_api_ecmp_member_add_internal(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles,
    switch_handle_t *member_handle) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_handle_t ecmp_member_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = 0;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(num_nhops != 0);
  SWITCH_ASSERT(nhop_handles != NULL);

  if (num_nhops == 0 || !nhop_handles || !ecmp_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ecmp member add failed on device %d ecmp handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member add failed on device %d ecmp handle 0x%lx: "
        "ecmp get failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_NHOP_TUNNEL_MGID_EVENT(
      ecmp_info, device, SWITCH_NHOP_MGID_TREE_DELETE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member add failed on device %d ecmp handle 0x%lx: "
        "mgid tree delete event send failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  for (index = 0; index < num_nhops; index++) {
    nhop_handle = nhop_handles[index];
    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "ecmp member add failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "nhop get failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));
    api_nhop_info = &spath_info->api_nhop_info;

    ecmp_member_handle = switch_ecmp_member_handle_create(device);
    if (ecmp_member_handle == SWITCH_API_INVALID_HANDLE) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "ecmp member add failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "ecmp member create failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_ecmp_member_get(device, ecmp_member_handle, &ecmp_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member add failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "ecmp member get failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_ECMP_MEMBER_INIT(ecmp_member);
    ecmp_member->member_handle = ecmp_member_handle;
    ecmp_member->ecmp_handle = ecmp_handle;
    ecmp_member->nhop_handle = nhop_handle;
    ecmp_member->active = TRUE;

    status = SWITCH_LIST_INSERT(
        &(mpath_info->members), &(ecmp_member->node), ecmp_member);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    status =
        switch_nhop_ecmp_member_list_add(device, nhop_info, ecmp_member_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "failed to add ecmp member_handle to nhop , device %d"
          " nhop info %lx: "
          " ecmp_member_handle: %lx"
          " with status (%s)\n",
          device,
          nhop_info,
          ecmp_member_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_ecmp_member_add(device,
                                       mpath_info->pd_group_hdl,
                                       handle_to_id(ecmp_member->nhop_handle),
                                       spath_info,
                                       &(ecmp_member->mbr_hdl));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member add failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "pd ecmp member add failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    if (SWITCH_RIF_HANDLE(api_nhop_info->rif_handle)) {
      status =
          switch_pd_urpf_bd_table_entry_add(device,
                                            handle_to_id(ecmp_handle),
                                            handle_to_id(spath_info->bd_handle),
                                            &(ecmp_member->urpf_pd_hdl));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "ecmp member add failed on device %d ecmp handle 0x%lx "
            "nhop handle 0x%lx: "
            "urpf bd add failed:(%s)\n",
            device,
            ecmp_handle,
            nhop_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    if (SWITCH_LIST_COUNT(&mpath_info->members) == 1) {
      status = switch_pd_ecmp_group_table_with_selector_add(
          device,
          handle_to_id(ecmp_handle),
          mpath_info->pd_group_hdl,
          &(mpath_info->hw_entry));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "ecmp member add failed on device %d ecmp handle 0x%lx "
            "nhop handle 0x%lx: "
            "ecmp selector add failed:(%s)\n",
            device,
            ecmp_handle,
            nhop_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    SWITCH_LOG_DETAIL(
        "ecmp member added on device %d ecmp handle 0x%lx "
        "nhop handle 0x%lx member handle 0x%lx\n",
        device,
        ecmp_handle,
        nhop_handle,
        ecmp_member_handle);
  }

  status = SEND_NHOP_TUNNEL_MGID_EVENT(
      ecmp_info, device, SWITCH_NHOP_MGID_TREE_CREATE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member add failed on device %d ecmp handle 0x%lx: "
        "mgid tree create event send failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ecmp member add on device %d ecmp handle 0x%lx num nhops %d\n",
      device,
      ecmp_handle,
      num_nhops);

  return status;
}

switch_status_t switch_api_ecmp_delete_internal(
    const switch_device_t device, const switch_handle_t ecmp_handle) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_node_t *node = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_handle_t *nhop_handles = NULL;
  switch_uint32_t num_nhops = 0;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp delete failed on device %d ecmp handle 0x%lx: "
        "ecmp handle invalid:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp delete failed on device %d ecmp handle 0x%lx: "
        "ecmp get failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (ecmp_info->id_type != SWITCH_NHOP_ID_TYPE_ECMP) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp delete failed on device %d ecmp handle 0x%lx: "
        "handle type not ecmp:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  num_nhops = SWITCH_LIST_COUNT(&mpath_info->members);

  if (num_nhops) {
    nhop_handles = SWITCH_MALLOC(device, sizeof(switch_handle_t), num_nhops);
    if (!nhop_handles) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "ecmp delete failed on device %d ecmp handle 0x%lx: "
          "memory allocation failed:(%s)\n",
          device,
          ecmp_handle,
          switch_error_to_string(status));
      return status;
    }

    FOR_EACH_IN_LIST(mpath_info->members, node) {
      ecmp_member = (switch_ecmp_member_t *)node->data;
      nhop_handles[index++] = ecmp_member->nhop_handle;
    }
    FOR_EACH_IN_LIST_END();

    status = switch_api_ecmp_member_delete(
        device, ecmp_handle, num_nhops, nhop_handles);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp delete failed on device %d ecmp handle 0x%lx: "
          "ecmp member delete failed:(%s)\n",
          device,
          ecmp_handle,
          switch_error_to_string(status));
      SWITCH_FREE(device, nhop_handles);
      return status;
    }
    SWITCH_FREE(device, nhop_handles);
  }

  status = switch_pd_ecmp_group_delete(device, mpath_info->pd_group_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp delete failed on device %d ecmp handle 0x%lx: "
        "ecmp group delete failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_handle_delete(device, ecmp_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("ecmp handle deleted on device %d ecmp handle 0x%lx\n",
                   device,
                   ecmp_handle);

  return status;
}

switch_status_t switch_api_ecmp_member_delete_internal(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_handle_t nhop_handle = 0;
  switch_handle_t member_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t index = 0;

  SWITCH_ASSERT(num_nhops != 0);
  SWITCH_ASSERT(nhop_handles != NULL);

  if (num_nhops == 0 || !nhop_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ecmp member delete failed on device %d ecmp handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ecmp member delete failed on device %d ecmp handle 0x%lx: "
        "ecmp handle invalid:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member delete failed on device %d ecmp handle 0x%lx: "
        "ecmp get failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  status = SEND_NHOP_TUNNEL_MGID_EVENT(
      ecmp_info, device, SWITCH_NHOP_MGID_TREE_DELETE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member delete failed on device %d ecmp handle 0x%lx: "
        "ecmp nhop mgid tree delete event send failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_nhops; index++) {
    nhop_handle = nhop_handles[index];

    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
    if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "ecmp member delete failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "nhop handle invalid:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member delete failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "nhop get failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));
    api_nhop_info = &spath_info->api_nhop_info;

    status = switch_ecmp_member_get_from_nhop(
        device, ecmp_handle, nhop_handle, &ecmp_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member delete failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "ecmp member from nhop failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));

    if (SWITCH_LIST_COUNT(&mpath_info->members) == 1) {
      status =
          switch_pd_ecmp_group_table_entry_delete(device, mpath_info->hw_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "ecmp member delete failed on device %d ecmp handle 0x%lx "
            "nhop handle 0x%lx: "
            "ecmp group delete failed:(%s)\n",
            device,
            ecmp_handle,
            nhop_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    status = switch_pd_ecmp_member_delete(
        device, mpath_info->pd_group_hdl, ecmp_member->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member delete failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "ecmp member delete failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    if (SWITCH_RIF_HANDLE(api_nhop_info->rif_handle)) {
      status = switch_pd_urpf_bd_table_entry_delete(device,
                                                    ecmp_member->urpf_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "ecmp member delete failed on device %d ecmp handle 0x%lx "
            "nhop handle 0x%lx: "
            "urpf bd member delete failed:(%s)\n",
            device,
            ecmp_handle,
            nhop_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    member_handle = ecmp_member->member_handle;
    status =
        switch_nhop_ecmp_member_list_remove(device, nhop_info, member_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp member delete failed on device %d ecmp handle 0x%lx "
          "nhop handle 0x%lx: "
          "ecmp member list remove failed:(%s)\n",
          device,
          ecmp_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_LIST_DELETE(&(mpath_info->members), &(ecmp_member->node));
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    status = switch_ecmp_member_handle_delete(device, member_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    SWITCH_LOG_DETAIL(
        "ecmp member deleted on device %d ecmp handle 0x%lx "
        "nhop handle 0x%lx member handle 0x%lx\n",
        device,
        ecmp_handle,
        nhop_handle,
        member_handle);
  }

  status = SEND_NHOP_TUNNEL_MGID_EVENT(
      ecmp_info, device, SWITCH_NHOP_MGID_TREE_CREATE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp member delete failed on device %d ecmp handle 0x%lx: "
        "ecmp nhop mgid tree create event send failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ecmp member deleted on device %d ecmp handle 0x%lx num nhops %d\n",
      device,
      ecmp_handle,
      num_nhops);

  return status;
}

switch_status_t switch_api_ecmp_members_delete_internal(
    switch_device_t device, switch_handle_t ecmp_handle) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_node_t *node = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_handle_t *nhop_handles = NULL;
  switch_uint32_t num_nhops = 0;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp members delete failed on device %d ecmp handle 0x%lx: "
        "ecmp handle invalid:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp members delete failed on device %d ecmp handle 0x%lx: "
        "ecmp get failed:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (ecmp_info->id_type != SWITCH_NHOP_ID_TYPE_ECMP) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp members delete failed on device %d ecmp handle 0x%lx: "
        "handle type not ecmp:(%s)\n",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  num_nhops = SWITCH_LIST_COUNT(&mpath_info->members);

  if (num_nhops) {
    nhop_handles = SWITCH_MALLOC(device, sizeof(switch_handle_t), num_nhops);
    if (!nhop_handles) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "ecmp members delete failed on device %d ecmp handle 0x%lx: "
          "nhop handles malloc failed:(%s)\n",
          device,
          ecmp_handle,
          switch_error_to_string(status));
      return status;
    }

    FOR_EACH_IN_LIST(mpath_info->members, node) {
      ecmp_member = (switch_ecmp_member_t *)node->data;
      nhop_handles[index++] = ecmp_member->nhop_handle;
    }
    FOR_EACH_IN_LIST_END();

    status = switch_api_ecmp_member_delete(
        device, ecmp_handle, num_nhops, nhop_handles);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ecmp members delete failed on device %d ecmp handle 0x%lx: "
          "ecmp member delete failed:(%s)\n",
          device,
          ecmp_handle,
          switch_error_to_string(status));
      SWITCH_FREE(device, nhop_handles);
      return status;
    }
    SWITCH_FREE(device, nhop_handles);
  }

  SWITCH_LOG_DEBUG("ecmp members deleted on device %d ecmp handle 0x%lx\n",
                   device,
                   ecmp_handle);

  return status;
}

switch_status_t switch_ecmp_member_activate(switch_device_t device,
                                            switch_handle_t ecmp_handle,
                                            switch_uint16_t num_nhops,
                                            switch_handle_t *nhop_handles,
                                            bool activate) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_handle_t nhop_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t index = 0;

  SWITCH_ASSERT(num_nhops != 0);
  SWITCH_ASSERT(nhop_handles != NULL);

  if (num_nhops == 0 || !nhop_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  for (index = 0; index < num_nhops; index++) {
    nhop_handle = nhop_handles[index];

    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
    if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_ecmp_member_get_from_nhop(
        device, ecmp_handle, nhop_handle, &ecmp_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (!ecmp_member->active && activate) {
      status = switch_pd_ecmp_member_activate(
          device, mpath_info->pd_group_hdl, &(ecmp_member->mbr_hdl));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    } else if (ecmp_member->active && !activate) {
      status = switch_pd_ecmp_member_deactivate(
          device, mpath_info->pd_group_hdl, &(ecmp_member->mbr_hdl));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("ecmp member activate failed on device %d: %s",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }
    ecmp_member->active = activate;
  }
  return status;

cleanup:
  return status;
}

switch_status_t switch_api_ecmp_member_activate_internal(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    switch_uint16_t num_nhops,
    switch_handle_t *nhop_handles) {
  return switch_ecmp_member_activate(
      device, ecmp_handle, num_nhops, nhop_handles, TRUE);
}

switch_status_t switch_api_ecmp_member_deactivate_internal(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    switch_uint16_t num_nhops,
    switch_handle_t *nhop_handles) {
  return switch_ecmp_member_activate(
      device, ecmp_handle, num_nhops, nhop_handles, FALSE);
}

switch_status_t switch_api_wcmp_create_internal(switch_device_t device,
                                                switch_handle_t *wcmp_handle) {
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *wcmp_handle = switch_wcmp_handle_create(device);
  if (*wcmp_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("failed to create wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, *wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to create wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  wcmp_info->id_type = SWITCH_NHOP_ID_TYPE_WCMP;
  SWITCH_NHOP_NUM_ECMP_MEMBER_REF(wcmp_info) = 0;
  SWITCH_NHOP_ECMP_MEMBER_REF_LIST(wcmp_info) = (Pvoid_t)NULL;

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));
  SWITCH_LIST_INIT(&(mpath_info->members));

  /* Set mgid state for nhop */
  SET_NHOP_TUNNEL_MGID_STATE(wcmp_info, switch_api_nhop_mgid_state_init);
  NHOP_TUNNEL_MGID_TUNNEL_LIST(wcmp_info) = (Pvoid_t)NULL;
  NHOP_TUNNEL_MGID_ROUTE_LIST(wcmp_info) = (Pvoid_t)NULL;

  status = switch_pd_wcmp_group_create(device,
                                       handle_to_id(*wcmp_handle),
                                       &(mpath_info->mbr_hdl),
                                       &(mpath_info->hw_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to create wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_wcmp_delete_internal(switch_device_t device,
                                                switch_handle_t wcmp_handle) {
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_wcmp_member_t *wcmp_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WCMP_HANDLE(wcmp_handle));
  if (!SWITCH_WCMP_HANDLE(wcmp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to delete wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to delete wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));
  FOR_EACH_IN_LIST(mpath_info->members, node) {
    wcmp_member = (switch_wcmp_member_t *)node->data;
    UNUSED(wcmp_member);
  }
  FOR_EACH_IN_LIST_END();

  status = switch_pd_wcmp_group_delete(
      device, mpath_info->mbr_hdl, mpath_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to delete wcmp on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_handle_delete(device, wcmp_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_wcmp_range_update(switch_device_t device,
                                         switch_handle_t wcmp_handle) {
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_wcmp_member_t *wcmp_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t sum_weights = 0;
  switch_uint8_t int_weight = 0;
  switch_uint8_t range_start = 0;
  switch_pd_hdl_t entry_hdl = SWITCH_PD_INVALID_HANDLE;
  double error = 0;

  SWITCH_ASSERT(SWITCH_WCMP_HANDLE(wcmp_handle));
  if (!SWITCH_WCMP_HANDLE(wcmp_handle)) {
    SWITCH_LOG_ERROR("wcmp range update failed on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wcmp range update failed on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));

  FOR_EACH_IN_LIST(mpath_info->members, node) {
    wcmp_member = (switch_wcmp_member_t *)node->data;
    sum_weights += wcmp_member->weight;
  }
  FOR_EACH_IN_LIST_END();

  FOR_EACH_IN_LIST(mpath_info->members, node) {
    wcmp_member = (switch_wcmp_member_t *)node->data;
    // Adjust and round the weights to integer values
    int_weight = wcmp_member->weight * MAX_WCMP_WEIGHT / sum_weights;
    if (error > 0 && wcmp_member->weight != 0) {
      int_weight += 1;
    }
    error += (wcmp_member->weight * MAX_WCMP_WEIGHT / sum_weights) - int_weight;

    status = switch_nhop_get(device, wcmp_member->nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("failed to delete ecmp member on device %d: %s",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));

    entry_hdl = wcmp_member->hw_entry;
    wcmp_member->hw_entry = SWITCH_PD_INVALID_HANDLE;

    if (int_weight != 0) {
      status = switch_pd_wcmp_member_add(device,
                                         handle_to_id(wcmp_member->nhop_handle),
                                         handle_to_id(wcmp_handle),
                                         range_start,
                                         range_start + int_weight - 1,
                                         spath_info,
                                         &(wcmp_member->hw_entry));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("wcmp range update failed on device %d",
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }

    if (entry_hdl != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_wcmp_member_delete(device, entry_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("wcmp range update failed on device %d",
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }
    range_start += int_weight;
  }
  FOR_EACH_IN_LIST_END();

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_wcmp_member_add_internal(
    switch_device_t device,
    switch_handle_t wcmp_handle,
    switch_uint16_t nhop_count,
    switch_handle_t *nhop_handles,
    switch_uint16_t *nhop_weights) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_wcmp_member_t *wcmp_member = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t nhop_weight = 0;
  switch_uint16_t count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WCMP_HANDLE(wcmp_handle));
  if (!SWITCH_WCMP_HANDLE(wcmp_handle)) {
    SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));

  for (count = 0; count < nhop_count; count++) {
    nhop_handle = nhop_handles[count];
    nhop_weight = nhop_weights[count];
    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));

    if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    wcmp_member = SWITCH_MALLOC(device, sizeof(switch_wcmp_member_t), 0x1);
    SWITCH_MEMSET(wcmp_member, 0x0, sizeof(switch_wcmp_member_t));

    wcmp_member->nhop_handle = nhop_handle;
    wcmp_member->weight = nhop_weight;
    wcmp_member->hw_entry = SWITCH_PD_INVALID_HANDLE;

    status = SWITCH_LIST_INSERT(
        &mpath_info->members, &wcmp_member->node, wcmp_member);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  status = switch_wcmp_range_update(device, wcmp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to add wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_wcmp_member_modify_internal(
    switch_device_t device,
    switch_handle_t wcmp_handle,
    switch_uint16_t nhop_count,
    switch_handle_t *nhop_handles,
    switch_uint16_t *nhop_weights) {
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_wcmp_member_t *wcmp_member = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_node_t *node = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t nhop_weight = 0;
  switch_uint16_t count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WCMP_HANDLE(wcmp_handle));
  if (!SWITCH_WCMP_HANDLE(wcmp_handle)) {
    SWITCH_LOG_ERROR("failed to modify wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to modify wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));

  for (count = 0; count < nhop_count; count++) {
    nhop_handle = nhop_handles[count];
    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));

    nhop_weight = nhop_weights[count];
    wcmp_member = NULL;
    FOR_EACH_IN_LIST(mpath_info->members, node) {
      wcmp_member = (switch_wcmp_member_t *)node->data;
      if (wcmp_member->nhop_handle == nhop_handle) {
        break;
      }
    }
    FOR_EACH_IN_LIST_END();

    if (!wcmp_member) {
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      SWITCH_LOG_ERROR("failed to modify wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    wcmp_member->weight = nhop_weight;
  }

  status = switch_wcmp_range_update(device, wcmp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to modify wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_wcmp_member_delete_internal(
    switch_device_t device,
    switch_handle_t wcmp_handle,
    switch_uint16_t nhop_count,
    switch_handle_t *nhop_handles) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_wcmp_info_t *wcmp_info = NULL;
  switch_wcmp_member_t *wcmp_member = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_node_t *node = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WCMP_HANDLE(wcmp_handle));
  if (!SWITCH_WCMP_HANDLE(wcmp_handle)) {
    SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wcmp_get(device, wcmp_handle, &wcmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_WCMP_MPATH_INFO(wcmp_info));

  for (count = 0; count < nhop_count; count++) {
    nhop_handle = nhop_handles[count];
    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));

    if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    wcmp_member = NULL;
    FOR_EACH_IN_LIST(mpath_info->members, node) {
      wcmp_member = (switch_wcmp_member_t *)node->data;
      if (wcmp_member->nhop_handle == nhop_handle) {
        break;
      }
    }
    FOR_EACH_IN_LIST_END();

    if (!wcmp_member) {
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (wcmp_member->hw_entry != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_wcmp_member_delete(device, wcmp_member->hw_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    status = SWITCH_LIST_DELETE(&mpath_info->members, &wcmp_member->node);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    SWITCH_FREE(device, wcmp_member);
  }

  status = switch_wcmp_range_update(device, wcmp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to delete wcmp member on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_nhop_id_type_get_internal(
    const switch_device_t device,
    const switch_handle_t nhop_handle,
    switch_nhop_id_type_t *nhop_type) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  SWITCH_ASSERT(nhop_type != NULL);

  *nhop_type = SWITCH_NHOP_ID_TYPE_NONE;

  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("failed to get nhop type on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get nhop type on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *nhop_type = nhop_info->id_type;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_nhop_get_internal(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_api_nhop_info_t *api_nhop_info) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop get failed on device %d nhop handle 0x%lx\n",
                     "nhop get failed:(%s)\n",
                     device,
                     nhop_handle,
                     switch_error_to_string(status));
    return status;
  }

  spath_info = &nhop_info->spath;
  SWITCH_MEMCPY(api_nhop_info,
                &spath_info->api_nhop_info,
                sizeof(switch_api_nhop_info_t));

  return status;
}

switch_status_t switch_nhop_update(switch_device_t device,
                                   switch_handle_t nhop_handle,
                                   switch_ifindex_t ifindex,
                                   switch_port_lag_index_t port_lag_index,
                                   switch_nhop_pd_action_t pd_action) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_mgid_t mc_index = 0;
  switch_tunnel_t tunnel_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "nhop ifindex update failed on device %d "
        "nhop handle 0x%lx ifindex %x: "
        "nhop handle invalid(%s)\n",
        device,
        nhop_handle,
        ifindex,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop ifindex update failed on device %d "
        "nhop handle 0x%lx ifindex %x: "
        "nhop get failed(%s)\n",
        device,
        nhop_handle,
        ifindex,
        switch_error_to_string(status));
    return status;
  }

  spath_info = &(SWITCH_NHOP_SPATH_INFO(nhop_info));

  status = switch_bd_get(device, spath_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop ifindex update failed on device %d "
        "nhop handle 0x%lx ifindex %x: "
        "bd get failed(%s)\n",
        device,
        nhop_handle,
        ifindex,
        switch_error_to_string(status));
    return status;
  }
  mc_index = handle_to_id(bd_info->flood_handle);
  spath_info->ifindex = ifindex;
  spath_info->port_lag_index = port_lag_index;

  status =
      switch_pd_nexthop_table_entry_update(device,
                                           handle_to_id(nhop_handle),
                                           handle_to_id(spath_info->bd_handle),
                                           spath_info->ifindex,
                                           spath_info->port_lag_index,
                                           pd_action,
                                           mc_index,
                                           tunnel_index,
                                           spath_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop ifindex update failed on device %d "
        "nhop handle 0x%lx ifindex %x: "
        "interface get failed(%s)\n",
        device,
        nhop_handle,
        ifindex,
        switch_error_to_string(status));
    return status;
  }

  /*
   * update all the ecmp members using this next-hop.
   */

  status = switch_nhop_ecmp_members_update(device, nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop ecmp member update failed on device %d "
        "nhop handle 0x%lx ifindex %x: port_lag_index: %x"
        " with sttus(%s)\n",
        device,
        nhop_handle,
        ifindex,
        port_lag_index,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "nhop update successful on device %d "
      "nhop handle 0x%lx ifindex %x: port_lag_index: %x",
      device,
      nhop_handle,
      ifindex,
      port_lag_index);

  return status;
}

switch_status_t switch_nhop_l3_vlan_interface_resolve(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t bd_handle,
    switch_mac_addr_t *mac_addr,
    bool neighbor_deleted) {
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_neighbor_dmac_entry_t neighbor_entry;
  switch_mac_entry_t mac_entry;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_ifindex_t ifindex = 0;
  switch_port_lag_index_t port_lag_index = 0;
  switch_interface_info_t *cpu_intf_info = NULL;
  bool neighbor_found = FALSE;
  bool mac_found = FALSE;
  switch_node_t *node = NULL;
  switch_nhop_pd_action_t pd_action = 0;
  switch_neighbor_nhop_list_t *nhop_list = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "l3 vlan interface resolve failed on device %d "
        "bd handle 0x%lx: "
        "bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 vlan interface resolve failed on device %d "
        "bd handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (bd_info->bd_type != SWITCH_BD_TYPE_L3) {
    return status;
  }

  SWITCH_MEMSET(&mac_entry, 0x0, sizeof(mac_entry));
  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, mac_addr, sizeof(switch_mac_addr_t));
  status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
    mac_found = TRUE;
    status = switch_mac_get(device, mac_handle, &mac_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 vlan interface resolve failed on device %d "
          "bd handle 0x%lx mac handle 0x%lx: "
          "mac get failed(%s)\n",
          device,
          bd_handle,
          mac_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  /* If the neighbor is deleted, just update the affected nhop */
  if (SWITCH_NHOP_HANDLE(nhop_handle) && neighbor_deleted) {
    status = switch_nhop_update(
        device, nhop_handle, ifindex, port_lag_index, pd_action);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 vlan interface resolve failed on device %d "
          "bd handle 0x%lx: "
          "nhop update failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    return status;
  }

  SWITCH_MEMSET(&neighbor_entry, 0x0, sizeof(neighbor_entry));
  neighbor_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&neighbor_entry.mac, mac_addr, sizeof(switch_mac_addr_t));
  status =
      switch_neighbor_entry_nhop_list_get(device, &neighbor_entry, &nhop_list);
  if (status == SWITCH_STATUS_SUCCESS) {
    neighbor_found = TRUE;
  }

  if (neighbor_found && mac_found) {
    ifindex = mac_info->ifindex;
    port_lag_index = mac_info->port_lag_index;
    pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
  } else if (neighbor_found) {
    ifindex = 0;
    port_lag_index = 0;
    pd_action = SWITCH_NHOP_PD_ACTION_FLOOD;
  } else {
    status = switch_api_hostif_cpu_intf_info_get(device, &cpu_intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 vlan interface resolve failed on device %d "
          "bd handle 0x%lx: "
          "cpu ifindex get failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    ifindex = cpu_intf_info->ifindex;
    port_lag_index = cpu_intf_info->port_lag_index;
    pd_action = SWITCH_NHOP_PD_ACTION_NON_TUNNEL;
  }

  if (neighbor_found && nhop_list) {
    FOR_EACH_IN_LIST(nhop_list->list, node) {
      neighbor_info = (switch_neighbor_info_t *)node->data;
      if (SWITCH_NHOP_HANDLE(neighbor_info->nhop_handle)) {
        status = switch_nhop_update(device,
                                    neighbor_info->nhop_handle,
                                    ifindex,
                                    port_lag_index,
                                    pd_action);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "l3 vlan interface resolve failed on device %d "
              "bd handle 0x%lx: "
              "nhop update get failed(%s)\n",
              device,
              bd_handle,
              switch_error_to_string(status));
          return status;
        }
      }
    }
    FOR_EACH_IN_LIST_END();
  }

  return status;
}

switch_status_t switch_api_ecmp_members_get_internal(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    switch_uint16_t *num_nhops,
    switch_handle_t **nhop_handles) {
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_node_t *node = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_handle));
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp members get failed on device %d "
        "ecmp handle 0x%lx: invalid ecmp handle(%s)",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ecmp_get(device, ecmp_handle, &ecmp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ecmp members get failed on device %d "
        "ecmp handle 0x%lx: ecmp get failed(%s)",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (ecmp_info->id_type != SWITCH_NHOP_ID_TYPE_ECMP) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ecmp members get failed on device %d "
        "ecmp handle 0x%lx: handle type not ecmp: %s",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  *num_nhops = SWITCH_LIST_COUNT(&mpath_info->members);
  if (!(*num_nhops)) {
    *nhop_handles = NULL;
    return status;
  }

  *nhop_handles = SWITCH_MALLOC(device, sizeof(switch_handle_t), *num_nhops);
  if (!(*nhop_handles)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "ecmp members get failed on device %d "
        "ecmp handle 0x%lx: handle type not ecmp: %s",
        device,
        ecmp_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(mpath_info->members, node) {
    ecmp_member = (switch_ecmp_member_t *)node->data;
    (*nhop_handles)[index++] = ecmp_member->nhop_handle;
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

switch_status_t switch_api_ecmp_member_activate(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list) {
  SWITCH_MT_WRAP(switch_api_ecmp_member_activate_internal(
      device, ecmp_handle, nhop_count, nhop_handle_list))
}

switch_status_t switch_api_ecmp_members_delete(switch_device_t device,
                                               switch_handle_t ecmp_handle) {
  SWITCH_MT_WRAP(switch_api_ecmp_members_delete_internal(device, ecmp_handle))
}

switch_status_t switch_api_nhop_handle_get(const switch_device_t device,
                                           const switch_nhop_key_t *nhop_key,
                                           switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_nhop_handle_get_internal(device, nhop_key, nhop_handle))
}

switch_status_t switch_api_wcmp_member_add(switch_device_t device,
                                           switch_handle_t wcmp_handle,
                                           uint16_t nhop_count,
                                           switch_handle_t *nhop_handle_list,
                                           uint16_t *nhop_weight_list) {
  SWITCH_MT_WRAP(switch_api_wcmp_member_add_internal(
      device, wcmp_handle, nhop_count, nhop_handle_list, nhop_weight_list))
}

switch_status_t switch_api_ecmp_create_with_members(
    const switch_device_t device,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles,
    switch_handle_t *ecmp_handle,
    switch_handle_t *member_handle) {
  SWITCH_MT_WRAP(switch_api_ecmp_create_with_members_internal(
      device, num_nhops, nhop_handles, ecmp_handle, member_handle))
}

switch_status_t switch_api_ecmp_create(const switch_device_t device,
                                       switch_handle_t *ecmp_handle) {
  SWITCH_MT_WRAP(switch_api_ecmp_create_internal(device, ecmp_handle))
}

switch_status_t switch_api_neighbor_handle_get(
    const switch_device_t device,
    const switch_handle_t nhop_handle,
    switch_handle_t *neighbor_handle) {
  SWITCH_MT_WRAP(switch_api_neighbor_handle_get_internal(
      device, nhop_handle, neighbor_handle))
}

switch_status_t switch_api_ecmp_delete(const switch_device_t device,
                                       const switch_handle_t ecmp_handle) {
  SWITCH_MT_WRAP(switch_api_ecmp_delete_internal(device, ecmp_handle))
}

switch_status_t switch_api_nhop_delete(const switch_device_t device,
                                       const switch_handle_t nhop_handle) {
  SWITCH_MT_WRAP(switch_api_nhop_delete_internal(device, nhop_handle))
}

switch_status_t switch_api_nhop_id_type_get(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            switch_nhop_id_type_t *nhop_type) {
  SWITCH_MT_WRAP(
      switch_api_nhop_id_type_get_internal(device, nhop_handle, nhop_type))
}

switch_status_t switch_api_ecmp_member_add(const switch_device_t device,
                                           const switch_handle_t ecmp_handle,
                                           const switch_uint32_t num_nhops,
                                           const switch_handle_t *nhop_handles,
                                           switch_handle_t *member_handle) {
  SWITCH_MT_WRAP(switch_api_ecmp_member_add_internal(
      device, ecmp_handle, num_nhops, nhop_handles, member_handle))
}

switch_status_t switch_api_nhop_create(
    const switch_device_t device,
    const switch_api_nhop_info_t *api_nhop_info,
    switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_nhop_create_internal(device, api_nhop_info, nhop_handle))
}

switch_status_t switch_api_wcmp_delete(switch_device_t device,
                                       switch_handle_t wcmp_handle) {
  SWITCH_MT_WRAP(switch_api_wcmp_delete_internal(device, wcmp_handle))
}

switch_status_t switch_api_nhop_get(switch_device_t device,
                                    switch_handle_t nhop_handle,
                                    switch_api_nhop_info_t *api_nhop_info) {
  SWITCH_MT_WRAP(
      switch_api_nhop_get_internal(device, nhop_handle, api_nhop_info));
}

switch_status_t switch_api_ecmp_member_delete(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles) {
  SWITCH_MT_WRAP(switch_api_ecmp_member_delete_internal(
      device, ecmp_handle, num_nhops, nhop_handles))
}

switch_status_t switch_api_wcmp_member_modify(switch_device_t device,
                                              switch_handle_t wcmp_handle,
                                              uint16_t nhop_count,
                                              switch_handle_t *nhop_handle_list,
                                              uint16_t *nhop_weight_list) {
  SWITCH_MT_WRAP(switch_api_wcmp_member_modify_internal(
      device, wcmp_handle, nhop_count, nhop_handle_list, nhop_weight_list))
}

switch_status_t switch_api_wcmp_create(switch_device_t device,
                                       switch_handle_t *wcmp_handle) {
  SWITCH_MT_WRAP(switch_api_wcmp_create_internal(device, wcmp_handle))
}

switch_status_t switch_api_ecmp_members_get(const switch_device_t device,
                                            const switch_handle_t ecmp_handle,
                                            switch_uint16_t *num_nhops,
                                            switch_handle_t **nhop_handles) {
  SWITCH_MT_WRAP(switch_api_ecmp_members_get_internal(
      device, ecmp_handle, num_nhops, nhop_handles))
}

switch_status_t switch_api_wcmp_member_delete(
    switch_device_t device,
    switch_handle_t wcmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list) {
  SWITCH_MT_WRAP(switch_api_wcmp_member_delete_internal(
      device, wcmp_handle, nhop_count, nhop_handle_list))
}

switch_status_t switch_api_ecmp_member_deactivate(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list) {
  SWITCH_MT_WRAP(switch_api_ecmp_member_deactivate_internal(
      device, ecmp_handle, nhop_count, nhop_handle_list))
}

switch_status_t switch_api_nhop_table_size_get(switch_device_t device,
                                               switch_size_t *tbl_size) {
  SWITCH_MT_WRAP(switch_api_nhop_table_size_get_internal(device, tbl_size))
}

static switch_status_t switch_api_nhop_add_route_to_list(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t route_handle) {
  PWord_t PValue;

  JLI(PValue, NHOP_TUNNEL_MGID_ROUTE_LIST(nhop_info), route_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR(
        "nhop add route failed on device %d"
        "nhop info 0x%lx: , route handle 0x%lx: ",
        device,
        nhop_info,
        route_handle);
    return SWITCH_STATUS_FAILURE;
  }

  NHOP_TUNNEL_NUM_ROUTES(nhop_info) += 1;

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_nhop_remove_route_from_list(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t route_handle) {
  int Rc_int;

  JLD(Rc_int, NHOP_TUNNEL_MGID_ROUTE_LIST(nhop_info), route_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR(
        "nhop remove route failed on device %d"
        "nhop info 0x%lx: , route handle 0x%lx: ",
        device,
        nhop_info,
        route_handle);
    return SWITCH_STATUS_FAILURE;
  }

  NHOP_TUNNEL_NUM_ROUTES(nhop_info) -= 1;

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_nhop_add_tunnel_to_list(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t tunnel_handle) {
  PWord_t PValue;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  JLI(PValue, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR(
        "nhop add tunnel failed on device %d"
        "nhop info 0x%lx: , tunnel handle 0x%lx: ",
        device,
        nhop_info,
        tunnel_handle);
    return SWITCH_STATUS_FAILURE;
  }

  NHOP_TUNNEL_NUM_TUNNELS(nhop_info) += 1;

  status = switch_api_tunnel_send_mgid_event(
      device,
      tunnel_handle,
      SWITCH_MGID_ADD,
      (void *)NHOP_TUNNEL_MGID_HANDLE(nhop_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop add tunnel failed on device %d"
        "nhop info 0x%lx: , tunnel handle 0x%lx: ",
        device,
        nhop_info,
        tunnel_handle);
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_nhop_remove_tunnel_from_list(
    switch_device_t device,
    switch_nhop_info_t *nhop_info,
    switch_handle_t tunnel_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  int Rc_int;
  JLD(Rc_int, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR(
        "nhop remove tunnel failed on device %d"
        "nhop info 0x%lx: , tunnel handle 0x%lx: ",
        device,
        nhop_info,
        tunnel_handle);
    return SWITCH_STATUS_FAILURE;
  }

  NHOP_TUNNEL_NUM_TUNNELS(nhop_info) -= 1;

  status = switch_api_tunnel_send_mgid_event(
      device, tunnel_handle, SWITCH_MGID_REMOVE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop remove tunnel failed on device %d"
        "nhop info 0x%lx: , tunnel handle 0x%lx: ",
        device,
        nhop_info,
        tunnel_handle);
    return status;
  }

  return status;
}

static switch_status_t switch_api_nhop_tunnel_mgid_create(
    switch_device_t device, switch_nhop_info_t *nhop_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_api_nhop_mgid_tree_create(device, nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop mgid tunnel create failed on device %d"
        "nhop info 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_info,
        switch_error_to_string(status));
    return status;
  }

  JLF(PValue, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  while (PValue != NULL) {
    status = switch_api_tunnel_send_mgid_event(
        device,
        tunnel_handle,
        SWITCH_MGID_ADD,
        (void *)NHOP_TUNNEL_MGID_HANDLE(nhop_info));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop mgid tunnel create failed on device %d"
          "nhop info 0x%lx: "
          "with status (%s)\n",
          device,
          nhop_info,
          switch_error_to_string(status));
      return status;
    }

    JLN(PValue, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  }

  return status;
}

static switch_status_t switch_api_nhop_tunnel_mgid_delete(
    switch_device_t device, switch_nhop_info_t *nhop_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;

  JLF(PValue, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  while (PValue != NULL) {
    status = switch_api_tunnel_send_mgid_event(
        device, tunnel_handle, SWITCH_MGID_REMOVE, NULL);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "nhop mgid tunnel delete failed on device %d"
          "nhop info 0x%lx: "
          "with status (%s)\n",
          device,
          nhop_info,
          switch_error_to_string(status));
      return status;
    }

    JLN(PValue, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), tunnel_handle);
  }

  status = switch_api_nhop_mgid_tree_delete(device, nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop mgid tunnel delete failed on device %d"
        "nhop info 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_info,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

static switch_status_t switch_api_nhop_mgid_tree_create(
    switch_device_t device, switch_nhop_info_t *nhop_info) {
  switch_mcast_info_t *mcast_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(nhop_info->id_type != SWITCH_NHOP_ID_TYPE_NONE);

  status = switch_api_multicast_index_create(
      device, &(NHOP_TUNNEL_MGID_HANDLE(nhop_info)));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop mgid tree create failed on device %d"
        "nhop info 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_info,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_mgid_get(device, NHOP_TUNNEL_MGID_HANDLE(nhop_info), &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop mgid tree create failed on device %d"
        "nhop info 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_info,
        switch_error_to_string(status));
    return status;
  }

  mcast_info->type = SWITCH_MGID_TYPE_UNICAST;

  status = switch_multicast_nhop_member_add(
      device, NHOP_TUNNEL_MGID_HANDLE(nhop_info), nhop_info->nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop mgid tree create failed on device %d"
        "nhop info 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_info,
        switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_nhop_mgid_tree_delete(
    switch_device_t device, switch_nhop_info_t *nhop_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(nhop_info->id_type != SWITCH_NHOP_ID_TYPE_NONE);

  status = switch_multicast_nhop_member_delete(
      device, NHOP_TUNNEL_MGID_HANDLE(nhop_info), nhop_info->nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop mgid tree delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_multicast_index_delete(
      device, NHOP_TUNNEL_MGID_HANDLE(nhop_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("nhop mgid tree delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_nhop_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_nhop_info_t *nhop_info;
  switch_handle_t route_handle;
  switch_status_t status;

  nhop_info = (switch_nhop_info_t *)info;

  switch (event) {
    case SWITCH_ROUTE_ADD:
      route_handle = (switch_handle_t)event_arg;
      status =
          switch_api_nhop_add_route_to_list(device, nhop_info, route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state init failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      SET_NHOP_TUNNEL_MGID_STATE(nhop_info, switch_api_nhop_mgid_state_no_mgid);
      break;

    case SWITCH_NHOP_MGID_TREE_CREATE:
      return SWITCH_STATUS_SUCCESS;

    case SWITCH_NHOP_MGID_TREE_DELETE:
      return SWITCH_STATUS_SUCCESS;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_nhop_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_nhop_info_t *nhop_info;
  int Rc_int;
  switch_handle_t tunnel_handle;
  switch_handle_t route_handle;
  switch_status_t status;

  nhop_info = (switch_nhop_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_CREATE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));

      status = switch_api_nhop_mgid_tree_create(device, nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state no mgid failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      status =
          switch_api_nhop_add_tunnel_to_list(device, nhop_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state no mgid failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      SET_NHOP_TUNNEL_MGID_STATE(nhop_info,
                                 switch_api_nhop_mgid_state_mgid_associated);

      break;

    case SWITCH_ROUTE_ADD:
      route_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      status =
          switch_api_nhop_add_route_to_list(device, nhop_info, route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state no mgid failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      break;

    case SWITCH_ROUTE_REMOVE:
      route_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      status = switch_api_nhop_remove_route_from_list(
          device, nhop_info, route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state no mgid failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      /* Check if no more routes use this nexthop */
      JLC(Rc_int, NHOP_TUNNEL_MGID_ROUTE_LIST(nhop_info), 0, -1);
      if (Rc_int == 0) {
        SET_NHOP_TUNNEL_MGID_STATE(nhop_info, switch_api_nhop_mgid_state_init);
      }
      break;

    case SWITCH_NHOP_MGID_TREE_CREATE:
      return SWITCH_STATUS_SUCCESS;

    case SWITCH_NHOP_MGID_TREE_DELETE:
      return SWITCH_STATUS_SUCCESS;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_nhop_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_nhop_info_t *nhop_info;
  int Rc_route, Rc_tun;
  switch_handle_t tunnel_handle;
  switch_handle_t route_handle;
  switch_status_t status;

  nhop_info = (switch_nhop_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_CREATE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));
      status =
          switch_api_nhop_add_tunnel_to_list(device, nhop_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state associated failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_TUNNEL_DELETE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));
      status = switch_api_nhop_remove_tunnel_from_list(
          device, nhop_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state associated failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      /* Check if the last tunnel using this nhop was deleted */
      JLC(Rc_tun, NHOP_TUNNEL_MGID_TUNNEL_LIST(nhop_info), 0, -1);
      if (Rc_tun == 0) {
        status = switch_api_nhop_mgid_tree_delete(device, nhop_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "nhop mgid state associated failed on device %d"
              "nhop info 0x%lx: "
              "with status (%s)\n",
              device,
              nhop_info,
              switch_error_to_string(status));
          return status;
        }

        SET_NHOP_TUNNEL_MGID_STATE(nhop_info,
                                   switch_api_nhop_mgid_state_no_mgid);
      }
      break;

    case SWITCH_ROUTE_ADD:
      route_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      status =
          switch_api_nhop_add_route_to_list(device, nhop_info, route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state associated failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      break;

    case SWITCH_ROUTE_REMOVE:
      route_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      status = switch_api_nhop_remove_route_from_list(
          device, nhop_info, route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state associated failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }

      /* Check that there is atleast one other route using this nexthop.
      If not, we should have deleted all the tunnels and the nhop
      should have been in NO_MGID state */
      JLC(Rc_route, NHOP_TUNNEL_MGID_ROUTE_LIST(nhop_info), 0, -1);
      SWITCH_ASSERT(Rc_route != 0);

      break;

    case SWITCH_NHOP_MGID_TREE_DELETE:
      /* Delete and recreate the tree */
      status = switch_api_nhop_tunnel_mgid_delete(device, nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state associated failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_NHOP_MGID_TREE_CREATE:
      status = switch_api_nhop_tunnel_mgid_create(device, nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "nhop mgid state no mgid failed on device %d"
            "nhop info 0x%lx: "
            "with status (%s)\n",
            device,
            nhop_info,
            switch_error_to_string(status));
        return status;
      }
      break;

    default:
      SWITCH_ASSERT(FALSE);
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_nhop_send_mgid_event(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_status_t status;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop send mgid event failed on device %d"
        "nhop handle 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_NHOP_TUNNEL_MGID_EVENT(nhop_info, device, event, event_arg);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop send mgid event failed on device %d"
        "nhop handle 0x%lx: "
        "with status (%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}
