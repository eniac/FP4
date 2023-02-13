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

#include "switchapi/switch_nat.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_NAT

#define SWITCH_NAT_HASH_TABLE_SIZE 1024
#define SWITCH_NAT_REWRITE_TABLE_SIZE (16 * 1024)

switch_status_t switch_nat_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_nat_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat default entry add failed on device %d: "
        "pd nat init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_nat_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_nat_hashtable_key_init(void *args,
                                              switch_uint8_t *key,
                                              switch_uint32_t *len) {
  switch_api_nat_info_t *api_nat_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  SWITCH_MEMSET(key, 0, SWITCH_NAT_HASH_KEY_SIZE);
  api_nat_info = (switch_api_nat_info_t *)args;

  SWITCH_MEMCPY(key, &api_nat_info->vrf_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  if (SWITCH_NAT_TYPE_IS_VALID_SRC(api_nat_info)) {
    SWITCH_MEMCPY(&key[*len],
                  &(SWITCH_NAT_SRC_IP(api_nat_info)),
                  sizeof(switch_uint32_t));
  }
  *len += sizeof(switch_uint32_t);

  if (SWITCH_NAT_TYPE_IS_VALID_SRC_PORT(api_nat_info)) {
    SWITCH_MEMCPY(&key[*len], &api_nat_info->src_port, sizeof(switch_uint16_t));
  }
  *len += sizeof(switch_uint16_t);

  if (SWITCH_NAT_TYPE_IS_VALID_DST(api_nat_info)) {
    SWITCH_MEMCPY(&key[*len],
                  &(SWITCH_NAT_DST_IP(api_nat_info)),
                  sizeof(switch_uint32_t));
  }
  *len += sizeof(switch_uint32_t);

  if (SWITCH_NAT_TYPE_IS_VALID_DST_PORT(api_nat_info)) {
    SWITCH_MEMCPY(&key[*len], &api_nat_info->dst_port, sizeof(switch_uint16_t));
  }
  *len += sizeof(switch_uint16_t);

  if (SWITCH_NAT_TYPE_IS_VALID_SRC_PORT(api_nat_info) ||
      SWITCH_NAT_TYPE_IS_VALID_DST_PORT(api_nat_info)) {
    SWITCH_MEMCPY(&key[*len], &api_nat_info->protocol, 2);
  }
  *len += 2;

  SWITCH_MEMCPY(&key[*len], &api_nat_info->nat_rw_type, 1);
  *len += 1;

  return status;
}

switch_int32_t switch_nat_hash_compare(const void *key1, const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_NAT_HASH_KEY_SIZE);
}

switch_status_t switch_nat_init(switch_device_t device) {
  switch_nat_context_t *nat_ctx = NULL;
  switch_size_t nat_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  nat_ctx = SWITCH_MALLOC(device, sizeof(switch_nat_context_t), 0x1);
  if (!nat_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "nat init failed on device %d: "
        "nat context memory alloc failed((%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(nat_ctx, 0x0, sizeof(switch_nat_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_NAT, (void *)nat_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat init failed on device %d: "
        "nat context set failed((%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_table_size_get(device, SWITCH_TABLE_NAT_DST, &nat_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat init failed on device %d: "
        "nat table size get failed((%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  nat_ctx->nat_hashtable.size = nat_table_size;
  nat_ctx->nat_hashtable.compare_func = switch_nat_hash_compare;
  nat_ctx->nat_hashtable.key_func = switch_nat_hashtable_key_init;
  nat_ctx->nat_hashtable.hash_seed = SWITCH_NAT_HASH_SEED;

  SWITCH_ASSERT(nat_table_size != 0);
  status = SWITCH_HASHTABLE_INIT(&nat_ctx->nat_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat init failed on device %d: "
        "nat hashtable init failed((%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_id_allocator_new(
      device, SWITCH_NAT_REWRITE_TABLE_SIZE, FALSE, &nat_ctx->nat_rw_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat init failed on device %d: "
        "nat id allcator failed((%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_EXIT();

  return status;

cleanup:

  SWITCH_FREE(device, nat_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_NAT, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_nat_free(switch_device_t device) {
  switch_nat_context_t *nat_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NAT, (void **)&nat_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat free failed on device %d: "
        "nat device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DONE(&nat_ctx->nat_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat free failed on device %d: "
        "nat hashtable free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, nat_ctx->nat_rw_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat free failed on device %d: "
        "nat id allocator free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_nat_add_internal(
    switch_device_t device, switch_api_nat_info_t *api_nat_info) {
  switch_nat_context_t *nat_ctx = NULL;
  switch_nat_info_t *nat_info = NULL;
  switch_id_t rw_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NAT, (void **)&nat_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &nat_ctx->nat_hashtable, (void *)api_nat_info, (void **)&nat_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat hash entry exists(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  nat_info = SWITCH_MALLOC(device, sizeof(switch_nat_info_t), 0x1);
  if (!nat_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat memory alloc failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_allocate(
      device, nat_ctx->nat_rw_allocator, &rw_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat index allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMCPY(
      &nat_info->api_nat_info, api_nat_info, sizeof(switch_api_nat_info_t));
  nat_info->rw_index = rw_index;

  status = switch_pd_nat_table_entry_add(device, nat_info, &nat_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat pd entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_nat_rewrite_table_entry_add(
      device, nat_info, &nat_info->rw_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat pd rewrite entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_INSERT(&nat_ctx->nat_hashtable,
                                   &nat_info->node,
                                   (void *)api_nat_info,
                                   (void *)nat_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat add failed on device %d: "
        "nat hashtable insert failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_nat_delete_internal(
    switch_device_t device, switch_api_nat_info_t *api_nat_info) {
  switch_nat_context_t *nat_ctx = NULL;
  switch_nat_info_t *nat_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NAT, (void **)&nat_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &nat_ctx->nat_hashtable, (void *)api_nat_info, (void **)&nat_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat hashtable entry not found(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_nat_table_entry_delete(device, nat_info, nat_info->hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat pd entry delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_nat_rewrite_table_entry_delete(device, nat_info->rw_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat pd rewrite entry delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_release(
      device, nat_ctx->nat_rw_allocator, nat_info->rw_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat index allocation free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(
      &nat_ctx->nat_hashtable, (void *)api_nat_info, (void *)&nat_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nat delete failed on device %d: "
        "nat hashtable delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, nat_info);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_nat_delete(switch_device_t device,
                                      switch_api_nat_info_t *api_nat_info) {
  SWITCH_MT_WRAP(switch_api_nat_delete_internal(device, api_nat_info))
}

switch_status_t switch_api_nat_add(switch_device_t device,
                                   switch_api_nat_info_t *api_nat_info) {
  SWITCH_MT_WRAP(switch_api_nat_add_internal(device, api_nat_info))
}
