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

#include "switchapi/switch_l2.h"

#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_L2

/*
 * Routine Description:
 *   @brief compute mac table entry hash
 *
 * Arguments:
 *   @param[in] args - mac entry
 *   @param[out] key - hash key
 *   @param[out] len - hash key length
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mac_table_entry_key_init(void *args,
                                                switch_uint8_t *key,
                                                switch_uint32_t *len) {
  switch_mac_entry_t *mac_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(args && key && len);
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l2 hash entry key init failed"
        "invalid parameters(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  *len = 0;
  mac_entry = (switch_mac_entry_t *)args;

  SWITCH_MEMCPY(key, &mac_entry->bd_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY((key + *len), &mac_entry->mac, SWITCH_MAC_LENGTH);
  *len += SWITCH_MAC_LENGTH;

  SWITCH_ASSERT(*len == SWITCH_MAC_HASH_KEY_SIZE);

  return status;
}

/*
 * Routine Description:
 *   @brief mac table entry hash compare
 *
 * Arguments:
 *   @param[in] key1 - hash key
 *   @param[in] key2 - mac info struct
 *
 * Return Values:
 *    @return 0 if key matches
 *            -1 o +1 depending which key is greater
 */
switch_int32_t switch_mac_entry_hash_compare(const void *key1,
                                             const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_MAC_HASH_KEY_SIZE);
}

/*
 * Routine Description:
 *   @brief l2 default entries
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_l2_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  /**
   * Set default action to malformed packet
   */
  status = switch_pd_validate_outer_ethernet_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 default entry add failed on target %d. "
        "validate outer ethernet default "
        "entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /**
   * Set smac and dmac default action to miss
   */
  status = switch_pd_mac_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 default entry add failed on target %d. "
        "mac table default entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /**
   * validate incoming packets
   * 1) source mac multicast - DROP_SRC_MAC_MULTICAST
   * 2) destination mac is zero - DROP_DST_MAC_ZERO
   * 3) ipv4/ipv6 ttl is zero - DROP_IP_TTL_ZERO
   * 4) ipv4 src is loopback - DROP_IP_SRC_LOOPBACK
   * 5) ipv4/ipv6 source is multicast - DROP_IP_SRC_MULTICAST
   * 6) ipv4/ipv6 version valid - DROP_IP_VERSION_INVALID
   * 7) broadcast
   * 8) ipv6 link local multicast
   * 9) multicast mac da
   * 10) ipv6 link local unicast
   * 11) default unicast
   */
  status = switch_pd_validate_packet_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 default entry add failed on target %d. "
        "validate packet table default "
        "entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_validate_outer_ethernet_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 default entry add failed on target %d. "
        "validate outer ethernet table default "
        "entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_learn_notify_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 default entry add failed on target %d. "
        "learn notify table default "
        "entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("l2 default entries added on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_l2_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  UNUSED(device);

  SWITCH_LOG_DETAIL("l2 default entries deleted on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief l2 initialization
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_l2_init(switch_device_t device) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_size_t mac_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  l2_ctx = SWITCH_MALLOC(device, sizeof(switch_l2_context_t), 0x1);
  if (!l2_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "l2 context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(l2_ctx, 0x0, sizeof(switch_l2_context_t));

  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_L2, (void *)l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "l2 context set failed (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_table_size_get(device, SWITCH_TABLE_DMAC, &mac_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "mac table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(mac_table_size != 0);

  l2_ctx->learn_client_data = SWITCH_MALLOC(device, sizeof(switch_device_t), 1);
  if (!l2_ctx->learn_client_data) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "learn client data allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * register for learn callback
   */
  status =
      switch_pd_mac_learn_callback_register(device, l2_ctx->learn_client_data);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "learn callback registration failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  l2_ctx->aging_client_data = SWITCH_MALLOC(device, sizeof(switch_device_t), 1);
  if (!l2_ctx->aging_client_data) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "aging client data allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * register for aging call back
   */
  status =
      switch_pd_mac_aging_callback_register(device,
                                            SWITCH_MAC_TABLE_DEFAULT_AGING_TIME,
                                            SWITCH_MAC_TABLE_MAX_AGING_TIME,
                                            SWITCH_MAC_QUERY_INTERVAL,
                                            l2_ctx->aging_client_data);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "aging callback registration failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * initialize mac hashtable
   */
  l2_ctx->mac_hashtable.size = mac_table_size;
  l2_ctx->mac_hashtable.compare_func = switch_mac_entry_hash_compare;
  l2_ctx->mac_hashtable.key_func = switch_mac_table_entry_key_init;
  l2_ctx->mac_hashtable.hash_seed = SWITCH_MAC_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&l2_ctx->mac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d: "
        "mac hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_MAC
   */
  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_MAC, mac_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 init failed on device %d :"
        "mac handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("l2 init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  status = switch_l2_free(device);
  SWITCH_ASSERT(status != SWITCH_STATUS_SUCCESS);
  return status;
}

/*
 * Routine Description:
 *   @brief l2 free
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_l2_free(switch_device_t device) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 free failed on device %d: "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(l2_ctx->mac_hashtable.num_entries == 0);
  status = SWITCH_HASHTABLE_DONE(&l2_ctx->mac_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 free failed on device %d: "
        "l2 hashtable done failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_MAC
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MAC);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l2 free failed on device %d: "
        "mac handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  if (l2_ctx->learn_client_data) {
    SWITCH_FREE(device, l2_ctx->learn_client_data);
    l2_ctx->learn_client_data = NULL;
  }

  if (l2_ctx->aging_client_data) {
    SWITCH_FREE(device, l2_ctx->aging_client_data);
    l2_ctx->aging_client_data = NULL;
  }

  SWITCH_FREE(device, l2_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_L2, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("l2 done successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief adds mac handle to network list
 *   and interface list
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] mac_handle - mac handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mac_list_insert(switch_device_t device,
                                       switch_handle_t mac_handle) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d: "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx: "
        "mac handle invalid(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx: "
        "mac get failed(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_handle = mac_info->mac_entry.bd_handle;

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx bd handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        mac_handle,
        mac_info->mac_entry.bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_ARRAY_INSERT(&bd_info->mac_array, mac_handle, (void *)mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx bd handle 0x%lx: "
        "bd mac array insert failed(%s)\n",
        device,
        mac_handle,
        mac_info->mac_entry.bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_INTERFACE_HANDLE(mac_info->handle)) {
    status = switch_interface_get(device, mac_info->handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list insert failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_ARRAY_INSERT(
        &intf_info->mac_array, mac_handle, (void *)mac_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list insert failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface mac array insert failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_HASHTABLE_INSERT(&l2_ctx->mac_hashtable,
                                   &(mac_info->node),
                                   (void *)(&mac_info->mac_entry),
                                   (void *)(mac_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx: hashtable insert failed(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

/*
 * Routine Description:
 *   @brief update interface handle in interface list
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] mac_handle - mac handle
 *   @param[in] handle - interface/nexthop/mgid handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mac_list_update(switch_device_t device,
                                       switch_handle_t mac_handle,
                                       switch_handle_t handle) {
  switch_mac_info_t *mac_info = NULL;
  switch_interface_info_t *old_intf_info = NULL;
  switch_interface_info_t *new_intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx: "
        "mac handle invalid(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list insert failed on device %d "
        "mac handle 0x%lx: "
        "mac get failed(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  if (mac_info->handle == handle) {
    return status;
  }

  if (SWITCH_INTERFACE_HANDLE(mac_info->handle)) {
    status = switch_interface_get(device, mac_info->handle, &old_intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list delete failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_ARRAY_DELETE(&old_intf_info->mac_array, mac_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list delete failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface mac array insert failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }
    mac_info->handle = SWITCH_API_INVALID_HANDLE;
  }

  if (SWITCH_INTERFACE_HANDLE(handle)) {
    status = switch_interface_get(device, handle, &new_intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list insert failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_ARRAY_INSERT(
        &new_intf_info->mac_array, mac_handle, (void *)mac_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list insert failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface mac array insert failed(%s)\n",
          device,
          mac_handle,
          mac_info->handle,
          switch_error_to_string(status));
      return status;
    }
    mac_info->handle = handle;
  }

  return status;
}

/*
 * Routine Description:
 *   @brief remove mac address from network and
 *   interface list
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] mac_handle - mac handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mac_list_delete(switch_device_t device,
                                       switch_handle_t mac_handle) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d: "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d "
        "mac handle 0x%lx: "
        "mac handle invalid(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d "
        "mac handle 0x%lx: "
        "mac get failed(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_handle = mac_info->mac_entry.bd_handle;

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d "
        "mac handle 0x%lx bd handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        mac_handle,
        mac_info->mac_entry.bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(&bd_info->mac_array, mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d "
        "mac handle 0x%lx bd handle 0x%lx: "
        "bd mac array remove failed(%s)\n",
        device,
        mac_handle,
        mac_info->mac_entry.bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_INTERFACE_HANDLE(mac_info->handle)) {
    status = switch_interface_get(device, mac_info->handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list remove failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          mac_handle,
          mac_info->mac_entry.bd_handle,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_ARRAY_DELETE(&intf_info->mac_array, mac_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac list remove failed on device %d "
          "mac handle 0x%lx handle 0x%lx: "
          "interface mac array remove failed(%s)\n",
          device,
          mac_handle,
          mac_info->mac_entry.bd_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_HASHTABLE_DELETE(&l2_ctx->mac_hashtable,
                                   (void *)(&mac_info->mac_entry),
                                   (void **)&mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac list remove failed on device %d "
        "mac handle 0x%lx bd handle 0x%lx: "
        "hashtable remove failed(%s)\n",
        device,
        mac_handle,
        mac_info->mac_entry.bd_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_mac_entry_handle_get_internal(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *mac_handle) {
  switch_mac_entry_t mac_entry;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_mac_entry != NULL);
  SWITCH_ASSERT(mac_handle != NULL);
  if (!api_mac_entry || !mac_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_bd_handle_get(device, api_mac_entry->network_handle, &bd_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d "
        "bd handle 0x%lx invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&mac_entry, 0x0, sizeof(switch_mac_entry_t));
  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, &api_mac_entry->mac, sizeof(switch_mac_addr_t));

  status = switch_mac_table_entry_find(device, &mac_entry, mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("mac entry handle get on device %d mac handle 0x%lx\n",
                   device,
                   *mac_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_mac_table_entry_find(switch_device_t device,
                                            switch_mac_entry_t *mac_entry,
                                            switch_handle_t *mac_handle) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mac_handle != NULL);
  SWITCH_ASSERT(mac_entry != NULL);

  if (!mac_handle || !mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry find failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry find failed on device %d "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *mac_handle = SWITCH_API_INVALID_HANDLE;

  status = SWITCH_HASHTABLE_SEARCH(
      &l2_ctx->mac_hashtable, (void *)mac_entry, (void **)&mac_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    *mac_handle = mac_info->mac_handle;
  }

  return status;
}

switch_status_t switch_mac_event_app_notify(switch_device_t device,
                                            switch_api_mac_entry_t *mac_entry,
                                            switch_mac_event_t mac_event) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(mac_entry != NULL);
  if (!mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("mac event app notify failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mac event app notify failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_MAC_EVENT_REGISTRATION_MAX; index++) {
    if (l2_ctx->mac_event_list[index].valid) {
      if (l2_ctx->mac_event_list[index].mac_event_flags & mac_event) {
        l2_ctx->mac_event_list[index].cb_fn(device, mac_entry, mac_event, NULL);
      }
    }
  }

  return status;
}

switch_status_t switch_mac_learn_notify(switch_device_t device,
                                        switch_pd_mac_info_t *pd_mac_entries,
                                        switch_uint16_t num_entries) {
  switch_pd_mac_info_t *pd_mac_entry = NULL;
  switch_api_mac_entry_t *mac_entries_new = NULL;
  switch_api_mac_entry_t *mac_entries_update = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_mac_entry_t mac_entry;
  switch_uint16_t index = 0;
  switch_size_t num_update_entries = 0;
  switch_size_t num_new_entries = 0;
  switch_api_mac_entry_t tmp_mac_entry;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(pd_mac_entries != NULL);
  SWITCH_ASSERT(num_entries != 0);
  if (!pd_mac_entries || num_entries == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac learn notify failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  mac_entries_new =
      SWITCH_MALLOC(device, sizeof(switch_api_mac_entry_t), num_entries);
  if (!mac_entries_new) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mac learn notify failed on device %d "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  mac_entries_update =
      SWITCH_MALLOC(device, sizeof(switch_api_mac_entry_t), num_entries);
  if (!mac_entries_update) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mac learn notify failed on device %d "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  for (index = 0; index < num_entries; index++) {
    pd_mac_entry = &pd_mac_entries[index];
    bd_handle = id_to_handle(SWITCH_HANDLE_TYPE_BD, pd_mac_entry->bd);
    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac learn notify ignored on device %d "
          "bd handle 0x%lx mac %x: "
          "bd handle invalid(%s)\n",
          device,
          bd_handle,
          switch_macaddress_to_string(&pd_mac_entry->mac),
          switch_error_to_string(status));
      continue;
    }

    if (!bd_info->learning) {
      status = SWITCH_STATUS_NOT_SUPPORTED;
      SWITCH_LOG_DETAIL(
          "mac learn notify failed on device %d "
          "bd handle 0x%lx mac %x: "
          "learning not enabled(%s)\n",
          device,
          bd_handle,
          switch_macaddress_to_string(&pd_mac_entry->mac),
          switch_error_to_string(status));
      continue;
    }

    status = switch_interface_handle_get(
        device, pd_mac_entry->ifindex, &intf_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac learn notify failed on device %d "
          "bd handle 0x%lx mac %s intf handle 0x%lx"
          "interface get failed(%s)\n",
          device,
          bd_handle,
          switch_macaddress_to_string(&pd_mac_entry->mac),
          intf_handle,
          switch_error_to_string(status));
      continue;
    }

    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac learn notify failed on device %d "
          "bd handle 0x%lx mac %s intf handle 0x%lx"
          "interface get failed(%s)\n",
          device,
          bd_handle,
          switch_macaddress_to_string(&pd_mac_entry->mac),
          intf_handle,
          switch_error_to_string(status));
      continue;
    }

    SWITCH_MEMSET(&mac_entry, 0x0, sizeof(switch_mac_entry_t));
    mac_entry.bd_handle = bd_handle;
    SWITCH_MEMCPY(
        &mac_entry.mac, &pd_mac_entry->mac, sizeof(switch_mac_addr_t));

    SWITCH_MEMSET(&tmp_mac_entry, 0x0, sizeof(switch_api_mac_entry_t));

    status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
    if (status == SWITCH_STATUS_SUCCESS) {
      tmp_mac_entry.entry_type = SWITCH_MAC_ENTRY_DYNAMIC;
      tmp_mac_entry.mac_action = SWITCH_MAC_ACTION_FORWARD;
      tmp_mac_entry.network_handle = bd_info->handle;
      tmp_mac_entry.handle = intf_handle;
      SWITCH_MEMCPY(
          &tmp_mac_entry.mac, &pd_mac_entry->mac, sizeof(switch_mac_addr_t));

      status = switch_mac_event_app_notify(
          device, &tmp_mac_entry, SWITCH_MAC_EVENT_MOVE);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_DETAIL(
            "mac learn notify failed on device %d "
            "bd handle 0x%lx mac %s intf handle 0x%lx"
            "mac move event notify failed(%s)\n",
            device,
            bd_handle,
            switch_macaddress_to_string(&pd_mac_entry->mac),
            intf_handle,
            switch_error_to_string(status));
      }
      SWITCH_MEMCPY(&mac_entries_update[num_update_entries],
                    &tmp_mac_entry,
                    sizeof(switch_api_mac_entry_t));
      num_update_entries++;
    } else {
      tmp_mac_entry.entry_type = SWITCH_MAC_ENTRY_DYNAMIC;
      tmp_mac_entry.mac_action = SWITCH_MAC_ACTION_FORWARD;
      tmp_mac_entry.network_handle = bd_info->handle;
      tmp_mac_entry.handle = intf_handle;
      SWITCH_MEMCPY(
          &tmp_mac_entry.mac, &pd_mac_entry->mac, sizeof(switch_mac_addr_t));

      status = switch_mac_event_app_notify(
          device, &tmp_mac_entry, SWITCH_MAC_EVENT_LEARN);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_DETAIL(
            "mac learn notify failed on device %d "
            "bd handle 0x%lx mac %s intf handle 0x%lx"
            "mac learn event notify failed(%s)\n",
            device,
            bd_handle,
            switch_macaddress_to_string(&pd_mac_entry->mac),
            intf_handle,
            switch_error_to_string(status));
      }
      SWITCH_MEMCPY(&mac_entries_new[num_new_entries],
                    &tmp_mac_entry,
                    sizeof(switch_api_mac_entry_t));
      num_new_entries++;
    }
  }

  if (SWITCH_CONFIG_SMAC_PROGRAM()) {
    if (num_new_entries) {
      status = switch_api_mac_table_entries_add(
          device, num_new_entries, mac_entries_new);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac learn notify failed on device %d "
            "mac table entry add failed(%s)\n",
            device,
            switch_error_to_string(status));
      }
    }

    if (num_update_entries) {
      status = switch_api_mac_table_entries_update(
          device, num_update_entries, mac_entries_update);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac learn notify failed on device %d "
            "mac table entry add failed(%s)\n",
            device,
            switch_error_to_string(status));
      }
    }

    SWITCH_LOG_DETAIL(
        "mac learn notify on device %d "
        "mac table add %d mac table update %d\n",
        device,
        num_new_entries,
        num_update_entries);
  }

  SWITCH_FREE(device, mac_entries_new);
  SWITCH_FREE(device, mac_entries_update);

  return status;

cleanup:

  return status;
}

switch_status_t switch_mac_aging_notify(switch_device_t device,
                                        switch_pd_hdl_t pd_hdl) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_api_mac_entry_t mac_entry;
  switch_handle_t intf_handle;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging notify failed on device %d pd handle 0x%lx"
        "l2 context get failed(%s)\n",
        device,
        pd_hdl,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_ARRAY_GET(&l2_ctx->smac_pd_hdl_array, pd_hdl, (void *)&mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging notify failed on device %d pd handle 0x%lx"
        "pd handle invalid(%s)\n",
        device,
        pd_hdl,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_CONFIG_SMAC_PROGRAM()) {
    status = switch_mac_get(device, mac_handle, &mac_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac aging notify failed on device %d "
          "mac handle 0x%lx: "
          "mac get failed(%s)\n",
          device,
          mac_handle,
          switch_error_to_string(status));
      return status;
    }
    /*
     * Flush only dynamic mac entries.
     */
    if (mac_info->entry_type != SWITCH_MAC_ENTRY_DYNAMIC) {
      return status;
    }

    /*
     * Notify mac_aging_event to application.
     */
    status = switch_bd_get(device, mac_info->mac_entry.bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac aging notify ignored on device %d "
          "bd handle 0x%lx mac handle 0x%lx "
          "bd handle invalid(%s)\n",
          device,
          mac_info->mac_entry.bd_handle,
          mac_handle,
          switch_error_to_string(status));
      return status;
    }

    status =
        switch_interface_handle_get(device, mac_info->ifindex, &intf_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac aging notify failed on device %d "
          "bd handle 0x%lx mac handle 0x%lx intf index %x"
          "interface get failed(%s)\n",
          device,
          mac_info->mac_entry.bd_handle,
          mac_handle,
          mac_info->ifindex,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(&mac_entry, 0x0, sizeof(switch_mac_entry_t));
    mac_entry.network_handle = bd_info->handle;
    mac_entry.handle = intf_handle;
    SWITCH_MEMCPY(
        &mac_entry.mac, &mac_info->mac_entry.mac, sizeof(switch_mac_addr_t));
    status =
        switch_mac_event_app_notify(device, &mac_entry, SWITCH_MAC_EVENT_AGE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DETAIL(
          "mac age notify failed on device %d "
          "bd handle 0x%lx mac handle 0x%lx intf handle 0x%lx"
          "mac age event notify failed(%s)\n",
          device,
          mac_info->mac_entry.bd_handle,
          mac_handle,
          intf_handle,
          switch_error_to_string(status));
    }

    status = switch_mac_table_entry_delete_by_handle(device, mac_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac aging notify failed on device %d pd handle 0x%lx"
          "pd handle invalid(%s)\n",
          device,
          pd_hdl,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "mac aging notify on device %d "
      "pd handle 0x%lx mac handle 0x%lx\n",
      device,
      pd_hdl,
      mac_handle);

  return status;
}

switch_status_t switch_mac_aging_poll(switch_device_t device) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_api_mac_entry_t *mac_entries = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_handle_t *mac_handle = NULL;
  switch_size_t num_entries = 0;
  switch_pd_hdl_t pd_hdl = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool is_hit = TRUE;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging poll failed on device %d "
        "l2 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_ARRAY(
      pd_hdl, l2_ctx->smac_pd_hdl_array, switch_handle_t, mac_handle) {
    UNUSED(mac_handle);
    status = switch_pd_smac_hit_state_get(device, pd_hdl, &is_hit);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac aging notify failed on device %d pd handle 0x%lx"
          "pd handle invalid(%s)\n",
          device,
          pd_hdl,
          switch_error_to_string(status));
      continue;
    }

    if (!is_hit) {
      SWITCH_MEMCPY(&mac_entries[num_entries],
                    &mac_info->mac_entry,
                    sizeof(switch_api_mac_entry_t));
      num_entries++;
    }
  }
  FOR_EACH_IN_ARRAY_END();

  status =
      switch_api_mac_table_entries_delete(device, num_entries, mac_entries);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging notify failed on device %d pd handle 0x%lx"
        "pd handle invalid(%s)\n",
        device,
        pd_hdl,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("mac aging poll on device %d\n", device);

  return status;
}

switch_status_t switch_api_mac_table_entry_add_internal(
    switch_device_t device, switch_api_mac_entry_t *api_mac_entry) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tmp_mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_mac_entry_t mac_entry = {0};
  switch_api_nhop_info_t api_nhop_info = {0};
  switch_handle_type_t handle_type = 0;
  switch_nhop_t nhop_index = 0;
  switch_mgid_t mgid_index = 0;
  switch_uint32_t aging_time = 0;
  switch_ifindex_t ifindex = 0;
  switch_port_lag_index_t port_lag_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_mac_entry != NULL);
  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac add received on device %d network handle 0x%lx "
      "mac %s handle 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      api_mac_entry->handle);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d network handle 0x%lx mac %s: "
        "l2 context get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_bd_handle_get(device, api_mac_entry->network_handle, &bd_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d network handle 0x%lx mac %s: "
        "bd get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, &api_mac_entry->mac, sizeof(switch_mac_addr_t));
  status = switch_mac_table_entry_find(device, &mac_entry, &tmp_mac_handle);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d network handle 0x%lx mac %s: "
        "mac entry exists(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  mac_handle = switch_mac_handle_create(device);
  if (mac_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d network handle 0x%lx mac %s: "
        "mac handle allocation failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  handle_type = switch_handle_type_get(api_mac_entry->handle);
  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_PORT:
    case SWITCH_HANDLE_TYPE_LAG:
      SWITCH_ASSERT(TRUE);
      break;

    case SWITCH_HANDLE_TYPE_INTERFACE:
      intf_handle = api_mac_entry->handle;
      status = switch_interface_get(device, intf_handle, &intf_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry add failed on device %d "
            "network handle %lx mac %s: "
            "interface get failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      if (SWITCH_INTF_TYPE(intf_info) == SWITCH_INTERFACE_TYPE_TUNNEL) {
        SWITCH_MEMSET(&api_nhop_info, 0x0, sizeof(api_nhop_info));
        api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_TUNNEL;
        api_nhop_info.tunnel_handle = intf_info->api_intf_info.handle;
        api_nhop_info.network_handle = api_mac_entry->network_handle;
        SWITCH_MEMCPY(&api_nhop_info.ip_addr,
                      &api_mac_entry->ip_addr,
                      sizeof(switch_ip_addr_t));
        status = switch_api_nhop_create(
            device, &api_nhop_info, &mac_info->l2_nhop_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mac table entry add failed on device %d "
              "network handle %lx mac %s: "
              "nhop get failed(%s)\n",
              device,
              api_mac_entry->network_handle,
              switch_macaddress_to_string(&api_mac_entry->mac),
              switch_error_to_string(status));
          return status;
        }
        nhop_index = handle_to_id(mac_info->l2_nhop_handle);
        handle_type = SWITCH_HANDLE_TYPE_NHOP;
      } else {
        ifindex = intf_info->ifindex;
        port_lag_index = intf_info->port_lag_index;
      }
      break;

    case SWITCH_HANDLE_TYPE_NHOP:
      status = switch_nhop_get(device, api_mac_entry->handle, &nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry add failed on device %d "
            "network handle %lx mac %s: "
            "nhop get failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      nhop_index = handle_to_id(api_mac_entry->handle);
      break;

    case SWITCH_HANDLE_TYPE_MGID:
      mgid_index = handle_to_id(api_mac_entry->handle);
      break;

    default:
      status = SWITCH_STATUS_INVALID_HANDLE;
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry add failed on device %d "
            "network handle %lx mac %s: "
            "handle invalid(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      break;
  }

  SWITCH_MEMCPY(&mac_info->mac_entry, &mac_entry, sizeof(mac_entry));
  mac_info->mac_action = api_mac_entry->mac_action;
  mac_info->entry_type = api_mac_entry->entry_type;
  mac_info->mac_handle = mac_handle;
  mac_info->ifindex = ifindex;
  mac_info->port_lag_index = port_lag_index;
  mac_info->handle = api_mac_entry->handle;

  status = switch_mac_list_insert(device, mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d "
        "network handle %lx mac %s: "
        "list insert failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_device_context_get(device, &device_ctx);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (api_mac_entry->entry_type == SWITCH_MAC_ENTRY_DYNAMIC) {
    aging_time = (bd_info->aging_interval == -1)
                     ? device_ctx->device_info.aging_interval
                     : bd_info->aging_interval;
    mac_info->aging_interval = aging_time;
  }

  status = switch_nhop_l3_vlan_interface_resolve(
      device, 0x0, bd_handle, &api_mac_entry->mac, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d "
        "network handle %lx mac %s: "
        "nhop resolution failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    goto cleanup;
  }

  if (!(SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_DMAC_ENTRY))) {
    status = switch_pd_dmac_table_entry_add(device,
                                            handle_type,
                                            bd_handle,
                                            mac_info,
                                            ifindex,
                                            port_lag_index,
                                            nhop_index,
                                            mgid_index,
                                            &mac_info->dmac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry add failed on device %d: "
          "network handle %lx mac %s: "
          "dmac entry add failed(%s)\n",
          device,
          api_mac_entry->network_handle,
          switch_macaddress_to_string(&api_mac_entry->mac),
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(mac_info, SWITCH_L2_PD_DMAC_ENTRY);
  }

  if (!(SWITCH_MGID_HANDLE(api_mac_entry->handle)) && bd_info->learning) {
    if (!(SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_SMAC_ENTRY))) {
      status = switch_pd_smac_table_entry_add(device,
                                              bd_handle,
                                              mac_info,
                                              ifindex,
                                              aging_time,
                                              &mac_info->smac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry add failed on device %d: "
            "network handle %lx mac %s: "
            "smac entry add failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(mac_info, SWITCH_L2_PD_SMAC_ENTRY);
      status = SWITCH_ARRAY_INSERT(
          &l2_ctx->smac_pd_hdl_array, mac_info->smac_entry, (void *)mac_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    }
  }

  status = switch_mac_event_app_notify(
      device, api_mac_entry, SWITCH_MAC_EVENT_CREATE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d: "
        "mac event notify failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac entry add successful on device %d "
      "network handle 0x%lx mac %s handle 0x%lx "
      "mac handle 0x%lx dmac entry 0x%lx smac entry 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      api_mac_entry->handle,
      mac_handle,
      mac_info->dmac_entry,
      mac_info->smac_entry);

  SWITCH_LOG_EXIT();

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_mac_table_entries_add_internal(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *api_mac_entry) {
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  for (index = 0; index < num_entries; index++) {
    status = switch_api_mac_table_entry_add(device, &api_mac_entry[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry add failed on device %d num entries %d: "
          "mac %d failed(%s)\n",
          device,
          num_entries,
          index,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_mac_table_entry_update_internal(
    switch_device_t device, switch_api_mac_entry_t *api_mac_entry) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_mac_entry_t mac_entry;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_type_t handle_type = 0;
  switch_nhop_t nhop_index = 0;
  switch_mgid_t mgid_index = 0;
  switch_ifindex_t ifindex = 0;
  switch_port_lag_index_t port_lag_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_mac_entry != NULL);
  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac update received on device %d network handle 0x%lx "
      "mac %s handle 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      api_mac_entry->handle);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "network handle 0x%lx mac %s: "
        "l2 context get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_bd_handle_get(device, api_mac_entry->network_handle, &bd_handle);

  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "network handle 0x%lx mac %s: "
        "bd get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, &api_mac_entry->mac, sizeof(switch_mac_addr_t));
  status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac entry not found(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry add failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  handle_type = switch_handle_type_get(api_mac_entry->handle);
  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_PORT:
    case SWITCH_HANDLE_TYPE_LAG:
      SWITCH_ASSERT(TRUE);
      break;

    case SWITCH_HANDLE_TYPE_INTERFACE:
      intf_handle = api_mac_entry->handle;
      status = switch_interface_get(device, intf_handle, &intf_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry update failed on device %d: "
            "interface get failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      ifindex = intf_info->ifindex;
      port_lag_index = intf_info->port_lag_index;
      break;

    case SWITCH_HANDLE_TYPE_NHOP:
      status = switch_nhop_get(device, api_mac_entry->handle, &nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry update failed on device %d: "
            "network handle 0x%lx mac %s: "
            "nhop get failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      nhop_index = handle_to_id(api_mac_entry->handle);
      break;

    case SWITCH_HANDLE_TYPE_MGID:
      mgid_index = handle_to_id(api_mac_entry->handle);
      break;

    default:
      status = SWITCH_STATUS_INVALID_HANDLE;
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry update failed on device %d: "
            "network handle 0x%lx mac %s: "
            "handle invalid(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        return status;
      }
      break;
  }

  status = switch_mac_list_update(device, mac_handle, api_mac_entry->handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "network handle 0x%lx mac %s: "
        "list update failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    goto cleanup;
  }

  mac_info->ifindex = ifindex;
  mac_info->port_lag_index = port_lag_index;
  if (api_mac_entry->mac_action != mac_info->mac_action) {
    mac_info->mac_action = api_mac_entry->mac_action;
  }
  status = switch_nhop_l3_vlan_interface_resolve(
      device, 0x0, bd_handle, &api_mac_entry->mac, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry update failed on device %d: "
        "network handle 0x%lx mac %s: "
        "nhop resolution failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    goto cleanup;
  }

  if (SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_DMAC_ENTRY)) {
    status = switch_pd_dmac_table_entry_update(device,
                                               handle_type,
                                               bd_handle,
                                               mac_info,
                                               intf_info->ifindex,
                                               intf_info->port_lag_index,
                                               nhop_index,
                                               mgid_index,
                                               mac_info->dmac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry update failed on device %d: "
          "network handle 0x%lx mac %s: "
          "dmac entry update failed(%s)\n",
          device,
          api_mac_entry->network_handle,
          switch_macaddress_to_string(&api_mac_entry->mac),
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (!(SWITCH_MGID_HANDLE(api_mac_entry->handle)) && bd_info->learning) {
    if (SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_SMAC_ENTRY)) {
      status = switch_pd_smac_table_entry_update(device,
                                                 bd_handle,
                                                 mac_info,
                                                 intf_info->ifindex,
                                                 mac_info->smac_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table entry update failed on device %d: "
            "network handle 0x%lx mac %s: "
            "smac entry update failed(%s)\n",
            device,
            api_mac_entry->network_handle,
            switch_macaddress_to_string(&api_mac_entry->mac),
            switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  SWITCH_LOG_DEBUG(
      "mac entry update successful on device %d "
      "network handle 0x%lx mac %s handle 0x%lx "
      "mac handle 0x%lx dmac entry 0x%lx smac entry 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      api_mac_entry->handle,
      mac_handle,
      mac_info->dmac_entry,
      mac_info->smac_entry);

  SWITCH_LOG_EXIT();

  return status;

cleanup:

  return status;
}

switch_status_t switch_api_mac_table_entries_update_internal(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *api_mac_entry) {
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  for (index = 0; index < num_entries; index++) {
    status = switch_api_mac_table_entry_update(device, &api_mac_entry[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry add failed on device %d num entries %d: ",
          "mac %d failed(%s)\n",
          device,
          num_entries,
          index,
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_mac_table_entry_delete_internal(
    switch_device_t device, switch_api_mac_entry_t *api_mac_entry) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_mac_entry_t mac_entry;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_mac_entry != NULL);
  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac delete received on device %d network handle 0x%lx "
      "mac %s handle 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      api_mac_entry->handle);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d "
        "network handle 0x%lx mac %s: "
        "l2 context get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_bd_handle_get(device, api_mac_entry->network_handle, &bd_handle);

  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d "
        "network handle 0x%lx mac %s: "
        "bd get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, &api_mac_entry->mac, sizeof(switch_mac_addr_t));
  status = switch_mac_table_entry_find(device, &mac_entry, &mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac entry not found(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_l3_vlan_interface_resolve(
      device, 0x0, bd_handle, &api_mac_entry->mac, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d: "
        "network handle 0x%lx mac %s: "
        "nhop resolution failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_NHOP_HANDLE(mac_info->l2_nhop_handle)) {
    status = switch_api_nhop_delete(device, mac_info->l2_nhop_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry delete failed on device %d: "
          "network handle 0x%lx mac %s: "
          "l2 nhop handle delete failed(%s)\n",
          device,
          api_mac_entry->network_handle,
          switch_macaddress_to_string(&api_mac_entry->mac),
          switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_SMAC_ENTRY)) {
    status = switch_pd_smac_table_entry_delete(device, mac_info->smac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry delete failed on device %d: "
          "network handle 0x%lx mac %s: "
          "smac table delete failed(%s)\n",
          device,
          api_mac_entry->network_handle,
          switch_macaddress_to_string(&api_mac_entry->mac),
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(mac_info, SWITCH_L2_PD_SMAC_ENTRY);

    SWITCH_ARRAY_DELETE(&l2_ctx->smac_pd_hdl_array, mac_info->smac_entry);
  }

  if (SWITCH_HW_FLAG_ISSET(mac_info, SWITCH_L2_PD_DMAC_ENTRY)) {
    status = switch_pd_dmac_table_entry_delete(device, mac_info->dmac_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry delete failed on device %d: "
          "network handle 0x%lx mac %s: "
          "mac get failed(%s)\n",
          device,
          api_mac_entry->network_handle,
          switch_macaddress_to_string(&api_mac_entry->mac),
          switch_error_to_string(status));
      return status;
    }

    SWITCH_HW_FLAG_CLEAR(mac_info, SWITCH_L2_PD_DMAC_ENTRY);
  }

  status = switch_mac_list_delete(device, mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_event_app_notify(
      device, api_mac_entry, SWITCH_MAC_EVENT_DELETE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete failed on device %d: "
        "mac event notify failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac entry delete successful on device %d "
      "network handle 0x%lx mac %s handle 0x%lx "
      "mac handle 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      mac_info->handle,
      mac_handle);

  status = switch_mac_handle_delete(device, mac_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_mac_table_entries_delete_internal(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *api_mac_entry) {
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  for (index = 0; index < num_entries; index++) {
    status = switch_api_mac_table_entry_delete(device, &api_mac_entry[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table entry delete failed on device %d num entries %d: ",
          "mac %d failed(%s)\n",
          device,
          num_entries,
          index,
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_mac_table_entry_delete_by_handle(
    switch_device_t device, switch_handle_t mac_handle) {
  switch_mac_info_t *mac_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_api_mac_entry_t api_mac_entry;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mac table entry delete by handle failed on device %d "
        "mac handle 0x%lx: mac handle invalid(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mac table entry delete by handle failed on device %d ",
                     "mac handle 0x%lx: mac handle invalid(%s)\n",
                     device,
                     mac_handle,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&api_mac_entry, 0x0, sizeof(switch_api_mac_entry_t));

  status = switch_bd_get(device, mac_info->mac_entry.bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mac table entry delete by handle failed on device %d ",
                     "mac handle 0x%lx: bd handle invalid(%s)\n",
                     device,
                     mac_handle,
                     switch_error_to_string(status));
    return status;
  }

  api_mac_entry.network_handle = bd_info->handle;
  SWITCH_MEMCPY(
      &api_mac_entry.mac, &mac_info->mac_entry.mac, sizeof(switch_mac_addr_t));

  status = switch_api_mac_table_entry_delete(device, &api_mac_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry delete by handle failed on device %d "
        "mac handle 0x%lx: mac table entry delete failed(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("mac table entry deleted on device %d mac handle 0x%lx\n",
                   device,
                   mac_handle);

  return status;
}

switch_status_t switch_api_multicast_l2mac_add(
    switch_device_t device, switch_api_mac_entry_t *api_mac_entry) {
  return switch_api_mac_table_entry_add(device, api_mac_entry);
}

switch_status_t switch_api_multicast_l2mac_delete(
    switch_device_t device, switch_api_mac_entry_t *api_mac_entry) {
  return switch_api_mac_table_entry_delete(device, api_mac_entry);
}

switch_status_t switch_api_mac_table_entry_flush_internal(
    switch_device_t device,
    switch_uint64_t flush_type,
    switch_handle_t network_handle,
    switch_handle_t intf_handle,
    switch_mac_entry_type_t mac_entry_type) {
  switch_interface_info_t *intf_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_handle_t *tmp_mac_handle = NULL;
  switch_array_t *mac_array = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t num_macs = 0;
  switch_mac_info_t *mac_info = NULL;

  SWITCH_LOG_ENTER();

  if (flush_type & SWITCH_MAC_FLUSH_TYPE_NETWORK) {
    SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
    if (!SWITCH_NETWORK_HANDLE(network_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mac table flush failed on device %d "
          "flush type %s network handle 0x%lx intf handle 0x%lx: "
          "network handle invalid(%s)\n",
          device,
          switch_mac_flush_type_to_string(flush_type),
          network_handle,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flush_type & SWITCH_MAC_FLUSH_TYPE_INTERFACE) {
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
    if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mac table flush failed on device %d "
          "flush type %s network handle 0x%lx intf handle 0x%lx: "
          "interface handle invalid(%s)\n",
          device,
          switch_mac_flush_type_to_string(flush_type),
          network_handle,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flush_type & SWITCH_MAC_FLUSH_TYPE_NETWORK) {
    status = switch_bd_handle_get(device, network_handle, &bd_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table flush failed on device %d "
          "flush type %s network handle 0x%lx: "
          "bd get failed(%s)\n",
          device,
          switch_mac_flush_type_to_string(flush_type),
          network_handle,
          switch_error_to_string(status));
      return status;
    }

    mac_array = &bd_info->mac_array;

  } else if (flush_type & SWITCH_MAC_FLUSH_TYPE_INTERFACE) {
    status = switch_interface_get(device, intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mac table flush failed on device %d "
          "flush type %s handle 0x%lx: "
          "interface get failed(%s)\n",
          device,
          switch_mac_flush_type_to_string(flush_type),
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    mac_array = &intf_info->mac_array;

  } else {
    FOR_EACH_HANDLE_BEGIN(device, SWITCH_HANDLE_TYPE_VLAN, vlan_handle) {
      if (SWITCH_VLAN_HANDLE(vlan_handle)) {
        flush_type |= SWITCH_MAC_FLUSH_TYPE_NETWORK;
        status = switch_api_mac_table_entry_flush(
            device, flush_type, vlan_handle, 0x0, mac_entry_type);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mac table flush failed on device %d "
              "flush type %s handle 0x%lx: "
              "interface get failed(%s)\n",
              device,
              switch_mac_flush_type_to_string(flush_type),
              intf_handle,
              switch_error_to_string(status));
          return status;
        }
      }
    }
    FOR_EACH_HANDLE_END()
  }

  if (mac_array) {
    num_macs = SWITCH_ARRAY_COUNT(mac_array);
    while (num_macs) {
      SWITCH_ARRAY_FIRST_GET(
          (*mac_array), mac_handle, switch_handle_t, tmp_mac_handle);
      UNUSED(tmp_mac_handle);
      num_macs--;

      status = switch_mac_get(device, mac_handle, &mac_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table flush failed on device %d "
            "mac handle 0x%lx: "
            "mac get failed(%s)\n",
            device,
            mac_handle,
            switch_error_to_string(status));
        return status;
      }

      if (flush_type & SWITCH_MAC_FLUSH_TYPE_MAC_TYPE) {
        if (mac_info->entry_type != mac_entry_type) {
          SWITCH_LOG_DEBUG(
              "mac entry type mismatch, "
              "don't flush handle on device %d: 0x%lx:\n",
              device,
              mac_handle);
          continue;
        }
      }

      if (flush_type & SWITCH_MAC_FLUSH_TYPE_INTERFACE) {
        if (mac_info->handle != intf_handle) {
          continue;
        }
      }

      status = switch_mac_table_entry_delete_by_handle(device, mac_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mac table flush failed on device %d "
            "flush type %s handle 0x%lx: "
            "interface get failed(%s)\n",
            device,
            switch_mac_flush_type_to_string(flush_type),
            intf_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  SWITCH_LOG_DEBUG(
      "mac table flush successful on device %d "
      "flush type %s network handle 0x%lx intf handle 0x%lx\n",
      device,
      switch_mac_flush_type_to_string(flush_type),
      network_handle,
      intf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_mac_table_set_learning_timeout_internal(
    switch_device_t device, switch_uint32_t timeout) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(timeout);

  return status;
}

switch_status_t switch_api_mac_notification_register_internal(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_uint16_t mac_event_flags,
    switch_mac_notification_fn cb_fn) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac event app register failed on device %d app id %d: "
        "l2 context get failed:(%s)\n",
        device,
        app_id,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_MAC_EVENT_REGISTRATION_MAX; index++) {
    if (l2_ctx->mac_event_list[index].valid) {
      if (l2_ctx->mac_event_list[index].app_id == app_id) {
        l2_ctx->mac_event_list[index].mac_event_flags = mac_event_flags;
        l2_ctx->mac_event_list[index].cb_fn = cb_fn;
        return status;
      }
    }
  }

  for (index = 0; index < SWITCH_MAC_EVENT_REGISTRATION_MAX; index++) {
    if (!l2_ctx->mac_event_list[index].valid) {
      l2_ctx->mac_event_list[index].mac_event_flags = mac_event_flags;
      l2_ctx->mac_event_list[index].cb_fn = cb_fn;
      l2_ctx->mac_event_list[index].valid = TRUE;
      l2_ctx->mac_event_list[index].app_id = app_id;
      return status;
    }
  }

  return SWITCH_STATUS_INSUFFICIENT_RESOURCES;
}

switch_status_t switch_api_mac_notification_deregister_internal(
    switch_device_t device, switch_app_id_t app_id) {
  switch_l2_context_t *l2_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L2, (void **)&l2_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac event app deregister failed on device %d app id %d: "
        "l2 context get failed:(%s)\n",
        device,
        app_id,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_MAC_EVENT_REGISTRATION_MAX; index++) {
    if (l2_ctx->mac_event_list[index].app_id == app_id) {
      l2_ctx->mac_event_list[index].mac_event_flags = 0;
      l2_ctx->mac_event_list[index].cb_fn = NULL;
      l2_ctx->mac_event_list[index].valid = FALSE;
      l2_ctx->mac_event_list[index].app_id = 0;
      return status;
    }
  }

  return SWITCH_STATUS_ITEM_NOT_FOUND;
}

switch_status_t switch_api_mac_handle_get_internal(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *mac_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_mac_entry_t mac_entry;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status =
      switch_bd_handle_get(device, api_mac_entry->network_handle, &bd_handle);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac handle get failed on device %d "
        "network handle 0x%lx mac %s: "
        "bd handle get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac handle get failed on device %d "
        "network handle 0x%lx mac %s: "
        "bd get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&mac_entry, 0x0, sizeof(switch_mac_entry_t));
  mac_entry.bd_handle = bd_handle;
  SWITCH_MEMCPY(&mac_entry.mac, &api_mac_entry->mac, sizeof(switch_mac_addr_t));
  status = switch_mac_table_entry_find(device, &mac_entry, mac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac handle get failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac entry not found(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mac handle get on device %d "
      "network handle 0x%lx mac %s\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      *mac_handle);

  return status;
}

switch_status_t switch_api_mac_entry_type_get_internal(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_entry_type_t *entry_type) {
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t mac_entry_handle;

  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "invalid mac entry on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_mac_handle_get(device, api_mac_entry, &mac_entry_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table handle get failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_entry_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  *entry_type = mac_info->entry_type;

  SWITCH_LOG_DEBUG(
      "mac handle get on device %d "
      "network handle 0x%lx mac %s entry type %s\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      switch_mac_entry_type_to_string(*entry_type));

  return status;
}

switch_status_t switch_api_mac_entry_port_id_get_internal(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *intf_handle) {
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t mac_entry_handle;

  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "invalid mac entry on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_mac_handle_get(device, api_mac_entry, &mac_entry_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table handle get failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_entry_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  *intf_handle = mac_info->handle;

  SWITCH_LOG_DEBUG(
      "mac handle get on device %d "
      "network handle 0x%lx mac %s intf handle 0x%lx\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      *intf_handle);
  return status;
}

switch_status_t switch_api_mac_entry_packet_action_get_internal(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_action_t *mac_action) {
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t mac_entry_handle;

  if (!api_mac_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "invalid mac entry on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_mac_handle_get(device, api_mac_entry, &mac_entry_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table handle get failed on device %d "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_entry_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac table entry get failed on device %d: "
        "network handle 0x%lx mac %s: "
        "mac get failed(%s)\n",
        device,
        api_mac_entry->network_handle,
        switch_macaddress_to_string(&api_mac_entry->mac),
        switch_error_to_string(status));
    return status;
  }

  *mac_action = mac_info->mac_action;

  SWITCH_LOG_DEBUG(
      "mac handle get on device %d "
      "network handle 0x%lx mac %s mac action %s\n",
      device,
      api_mac_entry->network_handle,
      switch_macaddress_to_string(&api_mac_entry->mac),
      switch_mac_action_to_string(*mac_action));

  return status;
}

switch_status_t switch_mac_entry_aging_hw_update(
    switch_device_t device,
    switch_handle_t mac_handle,
    switch_uint32_t aging_interval) {
  switch_mac_info_t *mac_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MAC_HANDLE(mac_handle));
  if (!SWITCH_MAC_HANDLE(mac_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mac aging time hw update on device %d mac handle 0x%lx: "
        "mac handle invalid:(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mac_get(device, mac_handle, &mac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging time hw update on device %d mac handle 0x%lx: "
        "mac get failed:(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  if (mac_info->entry_type != SWITCH_MAC_ENTRY_DYNAMIC) {
    return status;
  }

  status = switch_pd_mac_entry_aging_time_set(
      device, mac_info->smac_entry, aging_interval);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mac aging time hw update on device %d mac handle 0x%lx: "
        "mac hw aging time set failed:(%s)\n",
        device,
        mac_handle,
        switch_error_to_string(status));
    return status;
  }

  mac_info->aging_interval = aging_interval;

  SWITCH_LOG_DETAIL(
      "mac aging time set on device %d mac handle 0x%lx aging %d\n",
      device,
      mac_handle,
      aging_interval);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_mac_table_set_learning_timeout(
    switch_device_t device, uint32_t timeout) {
  SWITCH_MT_WRAP(
      switch_api_mac_table_set_learning_timeout_internal(device, timeout))
}

switch_status_t switch_api_mac_table_entries_add(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries) {
  SWITCH_MT_WRAP(switch_api_mac_table_entries_add_internal(
      device, num_entries, mac_entries))
}

switch_status_t switch_api_mac_table_entry_add(
    switch_device_t device, switch_api_mac_entry_t *mac_entry) {
  SWITCH_MT_WRAP(switch_api_mac_table_entry_add_internal(device, mac_entry))
}

switch_status_t switch_api_mac_entry_handle_get(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *mac_handle) {
  SWITCH_MT_WRAP(switch_api_mac_entry_handle_get_internal(
      device, api_mac_entry, mac_handle))
}

switch_status_t switch_api_mac_table_entries_update(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries) {
  SWITCH_MT_WRAP(switch_api_mac_table_entries_update_internal(
      device, num_entries, mac_entries))
}

switch_status_t switch_api_mac_table_entry_update(
    switch_device_t device, switch_api_mac_entry_t *mac_entry) {
  SWITCH_MT_WRAP(switch_api_mac_table_entry_update_internal(device, mac_entry))
}

switch_status_t switch_api_mac_table_entry_delete(
    switch_device_t device, switch_api_mac_entry_t *mac_entry) {
  SWITCH_MT_WRAP(switch_api_mac_table_entry_delete_internal(device, mac_entry))
}

switch_status_t switch_api_mac_table_entries_delete(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries) {
  SWITCH_MT_WRAP(switch_api_mac_table_entries_delete_internal(
      device, num_entries, mac_entries))
}

switch_status_t switch_api_mac_table_entry_flush(
    switch_device_t device,
    switch_uint64_t flush_type,
    switch_handle_t network_handle,
    switch_handle_t intf_handle,
    switch_mac_entry_type_t mac_entry_type) {
  SWITCH_MT_WRAP(switch_api_mac_table_entry_flush_internal(
      device, flush_type, network_handle, intf_handle, mac_entry_type))
}

switch_status_t switch_api_mac_notification_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_uint16_t mac_event_flags,
    switch_mac_notification_fn cb_fn) {
  SWITCH_MT_WRAP(switch_api_mac_notification_register_internal(
      device, app_id, mac_event_flags, cb_fn));
}

switch_status_t switch_api_mac_notification_deregister(switch_device_t device,
                                                       switch_app_id_t app_id) {
  SWITCH_MT_WRAP(
      switch_api_mac_notification_deregister_internal(device, app_id));
}

switch_status_t switch_api_mac_entry_type_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_entry_type_t *entry_type) {
  SWITCH_MT_WRAP(
      switch_api_mac_entry_type_get_internal(device, api_mac_entry, entry_type))
}

switch_status_t switch_api_mac_entry_port_id_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(switch_api_mac_entry_port_id_get_internal(
      device, api_mac_entry, intf_handle))
}

switch_status_t switch_api_mac_entry_packet_action_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_action_t *mac_action) {
  SWITCH_MT_WRAP(switch_api_mac_entry_packet_action_get_internal(
      device, api_mac_entry, mac_action))
}

switch_status_t switch_api_mac_handle_get(switch_device_t device,
                                          switch_api_mac_entry_t *api_mac_entry,
                                          switch_handle_t *mac_handle) {
  SWITCH_MT_WRAP(
      switch_api_mac_handle_get_internal(device, api_mac_entry, mac_handle))
}
