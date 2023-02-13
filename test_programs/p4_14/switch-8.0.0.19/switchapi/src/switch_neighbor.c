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

#include "switchapi/switch_neighbor.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_neighbor_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_neighbor_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_tunnel_dmac_rewrite_table_key_init(
    void *args, switch_uint8_t *key, switch_uint32_t *len) {
  switch_mac_addr_t *mac = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  mac = (switch_mac_addr_t *)args;

  SWITCH_MEMSET(key, 0x0, SWITCH_DMAC_REWRITE_HASH_KEY_SIZE);

  SWITCH_MEMCPY(key, mac, sizeof(switch_mac_addr_t));

  *len += sizeof(switch_mac_addr_t);

  SWITCH_ASSERT(*len == SWITCH_DMAC_REWRITE_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_tunnel_dmac_rewrite_hash_compare(const void *key1,
                                                       const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_DMAC_REWRITE_HASH_KEY_SIZE);
}

switch_status_t switch_neighbor_entry_hash_key_init(void *args,
                                                    switch_uint8_t *key,
                                                    switch_uint32_t *len) {
  switch_neighbor_dmac_entry_t *neighbor_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  neighbor_entry = (switch_neighbor_dmac_entry_t *)args;

  SWITCH_MEMSET(key, 0x0, SWITCH_NEIGHBOR_TABLE_HASH_KEY_SIZE);

  SWITCH_MEMCPY(key, &neighbor_entry->bd_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY(key + *len, &neighbor_entry->mac, sizeof(switch_mac_addr_t));
  *len += sizeof(switch_mac_addr_t);

  SWITCH_ASSERT(*len == SWITCH_NEIGHBOR_TABLE_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_neighbor_entry_hash_compare(const void *key1,
                                                  const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_NEIGHBOR_TABLE_HASH_KEY_SIZE);
}

switch_status_t switch_neighbor_init(switch_device_t device) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_size_t tunnel_dmac_table_size = 0;
  switch_size_t rewrite_table_size = 0;
  switch_size_t tmp_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  neighbor_ctx = SWITCH_MALLOC(device, sizeof(switch_neighbor_context_t), 0x1);
  if (!neighbor_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_NEIGHBOR, (void *)neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_TUNNEL_DMAC_REWRITE, &tunnel_dmac_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  neighbor_ctx->tunnel_dmac_rewrite_hashtable.size = tunnel_dmac_table_size;
  neighbor_ctx->tunnel_dmac_rewrite_hashtable.compare_func =
      switch_tunnel_dmac_rewrite_hash_compare;
  neighbor_ctx->tunnel_dmac_rewrite_hashtable.key_func =
      switch_tunnel_dmac_rewrite_table_key_init;
  neighbor_ctx->tunnel_dmac_rewrite_hashtable.hash_seed =
      SWITCH_TUNNEL_DMAC_REWRITE_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&neighbor_ctx->tunnel_dmac_rewrite_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status =
      switch_api_table_size_get(device, SWITCH_TABLE_REWRITE, &tmp_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  rewrite_table_size += tmp_table_size;
  tmp_table_size = 0;

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_TUNNEL_REWRITE, &tmp_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  rewrite_table_size += tmp_table_size;
  SWITCH_ASSERT(rewrite_table_size != 0);

  neighbor_ctx->neighbor_dmac_hashtable.size = rewrite_table_size;
  neighbor_ctx->neighbor_dmac_hashtable.compare_func =
      switch_neighbor_entry_hash_compare;
  neighbor_ctx->neighbor_dmac_hashtable.key_func =
      switch_neighbor_entry_hash_key_init;
  neighbor_ctx->neighbor_dmac_hashtable.hash_seed = SWITCH_NEIGHBOR_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&neighbor_ctx->neighbor_dmac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_id_allocator_new(
      device, tunnel_dmac_table_size, FALSE, &neighbor_ctx->dmac_rewrite_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_NEIGHBOR, rewrite_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  status = switch_neighbor_free(device);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_neighbor_free(switch_device_t device) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DONE(&neighbor_ctx->tunnel_dmac_rewrite_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&neighbor_ctx->neighbor_dmac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_NEIGHBOR);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status =
      switch_api_id_allocator_destroy(device, neighbor_ctx->dmac_rewrite_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  SWITCH_FREE(device, neighbor_ctx);
  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_NEIGHBOR, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_neighbor_tunnel_dmac_rewrite_add(
    switch_device_t device, switch_mac_addr_t *mac, switch_id_t *dmac_index) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_tunnel_dmac_rewrite_t *dmac_rewrite = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac add failed on device %d mac %s: "
        "neighbor context get failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&neighbor_ctx->tunnel_dmac_rewrite_hashtable,
                                   (void *)mac,
                                   (void **)&dmac_rewrite);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac add failed on device %d mac %s: "
        "neighbor tunnel dmac hashtable search failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    *dmac_index = dmac_rewrite->index;
    dmac_rewrite->ref_count++;
    return status;
  }

  dmac_rewrite = SWITCH_MALLOC(device, sizeof(switch_tunnel_dmac_rewrite_t), 1);
  if (!dmac_rewrite) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac add failed on device %d mac %s: "
        "neighbor tunnel dmac malloc failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(dmac_rewrite, 0x0, sizeof(switch_tunnel_dmac_rewrite_t));

  status = switch_api_id_allocator_allocate(
      device, neighbor_ctx->dmac_rewrite_index, dmac_index);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMCPY(&dmac_rewrite->mac, mac, sizeof(switch_mac_addr_t));

  dmac_rewrite->index = *dmac_index;
  dmac_rewrite->ref_count = 1;

  status = SWITCH_HASHTABLE_INSERT(&neighbor_ctx->tunnel_dmac_rewrite_hashtable,
                                   &dmac_rewrite->node,
                                   (void *)mac,
                                   (void *)dmac_rewrite);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_pd_tunnel_dmac_rewrite_table_entry_add(
      device, *dmac_index, mac, &dmac_rewrite->rewrite_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac add failed on device %d mac %s: "
        "neighbor tunnel dmac rewrite table add failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_neighbor_tunnel_dmac_rewrite_delete(
    switch_device_t device, switch_mac_addr_t *mac) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_tunnel_dmac_rewrite_t *dmac_rewrite = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac delete failed on device %d mac %s: "
        "neighbor context get failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&neighbor_ctx->tunnel_dmac_rewrite_hashtable,
                                   (void *)mac,
                                   (void **)&dmac_rewrite);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac delete failed on device %d mac %s: "
        "neighbor tunnel dmac hashtable search failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  if (dmac_rewrite->ref_count > 1) {
    dmac_rewrite->ref_count--;
    return SWITCH_STATUS_SUCCESS;
  }

  status = switch_pd_tunnel_dmac_rewrite_table_entry_delete(
      device, dmac_rewrite->rewrite_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor tunnel dmac delete failed on device %d mac %s: "
        "neighbor tunnel rewrite table delete failed:(%s)\n",
        device,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(&neighbor_ctx->tunnel_dmac_rewrite_hashtable,
                                   (void *)mac,
                                   (void *)dmac_rewrite);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_id_allocator_release(
      device, neighbor_ctx->dmac_rewrite_index, dmac_rewrite->index);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_FREE(device, dmac_rewrite);

  return status;
}

switch_status_t switch_neighbor_dmac_hashtable_insert(
    switch_device_t device,
    switch_neighbor_info_t *neighbor_info,
    switch_neighbor_dmac_entry_t *neighbor_entry) {
  switch_neighbor_nhop_list_t *nhop_list = NULL;
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = SWITCH_HASHTABLE_SEARCH(&neighbor_ctx->neighbor_dmac_hashtable,
                                   (void *)neighbor_entry,
                                   (void **)&nhop_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    nhop_list =
        SWITCH_MALLOC(nhop_list, sizeof(switch_neighbor_nhop_list_t), 1);
    if (!nhop_list) {
      SWITCH_LOG_ERROR("neighbor hash insert failed for device %d: %s %s\n",
                       device,
                       "failed malloc",
                       switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(nhop_list, 0x0, sizeof(switch_neighbor_nhop_list_t));
    status = SWITCH_LIST_INIT(&(nhop_list->list));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("neighbor hash insert failed for device %d: %s %s\n",
                       device,
                       "list init failed",
                       switch_error_to_string(status));
      return status;
    }

    nhop_list->handle = neighbor_entry->bd_handle;
    SWITCH_MEMCPY(
        &nhop_list->mac, &neighbor_entry->mac, sizeof(switch_mac_addr_t));
    status = SWITCH_HASHTABLE_INSERT(&neighbor_ctx->neighbor_dmac_hashtable,
                                     &(nhop_list->node),
                                     (void *)neighbor_entry,
                                     (void *)nhop_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("neighbor hash insert failed for device %d: %s %s\n",
                       device,
                       "hashtable insert failed",
                       switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_LIST_INSERT(
      &(nhop_list->list), &neighbor_info->node, neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor hash insert failed for device %d: %s %s\n",
                     device,
                     "list insert failed",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_neighbor_dmac_hashtable_delete(
    switch_device_t device,
    switch_neighbor_info_t *neighbor_info,
    switch_neighbor_dmac_entry_t *neighbor_entry) {
  switch_neighbor_nhop_list_t *nhop_list = NULL;
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = SWITCH_HASHTABLE_SEARCH(&neighbor_ctx->neighbor_dmac_hashtable,
                                   (void *)neighbor_entry,
                                   (void **)&nhop_list);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_LIST_DELETE(&nhop_list->list, &neighbor_info->node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("neighbor hash delete failed for device %d: %s %s\n",
                       device,
                       "list delete failed",
                       switch_error_to_string(status));
      return status;
    }
    /* Last neighbor deleted. Remove this list from the hash */
    if (SWITCH_LIST_COUNT(&nhop_list->list) == 0) {
      status = SWITCH_HASHTABLE_DELETE(&neighbor_ctx->neighbor_dmac_hashtable,
                                       (void *)neighbor_entry,
                                       (void **)&nhop_list);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("neighbor hash delete failed for device %d: %s\n",
                         device,
                         "hashtable delete failed",
                         switch_error_to_string(status));
        return status;
      }
      SWITCH_FREE(device, nhop_list);
    }
  }

  return status;
}

switch_status_t switch_neighbor_entry_rewrite_add(
    switch_device_t device, switch_handle_t neighbor_handle) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_neighbor_info_t *api_neighbor_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_mpls_info_t *mpls_info = NULL;
  switch_mpls_label_stack_info_t *ls_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_neighbor_dmac_entry_t neighbor_entry;
  switch_nhop_t nhop = 0;
  switch_bd_t bd = 0;
  switch_id_t tunnel_dmac_index = 0;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ls_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mpls_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_tunnel_t tunnel_index = 0;
  switch_vni_t tunnel_vni = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NEIGHBOR_HANDLE(neighbor_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor rewrite table add failed on device %d handle 0x%lx: "
        "neighbor context get failed:(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_get(device, neighbor_handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor rewrite table add failed on device %d handle 0x%lx: "
        "neighbor get failed:(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }
  neighbor_info->rewrite_pd_hdl = SWITCH_PD_INVALID_HANDLE;

  api_neighbor_info = &neighbor_info->api_neighbor_info;

  nhop_handle = neighbor_info->nhop_handle;
  nhop = handle_to_id(nhop_handle);

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (SWITCH_NHOP_TYPE(nhop_info) == SWITCH_NHOP_TYPE_IP) {
    rif_handle = nhop_info->spath.api_nhop_info.rif_handle;
    SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
    status = switch_rif_get(device, rif_handle, &rif_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    bd_handle = rif_info->bd_handle;
    bd = handle_to_id(bd_handle);

    status = switch_pd_rewrite_table_unicast_rewrite_entry_add(
        device,
        bd,
        nhop,
        api_neighbor_info->mac_addr,
        api_neighbor_info->rw_type,
        &neighbor_info->rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite table add failed on device %d handle 0x%lx: "
          "neighbor unicast rewrite entry add failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(&neighbor_entry, 0x0, sizeof(neighbor_entry));
    neighbor_entry.bd_handle = rif_info->bd_handle;
    SWITCH_MEMCPY(&neighbor_entry.mac,
                  &api_neighbor_info->mac_addr,
                  sizeof(switch_mac_addr_t));
    neighbor_info->handle = rif_info->bd_handle;
    SWITCH_MEMCPY(&neighbor_info->mac,
                  &api_neighbor_info->mac_addr,
                  sizeof(switch_mac_addr_t));
    status = switch_neighbor_dmac_hashtable_insert(
        device, neighbor_info, &neighbor_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite table add failed on device %d handle 0x%lx: "
          "neighbor hashtable insert failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }

    if (rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_VLAN ||
        rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_LN) {
      status = switch_nhop_l3_vlan_interface_resolve(
          device, nhop_handle, bd_handle, &api_neighbor_info->mac_addr, false);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "neighbor rewrite table add failed on device %d handle 0x%lx: "
            "neighbor l3 vlan resolve failed:(%s)\n",
            device,
            neighbor_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    if (SWITCH_MAC_VALID(api_neighbor_info->mac_addr)) {
      status = switch_neighbor_tunnel_dmac_rewrite_add(
          device,
          &api_neighbor_info->mac_addr,
          &neighbor_info->tunnel_dmac_index);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "neighbor rewrite table add failed on device %d handle 0x%lx: "
            "neighbor tunnel dmac add failed:(%s)\n",
            device,
            neighbor_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  } else if (SWITCH_NHOP_TYPE(nhop_info) == SWITCH_NHOP_TYPE_TUNNEL) {
    bd_handle = nhop_info->spath.bd_handle;
    bd = handle_to_id(bd_handle);
    tunnel_handle = nhop_info->spath.api_nhop_info.tunnel_handle;
    tunnel_vni = nhop_info->spath.api_nhop_info.tunnel_vni;
    status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    tunnel_index = handle_to_id(tunnel_handle);

    status = switch_pd_rewrite_table_tunnel_rewrite_entry_add(
        device,
        bd,
        nhop,
        api_neighbor_info,
        tunnel_index,
        tunnel_vni,
        nhop_info->spath.tunnel_dst_index,
        tunnel_info->egress_tunnel_type,
        &neighbor_info->rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite table add failed on device %d handle 0x%lx: "
          "neighbor tunnel rewrite entry add failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }
  } else if (SWITCH_NHOP_TYPE(nhop_info) == SWITCH_NHOP_TYPE_MPLS) {
    mpls_handle = nhop_info->spath.api_nhop_info.mpls_handle;
    SWITCH_ASSERT(SWITCH_MPLS_HANDLE(mpls_handle));
    status = switch_mpls_get(device, mpls_handle, &mpls_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    ls_handle = nhop_info->spath.api_nhop_info.label_stack_handle;
    SWITCH_ASSERT(SWITCH_MPLS_LABEL_STACK_HANDLE(ls_handle));
    status = switch_mpls_label_stack_get(device, ls_handle, &ls_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    tunnel_handle = handle_to_id(ls_info->tunnel_handle);
    tunnel_index = handle_to_id(tunnel_handle);
    tunnel_dmac_index = mpls_info->tunnel_dmac_index;
    bd = handle_to_id(mpls_info->bd_handle);

    status = switch_pd_rewrite_table_mpls_rewrite_entry_add(
        device,
        bd,
        nhop,
        tunnel_index,
        api_neighbor_info->neighbor_tunnel_type,
        api_neighbor_info->mac_addr,
        mpls_info->api_mpls_info.swap_label,
        ls_info->label_stack.num_labels,
        tunnel_dmac_index,
        &neighbor_info->rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite table add failed on device %d handle 0x%lx: "
          "neighbor mpls rewrite entry add failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_neighbor_create_internal(
    switch_device_t device,
    switch_api_neighbor_info_t *api_neighbor_info,
    switch_handle_t *neighbor_handle) {
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_nhop_info_t api_nhop_info;
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_neighbor_info != NULL);
  if (!api_neighbor_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "neighbor create failed on device %d: "
        "neighbor info invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (api_neighbor_info->neighbor_type == SWITCH_NEIGHBOR_TYPE_NHOP) {
    SWITCH_ASSERT(SWITCH_NHOP_HANDLE(api_neighbor_info->nhop_handle));
    if (!SWITCH_NHOP_HANDLE(api_neighbor_info->nhop_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "neighbor create failed on device %d nhop handle 0x%lx: "
          "nhop handle invalid:(%s)\n",
          device,
          api_neighbor_info->nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    nhop_handle = api_neighbor_info->nhop_handle;
  }

  if (api_neighbor_info->neighbor_type == SWITCH_NEIGHBOR_TYPE_IP) {
    SWITCH_MEMSET(&api_nhop_info, 0x0, sizeof(api_nhop_info));
    api_nhop_info.ip_addr = api_neighbor_info->ip_addr;
    SWITCH_ASSERT(SWITCH_RIF_HANDLE(api_neighbor_info->rif_handle));
    api_nhop_info.rif_handle = api_neighbor_info->rif_handle;
    api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_IP;
    status = switch_api_nhop_create(device, &api_nhop_info, &nhop_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor create failed on device %d rif handle 0x%lx: "
          "nhop create failed:(%s)\n",
          device,
          api_neighbor_info->rif_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  handle = switch_neighbor_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "neighbor create failed on device %d: "
        "handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_get(device, handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor create failed on device %d: "
        "neighbor get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  nhop_info->spath.neighbor_handle = handle;

  neighbor_info->neighbor_handle = handle;
  neighbor_info->nhop_handle = nhop_handle;
  SWITCH_MEMCPY(&neighbor_info->api_neighbor_info,
                api_neighbor_info,
                sizeof(switch_api_neighbor_info_t));

  status = switch_api_nhop_send_mgid_event(
      device, nhop_handle, SWITCH_NHOP_MGID_TREE_DELETE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor create failed on device %d nhop handle 0x%lx\n",
                     "nhop mgid event send tree delete failed:(%s)\n",
                     device,
                     nhop_handle,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_entry_rewrite_add(device, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor create failed on device %d handle 0x%lx: "
        "neighbor unicast rewrite entry add failed:(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_nhop_send_mgid_event(
      device, nhop_handle, SWITCH_NHOP_MGID_TREE_CREATE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor create failed on device %d nhop handle 0x%lx\n",
                     "nhop mgid event send tree add failed:(%s)\n",
                     device,
                     nhop_handle,
                     switch_error_to_string(status));
    return status;
  }

  *neighbor_handle = handle;

  SWITCH_LOG_DEBUG(
      "neighbor created on device %d neighbor handle 0x%lx\n", device, handle);

  return status;
}

switch_status_t switch_neighbor_entry_rewrite_delete(
    switch_device_t device, switch_handle_t neighbor_handle) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_neighbor_info_t *api_neighbor_info = NULL;
  switch_neighbor_dmac_entry_t neighbor_entry;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NEIGHBOR_HANDLE(neighbor_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor rewrite delete failed on device %d neighbor handle 0x%lx: "
        "neighbor context get failed:(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_get(device, neighbor_handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor rewrite delete failed on device %d neighbor handle 0x%lx: "
        "neighbor get failed:(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  api_neighbor_info = &neighbor_info->api_neighbor_info;
  nhop_handle = neighbor_info->nhop_handle;

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (SWITCH_NHOP_TYPE(nhop_info) == SWITCH_NHOP_TYPE_IP) {
    rif_handle = nhop_info->spath.api_nhop_info.rif_handle;
    SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
    status = switch_rif_get(device, rif_handle, &rif_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    bd_handle = rif_info->bd_handle;

    SWITCH_MEMSET(&neighbor_entry, 0x0, sizeof(neighbor_entry));
    neighbor_entry.bd_handle = bd_handle;
    SWITCH_MEMCPY(&neighbor_entry.mac,
                  &api_neighbor_info->mac_addr,
                  sizeof(switch_mac_addr_t));

    status = switch_neighbor_dmac_hashtable_delete(
        device, neighbor_info, &neighbor_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite delete failed on device %d neighbor handle 0x%lx: "
          "neighbor hashtable delete failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }

    if (rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_VLAN ||
        rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_LN) {
      status = switch_nhop_l3_vlan_interface_resolve(
          device, nhop_handle, bd_handle, &api_neighbor_info->mac_addr, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "neighbor rewrite delete failed on device %d neighbor handle "
            "0x%lx: "
            "neighbor l3 vlan resolve failed:(%s)\n",
            device,
            neighbor_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    status = switch_neighbor_tunnel_dmac_rewrite_delete(
        device, &api_neighbor_info->mac_addr);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite delete failed on device %d neighbor handle 0x%lx: "
          "neighbor tunnel dmac delete failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (neighbor_info->rewrite_pd_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_rewrite_table_entry_delete(
        device, neighbor_info->rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor rewrite delete failed on device %d neighbor handle 0x%lx: "
          "neighbor rewrite table delete failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_neighbor_delete_internal(
    switch_device_t device, switch_handle_t neighbor_handle) {
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_api_neighbor_info_t *api_neighbor_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NEIGHBOR_HANDLE(neighbor_handle));
  if (!SWITCH_NEIGHBOR_HANDLE(neighbor_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "neighbor delete failed on device %d neighbor handle 0x%lx: "
        "neighbor handle invalid(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_neighbor_get(device, neighbor_handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor delete failed on device %d neighbor handle 0x%lx: "
        "neighbor get failed(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  nhop_handle = neighbor_info->nhop_handle;
  api_neighbor_info = &neighbor_info->api_neighbor_info;

  if (SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = switch_neighbor_entry_rewrite_delete(device, neighbor_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor delete failed on device %d neighbor handle 0x%lx: "
          "neighbor rewrite delete failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_api_nhop_send_mgid_event(
        device, nhop_handle, SWITCH_NHOP_MGID_TREE_DELETE, NULL);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor delete failed on device %d neighbor handle 0x%lx: "
          "nhop mgid tree delete event send failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }

    if (api_neighbor_info->neighbor_type == SWITCH_NEIGHBOR_TYPE_IP) {
      status = switch_api_nhop_delete(device, nhop_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "neighbor delete failed on device %d neighbor handle 0x%lx: "
            "nhop delete failed:(%s)\n",
            device,
            neighbor_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  status = switch_neighbor_handle_delete(device, neighbor_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = switch_api_nhop_send_mgid_event(
        device, nhop_handle, SWITCH_NHOP_MGID_TREE_CREATE, NULL);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "neighbor delete failed on device %d neighbor handle 0x%lx: "
          "nhop mgid tree create event send failed:(%s)\n",
          device,
          neighbor_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG("neighbor deleted on device %d neighbor handle 0x%lx\n",
                   device,
                   neighbor_handle);

  return status;
}

switch_status_t switch_neighbor_entry_nhop_list_get(
    switch_device_t device,
    switch_neighbor_dmac_entry_t *neighbor_entry,
    switch_neighbor_nhop_list_t **nhop_list) {
  switch_neighbor_context_t *neighbor_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(neighbor_entry != NULL);
  if (!neighbor_entry) {
    SWITCH_LOG_ERROR("neighbor nhop list get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NEIGHBOR, (void **)&neighbor_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("neighbor nhop list get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&neighbor_ctx->neighbor_dmac_hashtable,
                                   (void *)neighbor_entry,
                                   (void **)nhop_list);

  return status;
}

switch_status_t switch_api_neighbor_entry_rewrite_mac_get_internal(
    switch_device_t device,
    switch_handle_t neighbor_handle,
    switch_mac_addr_t *dmac) {
  switch_neighbor_info_t *neighbor_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NEIGHBOR_HANDLE(neighbor_handle));

  status = switch_neighbor_get(device, neighbor_handle, &neighbor_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "neighbor rewrite mac get failed on device %d neighbor handle 0x%lx: "
        "neighbor get failed:(%s)\n",
        device,
        neighbor_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(dmac,
                &neighbor_info->api_neighbor_info.mac_addr,
                sizeof(switch_mac_addr_t));

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_neighbor_delete(switch_device_t device,
                                           switch_handle_t neighbor_handle) {
  SWITCH_MT_WRAP(switch_api_neighbor_delete_internal(device, neighbor_handle));
}

switch_status_t switch_api_neighbor_create(
    switch_device_t device,
    switch_api_neighbor_info_t *api_neighbor_info,
    switch_handle_t *neighbor_handle) {
  SWITCH_MT_WRAP(switch_api_neighbor_create_internal(
      device, api_neighbor_info, neighbor_handle))
}

switch_status_t switch_api_neighbor_entry_rewrite_mac_get(
    switch_device_t device,
    switch_handle_t neighbor_handle,
    switch_mac_addr_t *mac) {
  SWITCH_MT_WRAP(switch_api_neighbor_entry_rewrite_mac_get_internal(
      device, neighbor_handle, mac))
}
