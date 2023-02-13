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

#include "switchapi/switch_ila.h"
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_hashtable_t ila_hashtable;

switch_status_t switch_ila_table_entry_key_init(void *args,
                                                switch_uint8_t *key,
                                                switch_uint32_t *len) {
  switch_api_ila_info_t *api_ila_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(args && key && len);
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila hash entry key init failed : %s",
                     switch_error_to_string(status));
    return status;
  }

  *len = 0;
  api_ila_info = (switch_api_ila_info_t *)args;

  SWITCH_MEMCPY(key, &api_ila_info->vrf_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY(
      (key + *len), &api_ila_info->sir_addr, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_ASSERT(*len == SWITCH_ILA_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_ila_entry_hash_compare(const void *key1,
                                             const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_ILA_HASH_KEY_SIZE);
}

switch_status_t switch_ila_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  SWITCH_LOG_ENTER();

  ila_hashtable.size = SWITCH_ILA_HASH_TABLE_SIZE;
  ila_hashtable.compare_func = switch_ila_entry_hash_compare;
  ila_hashtable.key_func = switch_ila_table_entry_key_init;
  ila_hashtable.hash_seed = SWITCH_ILA_HASH_SEED;
  SWITCH_HASHTABLE_INIT(&ila_hashtable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_ila_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  SWITCH_LOG_ENTER();

  SWITCH_HASHTABLE_DONE(&ila_hashtable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_ila_update_internal(
    switch_device_t device,
    switch_api_ila_info_t *api_ila_info,
    switch_ip_addr_t ila_addr,
    switch_handle_t nhop_handle) {
  switch_ila_info_t *ila_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_ila_info != NULL);
  if (!api_ila_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_ila_info->vrf_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_VRF_HANDLE(api_ila_info->vrf_handle) ||
      !SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &ila_hashtable, (void *)api_ila_info, (void **)&ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ila_info->ecmp = SWITCH_NHOP_ID_TYPE_ECMP(nhop_info);
  ila_info->nhop_handle = nhop_handle;
  SWITCH_MEMCPY(&ila_info->ila_addr, &ila_addr, sizeof(switch_ip_addr_t));

  status = switch_pd_ila_table_entry_update(device, ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_ila_add_internal(switch_device_t device,
                                            switch_api_ila_info_t *api_ila_info,
                                            switch_ip_addr_t ila_addr,
                                            switch_handle_t nhop_handle) {
  switch_ila_info_t *ila_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_ila_info != NULL);
  if (!api_ila_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_ila_info->vrf_handle));
  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_VRF_HANDLE(api_ila_info->vrf_handle) ||
      !SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &ila_hashtable, (void *)api_ila_info, (void **)&ila_info);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ila_info = SWITCH_MALLOC(device, sizeof(switch_ila_info_t), 0x1);
  if (!ila_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ila_info->ecmp = SWITCH_NHOP_ID_TYPE_ECMP(nhop_info);
  ila_info->nhop_handle = nhop_handle;
  SWITCH_MEMCPY(
      &ila_info->api_ila_info, api_ila_info, sizeof(switch_api_ila_info_t));
  SWITCH_MEMCPY(&ila_info->ila_addr, &ila_addr, sizeof(switch_ip_addr_t));

  status = switch_pd_ila_table_entry_add(device, ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_INSERT(
      &ila_hashtable, &ila_info->node, (void *)api_ila_info, (void *)ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_ila_delete_internal(
    switch_device_t device, switch_api_ila_info_t *api_ila_info) {
  switch_ila_info_t *ila_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_ila_info != NULL);
  if (!api_ila_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_ila_info->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_ila_info->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &ila_hashtable, (void *)api_ila_info, (void **)&ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ila_table_entry_delete(device, ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(
      &ila_hashtable, (void *)&ila_info->api_ila_info, (void **)&ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, ila_info);

  return status;
}

switch_status_t switch_api_ila_get_internal(switch_device_t device,
                                            switch_api_ila_info_t *api_ila_info,
                                            switch_ip_addr_t *ila_addr,
                                            switch_handle_t *nhop_handle) {
  switch_ila_info_t *ila_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_ila_info != NULL && ila_addr != NULL &&
                nhop_handle != NULL);
  if (!api_ila_info || !ila_addr || !nhop_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_ila_info->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_ila_info->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *nhop_handle = SWITCH_API_INVALID_HANDLE;
  status = SWITCH_HASHTABLE_SEARCH(
      &ila_hashtable, (void *)api_ila_info, (void **)&ila_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ila entry delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(ila_addr, &ila_info->ila_addr, sizeof(switch_ip_addr_t));
  *nhop_handle = ila_info->nhop_handle;

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_ila_delete(switch_device_t device,
                                      switch_api_ila_info_t *api_ila_info) {
  SWITCH_MT_WRAP(switch_api_ila_delete_internal(device, api_ila_info))
}

switch_status_t switch_api_ila_update(switch_device_t device,
                                      switch_api_ila_info_t *api_ila_info,
                                      switch_ip_addr_t ila_addr,
                                      switch_handle_t nhop_handle) {
  SWITCH_MT_WRAP(switch_api_ila_update_internal(
      device, api_ila_info, ila_addr, nhop_handle))
}

switch_status_t switch_api_ila_get(switch_device_t device,
                                   switch_api_ila_info_t *api_ila_info,
                                   switch_ip_addr_t *ila_addr,
                                   switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_ila_get_internal(device, api_ila_info, ila_addr, nhop_handle))
}

switch_status_t switch_api_ila_add(switch_device_t device,
                                   switch_api_ila_info_t *api_ila_info,
                                   switch_ip_addr_t ila_addr,
                                   switch_handle_t nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_ila_add_internal(device, api_ila_info, ila_addr, nhop_handle))
}
