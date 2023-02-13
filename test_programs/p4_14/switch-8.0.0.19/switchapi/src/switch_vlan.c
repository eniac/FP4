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

#include "switchapi/switch_vlan.h"

#include "switch_internal.h"
#include "switch_pd.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_VLAN

/*
 * Routine Description:
 *   @brief add default entries for vlan
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vlan_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_port_vlan_to_bd_mapping_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan default entry add failed on device %d: "
        "port vlan mapping default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_port_vlan_to_ifindex_mapping_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan default entry add failed on device %d: "
        "port vlan mapping default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_vlan_xlate_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan default entry add failed on device %d: "
        "egress vlan xlate default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_vlan_decap_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan default entry add failed on device %d: "
        "vlan decap default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_vlan_decap_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan default entry add failed on device %d: "
        "vlan decap init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("vlan default entries added on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief remove default entries for vlan
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vlan_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_DETAIL("vlan default entries deleted on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief port vlan mapping table hash key generation
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_pv_table_entry_key_init(void *args,
                                               switch_uint8_t *key,
                                               switch_uint32_t *len) {
  switch_pv_key_t *pv_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  pv_key = (switch_pv_key_t *)args;

  SWITCH_MEMCPY(key, &pv_key->outer_vlan, sizeof(switch_vlan_t));
  *len += sizeof(switch_vlan_t);
  SWITCH_MEMCPY(key + *len, &pv_key->inner_vlan, sizeof(switch_vlan_t));
  *len += sizeof(switch_vlan_t);
  SWITCH_MEMCPY(
      key + *len, &pv_key->port_lag_index, sizeof(switch_port_lag_index_t));
  *len += sizeof(switch_port_lag_index_t);

  SWITCH_ASSERT(*len == SWITCH_PV_HASH_KEY_SIZE);

  return status;
}

/*
 * Routine Description:
 *   @brief port vlan mapping table hash comparison
 *
 * Return Values:
 *    @return -1 if key1 > key2
 *             0 if key1 = key2
 *             1 if key1 < key2
 */
switch_int32_t switch_pv_entry_hash_compare(const void *key1,
                                            const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_PV_HASH_KEY_SIZE);
}

/*
 * Routine Description:
 *   @brief initilize vlan structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vlan_init(switch_device_t device) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_size_t pv_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  vlan_ctx = SWITCH_MALLOC(device, sizeof(switch_vlan_context_t), 0x1);
  if (!vlan_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "vlan init failed for device %d: "
        "vlan device context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_VLAN, (void *)vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan init failed for device %d: "
        "vlan device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(vlan_ctx->vlan_handle_list,
                SWITCH_API_INVALID_HANDLE,
                sizeof(switch_handle_t) * SWITCH_MAX_VLANS);

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_VLAN, SWITCH_MAX_VLANS);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan init failed for device %d: "
        "vlan handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING, &pv_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan init failed for device %d: "
        "vlan table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  vlan_ctx->pv_hashtable.size = pv_table_size;
  vlan_ctx->pv_hashtable.compare_func = switch_pv_entry_hash_compare;
  vlan_ctx->pv_hashtable.key_func = switch_pv_table_entry_key_init;
  vlan_ctx->pv_hashtable.hash_seed = SWITCH_PV_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&vlan_ctx->pv_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan init failed for device %d: "
        "vlan hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("vlan init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief free vlan structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vlan_free(switch_device_t device) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VLAN, (void **)&vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan free failed for device: %d "
        "vlan device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_VLAN);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan free failed for device: %d "
        "vlan handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&vlan_ctx->pv_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan free failed for device: %d "
        "vlan hashtable free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_device_api_context_set(device, SWITCH_API_TYPE_VLAN, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan free failed for device: %d "
        "vlan device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, vlan_ctx);

  SWITCH_LOG_DEBUG("vlan free successful on device %d", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief map vlan id to vlan handle
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_id - vlan identifier
 *   @param[in] vlan_handle - vlan handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_id_to_handle_set(switch_device_t device,
                                                 switch_vlan_t vlan_id,
                                                 switch_handle_t vlan_handle) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VLAN, (void **)&vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan id to handle set failed on device %d "
        "vlan id %d vlan handle %lx: "
        "vlan device context get failed(%s)\n",
        device,
        vlan_id,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  if (vlan_id > SWITCH_API_MAX_VLANS) {
    status = SWITCH_STATUS_INVALID_VLAN_ID;
    SWITCH_LOG_ERROR(
        "vlan id to handle set failed on device %d "
        "vlan id %d vlan handle %lx: "
        "vlan id invalid(%s)\n",
        device,
        vlan_id,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  if (vlan_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
    if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "vlan id to handle set failed on device %d "
          "vlan id %d vlan handle %lx "
          "vlan handle invalid(%s)\n",
          device,
          vlan_id,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "vlan id to handle set successfully on device %d "
      "vlan id %d vlan handle %lx\n",
      device,
      vlan_id,
      vlan_handle);

  vlan_ctx->vlan_handle_list[vlan_id] = vlan_handle;

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief get vlan handle from vlan id
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_id - vlan identifier
 *   @param[out] vlan_handle - vlan handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_id_to_handle_get_internal(
    switch_device_t device,
    switch_vlan_t vlan_id,
    switch_handle_t *vlan_handle) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(vlan_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VLAN, (void **)&vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan id to handle get failed on device %d vlan id: %d "
        "vlan device context get failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  if (vlan_id >= SWITCH_API_MAX_VLANS) {
    status = SWITCH_STATUS_INVALID_VLAN_ID;
    SWITCH_LOG_ERROR(
        "vlan id to handle get failed on device %d vlan id: %d "
        "vlan id invalid(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  *vlan_handle = vlan_ctx->vlan_handle_list[vlan_id];

  if (SWITCH_VLAN_HANDLE((*vlan_handle))) {
    status = SWITCH_STATUS_SUCCESS;
  } else {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
  }

  SWITCH_LOG_DEBUG(
      "vlan id to handle get successfully on device %d "
      "vlan id %d vlan handle %lx\n",
      device,
      vlan_id,
      *vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief get vlan id from vlan handle
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_handle - vlan handle
 *   @param[out] vlan_id - vlan identifier
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_handle_to_id_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_vlan_t *vlan_id) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan handle to id get failed on device %d "
        "vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan handle to id get failed on device %d "
        "vlan handle %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  *vlan_id = vlan_info->vlan_id;

  SWITCH_LOG_DEBUG(
      "vlan handle to id get successfully on device %d "
      "vlan handle %lx vlan id %d(%d)\n",
      device,
      vlan_handle,
      vlan_id,
      vlan_info->vlan_id);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief create a vlan
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_id - vlan identifier
 *   @param[out] vlan_handle - vlan handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_create_internal(switch_device_t device,
                                                switch_vlan_t vlan_id,
                                                switch_handle_t *vlan_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_bd_info_t *bd_info_tmp = NULL;
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_handle_t bd_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (vlan_id > SWITCH_MAX_VLANS) {
    status = SWITCH_STATUS_INVALID_VLAN_ID;
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: vlan id invalid(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_id_to_handle_get(device, vlan_id, vlan_handle);
  if (status == SWITCH_STATUS_SUCCESS &&
      *vlan_handle != SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: vlan id already exists(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  *vlan_handle = switch_vlan_handle_create(device);
  if (*vlan_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: vlan handle allocation failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, *vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: vlan get failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(switch_bd_info_t));
  bd_info.bd_type = SWITCH_BD_TYPE_VLAN;
  bd_info.handle = *vlan_handle;

  bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_flags |= SWITCH_BD_ATTR_UUC_FLOODING_ENABLED;
  bd_flags |= SWITCH_BD_ATTR_UMC_FLOODING_ENABLED;
  bd_flags |= SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED;
  status = switch_api_multicast_index_create(device, &bd_info.flood_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: unknown ucast index create failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    goto cleanup;
  }

  bd_flags |= SWITCH_BD_ATTR_LEARNING;
  bd_info.learning = TRUE;

  bd_info.vlan = vlan_id;
  status = switch_bd_create(device, bd_flags, &bd_info, &bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: bd create failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    goto cleanup;
  }

  vlan_info->vlan_id = vlan_id;
  vlan_info->bd_handle = bd_handle;

  status = switch_api_vlan_id_to_handle_set(device, vlan_id, *vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: id to handle set failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info_tmp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: bd get failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  bd_info_tmp->vlan = vlan_id;

  status = switch_api_vlan_stats_enable(device, *vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: stats enable failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pktdriver_bd_to_vlan_mapping_add(
      device, handle_to_id(vlan_info->bd_handle), vlan_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan create failed on device %d "
        "vlan id %d: bd to vlan mapping add failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG(
      "vlan created successfully on device %d "
      "vlan id %d vlan handle %lx bd handle %lx\n",
      device,
      vlan_id,
      *vlan_handle,
      bd_handle);

  return status;

cleanup:
  return status;
}

/*
 * Routine Description:
 *   @brief delete a vlan
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_handle - vlan handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_delete_internal(
    const switch_device_t device, const switch_handle_t vlan_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "bd get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_stats_disable(device, vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "stats disable failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_multicast_index_delete(device, bd_info->flood_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "unknown ucast index delete failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_delete(device, vlan_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx: "
        "bd delete failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_id_to_handle_set(
      device, vlan_info->vlan_id, SWITCH_API_INVALID_HANDLE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx :"
        "vlan id to handle set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_bd_to_vlan_mapping_delete(
      device, handle_to_id(vlan_info->bd_handle), vlan_info->vlan_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan delete failed on device %d vlan handle %lx :"
        "vlan id to handle set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_handle_delete(device, vlan_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("vlan deleted successfully on device %d vlan handle %lx\n",
                   device,
                   vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief add port vlan membership
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_handle - vlan handle
 *   @param[in] num_ports - number of interfaces
 *   @param[in] intf_handles - interface list
 *   @param[out] member_handles - vlan member handles
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_member_add_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_handle_t intf_handle,
    switch_handle_t *member_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_mcast_member_t mcast_member;
  switch_vlan_t outer_vlan = 0;
  switch_vlan_t inner_vlan = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx: "
        "bd get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx "
        "intf handle %lx: invalid interface handle(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  outer_vlan = 0;
  inner_vlan = vlan_info->vlan_id;

  status = switch_pv_member_add(device,
                                vlan_info->bd_handle,
                                intf_handle,
                                outer_vlan,
                                inner_vlan,
                                0x0,
                                member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx "
        "intf handle %lx: pv member add failed(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_BD_MEMBER_HANDLE(*member_handle));

  status = switch_bd_member_get(device, *member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port add failed on device %d vlan handle %lx "
        "intf handle %lx: bd member get failed(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (bd_member->stp_state != SWITCH_PORT_STP_STATE_LEARNING &&
      bd_member->stp_state != SWITCH_PORT_STP_STATE_BLOCKING) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_member,
                               SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY))) {
      mcast_member.handle = intf_handle;
      mcast_member.network_handle = vlan_handle;
      status = switch_api_multicast_member_add(
          device, bd_info->flood_handle, 0x1, &mcast_member);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "vlan port add failed on device %d vlan handle %lx :"
            "intf handle %lx: mcast member add failed(%s)\n",
            device,
            vlan_handle,
            intf_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(bd_member, SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY);
    }
  }

  SWITCH_LOG_DEBUG(
      "vlan port added successfully on device %d "
      "vlan handle %lx intf handle %lx\n",
      device,
      vlan_handle,
      intf_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

// TODO: bug: assumption that vtags cannot be 0.
switch_status_t switch_pv_member_add(switch_device_t device,
                                     switch_handle_t bd_handle,
                                     switch_handle_t intf_handle,
                                     switch_vlan_t outer_vlan,
                                     switch_vlan_t inner_vlan,
                                     switch_uint64_t flags,
                                     switch_handle_t *member_handle) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_pv_entry_t *pv_entry = NULL;
  switch_pv_key_t pv_key = {0};
  switch_vlan_t pv_untagged_outer_vlan = 0;
  switch_vlan_t pv_untagged_inner_vlan = 0;
  switch_vlan_t native_vlan_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(flags);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VLAN, (void **)&vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "vlan device context get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "interface get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd member exists(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(&pv_entry, 0x0, sizeof(pv_entry));
  pv_key.port_lag_index = intf_info->port_lag_index;
  pv_key.outer_vlan = outer_vlan;
  pv_key.inner_vlan = inner_vlan;
  status = SWITCH_HASHTABLE_SEARCH(
      &vlan_ctx->pv_hashtable, (void *)&pv_key, (void **)&pv_entry);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "pv membership exists(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_add(device, bd_handle, member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd member add failed(%s)",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_get(device, (*member_handle), &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd member get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  bd_member->outer_vlan = outer_vlan;
  bd_member->inner_vlan = inner_vlan;
  bd_member->handle = intf_handle;
  bd_member->bd_handle = bd_handle;
  bd_member->member_handle = *member_handle;

  pv_untagged_inner_vlan = inner_vlan;
  if (SWITCH_INTERFACE_ACCESS(intf_info)) {
    pv_untagged_inner_vlan = 0;
  }

  if (SWITCH_INTERFACE_TRUNK(intf_info)) {
    status = switch_api_interface_native_vlan_id_get(
        device, intf_handle, &native_vlan_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member add failed on device %d "
          "bd handle %lx intf handle %lx: "
          "interface native vlan id get failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (native_vlan_id == pv_untagged_inner_vlan) {
      pv_untagged_inner_vlan = 0;
    }
  }

  status = switch_mcast_bd_member_rid_allocate(device, bd_handle, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "rid allocation failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (!(SWITCH_HW_FLAG_ISSET(bd_member,
                             SWITCH_BD_MEMBER_PD_PV_UNTAGGED_ENTRY))) {
    status = switch_pd_port_vlan_to_bd_mapping_table_entry_add(
        device,
        intf_info->port_lag_index,
        pv_untagged_inner_vlan,
        pv_untagged_outer_vlan,
        bd_info->bd_entry,
        &bd_member->pv_untagged_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member add failed on device %d "
          "bd handle %lx intf handle %lx: "
          "pv mapping table add failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_member, SWITCH_BD_MEMBER_PD_PV_UNTAGGED_ENTRY);
  }

  if (!(SWITCH_HW_FLAG_ISSET(bd_member,
                             SWITCH_BD_MEMBER_PD_PV_IFINDEX_ENTRY))) {
    status = switch_pd_port_vlan_to_ifindex_mapping_table_entry_add(
        device,
        intf_info->port_lag_index,
        pv_untagged_inner_vlan,
        pv_untagged_outer_vlan,
        intf_info->ifindex,
        bd_member->rid,
        &bd_member->pv_ifindex_entry);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member add failed on device %d "
          "bd handle %lx intf handle %lx: "
          "egress vlan xlate table add failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_member, SWITCH_BD_MEMBER_PD_PV_IFINDEX_ENTRY);
  }

  if (!(SWITCH_HW_FLAG_ISSET(bd_member, SWITCH_BD_MEMBER_PD_XLATE_ENTRY))) {
    status =
        switch_pd_egress_vlan_xlate_table_entry_add(device,
                                                    intf_info->ifindex,
                                                    handle_to_id(bd_handle),
                                                    pv_untagged_inner_vlan,
                                                    pv_untagged_outer_vlan,
                                                    &bd_member->xlate_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member add failed on device %d "
          "bd handle %lx intf handle %lx: "
          "egress vlan xlate table add failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_member, SWITCH_BD_MEMBER_PD_XLATE_ENTRY);
  }

  bd_member->pv_hw_inner_vlan = pv_untagged_inner_vlan;
  bd_member->pv_hw_outer_vlan = pv_untagged_outer_vlan;

  pv_entry = SWITCH_MALLOC(device, sizeof(switch_pv_entry_t), 0x1);
  if (!pv_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "memory allocation failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(pv_entry, 0x0, sizeof(switch_pv_entry_t));
  SWITCH_MEMCPY(&pv_entry->pv_key, &pv_key, sizeof(pv_key));
  pv_entry->bd_handle = bd_handle;
  pv_entry->intf_handle = intf_handle;
  pv_entry->member_handle = *member_handle;
  status = SWITCH_HASHTABLE_INSERT(&vlan_ctx->pv_hashtable,
                                   &pv_entry->node,
                                   (void *)&pv_key,
                                   (void *)pv_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member add failed on device %d "
        "bd handle %lx intf handle %lx: "
        "memory allocation failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DETAIL(
      "pv membership added successfully on device %d "
      "bd handle %lx intf handle %lx member handle %lx "
      "outer_vlan %d inner_vlan %d\n",
      device,
      bd_handle,
      intf_handle,
      *member_handle,
      outer_vlan,
      inner_vlan);

  return status;

cleanup:

  return status;
}

/*
 * Routine Description:
 *   @brief remove port vlan membership
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] vlan_handle - vlan handle
 *   @param[in] num_ports - number of interfaces
 *   @param[in] intf_handles - interface list
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vlan_member_remove_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_handle_t intf_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_vlan_info_t *vlan_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_mcast_member_t mcast_member;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d "
        "vlan handle %lx intf handle %lx: "
        "invalid interface handle(%s)",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d "
        "vlan handle %lx intf handle %lx: "
        "invalid interface handle(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (!(SWITCH_INTERFACE_ACCESS(intf_info)) &&
      !(SWITCH_INTERFACE_TRUNK(intf_info))) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d "
        "vlan handle %lx intf handle %lx: "
        "invalid interface type(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_find(
      device, vlan_info->bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d "
        "vlan handle %lx intf handle %lx: "
        "bd member find failed(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (SWITCH_HW_FLAG_ISSET(bd_member, SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY)) {
    mcast_member.handle = intf_handle;
    mcast_member.network_handle = vlan_handle;
    status = switch_api_multicast_member_delete(
        device, bd_info->flood_handle, 0x1, &mcast_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vlan port remove failed on device %d "
          "vlan handle %lx intf handle %lx: "
          "mcast member delete failed(%s)\n",
          device,
          vlan_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(bd_member, SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY);
  }

  status =
      switch_pv_member_delete(device, vlan_info->bd_handle, intf_handle, 0x0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan port remove failed on device %d "
        "vlan handle %lx intf handle %lx: "
        "pv member delete failed(%s)\n",
        device,
        vlan_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_vlan_member_remove_by_member_handle_internal(
    const switch_device_t device, const switch_handle_t member_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_BD_MEMBER_HANDLE(member_handle));
  if (!SWITCH_BD_MEMBER_HANDLE(member_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan member remove failed on device %d "
        "member handle %lx: "
        "member handle invalid(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_get(device, member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan member remove failed on device %d "
        "member handle %lx: "
        "bd member get failed(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_member->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan member remove failed on device %d "
        "member handle %lx: "
        "bd get failed(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(bd_info->handle));
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(bd_member->handle));

  status =
      switch_api_vlan_member_remove(device, bd_info->handle, bd_member->handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan member remove failed on device %d "
        "member handle %lx: "
        "vlan member remove failed(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_pv_member_delete(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_handle_t intf_handle,
                                        switch_uint64_t flags) {
  switch_vlan_context_t *vlan_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_pv_entry_t *pv_entry = NULL;
  switch_vlan_t native_vlan_id = 0;
  switch_pv_key_t pv_key;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VLAN, (void **)&vlan_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "vlan device context get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd handle invalid(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "interface handle invalid(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan ports remove failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  if (SWITCH_INTERFACE_TRUNK(intf_info)) {
    status = switch_api_interface_native_vlan_id_get(
        device, intf_handle, &native_vlan_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member delete failed on device %d "
          "bd handle %lx intf handle %lx: "
          "interface native vlan id get failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (native_vlan_id == bd_member->inner_vlan) {
      SWITCH_INTF_NATIVE_VLAN_HANDLE(intf_info) = SWITCH_API_INVALID_HANDLE;
    }
  }

  SWITCH_MEMSET(&pv_entry, 0x0, sizeof(pv_entry));
  SWITCH_MEMSET(&pv_key, 0x0, sizeof(pv_key));
  pv_key.outer_vlan = bd_member->outer_vlan;
  pv_key.inner_vlan = bd_member->inner_vlan;
  pv_key.port_lag_index = intf_info->port_lag_index;
  status = SWITCH_HASHTABLE_SEARCH(
      &vlan_ctx->pv_hashtable, (void *)&pv_key, (void **)&pv_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "pv member hash lookup failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(pv_entry->intf_handle == intf_handle);
  SWITCH_ASSERT(pv_entry->bd_handle == bd_handle);
  SWITCH_ASSERT(pv_entry->member_handle == bd_member->member_handle);

  if (SWITCH_HW_FLAG_ISSET(bd_member, SWITCH_BD_MEMBER_PD_PV_UNTAGGED_ENTRY)) {
    status = switch_pd_port_vlan_to_bd_mapping_table_entry_delete(
        device, bd_member->pv_untagged_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member delete failed on device %d "
          "bd handle %lx intf handle %lx: "
          "pv mapping table delete failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_HW_FLAG_CLEAR(bd_member, SWITCH_BD_MEMBER_PD_PV_UNTAGGED_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_member, SWITCH_BD_MEMBER_PD_PV_IFINDEX_ENTRY)) {
    status = switch_pd_port_vlan_to_ifindex_mapping_table_entry_delete(
        device, bd_member->pv_ifindex_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member delete failed on device %d "
          "bd handle %lx intf handle %lx: "
          "pv mapping table delete failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_HW_FLAG_CLEAR(bd_member, SWITCH_BD_MEMBER_PD_PV_IFINDEX_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_member, SWITCH_BD_MEMBER_PD_XLATE_ENTRY)) {
    status = switch_pd_egress_vlan_xlate_table_entry_delete(
        device, bd_member->xlate_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pv member delete failed on device %d "
          "bd handle %lx intf handle %lx: "
          "egress vlan xlate table delete failed(%s)\n",
          device,
          bd_handle,
          intf_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_HW_FLAG_CLEAR(bd_member, SWITCH_BD_MEMBER_PD_XLATE_ENTRY);
  }

  status = switch_mcast_bd_member_rid_free(device, bd_handle, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "rid release failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_member_delete(
      device, bd_member->bd_handle, bd_member->member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pv member delete failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd member delete failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_DELETE(
      &vlan_ctx->pv_hashtable, (void *)&pv_key, (void **)&pv_entry);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_FREE(device, pv_entry);

  SWITCH_LOG_DETAIL(
      "pv member deleted successfully on device %d "
      "bd handle %lx intf handle %lx\n",
      device,
      bd_handle,
      intf_handle);

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_vlan_interfaces_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_uint16_t *mbr_count,
    switch_vlan_interface_t **mbrs) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_node_t *node = NULL;
  switch_bd_member_t *vlan_member = NULL;
  switch_uint16_t mbr_count_max = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mbr_count != NULL);
  SWITCH_ASSERT(mbrs != NULL);
  if (!mbr_count || !mbrs) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("vlan interfaces get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("vlan interfaces get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("vlan interfaces get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan interfaces get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mbrs = NULL;
  *mbr_count = 0;

  FOR_EACH_IN_LIST(bd_info->members, node) {
    vlan_member = (switch_bd_member_t *)node->data;

    if (mbr_count_max == *mbr_count) {
      mbr_count_max += 16;
      *mbrs = SWITCH_REALLOC(
          device, *mbrs, (sizeof(switch_vlan_interface_t) * mbr_count_max));
    }
    (*mbrs)[*mbr_count].vlan_handle = vlan_handle;
    (*mbrs)[*mbr_count].intf_handle = vlan_member->handle;
    (*mbrs)[*mbr_count].member_handle = vlan_member->member_handle;
    (*mbr_count)++;
  }
  FOR_EACH_IN_LIST_END();

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_stats_enable_internal(
    switch_device_t device, switch_handle_t vlan_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan stats enable failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stats enable failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_enable(device, vlan_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stats enable failed on device %d "
        "vlan handle %lx: bd stats enable failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_vlan_stats_disable_internal(
    switch_device_t device, switch_handle_t vlan_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vlan stats disable failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stats disable failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_disable(device, vlan_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stats disable failed on device %d "
        "vlan handle %lx: bd stats disable failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_vlan_stats_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint8_t count,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);

  if (!counters || !counter_ids) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("vlan stats get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, count * sizeof(switch_counter_t));

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("vlan stats get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan stats get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_get(
      device, vlan_info->bd_handle, count, counter_ids, counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan stats get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_vlan_stats_clear_internal(
    const switch_device_t device, const switch_handle_t vlan_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("vlan stats clear failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan stats clear failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_clear(device, vlan_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan stats clear failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_vlan_learning_set_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan learning enabled set failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan learning enabled set failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_learning_set(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan learning enabled set failed on device %d "
        "vlan handle %lx: learn enable bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan learning enabled set on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_learning_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan learning enabled get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan learning enabled get failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_learning_get(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan learning enabled get failed on device %d "
        "vlan handle %lx: learn enable bd get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan learning enabled get on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      *enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_igmp_snooping_set_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled set failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled set failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_igmp_snooping_set(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled set failed on device %d "
        "vlan handle %lx: "
        "igmp snooping enable bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan igmp snooping enabled set on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_igmp_snooping_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled get failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_igmp_snooping_get(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan igmp snooping enabled get failed on device %d "
        "vlan handle %lx: "
        "igmp snooping enable bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan igmp snooping enabled get on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_mld_snooping_set_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled set failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled set failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_mld_snooping_set(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled set failed on device %d "
        "vlan handle %lx: "
        "mld snooping enable bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan mld snooping enabled set on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_mld_snooping_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_mld_snooping_get(device, vlan_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mld snooping enabled get failed on device %d "
        "vlan handle %lx: "
        "mld snooping enable bd get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan mld snooping enabled get on device %d "
      "vlan handle %lx enable %d\n",
      device,
      vlan_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_aging_interval_set_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_int32_t aging_interval) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan aging interval set failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan aging interval set failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_aging_interval_set(
      device, vlan_info->bd_handle, aging_interval);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan aging interval set failed on device %d "
        "vlan handle %lx: aging interval bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan aging interval set on device %d "
      "vlan handle %lx aging interval %d\n",
      device,
      vlan_handle,
      aging_interval);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_aging_interval_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_int32_t *aging_interval) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan aging interval get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan aging interval get failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_aging_interval_get(
      device, vlan_info->bd_handle, aging_interval);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan aging interval get failed on device %d "
        "vlan handle %lx: aging interval bd set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan aging interval get on device %d "
      "vlan handle %lx aging interval %d\n",
      device,
      vlan_handle,
      *aging_interval);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_stp_handle_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t stp_handle) {
  switch_handle_t stp_handle_tmp = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan stp handle set failed on device %d "
        "vlan handle %lx stp handle %lx: "
        "stp handle invalid(%s)\n",
        device,
        vlan_handle,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (stp_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
    if (!SWITCH_STP_HANDLE(stp_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "vlan stp handle set failed on device %d "
          "vlan handle %lx stp handle %lx: "
          "stp handle invalid(%s)\n",
          device,
          vlan_handle,
          stp_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_api_stp_group_member_add(device, stp_handle, vlan_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vlan stp handle set failed on device %d "
          "vlan handle %lx stp handle %lx: "
          "stp group member add failed(%s)\n",
          device,
          vlan_handle,
          stp_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    status =
        switch_api_vlan_stp_handle_get(device, vlan_handle, &stp_handle_tmp);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vlan stp handle set failed on device %d "
          "vlan handle %lx stp handle %lx: "
          "stp group member get failed(%s)\n",
          device,
          vlan_handle,
          stp_handle,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle_tmp));

    status =
        switch_api_stp_group_member_remove(device, stp_handle_tmp, vlan_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vlan stp handle set failed on device %d "
          "vlan handle %lx stp handle %lx: "
          "stp group member remove failed(%s)\n",
          device,
          vlan_handle,
          stp_handle_tmp,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "vlan stp handle set on device %d "
      "vlan handle %lx stp handle %lx\n",
      device,
      vlan_handle,
      stp_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_stp_handle_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t *stp_handle) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan stp handle get failed on device %d "
        "vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stp handle set failed on device %d "
        "vlan handle %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stp_handle_get(device, vlan_info->bd_handle, stp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan stp handle set failed on device %d "
        "vlan handle %lx: "
        "bd stp handle get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_mrpf_group_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_mrpf_group_t mrpf_group) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan mrpf group set failed on device %d "
        "vlan handle %lx mrpf group %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        mrpf_group,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mrpf group set failed on device %d "
        "vlan handle %lx mrpf group %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        mrpf_group,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_mrpf_group_set(device, vlan_info->bd_handle, mrpf_group);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mrpf group set failed on device %d "
        "vlan handle %lx mrpf group %lx: "
        "bd mrpf group set failed(%s)\n",
        device,
        vlan_handle,
        mrpf_group,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_mrpf_group_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_mrpf_group_t *mrpf_group) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan mrpf group get failed on device %d "
        "vlan handle %lx: "
        "vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mrpf group get failed on device %d "
        "vlan handle %lx: "
        "vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_mrpf_group_get(device, vlan_info->bd_handle, mrpf_group);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan mrpf group get failed on device %d "
        "vlan handle %lx: "
        "bd mrpf group get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_attribute_set_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    const switch_api_vlan_info_t *api_vlan_info) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan attribute set failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan attribute set failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));

  if (flags & SWITCH_VLAN_ATTR_LEARNING_ENABLED) {
    bd_info.learning = api_vlan_info->learning_enabled;
    bd_flags |= SWITCH_BD_ATTR_LEARNING;
  }

  if (flags & SWITCH_VLAN_ATTR_IGMP_SNOOPING_ENABLED) {
    bd_info.igmp_snooping = api_vlan_info->igmp_snooping_enabled;
    bd_flags |= SWITCH_BD_ATTR_IGMP_SNOOPING;
  }

  if (flags & SWITCH_VLAN_ATTR_MLD_SNOOPING_ENABLED) {
    bd_info.mld_snooping = api_vlan_info->mld_snooping_enabled;
    bd_flags |= SWITCH_BD_ATTR_MLD_SNOOPING;
  }

  if (flags & SWITCH_VLAN_ATTR_AGING_INTERVAL) {
    bd_info.aging_interval = api_vlan_info->aging_interval;
    bd_flags |= SWITCH_BD_ATTR_AGING_INTERVAL;
  }

  if (flags & SWITCH_VLAN_ATTR_MRPF_GROUP) {
    bd_info.mrpf_group = api_vlan_info->mrpf_group;
    bd_flags |= SWITCH_BD_ATTR_MRPF_GROUP;
  }

  status = switch_bd_update(device, vlan_info->bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan attribute set failed on device %d "
        "vlan handle %lx: bd attribute set failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vlan attribute set on device %d vlan handle %lx\n", device, vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_attribute_get_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    switch_api_vlan_info_t *api_vlan_info) {
  switch_vlan_info_t *vlan_info = NULL;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan attribute get failed on device %d "
        "vlan handle %lx: vlan handle invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan attribute get failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));

  if (flags & SWITCH_VLAN_ATTR_LEARNING_ENABLED) {
    bd_flags |= SWITCH_BD_ATTR_LEARNING;
  }

  if (flags & SWITCH_VLAN_ATTR_IGMP_SNOOPING_ENABLED) {
    bd_flags |= SWITCH_BD_ATTR_IGMP_SNOOPING;
  }

  if (flags & SWITCH_VLAN_ATTR_MLD_SNOOPING_ENABLED) {
    bd_flags |= SWITCH_BD_ATTR_MLD_SNOOPING;
  }

  if (flags & SWITCH_VLAN_ATTR_AGING_INTERVAL) {
    bd_flags |= SWITCH_BD_ATTR_AGING_INTERVAL;
  }

  if (flags & SWITCH_VLAN_ATTR_AGING_INTERVAL) {
    bd_flags |= SWITCH_BD_ATTR_MRPF_GROUP;
  }
  status =
      switch_bd_attribute_get(device, vlan_info->bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan attribute get failed on device %d "
        "vlan handle %lx: bd attribute get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_VLAN_ATTR_LEARNING_ENABLED) {
    api_vlan_info->learning_enabled = bd_info.learning;
  }

  if (flags & SWITCH_VLAN_ATTR_IGMP_SNOOPING_ENABLED) {
    api_vlan_info->igmp_snooping_enabled = bd_info.igmp_snooping;
  }

  if (flags & SWITCH_VLAN_ATTR_MLD_SNOOPING_ENABLED) {
    api_vlan_info->mld_snooping_enabled = bd_info.mld_snooping;
  }

  if (flags & SWITCH_VLAN_ATTR_AGING_INTERVAL) {
    api_vlan_info->aging_interval = bd_info.aging_interval;
  }

  if (flags & SWITCH_VLAN_ATTR_MRPF_GROUP) {
    api_vlan_info->mrpf_group = bd_info.mrpf_group;
  }

  SWITCH_LOG_DEBUG(
      "vlan attribute get on device %d vlan handle %lx\n", device, vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_vlan_ingress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_bd_label_t label) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;
  switch_uint64_t bd_flags = 0;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      bd_info->ingress_bd_label = label;
      break;
    case SWITCH_HANDLE_TYPE_VLAN:
      bd_info->ingress_bd_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }
  bd_info->ingress_acl_group_handle = acl_group;
  bd_flags |= SWITCH_BD_ATTR_INGRESS_LABEL;

  status = switch_bd_update(device, vlan_info->bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd label set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        vlan_info->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ACL label set on device %d "
      "bd handle %lx bd label %d bp_type %d\n",
      device,
      vlan_info->bd_handle,
      bd_info->ingress_bd_label,
      bp_type);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_ingress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t acl_group) {
  return switch_vlan_ingress_acl_group_label_set(
      device, vlan_handle, SWITCH_HANDLE_TYPE_VLAN, acl_group, 0);
}

switch_status_t switch_api_vlan_ingress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t label) {
  return switch_vlan_ingress_acl_group_label_set(
      device, vlan_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_vlan_egress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_bd_label_t label) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;
  switch_uint64_t bd_flags = 0;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      bd_info->egress_bd_label = label;
      break;
    case SWITCH_HANDLE_TYPE_VLAN:
      bd_info->egress_bd_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }

  bd_info->egress_acl_group_handle = acl_group;
  bd_flags |= SWITCH_BD_ATTR_EGRESS_LABEL;

  status = switch_bd_update(device, vlan_info->bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd label set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        vlan_info->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ACL label set on device %d "
      "bd handle %lx bd label %d bp_type %d\n",
      device,
      vlan_info->bd_handle,
      bd_info->egress_bd_label,
      bp_type);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_egress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t acl_group) {
  return switch_api_vlan_egress_acl_group_label_set(
      device, vlan_handle, SWITCH_HANDLE_TYPE_VLAN, acl_group, 0);
}

switch_status_t switch_api_vlan_egress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t label) {
  return switch_api_vlan_egress_acl_group_label_set(
      device, vlan_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_vlan_ingress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t *acl_group) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *acl_group = bd_info->ingress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_ingress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t *label) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *label = bd_info->ingress_bd_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_egress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_handle_t *acl_group) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *acl_group = bd_info->egress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_egress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t *label) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  status = switch_bd_get(device, vlan_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };
  if (bd_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *label = bd_info->egress_bd_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_vlan_member_vlan_id_get_internal(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_vlan_t *vlan_id) {
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_bd_member_get(device, vlan_member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member get failed on device %d "
        "bd member handle %lx: "
        "bd member get failed(%s)\n",
        device,
        vlan_member_handle,
        switch_error_to_string(status));
    return status;
  }
  *vlan_id = bd_member->inner_vlan;
  return status;
}

switch_status_t switch_api_vlan_member_vlan_tagging_mode_get_internal(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    bool *tag_mode) {
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_vlan_t native_vlan_id = 0;

  status = switch_bd_member_get(device, vlan_member_handle, &bd_member);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member get failed on device %d "
        "bd member handle %lx: "
        "bd member get failed(%s)\n",
        device,
        vlan_member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_interface_native_vlan_id_get(
      device, bd_member->handle, &native_vlan_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan id get failed on device %d "
        "vlan-member handle %lx: ",
        device,
        vlan_member_handle,
        switch_error_to_string(status));
    return status;
  }
  if (native_vlan_id == bd_member->inner_vlan) {
    *tag_mode = FALSE;
  } else {
    *tag_mode = TRUE;
  }
  return status;
}

switch_status_t switch_api_vlan_member_intf_handle_get_internal(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_handle_t *intf_handle) {
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_bd_member_get(device, vlan_member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "intf_handle get failed on device %d "
        "vlan member handle %lx : "
        "bd member get failed(%s)\n",
        device,
        vlan_member_handle,
        switch_error_to_string(status));
    return status;
  }
  *intf_handle = bd_member->handle;
  return status;
}

switch_status_t switch_api_vlan_bd_get_internal(switch_device_t device,
                                                switch_handle_t vlan_handle,
                                                switch_uint32_t *bd) {
  switch_status_t status;
  switch_vlan_info_t *vlan_info = NULL;

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *bd = handle_to_id(vlan_info->bd_handle);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_vlan_stats_disable(
    const switch_device_t device, const switch_handle_t vlan_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_stats_disable_internal(device, vlan_handle))
}

switch_status_t switch_api_vlan_create(const switch_device_t device,
                                       const switch_vlan_t vlan_id,
                                       switch_handle_t *vlan_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_create_internal(device, vlan_id, vlan_handle))
}

switch_status_t switch_api_vlan_mld_snooping_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_mld_snooping_get_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_stats_enable(
    const switch_device_t device, const switch_handle_t vlan_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_stats_enable_internal(device, vlan_handle))
}

switch_status_t switch_api_vlan_stats_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint8_t num_entries,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_vlan_stats_get_internal(
      device, vlan_handle, num_entries, counter_ids, counters))
}

switch_status_t switch_api_vlan_stats_clear(const switch_device_t device,
                                            const switch_handle_t vlan_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_stats_clear_internal(device, vlan_handle));
}

switch_status_t switch_api_vlan_member_remove_by_member_handle(
    const switch_device_t device, const switch_handle_t member_handles) {
  SWITCH_MT_WRAP(switch_api_vlan_member_remove_by_member_handle_internal(
      device, member_handles))
}

switch_status_t switch_api_vlan_interfaces_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_uint16_t *num_entries,
    switch_vlan_interface_t **mbrs) {
  SWITCH_MT_WRAP(switch_api_vlan_interfaces_get_internal(
      device, vlan_handle, num_entries, mbrs))
}

switch_status_t switch_api_vlan_delete(const switch_device_t device,
                                       const switch_handle_t vlan_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_delete_internal(device, vlan_handle))
}

switch_status_t switch_api_vlan_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_vlan_ingress_acl_group_get_internal(
      device, bd_handle, acl_group))
}

switch_status_t switch_api_vlan_ingress_acl_label_get(switch_device_t device,
                                                      switch_handle_t bd_handle,
                                                      switch_uint16_t *label) {
  SWITCH_MT_WRAP(
      switch_api_vlan_ingress_acl_label_get_internal(device, bd_handle, label))
}

switch_status_t switch_api_vlan_egress_acl_group_get(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_vlan_egress_acl_group_get_internal(
      device, bd_handle, acl_group))
}

switch_status_t switch_api_vlan_egress_acl_label_get(switch_device_t device,
                                                     switch_handle_t bd_handle,
                                                     switch_uint16_t *label) {
  SWITCH_MT_WRAP(
      switch_api_vlan_egress_acl_label_get_internal(device, bd_handle, label))
}

switch_status_t switch_api_vlan_mrpf_group_get(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_mrpf_group_t *mrpf_group) {
  SWITCH_MT_WRAP(
      switch_api_vlan_mrpf_group_get_internal(device, vlan_handle, mrpf_group))
}

switch_status_t switch_api_vlan_learning_set(const switch_device_t device,
                                             const switch_handle_t vlan_handle,
                                             const bool enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_learning_set_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_learning_get(const switch_device_t device,
                                             const switch_handle_t vlan_handle,
                                             bool *enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_learning_get_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_member_add(const switch_device_t device,
                                           const switch_handle_t vlan_handle,
                                           const switch_handle_t intf_handle,
                                           switch_handle_t *member_handles) {
  SWITCH_MT_WRAP(switch_api_vlan_member_add_internal(
      device, vlan_handle, intf_handle, member_handles))
}

switch_status_t switch_api_vlan_aging_interval_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_int32_t age_interval) {
  SWITCH_MT_WRAP(switch_api_vlan_aging_interval_set_internal(
      device, vlan_handle, age_interval))
}

switch_status_t switch_api_vlan_igmp_snooping_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_igmp_snooping_get_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_attribute_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    switch_api_vlan_info_t *api_vlan_info) {
  SWITCH_MT_WRAP(switch_api_vlan_attribute_get_internal(
      device, vlan_handle, flags, api_vlan_info))
}

switch_status_t switch_api_vlan_mld_snooping_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_mld_snooping_set_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_id_to_handle_get(const switch_device_t device,
                                                 const switch_vlan_t vlan_id,
                                                 switch_handle_t *vlan_handle) {
  SWITCH_MT_WRAP(
      switch_api_vlan_id_to_handle_get_internal(device, vlan_id, vlan_handle))
}

switch_status_t switch_api_vlan_handle_to_id_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_vlan_t *vlan_id) {
  SWITCH_MT_WRAP(
      switch_api_vlan_handle_to_id_get_internal(device, vlan_handle, vlan_id))
}

switch_status_t switch_api_vlan_stp_handle_set(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_handle_t stp_handle) {
  SWITCH_MT_WRAP(
      switch_api_vlan_stp_handle_set_internal(device, vlan_handle, stp_handle))
}

switch_status_t switch_api_vlan_mrpf_group_set(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_mrpf_group_t mrpf_group) {
  SWITCH_MT_WRAP(
      switch_api_vlan_mrpf_group_set_internal(device, vlan_handle, mrpf_group))
}

switch_status_t switch_api_vlan_ingress_acl_group_set(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t acl_group) {
  SWITCH_MT_WRAP(switch_api_vlan_ingress_acl_group_set_internal(
      device, bd_handle, acl_group))
}

switch_status_t switch_api_vlan_egress_acl_group_set(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t acl_group) {
  SWITCH_MT_WRAP(switch_api_vlan_egress_acl_group_set_internal(
      device, bd_handle, acl_group))
}

switch_status_t switch_api_vlan_ingress_acl_label_set(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t label) {
  SWITCH_MT_WRAP(switch_api_vlan_ingress_acl_label_set_internal(
      device, vlan_handle, label))
}

switch_status_t switch_api_vlan_egress_acl_label_set(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_bd_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_vlan_egress_acl_label_set_internal(device, vlan_handle, label))
}

switch_status_t switch_api_vlan_igmp_snooping_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable) {
  SWITCH_MT_WRAP(
      switch_api_vlan_igmp_snooping_set_internal(device, vlan_handle, enable))
}

switch_status_t switch_api_vlan_stp_handle_get(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_handle_t *stp_handle) {
  SWITCH_MT_WRAP(
      switch_api_vlan_stp_handle_get_internal(device, vlan_handle, stp_handle))
}

switch_status_t switch_api_vlan_aging_interval_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_int32_t *age_interval) {
  SWITCH_MT_WRAP(switch_api_vlan_aging_interval_get_internal(
      device, vlan_handle, age_interval))
}

switch_status_t switch_api_vlan_member_remove(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_handle_t intf_handle) {
  SWITCH_MT_WRAP(
      switch_api_vlan_member_remove_internal(device, vlan_handle, intf_handle))
}

switch_status_t switch_api_vlan_attribute_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    const switch_api_vlan_info_t *api_vlan_info) {
  SWITCH_MT_WRAP(switch_api_vlan_attribute_set_internal(
      device, vlan_handle, flags, api_vlan_info))
}

switch_status_t switch_api_vlan_member_vlan_id_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_vlan_t *vlan_id) {
  SWITCH_MT_WRAP(switch_api_vlan_member_vlan_id_get_internal(
      device, vlan_member_handle, vlan_id))
}

switch_status_t switch_api_vlan_member_vlan_tagging_mode_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    bool *tag_mode) {
  SWITCH_MT_WRAP(switch_api_vlan_member_vlan_tagging_mode_get_internal(
      device, vlan_member_handle, tag_mode))
}

switch_status_t switch_api_vlan_member_intf_handle_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(switch_api_vlan_member_intf_handle_get_internal(
      device, vlan_member_handle, intf_handle))
}

switch_status_t switch_api_vlan_bd_get(switch_device_t device,
                                       switch_handle_t vlan_handle,
                                       switch_uint32_t *bd) {
  SWITCH_MT_WRAP(switch_api_vlan_bd_get_internal(device, vlan_handle, bd))
}
