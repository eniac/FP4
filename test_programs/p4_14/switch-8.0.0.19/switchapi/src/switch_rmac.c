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

#include "switchapi/switch_rmac.h"

#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_RMAC

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Routine Description:
 *   @brief add default entries for rmac
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_outer_rmac_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac default entry add failed on device %d "
        "outer rmac default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_inner_rmac_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac default entry add failed on device %d "
        "inner rmac default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("rmac default entries added on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete default entries for rmac
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_DETAIL("rmac default entries deleted on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief smac rewrite table hash key generation
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_smac_rewrite_hash_key_init(void *args,
                                                  switch_uint8_t *key,
                                                  switch_uint32_t *len) {
  switch_mac_addr_t *mac = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  mac = (switch_mac_addr_t *)args;

  SWITCH_MEMCPY(key, mac, SWITCH_MAC_LENGTH);
  *len += SWITCH_MAC_LENGTH;

  SWITCH_ASSERT(*len == SWITCH_SMAC_HASH_KEY_SIZE);

  return status;
}

/*
 * Routine Description:
 *   @brief smac rewrite table hash comparison
 *
 * Return Values:
 *    @return -1 if key1 > key2
 *             0 if key1 = key2
 *             1 if key1 < key2
 */
switch_int32_t switch_smac_rewrite_hash_compare(const void *key1,
                                                const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_SMAC_HASH_KEY_SIZE);
}

switch_status_t switch_rmac_table_size_get(switch_device_t device,
                                           switch_size_t *rmac_table_size) {
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_table_id_t table_id = 0;

  SWITCH_ASSERT(rmac_table_size != NULL);

  *rmac_table_size = 0;

  for (table_id = SWITCH_TABLE_OUTER_RMAC; table_id <= SWITCH_TABLE_INNER_RMAC;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *rmac_table_size = 0;
      SWITCH_LOG_ERROR(
          "rmac handle size get failed on device %d: %s"
          "for table %s\n",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }
    *rmac_table_size += table_size;
  }
  return status;
}

/*
 * Routine Description:
 *   @brief initilize rmac structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_init(switch_device_t device) {
  switch_rmac_context_t *rmac_ctx = NULL;
  switch_size_t rmac_table_size = 0;
  switch_size_t smac_table_size = 0;
  switch_size_t tunnel_smac_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  rmac_ctx = SWITCH_MALLOC(device, sizeof(switch_rmac_context_t), 0x1);
  if (!rmac_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "rmac device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_RMAC, (void *)rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "rmac device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_table_size_get(device, &rmac_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "rmac table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_RMAC, rmac_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "rmac handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_SMAC_REWRITE, &smac_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "smac rewrite table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  rmac_ctx->smac_hashtable.size = smac_table_size;
  rmac_ctx->smac_hashtable.compare_func = switch_smac_rewrite_hash_compare;
  rmac_ctx->smac_hashtable.key_func = switch_smac_rewrite_hash_key_init;
  rmac_ctx->smac_hashtable.hash_seed = SWITCH_SMAC_REWRITE_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&rmac_ctx->smac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "smac hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_id_allocator_new(
      device, smac_table_size, FALSE, &(rmac_ctx->smac_allocator));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "smac rewrite allocator init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_TUNNEL_SMAC_REWRITE, &tunnel_smac_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "tunnel smac rewrite table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  rmac_ctx->tunnel_smac_hashtable.size = tunnel_smac_table_size;
  rmac_ctx->tunnel_smac_hashtable.compare_func =
      switch_smac_rewrite_hash_compare;
  rmac_ctx->tunnel_smac_hashtable.key_func = switch_smac_rewrite_hash_key_init;
  rmac_ctx->tunnel_smac_hashtable.hash_seed =
      SWITCH_TUNNEL_SMAC_REWRITE_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&rmac_ctx->tunnel_smac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "tunnel smac hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_id_allocator_new(device,
                                       tunnel_smac_table_size,
                                       FALSE,
                                       &(rmac_ctx->tunnel_smac_allocator));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac init failed on device %d "
        "tunnel smac rewrite allocator init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG("rmac init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  status = switch_rmac_free(device);
  SWITCH_ASSERT(status != SWITCH_STATUS_SUCCESS);
  return status;
}

/*
 * Routine Description:
 *   @brief initilize rmac structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_free(switch_device_t device) {
  switch_rmac_context_t *rmac_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_RMAC, (void **)&rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "rmac device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RMAC);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "rmac handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&rmac_ctx->smac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "smac hashtable free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, rmac_ctx->smac_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "smac allocator free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&rmac_ctx->tunnel_smac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "tunnel smac hashtable free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status =
      switch_api_id_allocator_destroy(device, rmac_ctx->tunnel_smac_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac free failed on device %d "
        "tunnel smac allocator free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, rmac_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_RMAC, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_smac_rewrite_entry_find(
    switch_device_t device,
    switch_mac_addr_t *mac,
    switch_smac_type_t smac_type,
    switch_smac_entry_t **smac_entry) {
  switch_rmac_context_t *rmac_ctx = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mac && smac_entry);
  if (!mac || !smac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "smac rewrite entry find failed on device %d: "
        "parameters null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_RMAC, (void **)&rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry find failed on device %d: "
        "rmac device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  hashtable = smac_type == SWITCH_SMAC_TYPE_REWRITE
                  ? &rmac_ctx->smac_hashtable
                  : &rmac_ctx->tunnel_smac_hashtable;

  status = SWITCH_HASHTABLE_SEARCH(hashtable, (void *)mac, (void **)smac_entry);

  SWITCH_LOG_DETAIL(
      "smac rewrite entry find on device %d "
      "mac %s mac type %s\n",
      device,
      switch_macaddress_to_string(mac),
      switch_smac_type_to_string(smac_type));

  return status;
}

switch_status_t switch_smac_rewrite_index_by_mac_get(
    switch_device_t device,
    switch_mac_addr_t *mac,
    switch_smac_type_t smac_type,
    switch_id_t *smac_index) {
  switch_smac_entry_t *smac_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mac != NULL && smac_index != NULL);
  if (!mac || !smac_index) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "smac rewrite entry find failed on device %d: "
        "parameters null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_smac_rewrite_entry_find(device, mac, smac_type, &smac_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry find failed on device %d: "
        "smac rewrite entry find failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *smac_index = smac_entry->smac_index;

  SWITCH_LOG_DETAIL(
      "smac rewrite index by mac get on device %d "
      "mac %s mac type %s mac index %d\n",
      device,
      switch_macaddress_to_string(mac),
      switch_smac_type_to_string(smac_type),
      *smac_index,
      switch_error_to_string(status));

  return status;
}

switch_status_t switch_smac_rewrite_index_by_rmac_handle_get(
    switch_device_t device,
    switch_handle_t rmac_handle,
    switch_smac_type_t smac_type,
    switch_id_t *smac_index) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_rmac_type_t rmac_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "smac rewrite index by rmac handle failed on device %d: "
        "rmac handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite index by rmac handle failed on device %d: "
        "rmac get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *smac_index = SWITCH_API_INVALID_ID;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  rmac_type = smac_type == SWITCH_SMAC_TYPE_REWRITE ? SWITCH_RMAC_TYPE_INNER
                                                    : SWITCH_RMAC_TYPE_OUTER;

  if (rmac_info->rmac_type & rmac_type) {
    FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
      rmac_entry = node->data;
      if (rmac_type != SWITCH_RMAC_TYPE_OUTER) {
        *smac_index = rmac_entry->smac_index;
      } else {
        *smac_index = rmac_entry->tunnel_smac_index;
      }
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
    FOR_EACH_IN_LIST_END();
  }

  SWITCH_LOG_DETAIL(
      "smac rewrite index by rmac handle get on device %d "
      "rmac handle 0x%lx mac type %s mac index %d\n",
      device,
      rmac_handle,
      switch_smac_type_to_string(smac_type),
      *smac_index,
      switch_error_to_string(status));

  return status;
}

switch_status_t switch_smac_rewrite_entry_add(switch_device_t device,
                                              switch_smac_type_t smac_type,
                                              switch_mac_addr_t *mac,
                                              switch_id_t *smac_index) {
  switch_rmac_context_t *rmac_ctx = NULL;
  switch_smac_entry_t *smac_entry = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_id_allocator_t *allocator = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_RMAC, (void **)&rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry add failed on device %d: "
        "rmac device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_smac_rewrite_entry_find(device, mac, smac_type, &smac_entry);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry add failed on device %d: "
        "smac rewrite entry find failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    smac_entry->ref_count++;
    *smac_index = smac_entry->smac_index;
    return status;
  }

  smac_entry = SWITCH_MALLOC(device, sizeof(switch_smac_entry_t), 0x1);
  if (!smac_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "smac rewrite entry add failed on device %d: "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(smac_entry, 0x0, sizeof(switch_smac_entry_t));
  SWITCH_MEMCPY(&smac_entry->mac, mac, SWITCH_MAC_LENGTH);
  smac_entry->ref_count = 1;

  if (smac_type == SWITCH_SMAC_TYPE_REWRITE) {
    hashtable = &rmac_ctx->smac_hashtable;
    allocator = rmac_ctx->smac_allocator;
  } else {
    hashtable = &rmac_ctx->tunnel_smac_hashtable;
    allocator = rmac_ctx->tunnel_smac_allocator;
  }

  status = switch_api_id_allocator_allocate(
      device, allocator, &smac_entry->smac_index);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry add failed on device %d: "
        "smac index allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (smac_type & SWITCH_SMAC_TYPE_REWRITE) {
    status = switch_pd_smac_rewrite_table_entry_add(
        device, smac_entry, &smac_entry->hw_smac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "smac rewrite entry add failed on device %d: "
          "smac rewrite table add failed(%s)\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  } else {
    status = switch_pd_tunnel_smac_rewrite_table_entry_add(
        device, smac_entry->smac_index, mac, &smac_entry->hw_smac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "smac rewrite entry add failed on device %d: "
          "tunnel smac rewrite table add failed(%s)\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  status = SWITCH_HASHTABLE_INSERT(
      hashtable, &(smac_entry->node), (void *)mac, (void *)smac_entry);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry add failed on device %d: "
        "smac hashtable insert failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  *smac_index = smac_entry->smac_index;

  SWITCH_LOG_DEBUG(
      "smac rewrite table added on device %d "
      "mac %s mac type %s smac index %d\n",
      device,
      switch_macaddress_to_string(mac),
      switch_smac_type_to_string(smac_type),
      *smac_index);

  return status;

cleanup:
  return status;
}

switch_status_t switch_smac_rewrite_entry_delete(switch_device_t device,
                                                 switch_smac_type_t smac_type,
                                                 switch_mac_addr_t *mac) {
  switch_rmac_context_t *rmac_ctx = NULL;
  switch_smac_entry_t *smac_entry = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_id_allocator_t *allocator = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_RMAC, (void **)&rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry delete failed on device %d: "
        "rmac device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_smac_rewrite_entry_find(device, mac, smac_type, &smac_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry delete failed on device %d: "
        "smac rewrite entry find failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  smac_entry->ref_count--;
  if (smac_entry->ref_count > 0) {
    return status;
  }

  if (smac_type == SWITCH_SMAC_TYPE_REWRITE) {
    status = switch_pd_smac_rewrite_table_entry_delete(
        device, smac_entry->hw_smac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "smac rewrite entry delete failed on device %d: "
          "smac rewrite table delete failed(%s)\n",
          device,
          switch_error_to_string(status));
    }
  } else {
    status = switch_pd_tunnel_smac_rewrite_table_entry_delete(
        device, smac_entry->hw_smac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "smac rewrite entry delete failed on device %d: "
          "tunnel smac rewrite table delete failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (smac_type == SWITCH_SMAC_TYPE_REWRITE) {
    hashtable = &rmac_ctx->smac_hashtable;
    allocator = rmac_ctx->smac_allocator;
  } else {
    hashtable = &rmac_ctx->tunnel_smac_hashtable;
    allocator = rmac_ctx->tunnel_smac_allocator;
  }

  status = switch_api_id_allocator_release(
      device, allocator, smac_entry->smac_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry delete failed on device %d: "
        "smac index free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_HASHTABLE_DELETE(hashtable, (void *)mac, (void **)&smac_entry);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "smac rewrite entry delete failed on device %d: "
        "smac hashtable delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "smac rewrite table added on device %d "
      "mac %s mac type %s smac index %d\n",
      device,
      switch_macaddress_to_string(mac),
      switch_smac_type_to_string(smac_type),
      smac_entry->smac_index);

  SWITCH_FREE(device, smac_entry);

  return status;
}

switch_status_t switch_api_router_mac_group_create_internal(
    const switch_device_t device,
    const switch_rmac_type_t rmac_type,
    switch_handle_t *rmac_handle) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  handle = switch_rmac_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "rmac group create failed on device %d rmac type %s: "
        "rmac handle create failed(%s)\n",
        device,
        switch_rmac_type_to_string(rmac_type),
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac group create failed on device %d rmac type %s: "
        "rmac get failed(%s)\n",
        device,
        switch_rmac_type_to_string(rmac_type),
        switch_error_to_string(status));
    return status;
  }

  rmac_info->rmac_type = rmac_type;

  status = SWITCH_LIST_INIT(&rmac_info->rmac_list);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  *rmac_handle = handle;

  SWITCH_LOG_DEBUG(
      "rmac group created on device %d "
      "rmac handle 0x%lx rmac type %s\n",
      device,
      handle,
      switch_rmac_type_to_string(rmac_type));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_router_mac_group_delete_internal(
    const switch_device_t device, const switch_handle_t rmac_handle) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rmac group delete failed on device %d rmac handle 0x%lx: "
        "rmac handle invalid(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rmac group delete failed on device %d rmac hadle 0x%lx: "
        "rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = node->data;
    status =
        switch_api_router_mac_delete(device, rmac_handle, &rmac_entry->mac);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "rmac group delete failed on device %d rmac handle 0x%lx: "
          "rmac mac delete failed(%s)\n",
          device,
          rmac_handle,
          switch_error_to_string(status));
    }
  }
  FOR_EACH_IN_LIST_END();

  status = switch_rmac_handle_delete(device, rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rmac group delete failed on device %d rmac handle 0x%lx: "
        "rmac handle delete failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "rmac group deleted on device %d "
      "rmac handle 0x%lx\n",
      device,
      rmac_handle);

  return status;
}

switch_status_t switch_api_rmac_group_handle_get(
    const switch_device_t device,
    const switch_size_t num_entries,
    const switch_mac_addr_t *mac_addr,
    switch_handle_t *rmac_handle) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_handle_t tmp_rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(rmac_handle && mac_addr && num_entries);
  if (!rmac_handle || !mac_addr || num_entries == 0) {
    SWITCH_LOG_ERROR(
        "rmac group handle get failed on device %d: "
        "parameters null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *rmac_handle = SWITCH_API_INVALID_HANDLE;

  FOR_EACH_HANDLE_BEGIN(device, SWITCH_HANDLE_TYPE_RMAC, tmp_rmac_handle) {
    SWITCH_ASSERT(SWITCH_RMAC_HANDLE(tmp_rmac_handle));

    status = switch_rmac_get(device, tmp_rmac_handle, &rmac_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "rmac group handle get failed on device %d: "
          "rmac get failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    if (SWITCH_LIST_COUNT(&rmac_info->rmac_list) != num_entries) {
      continue;
    }

    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    index = 0;

    FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
      rmac_entry = node->data;
      if (!SWITCH_MEMCMP(
              &rmac_entry->mac, &mac_addr[index], SWITCH_MAC_LENGTH)) {
        index++;
      }
    }
    FOR_EACH_IN_LIST_END();

    if (index == num_entries) {
      *rmac_handle = tmp_rmac_handle;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_HANDLE_END();

  SWITCH_LOG_DEBUG("rmac group handle get on device %d rmac handle 0x%lx",
                   device,
                   *rmac_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_int32_t switch_rmac_address_compare(const void *key1, const void *key2) {
  switch_rmac_entry_t *rmac_entry1 = NULL;
  switch_rmac_entry_t *rmac_entry2 = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!key1 || !key2) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("rmac address compare failed: parameters null(%s)\n",
                     switch_error_to_string(status));
    return -1;
  }

  rmac_entry1 = (switch_rmac_entry_t *)key1;
  rmac_entry2 = (switch_rmac_entry_t *)key2;

  return SWITCH_MEMCMP(&rmac_entry1->mac, &rmac_entry2->mac, SWITCH_MAC_LENGTH);
}

switch_status_t switch_api_router_mac_add_internal(
    const switch_device_t device,
    const switch_handle_t rmac_handle,
    const switch_mac_addr_t *mac) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac add failed on device %d rmac handle 0x%lx: "
        "rmac handle invalid(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac add failed on device %d rmac handle 0x%lx: "
        "rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t *)node->data;
    if (SWITCH_MEMCMP(&(rmac_entry->mac), mac, sizeof(switch_mac_addr_t)) ==
        0) {
      return SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    }
  }
  FOR_EACH_IN_LIST_END();

  rmac_entry = SWITCH_MALLOC(device, sizeof(switch_rmac_entry_t), 0x1);
  if (!rmac_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "router mac add failed on device %d rmac handle 0x%lx: "
        "memory allocation failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(rmac_entry, 0x0, sizeof(switch_rmac_entry_t));
  SWITCH_MEMCPY(&rmac_entry->mac, mac, sizeof(switch_mac_addr_t));

  SWITCH_LIST_INSERT(&(rmac_info->rmac_list), &(rmac_entry->node), rmac_entry);
  SWITCH_LIST_SORT(&rmac_info->rmac_list, switch_rmac_address_compare);

  if (rmac_info->rmac_type & SWITCH_RMAC_TYPE_INNER) {
    if (!(SWITCH_HW_FLAG_ISSET(rmac_entry, SWITCH_RMAC_PD_ENTRY_INNER))) {
      status =
          switch_pd_inner_rmac_table_entry_add(device,
                                               handle_to_id(rmac_handle),
                                               &rmac_entry->mac,
                                               &rmac_entry->inner_rmac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "router mac add failed on device %d "
            "rmac handle 0x%lx mac %s: "
            "inner rmac table add failed(%s)\n",
            device,
            rmac_handle,
            switch_macaddress_to_string(&rmac_entry->mac),
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(rmac_entry, SWITCH_RMAC_PD_ENTRY_INNER);
    }

    status = switch_smac_rewrite_entry_add(device,
                                           SWITCH_SMAC_TYPE_REWRITE,
                                           &rmac_entry->mac,
                                           &rmac_entry->smac_index);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "router mac add failed on device %d "
          "rmac handle 0x%lx mac %s: "
          "smac rewrite entry add failed(%s)\n",
          device,
          rmac_handle,
          switch_macaddress_to_string(&rmac_entry->mac),
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (rmac_info->rmac_type & SWITCH_RMAC_TYPE_OUTER) {
    if (!(SWITCH_HW_FLAG_ISSET(rmac_entry, SWITCH_RMAC_PD_ENTRY_OUTER))) {
      status =
          switch_pd_outer_rmac_table_entry_add(device,
                                               handle_to_id(rmac_handle),
                                               &rmac_entry->mac,
                                               &rmac_entry->outer_rmac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "router mac add failed on device %d "
            "rmac handle 0x%lx mac %s: "
            "outer rmac table add failed(%s)\n",
            device,
            rmac_handle,
            switch_macaddress_to_string(&rmac_entry->mac),
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(rmac_entry, SWITCH_RMAC_PD_ENTRY_OUTER);
    }

    status = switch_smac_rewrite_entry_add(device,
                                           SWITCH_SMAC_TYPE_TUNNEL_REWRITE,
                                           &rmac_entry->mac,
                                           &rmac_entry->tunnel_smac_index);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "router mac add failed on device %d "
          "rmac handle 0x%lx mac %s: "
          "tunnel smac rewrite entry add failed(%s)\n",
          device,
          rmac_handle,
          switch_macaddress_to_string(&rmac_entry->mac),
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  SWITCH_LOG_DEBUG("rmac added on device %d rmac handle 0x%lx mac %s\n",
                   device,
                   rmac_handle,
                   switch_macaddress_to_string(&rmac_entry->mac));

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_router_mac_delete_internal(
    const switch_device_t device,
    const switch_handle_t rmac_handle,
    const switch_mac_addr_t *mac) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  bool entry_found = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac delete failed on device %d rmac handle 0x%lx mac %s: "
        "rmac handle invalid(%s)\n",
        device,
        rmac_handle,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac delete failed on device %d rmac handle 0x%lx mac %s: "
        "rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t *)node->data;
    if (SWITCH_MEMCMP(&(rmac_entry->mac), mac, sizeof(switch_mac_addr_t)) ==
        0) {
      entry_found = TRUE;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!entry_found) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR(
        "router mac delete failed on device %d rmac handle 0x%lx mac %s: "
        "rmac mac entry find failed(%s)\n",
        device,
        rmac_handle,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  if (rmac_info->rmac_type & SWITCH_RMAC_TYPE_OUTER) {
    if (SWITCH_HW_FLAG_ISSET(rmac_entry, SWITCH_RMAC_PD_ENTRY_OUTER)) {
      status = switch_pd_outer_rmac_table_entry_delete(
          device, rmac_entry->outer_rmac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "router mac delete failed on device %d "
            "rmac handle 0x%lx mac %s: "
            "outer rmac table delete failed(%s)\n",
            device,
            rmac_handle,
            switch_macaddress_to_string(mac),
            switch_error_to_string(status));
        return status;
      }
      SWITCH_HW_FLAG_CLEAR(rmac_entry, SWITCH_RMAC_PD_ENTRY_OUTER);
    }

    status = switch_smac_rewrite_entry_delete(
        device, SWITCH_SMAC_TYPE_TUNNEL_REWRITE, &rmac_entry->mac);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "router mac delete failed on device %d "
          "rmac handle 0x%lx mac %s: "
          "tunnel smac rewrite entry delete failed(%s)\n",
          device,
          rmac_handle,
          switch_macaddress_to_string(mac),
          switch_error_to_string(status));
      return status;
    }
  }

  if (rmac_info->rmac_type & SWITCH_RMAC_TYPE_INNER) {
    if (SWITCH_HW_FLAG_ISSET(rmac_entry, SWITCH_RMAC_PD_ENTRY_INNER)) {
      status = switch_pd_inner_rmac_table_entry_delete(
          device, rmac_entry->inner_rmac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "router mac delete failed on device %d "
            "rmac handle 0x%lx mac %s: "
            "inner rmac table delete failed(%s)\n",
            device,
            rmac_handle,
            switch_macaddress_to_string(mac),
            switch_error_to_string(status));
        return status;
      }
      SWITCH_HW_FLAG_CLEAR(rmac_entry, SWITCH_RMAC_PD_ENTRY_INNER);
    }

    status = switch_smac_rewrite_entry_delete(
        device, SWITCH_SMAC_TYPE_REWRITE, &rmac_entry->mac);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "router mac delete failed on device %d "
          "rmac handle 0x%lx mac %s: "
          "smac rewrite entry delete failed(%s)\n",
          device,
          rmac_handle,
          switch_macaddress_to_string(mac),
          switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_LIST_DELETE(&(rmac_info->rmac_list), node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "router mac delete failed on device %d "
        "rmac handle 0x%lx mac %s: "
        "rmac list delete failed(%s)\n",
        device,
        rmac_handle,
        switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("rmac deleted on device %d rmac handle 0x%lx mac %s\n",
                   device,
                   rmac_handle,
                   switch_macaddress_to_string(mac));

  if (rmac_entry) {
    SWITCH_FREE(device, rmac_entry);
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rmac_macs_get_internal(switch_device_t device,
                                                  switch_handle_t rmac_handle,
                                                  switch_uint16_t *num_entries,
                                                  switch_mac_addr_t **macs) {
  switch_rmac_info_t *rmac_info = NULL;
  switch_rmac_entry_t *rmac_entry = NULL;
  switch_node_t *node = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac get failed on device %d rmac handle 0x%lx: "
        "rmac handle invalid(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "router mac get failed on device %d rmac handle 0x%lx: "
        "rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  *macs = SWITCH_MALLOC(device,
                        sizeof(switch_mac_addr_t),
                        SWITCH_LIST_COUNT(&rmac_info->rmac_list));
  if (!(*macs)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "router mac get failed on device %d rmac handle 0x%lx: "
        "rmac get failed(%s)\n",
        device,
        rmac_handle,
        switch_error_to_string(status));
    return status;
  }

  *num_entries = SWITCH_LIST_COUNT(&rmac_info->rmac_list);
  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t *)node->data;
    SWITCH_MEMCPY(
        &((*macs)[index++]), &rmac_entry->mac, sizeof(switch_mac_addr_t));
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DEBUG(
      "router mac get on device %d num entries %d\n", device, *num_entries);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_router_mac_group_delete(
    const switch_device_t device, const switch_handle_t rmac_handle) {
  SWITCH_MT_WRAP(
      switch_api_router_mac_group_delete_internal(device, rmac_handle))
}

switch_status_t switch_api_router_mac_add(const switch_device_t device,
                                          const switch_handle_t rmac_handle,
                                          const switch_mac_addr_t *mac) {
  SWITCH_MT_WRAP(switch_api_router_mac_add_internal(device, rmac_handle, mac))
}

switch_status_t switch_api_router_mac_group_create(
    const switch_device_t device,
    switch_rmac_type_t rmac_type,
    switch_handle_t *rmac_handle) {
  SWITCH_MT_WRAP(switch_api_router_mac_group_create_internal(
      device, rmac_type, rmac_handle))
}

switch_status_t switch_api_rmac_macs_get(switch_device_t device,
                                         switch_handle_t rmac_handle,
                                         switch_uint16_t *num_entries,
                                         switch_mac_addr_t **macs) {
  SWITCH_MT_WRAP(
      switch_api_rmac_macs_get_internal(device, rmac_handle, num_entries, macs))
}

switch_status_t switch_api_router_mac_delete(const switch_device_t device,
                                             const switch_handle_t rmac_handle,
                                             const switch_mac_addr_t *mac) {
  SWITCH_MT_WRAP(
      switch_api_router_mac_delete_internal(device, rmac_handle, mac))
}
