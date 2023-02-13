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

#include "switchapi/switch_mcast.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_mcast_group_entry_key_init(void *args,
                                                  switch_uint8_t *key,
                                                  switch_uint32_t *len) {
  switch_mcast_group_key_t *group_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  group_key = (switch_mcast_group_key_t *)args;

  SWITCH_MEMCPY((key + *len), &group_key->handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY((key + *len), &group_key->src_ip, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_MEMCPY((key + *len), &group_key->grp_ip, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_MEMCPY((key + *len), &group_key->sg_entry, sizeof(bool));
  *len += sizeof(bool);

  SWITCH_ASSERT(*len == SWITCH_MCAST_GROUP_HASH_KEY_SIZE);
  return status;
}

switch_int32_t switch_mcast_group_hash_compare(const void *key1,
                                               const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_MCAST_GROUP_HASH_KEY_SIZE);
}

switch_status_t switch_mcast_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_rid_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_replica_type_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ip_mcast_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_replica_type_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mcast_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_mcast_table_size_get(switch_device_t device,
                                            switch_size_t *mcast_table_size) {
  switch_table_id_t table_id = 0;
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mcast_table_size != NULL);

  *mcast_table_size = 0;

  for (table_id = SWITCH_TABLE_OUTER_MCAST_STAR_G;
       table_id <= SWITCH_TABLE_IPV6_MCAST_STAR_G;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *mcast_table_size = 0;
      SWITCH_LOG_ERROR(
          "mcast table size get failed on device %d: %s"
          " for table %s\n",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }
    *mcast_table_size += table_size;
  }

  return status;
}

switch_status_t switch_mcast_init(switch_device_t device) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_size_t rid_table_size = 0;
  switch_size_t mcast_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  mcast_ctx = SWITCH_MALLOC(device, sizeof(switch_mcast_context_t), 0x1);
  if (!mcast_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(mcast_ctx, 0x0, sizeof(switch_mcast_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_MCAST, (void *)mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_MGID, SWITCH_MGID_TABLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_MGID_ECMP, SWITCH_MGID_ECMP_TABLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_table_size_get(device, SWITCH_TABLE_RID, &rid_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  mcast_ctx->rid_hashtable.size = rid_table_size;
  status = SWITCH_HASHTABLE_INIT(&mcast_ctx->rid_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_mcast_table_size_get(device, &mcast_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  mcast_ctx->mcast_group_hashtable.size = mcast_table_size;
  mcast_ctx->mcast_group_hashtable.compare_func =
      switch_mcast_group_hash_compare;
  mcast_ctx->mcast_group_hashtable.key_func = switch_mcast_group_entry_key_init;
  mcast_ctx->mcast_group_hashtable.hash_seed = SWITCH_MAC_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&mcast_ctx->mcast_group_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_id_allocator_new(
      device, rid_table_size, FALSE, &(mcast_ctx->rid_allocator));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * RID gets reset to 0 for mirrored copies. This will be pruned by TM if
   * the global RID also remains 0. Hence the default is set to 0XFFFF
   */

  status = switch_pd_mcast_global_rid_set(device, SWITCH_MCAST_GLOBAL_RID);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast init failed for device %d: %s "
        "global rid set failed\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  tmp_status = switch_mcast_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_mcast_free(switch_device_t device) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MGID);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MGID_ECMP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&mcast_ctx->mcast_group_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, mcast_ctx->rid_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  SWITCH_FREE(device, mcast_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_MCAST, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_multicast_index_create_internal(
    const switch_device_t device, switch_handle_t *mgid_handle) {
  switch_mcast_info_t *mcast_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mgid_handle != NULL);
  if (!mgid_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mcast index create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mgid_handle = switch_mgid_handle_create(device);

  status = switch_mgid_get(device, *mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mcast_mgrp_tree_create(
      device, handle_to_id(*mgid_handle), mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_LIST_INIT(&mcast_info->node_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:

  tmp_status = switch_mgid_handle_delete(device, *mgid_handle);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  *mgid_handle = SWITCH_API_INVALID_HANDLE;

  return status;
}

switch_status_t switch_api_multicast_index_delete_internal(
    switch_device_t device, switch_handle_t mgid_handle) {
  switch_mcast_info_t *mcast_info = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle)) {
    SWITCH_LOG_ERROR("mcast index delete failed on device %d: %s\n",
                     status,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(mcast_info->node_list, node) {
    // Multicast member delete
  }
  FOR_EACH_IN_LIST_END();

  status = switch_pd_mcast_mgrp_tree_delete(device, mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_handle_delete(device, mgid_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast index delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mcast_node_get(switch_device_t device,
                                      switch_rid_t rid,
                                      switch_xid_t xid,
                                      switch_mcast_node_type_t node_type,
                                      switch_mcast_info_t *mcast_info,
                                      switch_mcast_node_t **mcast_node) {
  switch_mcast_node_t *tmp_mcast_node = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mcast_info != NULL);
  SWITCH_ASSERT(mcast_node != NULL);

  *mcast_node = NULL;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  FOR_EACH_IN_LIST(mcast_info->node_list, node) {
    tmp_mcast_node = (switch_mcast_node_t *)node->data;
    if ((node_type == SWITCH_NODE_TYPE_SINGLE &&
         SWITCH_MCAST_NODE_RID(tmp_mcast_node) == rid &&
         tmp_mcast_node->xid == xid) ||
        (node_type == SWITCH_NODE_TYPE_ECMP && tmp_mcast_node->xid == xid)) {
      *mcast_node = tmp_mcast_node;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_mcast_ecmp_node_get(
    switch_device_t device,
    switch_mcast_info_t *mcast_info,
    switch_handle_t ecmp_nhop_handle,
    switch_mcast_node_t **ecmp_mcast_node) {
  switch_mcast_node_t *tmp_mcast_node = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mcast_info != NULL);
  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_nhop_handle));
  SWITCH_ASSERT(ecmp_mcast_node != NULL);

  *ecmp_mcast_node = NULL;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  FOR_EACH_IN_LIST(mcast_info->node_list, node) {
    tmp_mcast_node = (switch_mcast_node_t *)node->data;
    if (SWITCH_MCAST_ECMP_INFO_HDL(tmp_mcast_node) == ecmp_nhop_handle) {
      *ecmp_mcast_node = tmp_mcast_node;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_mcast_ecmp_member_node_get(
    switch_device_t device,
    switch_rid_t rid,
    switch_xid_t xid,
    switch_mcast_node_t *ecmp_mcast_node,
    switch_mcast_node_t **mcast_node) {
  switch_mcast_node_t *tmp_mcast_node = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mcast_node != NULL);
  SWITCH_ASSERT(ecmp_mcast_node != NULL);

  *mcast_node = NULL;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  FOR_EACH_IN_LIST(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node), node) {
    tmp_mcast_node = (switch_mcast_node_t *)node->data;
    if ((SWITCH_MCAST_NODE_RID(tmp_mcast_node) == rid) &&
        (tmp_mcast_node->xid == xid)) {
      *mcast_node = tmp_mcast_node;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

bool switch_mcast_node_empty(switch_mcast_node_t *node) {
  switch_mc_lag_map_t *lag_map = NULL;
  switch_mc_port_map_t *port_map = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(node != NULL);

  switch (node->node_type) {
    case SWITCH_NODE_TYPE_SINGLE:
      lag_map = &(SWITCH_MCAST_NODE_INFO_LAG_MAP(node));
      port_map = &(SWITCH_MCAST_NODE_INFO_PORT_MAP(node));

      for (index = 0; index < SWITCH_PORT_ARRAY_SIZE; index++) {
        if ((*port_map)[index]) {
          return FALSE;
        }
      }
      for (index = 0; index < SWITCH_LAG_ARRAY_SIZE; index++) {
        if ((*lag_map)[index]) {
          return FALSE;
        }
      }
      break;
    case SWITCH_NODE_TYPE_ECMP:
      if ((SWITCH_MCAST_ECMP_INFO_NODE_LIST(node)).num_entries == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_ASSERT(0);
      SWITCH_LOG_ERROR("mcast node type invalid: %s\n",
                       switch_error_to_string(status));
      return status;
  }

  return TRUE;
}

switch_status_t switch_mcast_port_map_update(switch_device_t device,
                                             switch_mcast_node_t *node,
                                             switch_handle_t intf_handle,
                                             bool set) {
  switch_interface_info_t *intf_info = NULL;
  switch_id_t id = 0;
  bool lag = FALSE;
  switch_port_info_t *port_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(node != NULL);
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));

  status = switch_interface_get(device, intf_handle, &intf_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  switch (SWITCH_INTF_TYPE(intf_info)) {
    case SWITCH_INTERFACE_TYPE_TUNNEL:
      break;

    // continue using egress intf.
    case SWITCH_INTERFACE_TYPE_ACCESS:
    case SWITCH_INTERFACE_TYPE_TRUNK:
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      handle = SWITCH_INTF_ATTR_HANDLE(intf_info);
      if (SWITCH_LAG_HANDLE(handle)) {
        lag = TRUE;
        id = handle_to_id(handle);
      } else {
        SWITCH_ASSERT(SWITCH_PORT_HANDLE(handle));
        status = switch_port_get(device, handle, &port_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("mcast port map update failed on device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }

        id = port_info->dev_port;
      }
      break;
    default:
      status = SWITCH_STATUS_INVALID_HANDLE;
      CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  }

  if (set) {
    if (lag) {
      SWITCH_MC_LAG_MAP_SET(SWITCH_MCAST_NODE_INFO_LAG_MAP(node), id);
    } else {
      SWITCH_MC_PORT_MAP_SET(SWITCH_MCAST_NODE_INFO_PORT_MAP(node), id);
    }
  } else {
    if (lag) {
      SWITCH_MC_LAG_MAP_CLEAR(SWITCH_MCAST_NODE_INFO_LAG_MAP(node), id);
    } else {
      SWITCH_MC_PORT_MAP_CLEAR(SWITCH_MCAST_NODE_INFO_PORT_MAP(node), id);
    }
  }

  return status;
}

switch_status_t switch_mcast_rid_allocate(switch_device_t device,
                                          switch_rid_t *rid) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(rid != NULL);
  if (!rid) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mcast rid allocate failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_allocate(device, mcast_ctx->rid_allocator, rid);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mcast_rid_release(switch_device_t device,
                                         switch_rid_t rid) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_release(device, mcast_ctx->rid_allocator, rid);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mcast_bd_member_rid_allocate(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t intf_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_rid_t rid = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_rid_allocate(device, &rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid allocate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  bd_member->rid = rid;

  return status;
}

switch_status_t switch_mcast_bd_member_rid_free(switch_device_t device,
                                                switch_handle_t bd_handle,
                                                switch_handle_t intf_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_rid_release(device, bd_member->rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid release failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  bd_member->rid = 0;

  return status;
}

switch_status_t switch_mcast_rid_get(switch_device_t device,
                                     switch_handle_t bd_handle,
                                     switch_handle_t intf_handle,
                                     switch_rid_t *rid) {
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR("mcast rid get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR("mcast rid get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rid get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *rid = bd_member->rid;

  return status;
}

switch_status_t switch_mcast_interface_get(
    const switch_device_t device,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs,
    switch_mcast_member_info_t **intf_mbrs) {
  switch_interface_info_t *intf_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t network_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(intf_mbrs != NULL);

  *intf_mbrs = SWITCH_MALLOC(device, sizeof(switch_mcast_member_t), num_mbrs);
  if (!(*intf_mbrs)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("mcast interface get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_mbrs; index++) {
    if (mbrs[index].network_handle != SWITCH_API_INVALID_HANDLE) {
      network_handle = mbrs[index].network_handle;
      status = switch_bd_handle_get(device, network_handle, &bd_handle);
      CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
    }

    (*intf_mbrs)[index].intf_handle = SWITCH_API_INVALID_HANDLE;
    (*intf_mbrs)[index].bd_handle = SWITCH_API_INVALID_HANDLE;

    switch (switch_handle_type_get(mbrs[index].handle)) {
      case SWITCH_HANDLE_TYPE_RIF:
        status = switch_rif_get(device, mbrs[index].handle, &rif_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        CHECK_CLEAN(rif_info->api_rif_info.rif_type != SWITCH_RIF_TYPE_INTF,
                    SWITCH_STATUS_INVALID_PARAMETER);
        intf_handle = rif_info->api_rif_info.intf_handle;
        bd_handle = rif_info->bd_handle;
        break;
      case SWITCH_HANDLE_TYPE_INTERFACE:
        intf_handle = mbrs[index].handle;
        break;
      default:
        SWITCH_LOG_ERROR("unexpected handle type!");
        return SWITCH_STATUS_INVALID_PARAMETER;
    }

    status = switch_interface_get(device, intf_handle, &intf_info);
    CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

    (*intf_mbrs)[index].intf_handle = intf_handle;
    (*intf_mbrs)[index].bd_handle = bd_handle;
  }

  return status;
clean:
  if (*intf_mbrs) {
    SWITCH_FREE(device, *intf_mbrs);
  }
  return status;
}

switch_status_t switch_mcast_node_add(switch_device_t device,
                                      switch_handle_t mgid_handle,
                                      switch_mcast_node_type_t node_type,
                                      switch_rid_t rid,
                                      switch_xid_t xid,
                                      switch_handle_t intf_handle) {
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *ecmp_mcast_node = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node update failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: mgid get failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (node_type == SWITCH_NODE_TYPE_ECMP) {
    status = switch_mcast_node_get(
        device, 0, 0, SWITCH_NODE_TYPE_ECMP, mcast_info, &ecmp_mcast_node);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mcast node get failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (!ecmp_mcast_node) {
      ecmp_mcast_node = SWITCH_MALLOC(device, sizeof(switch_mcast_node_t), 0x1);
      if (!ecmp_mcast_node) {
        status = SWITCH_STATUS_NO_MEMORY;
        SWITCH_LOG_ERROR(
            "mcast node update failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mcast node allocation failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(ecmp_mcast_node, 0x0, sizeof(switch_mcast_node_t));

      status = SWITCH_LIST_INSERT(
          &mcast_info->node_list, &ecmp_mcast_node->node, ecmp_mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node update failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: mgid get failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      ecmp_mcast_node->xid = 0;
      ecmp_mcast_node->node_type = node_type;

      status = switch_pd_mcast_ecmp_group_create(device, ecmp_mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node update failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "ecmp group create failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      status = switch_pd_mcast_mgid_table_ecmp_entry_add(
          device, mcast_info->mgrp_hdl, ecmp_mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node update failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mgid table entry add failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  mcast_node = SWITCH_MALLOC(device, sizeof(switch_mcast_node_t), 0x1);
  if (!mcast_node) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mcast node update failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "memory alloc failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(mcast_node, 0x0, sizeof(switch_mcast_node_t));

  SWITCH_MCAST_NODE_RID(mcast_node) = rid;
  mcast_node->xid = xid;
  mcast_node->node_type = SWITCH_NODE_TYPE_SINGLE;

  status = switch_mcast_port_map_update(device, mcast_node, intf_handle, TRUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node update failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "port map update failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mcast_entry_add(device, mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node update failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "pd mcast entry add failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (node_type == SWITCH_NODE_TYPE_ECMP) {
    status =
        SWITCH_LIST_INSERT(&(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)),
                           &mcast_node->node,
                           mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "list insert failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    status =
        switch_pd_mcast_ecmp_entry_add(device, ecmp_mcast_node, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "ecmp member node add failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    status = SWITCH_LIST_INSERT(
        &mcast_info->node_list, &mcast_node->node, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: mgid get failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_mcast_mgid_table_entry_add(
        device, mcast_info->mgrp_hdl, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mgid table entry add failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_multicast_nhop_member_add_i(
    switch_device_t device,
    switch_handle_t mgid_handle,
    switch_mcast_node_type_t node_type,
    switch_handle_t nhop_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_api_neighbor_info_t *neighbor = NULL;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_mac_info_t *mac_info = NULL;
  switch_mac_addr_t mac;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_tunnel_type_egress_t tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_NONE;
  switch_tunnel_t tunnel_id = 0;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t glean_nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t drop_nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast context get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle) || !SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "invalid handle(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "nhop get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  intf_handle = nhop_info->spath.api_nhop_info.rif_handle;
  bd_handle = 0;
  xid = 0;
  if (SWITCH_RIF_HANDLE(intf_handle)) {
    status = switch_rif_get(device, intf_handle, &rif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    bd_handle = rif_info->bd_handle;
    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "bd get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_VLAN) {
      status = switch_neighbor_get(
          device, nhop_info->spath.neighbor_handle, &neighbor_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "neighbor get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      neighbor = &neighbor_info->api_neighbor_info;
      switch_mac_entry_t mac_entry;
      SWITCH_MEMSET(&mac_entry, 0x0, sizeof(mac_entry));
      mac_entry.bd_handle = bd_handle;
      SWITCH_MEMCPY(
          &mac_entry.mac, &neighbor->mac_addr, sizeof(switch_mac_addr_t));
      status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "mac table entry get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
      status = switch_mac_get(device, mac_handle, &mac_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "bd handle 0x%lx mac handle 0x%lx: "
            "mac get failed(%s)\n",
            device,
            bd_handle,
            mac_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      status =
          switch_interface_handle_get(device, mac_info->ifindex, &intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "interface handle get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    } else {
      intf_handle = rif_info->api_rif_info.intf_handle;
    }
    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    xid = bd_info->xid;
  } else {
    status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &glean_nhop_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &drop_nhop_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    if (nhop_handle != glean_nhop_handle && nhop_handle != drop_nhop_handle) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "nhop handle is not glean or drop(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  rid = nhop_info->spath.rid;
  if (nhop_info->spath.rid == SWITCH_RID_INVALID) {
    status = switch_mcast_rid_allocate(device, &rid);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid alloation failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }

    rid_info = SWITCH_MALLOC(device, sizeof(switch_rid_info_t), 0x1);
    if (!rid_info) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid info alloation failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_MEMSET(rid_info, 0x0, sizeof(switch_rid_info_t));
    rid_info->rid = rid;
    rid_info->ref_count = 0;
    rid_info->rid_pd_hdl = SWITCH_PD_INVALID_HANDLE;
    nhop_info->spath.rid = rid;

    SWITCH_MEMSET(&mac, 0x0, sizeof(switch_mac_addr_t));
    if (SWITCH_NEIGHBOR_HANDLE(nhop_info->spath.neighbor_handle)) {
      status = switch_neighbor_get(
          device, nhop_info->spath.neighbor_handle, &neighbor_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        status = SWITCH_STATUS_NO_MEMORY;
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "interface get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      if (!(SWITCH_HW_FLAG_ISSET(rid_info, SWITCH_RID_PD_ENTRY))) {
        status = switch_pd_rid_table_entry_add(device,
                                               SWITCH_RID_TYPE_UNICAST,
                                               rid,
                                               handle_to_id(bd_handle),
                                               tunnel_type,
                                               tunnel_id,
                                               neighbor_info->tunnel_dmac_index,
                                               &rid_info->rid_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "multicast member add failed on device %d "
              "mgid handle 0x%lx nhop handle 0x%lx: "
              "rid table add failed(%s)\n",
              device,
              mgid_handle,
              nhop_handle,
              switch_error_to_string(status));
          goto cleanup;
        }
        SWITCH_HW_FLAG_SET(rid_info, SWITCH_RID_PD_ENTRY);
      }

      if (!(SWITCH_HW_FLAG_ISSET(rid_info, SWITCH_RID_IFINDEX_PD_ENTRY))) {
        status = switch_pd_mcast_egress_ifindex_table_entry_add(
            device,
            rid,
            intf_info->ifindex,
            &rid_info->mcast_egress_ifindex_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "multicast member add failed on device %d "
              "mgid handle 0x%lx nhop handle 0x%lx: "
              "rid ifindex table add failed(%s)\n",
              device,
              mgid_handle,
              nhop_handle,
              switch_error_to_string(status));
          goto cleanup;
        }
        SWITCH_HW_FLAG_SET(rid_info, SWITCH_RID_IFINDEX_PD_ENTRY);
      }
    } else {
      status = switch_api_hostif_cpu_intf_handle_get(device, &intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member add failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "cpu interface get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }

    status = SWITCH_ARRAY_INSERT(&mcast_ctx->rid_array, rid, (void *)rid_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid array insert failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    rid_info->ref_count = 1;
  } else {
    status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid array get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    rid = rid_info->rid;
    rid_info->ref_count++;
  }

  status = switch_mcast_node_add(
      device, mgid_handle, node_type, rid, xid, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast node update failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_multicast_nhop_member_add(switch_device_t device,
                                                 switch_handle_t mgid_handle,
                                                 switch_handle_t nhop_handle) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t *mbrs = NULL;
  switch_uint16_t num_mbrs = 0;
  switch_uint16_t index = 0;
  switch_mcast_node_type_t node_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle) ||
                SWITCH_ECMP_HANDLE(nhop_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle) || !SWITCH_NHOP_HANDLE(nhop_handle) ||
      !SWITCH_ECMP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "invalid handle(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "nhop get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (nhop_info->id_type == SWITCH_NHOP_ID_TYPE_ONE_PATH) {
    num_mbrs = 1;
    mbrs = &nhop_handle;
    node_type = SWITCH_NODE_TYPE_SINGLE;
  } else {
    status = switch_api_ecmp_members_get(device, nhop_handle, &num_mbrs, &mbrs);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "ecmp members get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    node_type = SWITCH_NODE_TYPE_ECMP;
  }

  for (index = 0; index < num_mbrs; index++) {
    status = switch_multicast_nhop_member_add_i(
        device, mgid_handle, node_type, mbrs[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "nhop member add internal failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_mcast_node_delete(switch_device_t device,
                                         switch_handle_t mgid_handle,
                                         switch_mcast_node_type_t node_type,
                                         switch_rid_t rid,
                                         switch_xid_t xid,
                                         switch_handle_t intf_handle) {
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *ecmp_mcast_node = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  bool delete_node = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node delete failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "mgid get failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (node_type == SWITCH_NODE_TYPE_ECMP) {
    status = switch_mcast_node_get(
        device, 0, 0, SWITCH_NODE_TYPE_ECMP, mcast_info, &ecmp_mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mcast ecmp node get failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_mcast_ecmp_member_node_get(
        device, rid, xid, ecmp_mcast_node, &mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node update failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mcast ecmp member node get failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    status = switch_mcast_node_get(
        device, rid, xid, node_type, mcast_info, &mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node delete failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mgid table node get failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_mcast_port_map_update(device, mcast_node, intf_handle, FALSE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node delete failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "mgid port map update failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  delete_node = switch_mcast_node_empty(mcast_node);

  if (delete_node) {
    if (node_type == SWITCH_NODE_TYPE_SINGLE) {
      status = SWITCH_LIST_DELETE(&mcast_info->node_list, &mcast_node->node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node delete failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mcast list delete failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      status = switch_pd_mcast_mgid_table_entry_delete(
          device, mcast_info->mgrp_hdl, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node delete failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mgid table entry delete failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  if (node_type == SWITCH_NODE_TYPE_ECMP) {
    status =
        switch_pd_mcast_ecmp_entry_remove(device, ecmp_mcast_node, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node delete failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mcast ecmp entry remove failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    status =
        SWITCH_LIST_DELETE(&(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)),
                           &mcast_node->node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mcast node delete failed on device %d mgid handle 0x%lx "
          "rid %d xid %d intf handle 0x%lx: "
          "mcast ecmp list delete failed(%s)\n",
          device,
          mgid_handle,
          rid,
          xid,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    if ((SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)).num_entries == 0) {
      status = switch_pd_mcast_mgid_table_ecmp_entry_remove(
          device, mcast_info->mgrp_hdl, ecmp_mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node delete failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mcast table ecmp node delete failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      status = switch_pd_mcast_ecmp_group_delete(device, ecmp_mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node delete failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "ecmp group delete failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      status =
          SWITCH_LIST_DELETE(&mcast_info->node_list, &ecmp_mcast_node->node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mcast node delete failed on device %d mgid handle 0x%lx "
            "rid %d xid %d intf handle 0x%lx: "
            "mcast ecmp node delete failed(%s)\n",
            device,
            mgid_handle,
            rid,
            xid,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }

      SWITCH_FREE(device, ecmp_mcast_node);
    }
  }

  status = switch_pd_mcast_entry_delete(device, mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mcast node delete failed on device %d mgid handle 0x%lx "
        "rid %d xid %d intf handle 0x%lx: "
        "mcast entry delete failed(%s)\n",
        device,
        mgid_handle,
        rid,
        xid,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, mcast_node);

  return status;
}

switch_status_t switch_multicast_nhop_member_delete_i(
    switch_device_t device,
    switch_handle_t mgid_handle,
    switch_mcast_node_type_t node_type,
    switch_handle_t nhop_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t glean_nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t drop_nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_neighbor_info_t *neighbor = NULL;
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast context get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle) || !SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "invalid handle(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "nhop get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  intf_handle = nhop_info->spath.api_nhop_info.rif_handle;
  if (SWITCH_RIF_HANDLE(intf_handle)) {
    status = switch_rif_get(device, intf_handle, &rif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rif get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    bd_handle = rif_info->bd_handle;
    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "bd get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_VLAN) {
      status = switch_neighbor_get(
          device, nhop_info->spath.neighbor_handle, &neighbor_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member delete failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "neighbor get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      neighbor = &neighbor_info->api_neighbor_info;
      switch_mac_entry_t mac_entry;
      SWITCH_MEMSET(&mac_entry, 0x0, sizeof(mac_entry));
      mac_entry.bd_handle = bd_handle;
      SWITCH_MEMCPY(
          &mac_entry.mac, &neighbor->mac_addr, sizeof(switch_mac_addr_t));
      status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member delete failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "mac table entry get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
      status = switch_mac_get(device, mac_handle, &mac_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member delete failed on device %d "
            "bd handle 0x%lx mac handle 0x%lx: "
            "mac get failed(%s)\n",
            device,
            bd_handle,
            mac_handle,
            switch_error_to_string(status));
        goto cleanup;
      }

      status =
          switch_interface_handle_get(device, mac_info->ifindex, &intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "multicast member delete failed on device %d "
            "mgid handle 0x%lx nhop handle 0x%lx: "
            "interface handle get failed(%s)\n",
            device,
            mgid_handle,
            nhop_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    } else {
      intf_handle = rif_info->api_rif_info.intf_handle;
    }

    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    xid = bd_info->xid;
  } else {
    status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &glean_nhop_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &drop_nhop_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    if (nhop_handle != glean_nhop_handle && nhop_handle != drop_nhop_handle) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "nhop handle is not glean or drop(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  rid = nhop_info->spath.rid;

  if (rid == SWITCH_RID_INVALID) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "rid is invalid(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "rid array get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (rid_info->ref_count > 1) {
    rid_info->ref_count--;
    return status;
  }

  if ((SWITCH_HW_FLAG_ISSET(rid_info, SWITCH_RID_PD_ENTRY))) {
    status = switch_pd_rid_table_entry_delete(device, rid_info->rid_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid table delete failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(rid_info, SWITCH_RID_PD_ENTRY);
  }

  if ((SWITCH_HW_FLAG_ISSET(rid_info, SWITCH_RID_IFINDEX_PD_ENTRY))) {
    status = switch_pd_mcast_egress_ifindex_table_entry_delete(
        device, rid_info->mcast_egress_ifindex_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member delete failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "rid ifindex table delete failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(rid_info, SWITCH_RID_IFINDEX_PD_ENTRY);
  }

  status = SWITCH_ARRAY_DELETE(&mcast_ctx->rid_array, rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast rid delete failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_mcast_rid_release(device, rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast rid release failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  nhop_info->spath.rid = SWITCH_RID_INVALID;

  status = switch_mcast_node_delete(
      device, mgid_handle, node_type, rid, xid, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member delete failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "mcast node delete failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_multicast_nhop_member_delete(
    switch_device_t device,
    switch_handle_t mgid_handle,
    switch_handle_t nhop_handle) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t *mbrs = NULL;
  switch_uint16_t num_mbrs = 0;
  switch_uint16_t index = 0;
  switch_mcast_node_type_t node_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle) ||
                SWITCH_ECMP_HANDLE(nhop_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle) || !SWITCH_NHOP_HANDLE(nhop_handle) ||
      !SWITCH_ECMP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "invalid handle(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "multicast member add failed on device %d "
        "mgid handle 0x%lx nhop handle 0x%lx: "
        "nhop get failed(%s)\n",
        device,
        mgid_handle,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (nhop_info->id_type == SWITCH_NHOP_ID_TYPE_ONE_PATH) {
    num_mbrs = 1;
    mbrs = &nhop_handle;
    node_type = SWITCH_NODE_TYPE_SINGLE;
  } else {
    status = switch_api_ecmp_members_get(device, nhop_handle, &num_mbrs, &mbrs);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "ecmp members get failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    node_type = SWITCH_NODE_TYPE_ECMP;
  }

  for (index = 0; index < num_mbrs; index++) {
    status = switch_multicast_nhop_member_delete_i(
        device, mgid_handle, node_type, mbrs[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "multicast member add failed on device %d "
          "mgid handle 0x%lx nhop handle 0x%lx: "
          "nhop member add internal failed(%s)\n",
          device,
          mgid_handle,
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_multicast_member_add_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_mcast_member_info_t *intf_mbrs = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_rid_type_t rid_type = SWITCH_RID_TYPE_INNER_REPLICA;
  switch_uint16_t index = 0;
  switch_tunnel_t tunnel_id = 0;
  switch_tunnel_type_egress_t tunnel_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_id_t dmac_index = 0;

  SWITCH_ASSERT(mbrs != NULL);
  SWITCH_ASSERT(num_mbrs != 0);
  if (!mbrs || num_mbrs == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_interface_get(device, num_mbrs, mbrs, &intf_mbrs);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_mbrs; index++) {
    intf_handle = intf_mbrs[index].intf_handle;
    bd_handle = intf_mbrs[index].bd_handle;

    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_mcast_rid_get(device, bd_handle, intf_handle, &rid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
      rid_info = SWITCH_MALLOC(device, sizeof(switch_rid_info_t), 0x1);
      if (!rid_info) {
        status = SWITCH_STATUS_NO_MEMORY;
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(rid_info, 0x0, sizeof(switch_rid_info_t));
      rid_info->rid = rid;
      rid_info->rid_pd_hdl = SWITCH_PD_INVALID_HANDLE;

      status = switch_pd_rid_table_entry_add(device,
                                             rid_type,
                                             rid,
                                             handle_to_id(bd_handle),
                                             tunnel_type,
                                             tunnel_id,
                                             dmac_index,
                                             &rid_info->rid_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status =
          SWITCH_ARRAY_INSERT(&mcast_ctx->rid_array, rid, (void *)rid_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_egress_ifindex_table_entry_add(
          device,
          rid,
          intf_info->ifindex,
          &rid_info->mcast_egress_ifindex_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    rid_info->ref_count++;
    xid = bd_info->xid;

    status = switch_mcast_node_get(
        device, rid, xid, SWITCH_NODE_TYPE_SINGLE, mcast_info, &mcast_node);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (!mcast_node) {
      mcast_node = SWITCH_MALLOC(device, sizeof(switch_mcast_node_t), 0x1);
      if (!mcast_node) {
        status = SWITCH_STATUS_NO_MEMORY;
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(mcast_node, 0x0, sizeof(switch_mcast_node_t));

      status = SWITCH_LIST_INSERT(
          &mcast_info->node_list, &mcast_node->node, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MCAST_NODE_RID(mcast_node) = rid;
      mcast_node->xid = xid;

      status =
          switch_mcast_port_map_update(device, mcast_node, intf_handle, TRUE);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_entry_add(device, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_mgid_table_entry_add(
          device, mcast_info->mgrp_hdl, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    } else {
      status =
          switch_mcast_port_map_update(device, mcast_node, intf_handle, TRUE);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_entry_update(device, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_multicast_member_delete_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_mcast_member_info_t *intf_mbrs = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_uint16_t index = 0;
  bool delete_node = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mbrs != NULL);
  SWITCH_ASSERT(num_mbrs != 0);
  if (!mbrs || num_mbrs == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_interface_get(device, num_mbrs, mbrs, &intf_mbrs);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    if (intf_mbrs) {
      SWITCH_FREE(device, intf_mbrs);
    }
    return status;
  }

  for (index = 0; index < num_mbrs; index++) {
    intf_handle = intf_mbrs[index].intf_handle;
    bd_handle = intf_mbrs[index].bd_handle;

    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
    if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
    if (!SWITCH_BD_HANDLE(bd_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_mcast_rid_get(device, bd_handle, intf_handle, &rid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    rid_info->ref_count--;
    if (rid_info->ref_count == 0) {
      status = switch_pd_rid_table_entry_delete(device, rid_info->rid_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = SWITCH_ARRAY_DELETE(&mcast_ctx->rid_array, rid);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_egress_ifindex_table_entry_delete(
          device, rid_info->mcast_egress_ifindex_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    xid = bd_info->xid;

    status = switch_mcast_node_get(
        device, rid, xid, SWITCH_NODE_TYPE_SINGLE, mcast_info, &mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status =
        switch_mcast_port_map_update(device, mcast_node, intf_handle, FALSE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    delete_node = switch_mcast_node_empty(mcast_node);
    if (delete_node) {
      status = switch_pd_mcast_mgid_table_entry_delete(
          device, mcast_info->mgrp_hdl, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_pd_mcast_entry_delete(device, mcast_node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      status = SWITCH_LIST_DELETE(&mcast_info->node_list, &mcast_node->node);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_FREE(device, mcast_node);
    }
  }

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_multicast_member_get_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    switch_uint16_t *num_mbrs,
    switch_mcast_member_t **mbrs) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_mcast_rpf_group_get(const switch_device_t device,
                                           const switch_mcast_mode_t mc_mode,
                                           const switch_uint16_t num_rpf_bd,
                                           const switch_handle_t *rpf_bd_list,
                                           switch_mrpf_group_t *mrpf_group) {
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(rpf_bd_list || mrpf_group || num_rpf_bd != 0);
  if (!rpf_bd_list || !mrpf_group || num_rpf_bd == 0) {
    SWITCH_LOG_ERROR("mgid rpf group get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (mc_mode == SWITCH_API_MCAST_IPMC_NONE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mgid rpf group get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * TODO: Create a mrpf group to manage the list of rpf BD (or)
   * new API to create mrpf group
   */
  if (num_rpf_bd != 1) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mgid rpf group get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_BIDIR) {
    *mrpf_group = rpf_bd_list[0];
    return status;
  }

  handle = rpf_bd_list[0];
  status = switch_bd_handle_get(device, handle, &bd_handle);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast rpf group get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mrpf_group = handle_to_id(bd_handle);

  return status;
}

switch_status_t switch_api_multicast_ecmp_member_add_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  switch_mcast_node_t *ecmp_mcast_node = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t *ecmp_mbrs = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_rid_type_t rid_type = SWITCH_RID_TYPE_INNER_REPLICA;
  switch_uint16_t index = 0;
  switch_tunnel_t tunnel_id = 0;
  switch_tunnel_type_egress_t tunnel_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t num_ecmp_mbrs = 0;
  switch_rif_info_t *rif_info = NULL;

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_nhop_handle));
  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_ecmp_node_get(
      device, mcast_info, ecmp_nhop_handle, &ecmp_mcast_node);

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_STATUS_ITEM_ALREADY_EXISTS;
  }

  status = switch_api_ecmp_members_get(
      device, ecmp_nhop_handle, &num_ecmp_mbrs, &ecmp_mbrs);
  if (status != SWITCH_STATUS_SUCCESS || num_ecmp_mbrs == 0) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ecmp_mcast_node = SWITCH_MALLOC(device, sizeof(switch_mcast_node_t), 0x1);
  if (!ecmp_mcast_node) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  SWITCH_MEMSET(ecmp_mcast_node, 0x0, sizeof(switch_mcast_node_t));
  ecmp_mcast_node->node_type = SWITCH_NODE_TYPE_ECMP;
  SWITCH_MCAST_ECMP_INFO_HDL(ecmp_mcast_node) = ecmp_nhop_handle;

  status =
      SWITCH_LIST_INIT(&(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  /* Create ECMP group */
  status = switch_pd_mcast_ecmp_group_create(device, ecmp_mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  for (index = 0; index < num_ecmp_mbrs; index++) {
    nhop_handle = ecmp_mbrs[index];
    tunnel_id = 0;
    tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_NONE;

    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    intf_handle = nhop_info->spath.api_nhop_info.rif_handle;

    switch (switch_handle_type_get(intf_handle)) {
      case SWITCH_HANDLE_TYPE_RIF:
        status = switch_rif_get(device, intf_handle, &rif_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        CHECK_CLEAN(rif_info->api_rif_info.rif_type != SWITCH_RIF_TYPE_INTF,
                    SWITCH_STATUS_INVALID_PARAMETER);
        intf_handle = rif_info->api_rif_info.intf_handle;

        status = switch_interface_get(device, intf_handle, &intf_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        bd_handle = rif_info->bd_handle;

        break;
      case SWITCH_HANDLE_TYPE_INTERFACE:
        CHECK_CLEAN(SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_TUNNEL,
                    SWITCH_STATUS_INVALID_PARAMETER);

        status = switch_interface_get(device, intf_handle, &intf_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        bd_handle = intf_info->bd_handle;
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
    }

    SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = switch_mcast_rid_get(device, bd_handle, intf_handle, &rid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
      rid_info = SWITCH_MALLOC(device, sizeof(switch_rid_info_t), 0x1);
      if (!rid_info) {
        status = SWITCH_STATUS_NO_MEMORY;
        SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto clean;
      }

      SWITCH_MEMSET(rid_info, 0x0, sizeof(switch_rid_info_t));
      rid_info->rid = rid;
      rid_info->rid_pd_hdl = SWITCH_PD_INVALID_HANDLE;

      tunnel_id = 0;
      tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_NONE;

      if (SWITCH_INTF_TYPE(intf_info) == SWITCH_INTERFACE_TYPE_TUNNEL) {
        // tunnel_type = intf_info->egress_tunnel_type;
        tunnel_id = handle_to_id(intf_handle);
      }

      status = switch_pd_rid_table_entry_add(device,
                                             rid_type,
                                             rid,
                                             handle_to_id(bd_handle),
                                             tunnel_type,
                                             tunnel_id,
                                             0,
                                             &rid_info->rid_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto clean;
      }

      status =
          SWITCH_ARRAY_INSERT(&mcast_ctx->rid_array, rid, (void *)rid_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto clean;
      }
    }

    rid_info->ref_count++;
    xid = bd_info->xid;

    mcast_node = SWITCH_MALLOC(device, sizeof(switch_mcast_node_t), 0x1);
    if (!mcast_node) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    SWITCH_MEMSET(mcast_node, 0x0, sizeof(switch_mcast_node_t));

    SWITCH_MCAST_NODE_RID(mcast_node) = rid;
    mcast_node->xid = xid;

    status =
        switch_mcast_port_map_update(device, mcast_node, intf_handle, TRUE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = switch_pd_mcast_entry_add(device, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status =
        SWITCH_LIST_INSERT(&(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)),
                           &mcast_node->node,
                           mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status =
        switch_pd_mcast_ecmp_entry_add(device, ecmp_mcast_node, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }
  }

  status = SWITCH_LIST_INSERT(
      &mcast_info->node_list, &ecmp_mcast_node->node, ecmp_mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  status = switch_pd_mcast_mgid_table_ecmp_entry_add(
      device, mcast_info->mgrp_hdl, ecmp_mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

clean:
  if (ecmp_mbrs) {
    SWITCH_FREE(device, ecmp_mbrs);
  }

  return status;
}

switch_status_t switch_api_multicast_ecmp_member_delete_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_info_t *mcast_info = NULL;
  switch_mcast_node_t *mcast_node = NULL;
  switch_mcast_node_t *ecmp_mcast_node = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_rid_info_t *rid_info = NULL;
  switch_handle_t *ecmp_mbrs = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_rid_t rid = 0;
  switch_xid_t xid = 0;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t num_ecmp_mbrs = 0;
  switch_rif_info_t *rif_info = NULL;

  SWITCH_ASSERT(SWITCH_ECMP_HANDLE(ecmp_nhop_handle));
  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mgid_get(device, mgid_handle, &mcast_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_ecmp_members_get(
      device, ecmp_nhop_handle, &num_ecmp_mbrs, &ecmp_mbrs);
  if (status != SWITCH_STATUS_SUCCESS || num_ecmp_mbrs == 0) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /* Find the ECMP mcast node corresponding to the ecmp nexthop */
  status = switch_mcast_ecmp_node_get(
      device, mcast_info, ecmp_nhop_handle, &ecmp_mcast_node);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_ecmp_mbrs; index++) {
    nhop_handle = ecmp_mbrs[index];

    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
    status = switch_nhop_get(device, nhop_handle, &nhop_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    intf_handle = nhop_info->spath.api_nhop_info.rif_handle;

    switch (switch_handle_type_get(intf_handle)) {
      case SWITCH_HANDLE_TYPE_RIF:
        status = switch_rif_get(device, intf_handle, &rif_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        CHECK_CLEAN(rif_info->api_rif_info.rif_type != SWITCH_RIF_TYPE_INTF,
                    SWITCH_STATUS_INVALID_PARAMETER);
        intf_handle = rif_info->api_rif_info.intf_handle;

        status = switch_interface_get(device, intf_handle, &intf_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        bd_handle = rif_info->bd_handle;

        break;
      case SWITCH_HANDLE_TYPE_INTERFACE:
        CHECK_CLEAN(SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_TUNNEL,
                    SWITCH_STATUS_INVALID_PARAMETER);

        status = switch_interface_get(device, intf_handle, &intf_info);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
        bd_handle = intf_info->bd_handle;
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
    }

    SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = switch_mcast_rid_get(device, bd_handle, intf_handle, &rid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = SWITCH_ARRAY_GET(&mcast_ctx->rid_array, rid, (void **)&rid_info);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    rid_info->ref_count--;
    if (rid_info->ref_count == 0) {
      status = switch_pd_rid_table_entry_delete(device, rid_info->rid_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto clean;
      }

      status = SWITCH_ARRAY_DELETE(&mcast_ctx->rid_array, rid);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto clean;
      }
    }

    xid = bd_info->xid;

    status = switch_mcast_ecmp_member_node_get(
        device, rid, xid, ecmp_mcast_node, &mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status =
        switch_mcast_port_map_update(device, mcast_node, intf_handle, FALSE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status =
        switch_pd_mcast_ecmp_entry_remove(device, ecmp_mcast_node, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status = switch_pd_mcast_entry_delete(device, mcast_node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    status =
        SWITCH_LIST_DELETE(&(SWITCH_MCAST_ECMP_INFO_NODE_LIST(ecmp_mcast_node)),
                           &mcast_node->node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto clean;
    }

    SWITCH_FREE(device, mcast_node);
  }

  status = SWITCH_LIST_DELETE(&mcast_info->node_list, &ecmp_mcast_node->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  status = switch_pd_mcast_mgid_table_ecmp_entry_remove(
      device, mcast_info->mgrp_hdl, ecmp_mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  status = switch_pd_mcast_ecmp_group_delete(device, ecmp_mcast_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast ecmp member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto clean;
  }

  SWITCH_FREE(device, ecmp_mcast_node);

clean:

  return status;
}

switch_status_t switch_api_multicast_mroute_add_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  bool core_entry = TRUE;
  switch_rpf_info_t *rpf_info = NULL;
  switch_mcast_group_key_t group_key;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (mgid_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
    if (!SWITCH_MGID_HANDLE(mgid_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mroute add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  if (rpf_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_handle));
    if (!SWITCH_RPF_GROUP_HANDLE(rpf_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "mroute add failed on device %d "
          "rpf group handle %lx: "
          "rpf group handle invalid(%s)\n",
          device,
          rpf_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_rpf_group_get(device, rpf_handle, &rpf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mroute group add failed on device %d: "
          "rpf group handle %lx: "
          "rpf group get failed(%s)\n",
          device,
          rpf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  group_info = SWITCH_MALLOC(device, sizeof(switch_mcast_group_info_t), 0x1);
  if (!group_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMCPY(&group_info->group_key, &group_key, sizeof(group_key));
  group_info->mgid_handle = mgid_handle;
  group_info->rpf_handle = rpf_handle;

  status = switch_pd_mcast_table_entry_add(device,
                                           handle_to_id(mgid_handle),
                                           mc_mode,
                                           group_info,
                                           core_entry,
                                           TRUE,
                                           rpf_info->rpf_group);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_INSERT(&mcast_ctx->mcast_group_hashtable,
                                   &group_info->node,
                                   (void *)&group_key,
                                   (void *)group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;
cleanup:
  SWITCH_FREE(device, group_info);
  return status;
}

switch_status_t switch_api_multicast_mroute_delete_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  bool core_entry = TRUE;
  switch_mcast_group_key_t group_key;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status =
      switch_pd_mcast_table_entry_delete(device, group_info, core_entry, TRUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_DELETE(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_FREE(device, group_info);

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_multicast_mroute_stats_get_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_counter_t *counter) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  bool core_entry = TRUE;
  switch_mcast_group_key_t group_key;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmroute stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(counter != NULL);
  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mcast_table_entry_stats_get(
      device, group_info, core_entry, TRUE, counter);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_multicast_mroute_miss_mgid_set_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vlan_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute miss mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (mgid_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
    if (!SWITCH_MGID_HANDLE(mgid_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mroute miss mgid set failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mroute miss mgid set failed for device %d: %s\n"
        "vlan handle %lx: ",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mroute miss mgid set failed for device %d: %s\n"
        "vlan handle %lx: ",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_bd_mrouters_handle_set(device, vlan_info->bd_handle, mgid_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute miss mgid set failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_multicast_mroute_mgid_set_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  bool core_entry = TRUE;
  switch_mcast_group_key_t group_key;
  switch_rpf_info_t *rpf_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (mgid_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
    if (!SWITCH_MGID_HANDLE(mgid_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, group_info->rpf_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mroute mgid set failed on device %d: "
        "rpf group handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        group_info->rpf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mcast_table_entry_update(device,
                                              handle_to_id(mgid_handle),
                                              mc_mode,
                                              group_info,
                                              core_entry,
                                              TRUE,
                                              rpf_info->rpf_group);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute mgid set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
  group_info->mgid_handle = mgid_handle;

  return status;
cleanup:
  return status;
}

switch_status_t switch_api_multicast_mroute_rpf_set_internal(
    const switch_device_t device,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  bool core_entry = TRUE;
  switch_mcast_group_key_t group_key;
  switch_rpf_info_t *rpf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (rpf_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_handle));
    if (!SWITCH_RPF_GROUP_HANDLE(rpf_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_rpf_group_get(device, rpf_handle, &rpf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mroute rpf set failed on device %d: "
          "rpf group handle %lx: "
          "rpf group get failed(%s)\n",
          device,
          rpf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_mcast_table_entry_update(device,
                                         handle_to_id(group_info->mgid_handle),
                                         mc_mode,
                                         group_info,
                                         core_entry,
                                         TRUE,
                                         rpf_info->rpf_group);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute rpf set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
  group_info->rpf_handle = rpf_handle;

  return status;
cleanup:
  return status;
}

switch_status_t switch_api_multicast_l2route_add_internal(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t network_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_mcast_group_key_t group_key;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  bool core_entry = FALSE;
  switch_mcast_mode_t mc_mode = SWITCH_API_MCAST_IPMC_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_bd_handle_get(device, network_handle, &bd_handle);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));
  if (!SWITCH_MGID_HANDLE(mgid_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = bd_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  group_info = SWITCH_MALLOC(device, sizeof(switch_mcast_group_info_t), 0x1);
  if (!group_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&group_info->group_key, &group_key, sizeof(group_key));
  group_info->mgid_handle = mgid_handle;

  status = switch_pd_mcast_table_entry_add(device,
                                           handle_to_id(mgid_handle),
                                           mc_mode,
                                           group_info,
                                           core_entry,
                                           FALSE,
                                           0x0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_INSERT(&mcast_ctx->mcast_group_hashtable,
                                   &group_info->node,
                                   (void *)&group_key,
                                   (void *)group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:

  SWITCH_FREE(device, group_info);

  return status;
}

switch_status_t switch_api_multicast_l2route_delete_internal(
    const switch_device_t device,
    const switch_handle_t network_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_mcast_group_key_t group_key;
  bool core_entry = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_handle_get(device, network_handle, &bd_handle);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  if (!src_ip || !grp_ip) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = bd_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_mcast_table_entry_delete(device, group_info, core_entry, FALSE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_DELETE(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_FREE(device, group_info);

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_multicast_mroute_tree_get_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle,
    switch_handle_t *rpf_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_mcast_group_key_t group_key;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    SWITCH_LOG_ERROR("mcast mroute tree get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  SWITCH_ASSERT(mgid_handle != NULL);
  if (!src_ip || !grp_ip || !mgid_handle) {
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = vrf_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mroute delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mgid_handle = group_info->mgid_handle;
  *rpf_handle = group_info->rpf_handle;
  return status;
}

switch_status_t switch_api_multicast_l2route_tree_get_internal(
    const switch_device_t device,
    const switch_handle_t network_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle) {
  switch_mcast_context_t *mcast_ctx = NULL;
  switch_mcast_group_info_t *group_info = NULL;
  switch_mcast_group_key_t group_key;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MCAST, (void **)&mcast_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mmcast init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_bd_handle_get(device, network_handle, &bd_handle);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L2 mroute add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(src_ip != NULL);
  SWITCH_ASSERT(grp_ip != NULL);
  SWITCH_ASSERT(mgid_handle != NULL);
  if (!src_ip || !grp_ip || !mgid_handle) {
    SWITCH_LOG_ERROR("mcast mroute tree get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMSET(&group_key, 0, sizeof(switch_mcast_group_key_t));
  SWITCH_MEMCPY(&group_key.src_ip, src_ip, sizeof(switch_ip_addr_t));
  SWITCH_MEMCPY(&group_key.grp_ip, grp_ip, sizeof(switch_ip_addr_t));
  group_key.handle = bd_handle;
  group_key.sg_entry = (group_key.src_ip.prefix_len == 0) ? false : true;

  status = SWITCH_HASHTABLE_SEARCH(&mcast_ctx->mcast_group_hashtable,
                                   (void *)&group_key,
                                   (void **)&group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mcast mroute tree get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mgid_handle = group_info->mgid_handle;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_multicast_index_create(
    const switch_device_t device, switch_handle_t *mgid_handle) {
  SWITCH_MT_WRAP(
      switch_api_multicast_index_create_internal(device, mgid_handle))
}

switch_status_t switch_api_multicast_mroute_delete(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_delete_internal(
      device, vrf_handle, src_ip, grp_ip))
}

switch_status_t switch_api_multicast_mroute_stats_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_counter_t *counter) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_stats_get_internal(
      device, vrf_handle, src_ip, grp_ip, counter))
}

switch_status_t switch_api_multicast_mroute_miss_mgid_set(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vlan_handle) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_miss_mgid_set_internal(
      device, mgid_handle, vlan_handle))
}

switch_status_t switch_api_multicast_index_delete(
    const switch_device_t device, const switch_handle_t mgid_handle) {
  SWITCH_MT_WRAP(
      switch_api_multicast_index_delete_internal(device, mgid_handle))
}

switch_status_t switch_api_multicast_mroute_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_add_internal(
      device, mgid_handle, rpf_handle, vrf_handle, src_ip, grp_ip, mc_mode))
}

switch_status_t switch_api_multicast_mroute_mgid_set(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_mgid_set_internal(
      device, mgid_handle, vrf_handle, src_ip, grp_ip, mc_mode))
}

switch_status_t switch_api_multicast_mroute_rpf_set(
    const switch_device_t device,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_rpf_set_internal(
      device, rpf_handle, vrf_handle, src_ip, grp_ip, mc_mode))
}

switch_status_t switch_api_multicast_l2route_delete(
    const switch_device_t device,
    const switch_handle_t bd_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  SWITCH_MT_WRAP(switch_api_multicast_l2route_delete_internal(
      device, bd_handle, src_ip, grp_ip))
}

switch_status_t switch_api_multicast_l2route_tree_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle) {
  SWITCH_MT_WRAP(switch_api_multicast_l2route_tree_get_internal(
      device, vlan_handle, src_ip, grp_ip, mgid_handle))
}

switch_status_t switch_api_multicast_member_delete(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs) {
  SWITCH_MT_WRAP(switch_api_multicast_member_delete_internal(
      device, mgid_handle, num_mbrs, mbrs))
}

switch_status_t switch_api_multicast_ecmp_member_delete(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle) {
  SWITCH_MT_WRAP(switch_api_multicast_ecmp_member_delete_internal(
      device, mgid_handle, ecmp_nhop_handle))
}

switch_status_t switch_api_multicast_member_get(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    switch_uint16_t *num_mbrs,
    switch_mcast_member_t **mbrs) {
  SWITCH_MT_WRAP(switch_api_multicast_member_get_internal(
      device, mgid_handle, num_mbrs, mbrs))
}

switch_status_t switch_api_multicast_member_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs) {
  SWITCH_MT_WRAP(switch_api_multicast_member_add_internal(
      device, mgid_handle, num_mbrs, mbrs))
}

switch_status_t switch_api_multicast_ecmp_member_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle) {
  SWITCH_MT_WRAP(switch_api_multicast_ecmp_member_add_internal(
      device, mgid_handle, ecmp_nhop_handle))
}

switch_status_t switch_api_multicast_l2route_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t bd_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip) {
  SWITCH_MT_WRAP(switch_api_multicast_l2route_add_internal(
      device, mgid_handle, bd_handle, src_ip, grp_ip))
}

switch_status_t switch_api_multicast_mroute_tree_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle,
    switch_handle_t *rpf_handle) {
  SWITCH_MT_WRAP(switch_api_multicast_mroute_tree_get_internal(
      device, vrf_handle, src_ip, grp_ip, mgid_handle, rpf_handle))
}
