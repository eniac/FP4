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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_L3

switch_status_t switch_route_table_hash_lookup(
    switch_device_t device,
    switch_route_entry_t *route_entry,
    switch_handle_t *route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_route_info_t *route_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(route_entry != NULL);
  if (!route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "route table entry find failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table entry find failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(
      &l3_ctx->route_hashtable, (void *)route_entry, (void **)&route_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    *route_handle = route_info->route_handle;
  }

  return status;
}

switch_status_t switch_route_lpm_trie_insert(switch_device_t device,
                                             switch_handle_t route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_lpm_trie_t *lpm_trie = NULL;
  switch_uint8_t *prefix = NULL;
  switch_route_info_t *route_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t v4addr = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route trie insert failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie insert failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie insert failed on device %d "
        "route handle 0x%lx: l3 device context get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;
  vrf_handle = route_entry->vrf_handle;
  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie insert failed on device %d "
        "route handle 0x%lx: vrf get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  if (route_entry->ip.type == SWITCH_API_IP_ADDR_V4) {
    lpm_trie = vrf_info->ipv4_lpm_trie;
    v4addr = htonl(route_entry->ip.ip.v4addr);
    prefix = (switch_uint8_t *)(&v4addr);
  } else {
    lpm_trie = vrf_info->ipv6_lpm_trie;
    prefix = (switch_uint8_t *)(route_entry->ip.ip.v6addr.u.addr8);
  }

  status = switch_lpm_trie_insert(device,
                                  lpm_trie,
                                  prefix,
                                  route_entry->ip.prefix_len,
                                  (switch_uint64_t)(route_handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie insert failed on device %d "
        "route handle 0x%lx: lpm trie insert failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_route_lpm_trie_remove(switch_device_t device,
                                             switch_handle_t route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_lpm_trie_t *lpm_trie = NULL;
  switch_uint8_t *prefix = NULL;
  switch_route_info_t *route_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t v4addr = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route trie delete failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie delete failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie delete failed on device %d "
        "route handle 0x%lx: l3 context get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;
  vrf_handle = route_entry->vrf_handle;
  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie delete failed on device %d "
        "route handle 0x%lx: vrf get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  if (route_entry->ip.type == SWITCH_API_IP_ADDR_V4) {
    lpm_trie = vrf_info->ipv4_lpm_trie;
    v4addr = htonl(route_entry->ip.ip.v4addr);
    prefix = (switch_uint8_t *)(&v4addr);
  } else {
    lpm_trie = vrf_info->ipv6_lpm_trie;
    prefix = (switch_uint8_t *)(route_entry->ip.ip.v6addr.u.addr8);
  }

  status = switch_lpm_trie_delete(
      device, lpm_trie, prefix, route_entry->ip.prefix_len);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie delete failed on device %d "
        "route handle 0x%lx: lpm trie delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_l3_lpm_trie_lookup(switch_device_t device,
                                          switch_route_entry_t *route_entry,
                                          switch_handle_t *route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_lpm_trie_t *lpm_trie = NULL;
  switch_uint8_t *prefix = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t v4addr = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(route_entry != NULL);
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie lookup failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  vrf_handle = route_entry->vrf_handle;
  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route trie lookup failed on device %d: "
        "vrf get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (route_entry->ip.type == SWITCH_API_IP_ADDR_V4) {
    lpm_trie = vrf_info->ipv4_lpm_trie;
    v4addr = htonl(route_entry->ip.ip.v4addr);
    prefix = (switch_uint8_t *)(&v4addr);
  } else {
    lpm_trie = vrf_info->ipv6_lpm_trie;
    prefix = (switch_uint8_t *)(route_entry->ip.ip.v6addr.u.addr8);
  }

  status = switch_lpm_trie_lookup(
      lpm_trie, prefix, (switch_uint64_t *)(route_handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "route trie lookup failed on device %d: "
        "lpm trie lookup failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_l3_route_lookup_internal(
    switch_device_t device,
    switch_api_route_entry_t *api_route_entry,
    switch_handle_t *nhop_handle) {
  switch_route_info_t *route_info = NULL;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_route_entry != NULL);
  SWITCH_ASSERT(nhop_handle != NULL);
  if (!api_route_entry || !nhop_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route lookup failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route lookup failed on device %d "
        "vrf handle 0x%lx: vrf handle invalid(%s)\n",
        device,
        api_route_entry->vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_l3_route_handle_lookup(device, api_route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "l3 route lookup failed on device %d "
        "vrf handle 0x%lx ip address %s: "
        "lpm trie lookup failed(%s)\n",
        device,
        api_route_entry->vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route lookup failed on device %d "
        "vrf handle 0x%lx ip address %s: "
        "lpm trie lookup failed(%s)\n",
        device,
        api_route_entry->vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  *nhop_handle = route_info->nhop_handle;

  return status;
}

switch_status_t switch_api_l3_route_handle_lookup_internal(
    const switch_device_t device,
    const switch_api_route_entry_t *api_route_entry,
    switch_handle_t *route_handle) {
  switch_route_entry_t route_entry;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_route_entry != NULL);
  SWITCH_ASSERT(route_handle != NULL);
  if (!api_route_entry || !route_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route lookup failed on device %d: "
        "parametsrs invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route lookup failed on device %d "
        "vrf handle 0x%lx: vrf handle invalid(%s)\n",
        device,
        api_route_entry->vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
  route_entry.vrf_handle = api_route_entry->vrf_handle;
  route_entry.ip = api_route_entry->ip_address;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  route_entry.neighbor_installed = api_route_entry->neighbor_installed;
#else
  route_entry.neighbor_installed = FALSE;
#endif

  if (switch_l3_host_entry(&api_route_entry->ip_address)) {
    status = switch_route_table_hash_lookup(device, &route_entry, route_handle);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_ITEM_NOT_FOUND) {
      SWITCH_LOG_DEBUG(
          "l3 route lookup failed on device %d "
          "vrf handle 0x%lx ip address %s: "
          "hash lookup failed(%s)\n",
          device,
          api_route_entry->vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          switch_error_to_string(status));
      return status;
    }

    if (status == SWITCH_STATUS_SUCCESS) {
      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(*route_handle));
      SWITCH_LOG_DETAIL("l3 route lookup on device %d route handle 0x%lx\n",
                        device,
                        *route_handle);
      return status;
    }
  }

  status = switch_l3_lpm_trie_lookup(device, &route_entry, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "l3 route lookup failed on device %d "
        "vrf handle 0x%lx ip address %s: "
        "lpm trie lookup failed(%s)\n",
        device,
        api_route_entry->vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(*route_handle));
  SWITCH_LOG_DETAIL(
      "l3 route lookup on device %d route handle 0x%lx", device, *route_handle);
  return status;
}

switch_int32_t switch_mtu_table_entry_hash_compare(const void *key1,
                                                   const void *key2) {
  return SWITCH_MEMCMP(key1, key2, sizeof(switch_mtu_t));
}

switch_status_t switch_mtu_table_entry_key_init(void *args,
                                                switch_uint8_t *key,
                                                switch_uint32_t *len) {
  switch_mtu_t *switch_mtu = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  switch_mtu = (switch_mtu_t *)args;
  SWITCH_MEMCPY(key, switch_mtu, sizeof(switch_mtu_t));
  *len = sizeof(switch_mtu_t);

  SWITCH_ASSERT(*len == sizeof(switch_mtu_t));

  return status;
}

switch_status_t switch_route_table_entry_key_init(void *args,
                                                  switch_uint8_t *key,
                                                  switch_uint32_t *len) {
  switch_route_entry_t *route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  route_entry = (switch_route_entry_t *)args;

  SWITCH_MEMCPY(key, &route_entry->vrf_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY((key + *len), &route_entry->ip, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_MEMCPY((key + *len), &route_entry->neighbor_installed, sizeof(bool));
  *len += sizeof(bool);

  SWITCH_ASSERT(*len == SWITCH_ROUTE_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_route_entry_hash_compare(const void *key1,
                                               const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_ROUTE_HASH_KEY_SIZE);
}

switch_status_t switch_l3_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_validate_outer_ip_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ip_fib_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ip_urpf_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_l3_rewrite_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_l3_rewrite_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mtu_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_l3_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  return status;
}

switch_status_t switch_l3_route_table_size_get(
    switch_device_t device, switch_size_t *route_table_size) {
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_table_id_t table_id = 0;

  SWITCH_ASSERT(route_table_size != NULL);

  *route_table_size = 0;

  for (table_id = SWITCH_TABLE_IPV4_HOST; table_id <= SWITCH_TABLE_IPV6_LPM;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *route_table_size = 0;
      SWITCH_LOG_ERROR(
          "route table size get failed on device %d: %s"
          "for table %s",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }
    *route_table_size += table_size;
  }
  return status;
}

switch_status_t switch_api_route_table_size_get_internal(
    switch_device_t device, switch_size_t *route_table_size) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("L3 context get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }
  *route_table_size = l3_ctx->route_hashtable.size;
  return status;
}

switch_status_t switch_l3_init(switch_device_t device) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_size_t route_table_size = 0;
  switch_size_t mtu_table_size = 0;
  switch_size_t urpf_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  l3_ctx = SWITCH_MALLOC(device, sizeof(switch_l3_context_t), 0x1);
  if (!l3_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d "
        "l3 device context memoary allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_L3, (void *)l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d "
        "l3 context set failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_l3_route_table_size_get(device, &route_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d "
        "l3 route table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(device, SWITCH_TABLE_MTU, &mtu_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d "
        "l3 route table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(mtu_table_size != 0);

  l3_ctx->mtu_hashtable.size = mtu_table_size;
  l3_ctx->mtu_hashtable.compare_func = switch_mtu_table_entry_hash_compare;
  l3_ctx->mtu_hashtable.key_func = switch_mtu_table_entry_key_init;
  l3_ctx->mtu_hashtable.hash_seed = SWITCH_ROUTE_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&l3_ctx->mtu_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "l3 mtu hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  l3_ctx->route_hashtable.size = route_table_size;
  l3_ctx->route_hashtable.compare_func = switch_route_entry_hash_compare;
  l3_ctx->route_hashtable.key_func = switch_route_table_entry_key_init;
  l3_ctx->route_hashtable.hash_seed = SWITCH_ROUTE_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&l3_ctx->route_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "l3 hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_table_size_get(device, SWITCH_TABLE_URPF, &urpf_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "urpf table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_URPF, urpf_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "urpf handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_ROUTE
   */
  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_ROUTE, route_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "route handle init failed (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_MTU, mtu_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 init failed on device %d: "
        "mtu handle init failed (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("l3 init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_l3_free(switch_device_t device) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&l3_ctx->mtu_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "l3 mtu hashtable done failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&l3_ctx->route_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "l3 hashtable done failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_URPF);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "urpf handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ROUTE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "route handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MTU);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 free failed on device %d: "
        "mtu handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, l3_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_L3, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("l3 free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_route_vrf_list_insert(switch_device_t device,
                                             switch_handle_t route_handle) {
  switch_route_info_t *route_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_list_t *vrf_list = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route vrf list insert failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list insert failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route vrf list insert failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, route_entry->vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list insert failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  if (route_entry->ip.type == SWITCH_API_IP_ADDR_V4) {
    vrf_list = &vrf_info->ipv4_routes;
  } else {
    vrf_list = &vrf_info->ipv6_routes;
  }

  status =
      SWITCH_LIST_INSERT(vrf_list, &route_info->vrf_node, (void *)route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list insert failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle,
        switch_error_to_string(status));

    return status;
  }

  SWITCH_LOG_DETAIL(
      "vrf list added on device %d "
      "vrf handle 0x%lx route handle 0x%lx\n",
      device,
      route_entry->vrf_handle,
      route_handle);

  return status;
}

switch_status_t switch_route_vrf_list_remove(switch_device_t device,
                                             switch_handle_t route_handle) {
  switch_route_info_t *route_info = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_list_t *vrf_list = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route vrf list delete failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle);
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list delete failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle);
    return status;
  }

  route_entry = &route_info->route_entry;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route vrf list delete failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle);
    return status;
  }

  switch_vrf_get(device, route_entry->vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list delete failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle);
    return status;
  }

  if (route_entry->ip.type == SWITCH_API_IP_ADDR_V4) {
    vrf_list = &vrf_info->ipv4_routes;
  } else {
    vrf_list = &vrf_info->ipv6_routes;
  }

  status = SWITCH_LIST_DELETE(vrf_list, &route_info->vrf_node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route vrf list delete failed on device %d "
        "vrf handle 0x%lx route handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        route_entry->vrf_handle,
        route_handle);
    return status;
  }

  SWITCH_LOG_DETAIL(
      "vrf list deleted on device %d "
      "vrf handle 0x%lx route handle 0x%lx\n",
      device,
      route_entry->vrf_handle,
      route_handle);

  return status;
}

switch_status_t switch_route_hashtable_insert(switch_device_t device,
                                              switch_handle_t route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_route_info_t *route_info = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: l3 context get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_INSERT(&l3_ctx->route_hashtable,
                                   &((route_info)->node),
                                   (void *)route_entry,
                                   (void *)(route_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: hashtable insert failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_route_hashtable_remove(switch_device_t device,
                                              switch_handle_t route_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_route_info_t *route_info = NULL;
  switch_route_entry_t *route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: l3 context get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(
      &l3_ctx->route_hashtable, (void *)route_entry, (void **)&route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: l3 hashtable delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

static inline bool switch_api_l3_route_is_host_route(
    switch_api_route_entry_t *api_route_entry) {
  if (api_route_entry->ip_address.type == SWITCH_API_IP_ADDR_V4) {
    if (api_route_entry->ip_address.prefix_len ==
        SWITCH_IPV4_PREFIX_LENGTH_IN_BITS) {
      return (TRUE);
    }
  } else {
    if (api_route_entry->ip_address.prefix_len ==
        SWITCH_IPV6_PREFIX_LENGTH_IN_BITS) {
      return (TRUE);
    }
  }

  return (FALSE);
}

switch_status_t switch_api_l3_interface_address_add_internal(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  switch_interface_ip_addr_t *ip_addr_info = NULL;
  switch_node_t *node = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 interface address add failed on device %d:  "
        "api route entry null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 interface address add failed on device %d:  "
        "vrf handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (api_route_entry->route_type != SWITCH_ROUTE_TYPE_MYIP) {
    status = switch_rif_get(device, api_route_entry->rif_handle, &rif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 interface address add failed on device %d:  "
          "interface get failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    FOR_EACH_IN_LIST(rif_info->ip_list, node) {
      ip_addr_info = (switch_interface_ip_addr_t *)node->data;
      if (!SWITCH_MEMCMP(&ip_addr_info->ip_address,
                         &api_route_entry->ip_address,
                         sizeof(switch_ip_addr_t))) {
        status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
        SWITCH_LOG_ERROR(
            "l3 interface address add failed on device %d:  "
            "interface ip already exists(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
    FOR_EACH_IN_LIST_END();

    ip_addr_info = SWITCH_MALLOC(device, sizeof(switch_interface_ip_addr_t), 1);
    if (!ip_addr_info) {
      SWITCH_LOG_ERROR(
          "l3 interface address add failed on device %d:  "
          "interface ip address memory allocation failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    ip_addr_info->vrf_handle = api_route_entry->vrf_handle;

    SWITCH_ASSERT(api_route_entry->vrf_handle ==
                  rif_info->api_rif_info.vrf_handle);
    SWITCH_MEMCPY(&ip_addr_info->ip_address,
                  &api_route_entry->ip_address,
                  sizeof(switch_ip_addr_t));
    ip_addr_info->primary = FALSE;
    SWITCH_LIST_INSERT(&rif_info->ip_list, &ip_addr_info->node, ip_addr_info);

    if (!switch_api_l3_route_is_host_route(api_route_entry)) {
      status = switch_api_hostif_nhop_get(
          device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 interface address add failed on device %d:  "
            "glean nhop get failed(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      api_route_entry->nhop_handle = nhop_handle;
      api_route_entry->neighbor_installed = FALSE;
      status = switch_api_l3_route_add(device, api_route_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 interface address add failed on device %d:  "
            "glean route add failed(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_MYIP, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address add failed on device %d:  "
        "myip nhop get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  api_route_entry->nhop_handle = nhop_handle;
  api_route_entry->route_type = SWITCH_ROUTE_TYPE_MYIP;
  if (api_route_entry->ip_address.type == SWITCH_API_IP_ADDR_V4) {
    api_route_entry->ip_address.prefix_len = SWITCH_IPV4_PREFIX_LENGTH_IN_BITS;
  } else {
    api_route_entry->ip_address.prefix_len = SWITCH_IPV6_PREFIX_LENGTH_IN_BITS;
  }
  status = switch_api_l3_route_add(device, api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address add failed on device %d:  "
        "myiproute add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (ip_addr_info == NULL) {
    ip_addr_info = SWITCH_MALLOC(device, sizeof(switch_interface_ip_addr_t), 1);
    if (!ip_addr_info) {
      SWITCH_LOG_ERROR(
          "l3 interface address add failed on device %d:  "
          "interface ip address memory allocation failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    ip_addr_info->vrf_handle = api_route_entry->vrf_handle;

    SWITCH_MEMCPY(&ip_addr_info->ip_address,
                  &api_route_entry->ip_address,
                  sizeof(switch_ip_addr_t));
    ip_addr_info->primary = FALSE;
  }
  if (SWITCH_L3_IP_TYPE(ip_addr_info->ip_address) == SWITCH_API_IP_ADDR_V6) {
    status = switch_sr_endpoint_add(device, ip_addr_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 interface address add failed on device %d:  "
          "ipv6 sr endpoint add failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "l3 interface address add on device %d "
      "rif handle 0x%lx vrf handle 0x%lx ip address %s\n",
      device,
      api_route_entry->rif_handle,
      api_route_entry->vrf_handle,
      switch_ipaddress_to_string(&api_route_entry->ip_address));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_interface_address_delete_internal(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  switch_rif_info_t *rif_info = NULL;
  switch_interface_ip_addr_t *ip_addr_info = NULL;
  switch_node_t *node = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_route_entry != NULL);
  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "parameter null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_route_entry->vrf_handle));
  if (!SWITCH_VRF_HANDLE(api_route_entry->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "vrf handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, api_route_entry->rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "interface get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  FOR_EACH_IN_LIST(rif_info->ip_list, node) {
    ip_addr_info = (switch_interface_ip_addr_t *)node->data;
    if (!SWITCH_MEMCMP(&ip_addr_info->ip_address,
                       &api_route_entry->ip_address,
                       sizeof(switch_ip_addr_t))) {
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "interface ip address node not found(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LIST_DELETE(&rif_info->ip_list, &ip_addr_info->node);

  if (!switch_api_l3_route_is_host_route(api_route_entry)) {
    status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 interface address delete failed on device %d: "
          "glean nhop get failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    api_route_entry->nhop_handle = nhop_handle;
    api_route_entry->neighbor_installed = FALSE;
    status = switch_api_l3_route_delete(device, api_route_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 interface address delete failed on device %d: "
          "glean route delete failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_MYIP, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "myip nhop get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  api_route_entry->nhop_handle = nhop_handle;
  api_route_entry->route_type = SWITCH_ROUTE_TYPE_MYIP;
  if (api_route_entry->ip_address.type == SWITCH_API_IP_ADDR_V4) {
    api_route_entry->ip_address.prefix_len = SWITCH_IPV4_PREFIX_LENGTH_IN_BITS;
  } else {
    api_route_entry->ip_address.prefix_len = SWITCH_IPV6_PREFIX_LENGTH_IN_BITS;
  }

  status = switch_api_l3_route_delete(device, api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 interface address delete failed on device %d: "
        "myiproute delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_L3_IP_TYPE(ip_addr_info->ip_address) == SWITCH_API_IP_ADDR_V6) {
    status = switch_sr_endpoint_delete(device, ip_addr_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 interface address delete failed on device %d: "
          "ipv6 sr endpoint delete failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "l3 interface address delete on device %d "
      "rif handle 0x%lx vrf handle 0x%lx ip address %s\n",
      device,
      api_route_entry->rif_handle,
      api_route_entry->vrf_handle,
      switch_ipaddress_to_string(&api_route_entry->ip_address));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_route_table_insert(switch_device_t device,
                                          switch_handle_t route_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route table insert failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_hashtable_insert(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table insert failed on device %d "
        "route handle 0x%lx: "
        "route hashtable insert failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_lpm_trie_insert(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table insert failed on device %d "
        "route handle 0x%lx: "
        "route lpm insert failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_vrf_list_insert(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table insert failed on device %d "
        "route handle 0x%lx: "
        "route lpm insert failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("route table inserted on device %d route handle 0x%lx\n",
                    device,
                    route_handle);

  return status;
}

switch_status_t switch_route_table_delete(switch_device_t device,
                                          switch_handle_t route_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "route table delete failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_hashtable_remove(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table delete failed on device %d "
        "route handle 0x%lx: "
        "route hashtable delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_lpm_trie_remove(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table delete failed on device %d "
        "route handle 0x%lx: "
        "route lpm delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_vrf_list_remove(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "route table delete failed on device %d "
        "route handle 0x%lx: "
        "route lpm delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("route table deleted on device %d route handle 0x%lx\n",
                    device,
                    route_handle);

  return status;
}

switch_status_t switch_api_l3_route_add_internal(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  switch_route_info_t *route_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_route_entry_t route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  vrf_handle = api_route_entry->vrf_handle;
  nhop_handle = api_route_entry->nhop_handle;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "nhop handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  memset(&route_entry, 0, sizeof(route_entry));
  route_entry.vrf_handle = vrf_handle;
  route_entry.ip = api_route_entry->ip_address;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  route_entry.neighbor_installed = api_route_entry->neighbor_installed;
#else
  route_entry.neighbor_installed = FALSE;
#endif

  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route table lookup failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  route_handle = switch_route_handle_create(device);
  if (route_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route handle create failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  /* Initialize mgid state for route */
  SET_ROUTE_TUNNEL_MGID_STATE(route_info, switch_api_l3_route_mgid_state_init);
  ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info) = (Pvoid_t)NULL;

  route_info->nhop_handle = nhop_handle;
  route_info->route_handle = route_handle;
  route_info->route_entry.vrf_handle = vrf_handle;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  route_info->route_entry.neighbor_installed =
      api_route_entry->neighbor_installed;
#else
  route_info->route_entry.neighbor_installed = FALSE;
#endif
  SWITCH_MEMCPY(&route_info->route_entry.ip,
                &api_route_entry->ip_address,
                sizeof(switch_ip_addr_t));

  status = switch_route_table_insert(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route table insert failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (!(SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_ROUTE_ENTRY))) {
    status =
        switch_pd_ip_fib_entry_add(device,
                                   handle_to_id(vrf_handle),
                                   &api_route_entry->ip_address,
                                   SWITCH_NHOP_ID_TYPE_ECMP(nhop_info),
                                   handle_to_id(nhop_handle),
                                   api_route_entry->route_type,
                                   &route_info->route_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
                                   (api_route_entry->neighbor_installed == TRUE)
                                       ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
                                       :
#endif
                                       0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table add failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "route table add failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_SET(route_info, SWITCH_ROUTE_PD_ROUTE_ENTRY);
  }

  if (!(SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_URPF_ENTRY))) {
    status =
        switch_pd_urpf_entry_add(device,
                                 handle_to_id(vrf_handle),
                                 &api_route_entry->ip_address,
                                 handle_to_id(nhop_handle),
                                 &route_info->urpf_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
                                 (api_route_entry->neighbor_installed == TRUE)
                                     ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
                                     :
#endif
                                     0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table add failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "urpf table add failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_SET(route_info, SWITCH_ROUTE_PD_URPF_ENTRY);
  }

  status = SEND_ROUTE_TUNNEL_MGID_EVENT(
      route_info, device, SWITCH_ROUTE_ADD, (void *)nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route table insert failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "l3 route added on device %d vrf handle 0x%lx "
      "ip address %s nhop handle 0x%lx route handle 0x%lx\n",
      device,
      vrf_handle,
      switch_ipaddress_to_string(&api_route_entry->ip_address),
      nhop_handle,
      route_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_route_update_internal(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  switch_route_info_t *route_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_route_entry_t route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  vrf_handle = api_route_entry->vrf_handle;
  nhop_handle = api_route_entry->nhop_handle;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "nhop handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  memset(&route_entry, 0, sizeof(route_entry));
  route_entry.vrf_handle = vrf_handle;
  route_entry.ip = api_route_entry->ip_address;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  route_entry.neighbor_installed = api_route_entry->neighbor_installed;
#else
  route_entry.neighbor_installed = FALSE;
#endif

  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route table lookup failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_ROUTE_TUNNEL_MGID_EVENT(
      route_info, device, SWITCH_ROUTE_REMOVE, (void *)route_info->nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle %lx ip address %s nhop handle %lx: "
        "tunnel mgid event route remove failed(%s)\n",
        device,
        vrf_handle,
        "ip address",
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  route_info->nhop_handle = nhop_handle;

  if (SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_ROUTE_ENTRY)) {
    status = switch_pd_ip_fib_entry_update(
        device,
        handle_to_id(vrf_handle),
        &api_route_entry->ip_address,
        SWITCH_NHOP_ID_TYPE_ECMP(nhop_info),
        handle_to_id(nhop_handle),
        route_info->route_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        (api_route_entry->neighbor_installed == TRUE)
            ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
            :
#endif
            0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table update failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "route table add failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_URPF_ENTRY)) {
    status = switch_pd_urpf_entry_update(
        device,
        handle_to_id(vrf_handle),
        &api_route_entry->ip_address,
        handle_to_id(nhop_handle),
        route_info->urpf_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        (api_route_entry->neighbor_installed == TRUE)
            ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
            :
#endif
            0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table add failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "urpf table add failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = SEND_ROUTE_TUNNEL_MGID_EVENT(
      route_info, device, SWITCH_ROUTE_ADD, (void *)nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table update failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "l3 route updated on device %d vrf handle 0x%lx "
      "ip address %s nhop handle 0x%lx route handle 0x%lx\n",
      device,
      vrf_handle,
      switch_ipaddress_to_string(&api_route_entry->ip_address),
      nhop_handle,
      route_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_route_delete_internal(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  switch_route_info_t *route_info = NULL;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_route_entry_t route_entry;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  vrf_handle = api_route_entry->vrf_handle;
  nhop_handle = api_route_entry->nhop_handle;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  route_entry.vrf_handle = vrf_handle;
  route_entry.ip = api_route_entry->ip_address;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  route_entry.neighbor_installed = api_route_entry->neighbor_installed;
#else
  route_entry.neighbor_installed = FALSE;
#endif
  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route entry hash find failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route get failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  nhop_handle = route_info->nhop_handle;

  if (SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_ROUTE_ENTRY)) {
    status = switch_pd_ip_fib_entry_delete(
        device,
        &api_route_entry->ip_address,
        route_info->route_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        (api_route_entry->neighbor_installed == TRUE)
            ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
            :
#endif
            0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table delete failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "route table delete failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(route_info, SWITCH_ROUTE_PD_ROUTE_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(route_info, SWITCH_ROUTE_PD_URPF_ENTRY)) {
    status = switch_pd_urpf_entry_delete(
        device,
        handle_to_id(vrf_handle),
        &api_route_entry->ip_address,
        route_info->urpf_pd_hdl,
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        (api_route_entry->neighbor_installed == TRUE)
            ? SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST
            :
#endif
            0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 route table delete failed on device %d "
          "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
          "urpf table delete failed(%s)\n",
          device,
          vrf_handle,
          switch_ipaddress_to_string(&api_route_entry->ip_address),
          nhop_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(route_info, SWITCH_ROUTE_PD_URPF_ENTRY);
  }

  status = switch_route_table_delete(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "vrf handle 0x%lx ip address %s nhop handle 0x%lx: "
        "route table delete failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_ROUTE_TUNNEL_MGID_EVENT(
      route_info, device, SWITCH_ROUTE_REMOVE, (void *)nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route table delete failed on device %d "
        "vrf handle %lx ip address %s nhop handle %lx: "
        "tunnel mgid route remove event failed:(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(&api_route_entry->ip_address),
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_handle_delete(device, route_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "l3 route deleted on device %d vrf handle 0x%lx "
      "ip address %s nhop handle 0x%lx route handle 0x%lx\n",
      device,
      vrf_handle,
      switch_ipaddress_to_string(&api_route_entry->ip_address),
      nhop_handle,
      route_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_l3_route_delete_by_handle(switch_device_t device,
                                                 switch_handle_t route_handle) {
  switch_route_info_t *route_info = NULL;
  switch_api_route_entry_t api_route_entry;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    SWITCH_LOG_ERROR(
        "l3 route delete by handle failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route delete by handle failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
  api_route_entry.vrf_handle = route_info->route_entry.vrf_handle;
  api_route_entry.nhop_handle = route_info->nhop_handle;
  SWITCH_MEMCPY(&api_route_entry.ip_address,
                &route_info->route_entry.ip,
                sizeof(switch_ip_addr_t));

  status = switch_api_l3_route_delete(device, &api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route delete by handle failed on device %d "
        "route handle 0x%lx: route delete failed(%s)\n",
        device,
        route_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("route delete by handle on device %d route handle 0x%lx",
                   device,
                   route_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_route_nhop_get_internal(
    switch_device_t device,
    switch_handle_t vrf_handle,
    switch_ip_addr_t *ip_addr,
    switch_handle_t *nhop_handle) {
  switch_route_info_t *route_info = NULL;
  switch_route_entry_t route_entry;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "l3 route nhop get failed on device %d "
        "vrf handle 0x%lx ip address %s "
        "vrf handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(ip_addr),
        switch_error_to_string(status));
    return status;
  }

  memset(&route_entry, 0, sizeof(route_entry));
  route_entry.vrf_handle = vrf_handle;
  route_entry.ip = *ip_addr;

  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route nhop get failed on device %d "
        "vrf handle 0x%lx ip address %s "
        "route table lookup failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(ip_addr),
        switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route nhop get failed on device %d "
        "vrf handle 0x%lx ip address %s "
        "route get failed(%s)\n",
        device,
        vrf_handle,
        switch_ipaddress_to_string(ip_addr),
        switch_error_to_string(status));
    return status;
  }

  *nhop_handle = route_info->nhop_handle;

  return status;
}

switch_status_t switch_l3_default_route_entries_add(
    switch_device_t device, switch_handle_t vrf_handle) {
  switch_api_route_entry_t api_route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route add failed on device %d "
        "vrf handle 0x%lx: null nhop get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  // 127/8, drop
  SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
  api_route_entry.vrf_handle = vrf_handle;
  api_route_entry.ip_address.type = SWITCH_API_IP_ADDR_V4;
  api_route_entry.ip_address.ip.v4addr = 0x7f000000;
  api_route_entry.ip_address.prefix_len = 8;
  api_route_entry.nhop_handle = nhop_handle;
  status = switch_api_l3_route_add(device, &api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route add failed on device %d "
        "vrf handle 0x%lx: 127/8 route add failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  // ::1/128, drop
  SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
  api_route_entry.vrf_handle = vrf_handle;
  api_route_entry.ip_address.type = SWITCH_API_IP_ADDR_V6;
  api_route_entry.ip_address.ip.v6addr.u.addr8[15] = 1;
  api_route_entry.ip_address.prefix_len = 128;
  api_route_entry.nhop_handle = nhop_handle;
  status = switch_api_l3_route_add(device, &api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route add failed on device %d "
        "vrf handle 0x%lx: ::1/128  route add failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("l3 default entries added on device %d vrf handle 0x%lx\n",
                   device,
                   vrf_handle);
  return status;
}

switch_status_t switch_l3_default_route_entries_delete(
    switch_device_t device, switch_handle_t vrf_handle) {
  switch_api_route_entry_t api_route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route delete failed on device %d "
        "vrf handle 0x%lx: null nhop get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  // 127/8, drop
  SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
  api_route_entry.vrf_handle = vrf_handle;
  api_route_entry.ip_address.type = SWITCH_API_IP_ADDR_V4;
  api_route_entry.ip_address.ip.v4addr = 0x7f000000;
  api_route_entry.ip_address.prefix_len = 8;
  api_route_entry.nhop_handle = nhop_handle;
  status = switch_api_l3_route_delete(device, &api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route delete failed on device %d "
        "vrf handle 0x%lx: 127/8 route add failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 default route init failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // ::1/128, drop
  SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
  api_route_entry.vrf_handle = vrf_handle;
  api_route_entry.ip_address.type = SWITCH_API_IP_ADDR_V6;
  api_route_entry.ip_address.ip.v6addr.u.addr8[15] = 1;
  api_route_entry.ip_address.prefix_len = 128;
  api_route_entry.nhop_handle = nhop_handle;
  status = switch_api_l3_route_delete(device, &api_route_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 default route delete failed on device %d "
        "vrf handle 0x%lx: ::1/128  route add failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("l3 default entries deleted on device %d vrf handle 0x%lx\n",
                   device,
                   vrf_handle);

  return status;
}

switch_status_t switch_mtu_hashtable_insert(switch_device_t device,
                                            switch_handle_t mtu_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MTU_HANDLE(mtu_handle));
  if (!SWITCH_MTU_HANDLE(mtu_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mtu hashtable insert failed on device %d "
        "mtu handle 0x%lx: mtu handle invalid(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, mtu_handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable insert failed on device %d "
        "mtu handle 0x%lx: mtu get failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable insert failed on device %d "
        "mtu handle 0x%lx: l3 context get failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_INSERT(&l3_ctx->mtu_hashtable,
                                   &((mtu_info)->node),
                                   (void *)&mtu_info->mtu,
                                   (void *)(mtu_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable insert failed on device %d "
        "mtu handle 0x%lx: hashtable insert failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mtu_hashtable_remove(switch_device_t device,
                                            switch_handle_t mtu_handle) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MTU_HANDLE(mtu_handle));
  if (!SWITCH_MTU_HANDLE(mtu_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mtu hashtable insert remove on device %d "
        "mtu handle 0x%lx: mtu handle invalid(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, mtu_handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable remove failed on device %d "
        "mtu handle 0x%lx: mtu get failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable remove failed on device %d "
        "mtu handle 0x%lx: l3 context get failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }
  status = SWITCH_HASHTABLE_DELETE(
      &l3_ctx->mtu_hashtable, (void *)&mtu_info->mtu, (void **)&mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MTU hashtable delete failed on device %d "
        "mtu handle 0x%lx: l3 hashtable delete failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_l3_mtu_create_internal(switch_device_t device,
                                                  switch_uint64_t flags,
                                                  switch_mtu_t mtu,
                                                  switch_handle_t *mtu_handle) {
  switch_mtu_info_t *mtu_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  handle = switch_mtu_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR("l3 mtu create failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 mtu create failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mtu_info->mtu = mtu;

  if (flags & SWITCH_MTU_TYPE_IPV4) {
    status = switch_pd_mtu_table_entry_add(
        device, handle_to_id(handle), mtu, TRUE, &mtu_info->v4_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu create failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_SET(mtu_info, SWITCH_MTU_PD_IPV4_ENTRY);
  }

  if (flags & SWITCH_MTU_TYPE_IPV6) {
    status = switch_pd_mtu_table_entry_add(
        device, handle_to_id(handle), mtu, FALSE, &mtu_info->v6_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu create failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_SET(mtu_info, SWITCH_MTU_PD_IPV6_ENTRY);
  }

  *mtu_handle = handle;
  mtu_info->handle = handle;
  mtu_info->l3intf_count = 1;

  status = switch_mtu_hashtable_insert(device, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable insert failed on device %d "
        "mtu handle 0x%lx: "
        "mtu hashtable insert failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mtu created on device %d mtu handle 0x%lx mtu %d", device, handle, mtu);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_mtu_update_internal(switch_device_t device,
                                                  switch_handle_t mtu_handle,
                                                  switch_mtu_t mtu) {
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_MTU_HANDLE(mtu_handle));
  if (!SWITCH_MTU_HANDLE(mtu_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("l3 mtu update failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, mtu_handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 mtu update failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mtu_info->mtu = mtu;

  if (SWITCH_HW_FLAG_ISSET(mtu_info, SWITCH_MTU_PD_IPV4_ENTRY)) {
    status = switch_pd_mtu_table_entry_update(
        device, handle_to_id(mtu_handle), mtu, TRUE, mtu_info->v4_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu update failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_HW_FLAG_ISSET(mtu_info, SWITCH_MTU_PD_IPV6_ENTRY)) {
    status = switch_pd_mtu_table_entry_update(
        device, handle_to_id(mtu_handle), mtu, FALSE, mtu_info->v6_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu update failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG("mtu updated on device %d mtu handle 0x%lx mtu %d",
                   device,
                   mtu_handle,
                   mtu);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_mtu_get_internal(switch_device_t device,
                                               switch_handle_t mtu_handle,
                                               switch_mtu_t *mtu) {
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_MTU_HANDLE(mtu_handle));
  if (!SWITCH_MTU_HANDLE(mtu_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("l3 mtu get failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, mtu_handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 mtu get failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mtu = mtu_info->mtu;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_mtu_delete_internal(switch_device_t device,
                                                  switch_handle_t mtu_handle) {
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_MTU_HANDLE(mtu_handle));
  if (!SWITCH_MTU_HANDLE(mtu_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("l3 mtu delete failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mtu_get(device, mtu_handle, &mtu_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("l3 mtu delete failed on device %d ",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (SWITCH_HW_FLAG_ISSET(mtu_info, SWITCH_MTU_PD_IPV4_ENTRY)) {
    status = switch_pd_mtu_table_entry_delete(device, mtu_info->v4_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu delete failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(mtu_info, SWITCH_MTU_PD_IPV4_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(mtu_info, SWITCH_MTU_PD_IPV6_ENTRY)) {
    status = switch_pd_mtu_table_entry_delete(device, mtu_info->v6_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("l3 mtu delete failed on device %d ",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(mtu_info, SWITCH_MTU_PD_IPV6_ENTRY);
  }

  status = switch_mtu_hashtable_remove(device, mtu_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu hashtable remove failed on device %d "
        "mtu handle 0x%lx: "
        "mtu hashtable remove failed(%s)\n",
        device,
        mtu_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "mtu deleted on device %d mtu handle 0x%lx", device, mtu_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_table_flush(switch_device_t device,
                                          switch_uint64_t flags,
                                          switch_handle_t vrf_handle) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_node_t *node = NULL;
  switch_list_t v4_list;
  switch_list_t v6_list;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tmp_vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint64_t tmp_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (flags == 0 || flags > SWITCH_ROUTE_FLUSH_TYPE_IPV6) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "l3 table flush failed on device %d "
        "flags 0x%x: flags invalid(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_ROUTE_FLUSH_TYPE_VRF) {
    SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
    if (!SWITCH_VRF_HANDLE(vrf_handle)) {
      status = SWITCH_STATUS_INVALID_HANDLE;
      SWITCH_LOG_ERROR(
          "l3 table flush failed on device %d "
          "vrf handle 0x%lx: vrf handle invalid(%s)\n",
          device,
          vrf_handle,
          switch_error_to_string(status));
      return status;
    }

    switch_vrf_get(device, vrf_handle, &vrf_info, status);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "l3 table flush failed on device %d "
          "vrf handle 0x%lx: vrf get failed(%s)\n",
          device,
          vrf_handle,
          switch_error_to_string(status));
      return status;
    }

    if (flags & SWITCH_ROUTE_FLUSH_TYPE_IPV4) {
      v4_list = vrf_info->ipv4_routes;
    }

    if (flags & SWITCH_ROUTE_FLUSH_TYPE_IPV6) {
      v6_list = vrf_info->ipv6_routes;
    }
  } else {
    if (flags & SWITCH_ROUTE_FLUSH_TYPE_IPV4) {
      tmp_flags |= SWITCH_ROUTE_FLUSH_TYPE_VRF;
      tmp_flags |= SWITCH_ROUTE_FLUSH_TYPE_IPV4;
    }

    if (flags & SWITCH_ROUTE_FLUSH_TYPE_IPV6) {
      tmp_flags |= SWITCH_ROUTE_FLUSH_TYPE_VRF;
      tmp_flags |= SWITCH_ROUTE_FLUSH_TYPE_IPV6;
    }

    FOR_EACH_HANDLE_BEGIN(device, SWITCH_HANDLE_TYPE_VRF, tmp_vrf_handle) {
      status = switch_api_l3_table_flush(device, tmp_flags, tmp_vrf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 table flush failed on device %d "
            "vrf handle 0x%lx: vrf handle invalid(%s)\n",
            device,
            vrf_handle,
            switch_error_to_string(status));
        return status;
      }
    }
    FOR_EACH_HANDLE_END();
    return status;
  }

  if (SWITCH_LIST_COUNT(&v4_list)) {
    FOR_EACH_IN_LIST(v4_list, node) {
      route_handle = (switch_handle_t)node->data;
      status = switch_l3_route_delete_by_handle(device, route_handle);
      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    }
    FOR_EACH_IN_LIST_END();
  }

  if (SWITCH_LIST_COUNT(&v6_list)) {
    FOR_EACH_IN_LIST(v6_list, node) {
      route_handle = (switch_handle_t)node->data;
      status = switch_l3_route_delete_by_handle(device, route_handle);
      SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    }
    FOR_EACH_IN_LIST_END();
  }

  SWITCH_LOG_DEBUG("l3 route flush on device %d vrf handle 0x%lx flags %x",
                   device,
                   vrf_handle,
                   flags);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_l3_mtu_size_create_internal(
    switch_device_t device, switch_mtu_t mtu, switch_handle_t *mtu_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_l3_context_t *l3_ctx = NULL;
  switch_mtu_info_t *mtu_info = NULL;
  switch_uint64_t flags = 0;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu size handle get failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = SWITCH_HASHTABLE_SEARCH(
      &l3_ctx->mtu_hashtable, (void *)&mtu, (void **)&mtu_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    *mtu_handle = mtu_info->handle;
    mtu_info->l3intf_count++;
  } else if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    // Create a new mtu handle for the MTU size and insert into the hash table.
    flags = (SWITCH_MTU_TYPE_IPV4 || SWITCH_MTU_TYPE_IPV6);
    status = switch_api_l3_mtu_create(device, flags, mtu, mtu_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mtu size handle get failed on device %d: "
          "mtu handle create failed(%s)\n",
          device,
          switch_error_to_string(status));
    }
  }
  return status;
}

switch_status_t switch_api_l3_mtu_size_delete_internal(switch_device_t device,
                                                       switch_mtu_t mtu) {
  switch_l3_context_t *l3_ctx = NULL;
  switch_mtu_info_t *mtu_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_L3, (void **)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mtu size handle delete failed on device %d: "
        "l3 context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = SWITCH_HASHTABLE_SEARCH(
      &l3_ctx->mtu_hashtable, (void *)&mtu, (void **)&mtu_info);
  if (mtu_info->l3intf_count) {
    mtu_info->l3intf_count--;
    if (mtu_info->l3intf_count == 0) {
      status = switch_api_l3_mtu_delete(device, mtu_info->handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mtu size handle delete failed on device %d: "
            "mtu delete failed(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  }
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_l3_route_nhop_get(switch_device_t device,
                                             switch_handle_t vrf,
                                             switch_ip_addr_t *ip_addr,
                                             switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_l3_route_nhop_get_internal(device, vrf, ip_addr, nhop_handle))
}

switch_status_t switch_api_l3_route_update(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  SWITCH_MT_WRAP(switch_api_l3_route_update_internal(device, api_route_entry))
}

switch_status_t switch_api_l3_route_delete(
    switch_device_t device, switch_api_route_entry_t *route_entry) {
  SWITCH_MT_WRAP(switch_api_l3_route_delete_internal(device, route_entry))
}

switch_status_t switch_api_l3_mtu_create(switch_device_t device,
                                         switch_uint64_t flags,
                                         switch_mtu_t mtu,
                                         switch_handle_t *mtu_handle) {
  SWITCH_MT_WRAP(
      switch_api_l3_mtu_create_internal(device, flags, mtu, mtu_handle))
}

switch_status_t switch_api_l3_route_add(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  SWITCH_MT_WRAP(switch_api_l3_route_add_internal(device, api_route_entry))
}

switch_status_t switch_api_l3_route_handle_lookup(
    const switch_device_t device,
    const switch_api_route_entry_t *api_route_entry,
    switch_handle_t *route_handle) {
  SWITCH_MT_WRAP(switch_api_l3_route_handle_lookup_internal(
      device, api_route_entry, route_handle))
}

switch_status_t switch_api_l3_interface_address_add(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  SWITCH_MT_WRAP(
      switch_api_l3_interface_address_add_internal(device, api_route_entry))
}

switch_status_t switch_api_l3_mtu_update(switch_device_t device,
                                         switch_handle_t mtu_handle,
                                         switch_mtu_t mtu) {
  SWITCH_MT_WRAP(switch_api_l3_mtu_update_internal(device, mtu_handle, mtu))
}

switch_status_t switch_api_l3_mtu_delete(switch_device_t device,
                                         switch_handle_t mtu_handle) {
  SWITCH_MT_WRAP(switch_api_l3_mtu_delete_internal(device, mtu_handle))
}

switch_status_t switch_api_l3_route_lookup(
    switch_device_t device,
    switch_api_route_entry_t *api_route_entry,
    switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_l3_route_lookup_internal(device, api_route_entry, nhop_handle))
}

switch_status_t switch_api_l3_mtu_get(switch_device_t device,
                                      switch_handle_t mtu_handle,
                                      switch_mtu_t *mtu) {
  SWITCH_MT_WRAP(switch_api_l3_mtu_get_internal(device, mtu_handle, mtu))
}

switch_status_t switch_api_l3_interface_address_delete(
    switch_device_t device, switch_api_route_entry_t *api_route_entry) {
  SWITCH_MT_WRAP(
      switch_api_l3_interface_address_delete_internal(device, api_route_entry))
}

switch_status_t switch_api_route_table_size_get(switch_device_t device,
                                                switch_size_t *tbl_size) {
  SWITCH_MT_WRAP(switch_api_route_table_size_get_internal(device, tbl_size))
}

switch_status_t switch_api_l3_mtu_size_create(switch_device_t device,
                                              switch_mtu_t mtu,
                                              switch_handle_t *mtu_handle) {
  SWITCH_MT_WRAP(
      switch_api_l3_mtu_size_create_internal(device, mtu, mtu_handle))
}

switch_status_t switch_api_l3_mtu_size_delete(switch_device_t device,
                                              switch_mtu_t mtu) {
  SWITCH_MT_WRAP(switch_api_l3_mtu_size_delete_internal(device, mtu))
}

static switch_status_t switch_api_l3_route_add_tunnel_to_list(
    switch_device_t device,
    switch_route_info_t *route_info,
    switch_handle_t tunnel_handle) {
  PWord_t PValue;

  JLI(PValue, ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info), tunnel_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR(
        "route add tunnel failed on device %d"
        "route handle 0x%lx: , tunnel handle 0x%lx: ",
        device,
        route_info->route_handle,
        tunnel_handle);
    return SWITCH_STATUS_FAILURE;
  }

  ROUTE_TUNNEL_MGID_NUM_TUNNELS(route_info) += 1;

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_l3_route_remove_tunnel_from_list(
    switch_device_t device,
    switch_route_info_t *route_info,
    switch_handle_t tunnel_handle) {
  int Rc_int;
  JLD(Rc_int, ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info), tunnel_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR(
        "route remove tunnel failed on device %d"
        "route handle 0x%lx: , tunnel handle 0x%lx: ",
        device,
        route_info->route_handle,
        tunnel_handle);
    return SWITCH_STATUS_FAILURE;
  }

  ROUTE_TUNNEL_MGID_NUM_TUNNELS(route_info) -= 1;

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_l3_route_tunnel_list_get(
    switch_device_t device,
    switch_handle_t **tunnel_handle_list,
    switch_uint32_t *tunnel_handle_count,
    switch_route_info_t *route_info) {
  int Rc_int, count;
  PWord_t PValue;

  JLC(Rc_int, ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info), 0, -1);
  *tunnel_handle_count = Rc_int;

  if (*tunnel_handle_count) {
    *tunnel_handle_list =
        SWITCH_MALLOC(device, sizeof(switch_handle_t), Rc_int);

    for (count = 1; count <= Rc_int; count++) {
      JLBC(PValue,
           ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info),
           count,
           (*tunnel_handle_list)[count - 1]);
    }
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_l3_route_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_route_info_t *route_info;
  switch_handle_t *tunnel_list;
  switch_uint32_t tunnel_count = 0;
  switch_uint32_t count;
  switch_handle_t nhop_handle;
  switch_status_t status;

  route_info = (switch_route_info_t *)info;

  switch (event) {
    case SWITCH_ROUTE_ADD:
      nhop_handle = (switch_handle_t)event_arg;

      /* Send an event to the corresponding nexthop */
      SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
      switch_api_nhop_send_mgid_event(device,
                                      nhop_handle,
                                      SWITCH_ROUTE_ADD,
                                      (void *)route_info->route_handle);

      /* Check if this new route provides reachability
      to any tunnel destination */
      status = switch_api_tunnel_dest_list_get(
          device, &tunnel_list, &tunnel_count, route_info->route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state init on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      /* Set the initial state to NO_MGID. If some tunnel
      determines that this is the best route, then a
      TUNNEL_CREATE message will arrive and set the state
      to MGID_ASSOCIATED */
      SET_ROUTE_TUNNEL_MGID_STATE(route_info,
                                  switch_api_l3_route_mgid_state_no_mgid);

      /* Loop through the tunnels and let them evaluate
      if this is the best route. */
      if (tunnel_count) {
        for (count = 0; count < tunnel_count; count++) {
          status = switch_api_tunnel_send_mgid_event(
              device, tunnel_list[count], SWITCH_ROUTE_ADD, NULL);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "l3 route mgid state init on device %d "
                "with error: (%s)\n",
                device,
                switch_error_to_string(status));
            return status;
          }
        }

        SWITCH_FREE(device, tunnel_list);
      }

      break;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_l3_route_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_route_info_t *route_info;
  switch_handle_t tunnel_handle;
  switch_handle_t nhop_handle;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  route_info = (switch_route_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_CREATE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));

      status = switch_api_l3_route_add_tunnel_to_list(
          device, route_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state no mgid on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      SET_ROUTE_TUNNEL_MGID_STATE(
          route_info, switch_api_l3_route_mgid_state_mgid_associated);
      break;

    case SWITCH_ROUTE_REMOVE:
      nhop_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
      status =
          switch_api_nhop_send_mgid_event(device,
                                          nhop_handle,
                                          SWITCH_ROUTE_REMOVE,
                                          (void *)route_info->route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state no mgid on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      SET_ROUTE_TUNNEL_MGID_STATE(route_info,
                                  switch_api_l3_route_mgid_state_init);
      break;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_l3_route_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_route_info_t *route_info;
  switch_handle_t tunnel_handle;
  switch_handle_t nhop_handle;
  switch_uint32_t Rc_tun;
  switch_handle_t *tunnel_list;
  switch_uint32_t tunnel_count = 0;
  switch_uint32_t count;
  switch_status_t status;

  route_info = (switch_route_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_CREATE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));

      status = switch_api_l3_route_add_tunnel_to_list(
          device, route_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state mgid assiciated on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      break;

    case SWITCH_TUNNEL_DELETE:
      tunnel_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));

      status = switch_api_l3_route_remove_tunnel_from_list(
          device, route_info, tunnel_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state mgid assiciated on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      /* Check if the last tunnel using this route was deleted */
      JLC(Rc_tun, ROUTE_TUNNEL_MGID_TUNNEL_LIST(route_info), 0, -1);
      if (Rc_tun == 0) {
        SET_ROUTE_TUNNEL_MGID_STATE(route_info,
                                    switch_api_l3_route_mgid_state_no_mgid);
      }
      break;

    case SWITCH_ROUTE_REMOVE:
      nhop_handle = (switch_handle_t)event_arg;

      /* Re-evaluate reachability for all tunnels using this route */
      status = switch_api_l3_route_tunnel_list_get(
          device, &tunnel_list, &tunnel_count, route_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state mgid assiciated on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      /*Note this function can be called back from each tunnel.
      So this function needs to be re-entrant */
      if (tunnel_count) {
        for (count = 0; count < tunnel_count; count++) {
          status = switch_api_tunnel_send_mgid_event(
              device, tunnel_list[count], SWITCH_ROUTE_REMOVE, NULL);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "l3 route mgid state mgid associated on device %d "
                "with error: (%s)\n",
                device,
                switch_error_to_string(status));
            return status;
          }
        }

        SWITCH_FREE(device, tunnel_list);
      }

      SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
      status =
          switch_api_nhop_send_mgid_event(device,
                                          nhop_handle,
                                          SWITCH_ROUTE_REMOVE,
                                          (void *)route_info->route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route mgid state mgid assiciated on device %d "
            "with error: (%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      SET_ROUTE_TUNNEL_MGID_STATE(route_info,
                                  switch_api_l3_route_mgid_state_init);

      break;
    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_l3_route_send_mgid_event(
    switch_device_t device,
    switch_handle_t route_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_route_info_t *route_info;
  switch_status_t status;

  SWITCH_ASSERT(SWITCH_ROUTE_HANDLE(route_handle));
  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route send mgid on device %d "
        "with error: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_ROUTE_TUNNEL_MGID_EVENT(route_info, device, event, event_arg);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route send mgid on device %d "
        "with error: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_l3_v4_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_route_info_t *route_info = NULL;
  switch_node_t *node = NULL;
  switch_api_route_entry_t api_route_entry;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_MT_LOCK(device);

  switch_vrf_get_internal(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 v4 route entries get by vrf failed on device %d "
        "vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    SWITCH_MT_UNLOCK(device);
    return status;
  }

  if (SWITCH_LIST_COUNT(&vrf_info->ipv4_routes)) {
    FOR_EACH_IN_LIST(vrf_info->ipv4_routes, node) {
      route_handle = (switch_handle_t)node->data;
      status = switch_route_get(device, route_handle, &route_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 v4 route entries get by vrf failed on device %d "
            "vrf handle 0x%lx: "
            "route get failed(%s)\n",
            device,
            vrf_handle,
            switch_error_to_string(status));
        SWITCH_MT_UNLOCK(device);
        return status;
      }

      SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
      api_route_entry.vrf_handle = route_info->route_entry.vrf_handle;
      api_route_entry.nhop_handle = route_info->nhop_handle;
      SWITCH_MEMCPY(&api_route_entry.ip_address,
                    &route_info->route_entry.ip,
                    sizeof(switch_ip_addr_t));
      SWITCH_MT_UNLOCK(device);
      status = iterator_fn(&api_route_entry);
      SWITCH_MT_LOCK(device);
    }
    FOR_EACH_IN_LIST_END();
  }

  SWITCH_MT_UNLOCK(device);
  return status;
}

switch_status_t switch_api_l3_v6_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_route_info_t *route_info = NULL;
  switch_node_t *node = NULL;
  switch_api_route_entry_t api_route_entry;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_MT_LOCK(device);

  switch_vrf_get_internal(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 v6 route entries get by vrf failed on device %d "
        "vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_LIST_COUNT(&vrf_info->ipv6_routes)) {
    FOR_EACH_IN_LIST(vrf_info->ipv6_routes, node) {
      route_handle = (switch_handle_t)node->data;
      status = switch_route_get(device, route_handle, &route_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 v6 route entries get by vrf failed on device %d "
            "vrf handle: 0x%lx: "
            "route get failed(%s)\n",
            device,
            vrf_handle,
            switch_error_to_string(status));
        SWITCH_MT_UNLOCK(device);
        return status;
      }

      SWITCH_MEMSET(&api_route_entry, 0x0, sizeof(api_route_entry));
      api_route_entry.vrf_handle = route_info->route_entry.vrf_handle;
      api_route_entry.nhop_handle = route_info->nhop_handle;
      SWITCH_MEMCPY(&api_route_entry.ip_address,
                    &route_info->route_entry.ip,
                    sizeof(switch_ip_addr_t));
      SWITCH_MT_UNLOCK(device);
      iterator_fn(&api_route_entry);
      SWITCH_MT_LOCK(device);
    }
    FOR_EACH_IN_LIST_END();
  }

  SWITCH_MT_UNLOCK(device);
  return status;
}

switch_status_t switch_api_l3_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));

  status = switch_api_l3_v4_route_entries_get_by_vrf(
      device, vrf_handle, iterator_fn);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route entries get by vrf failed on device %d vrf handle 0x%lx: "
        "v4 route entries get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_l3_v6_route_entries_get_by_vrf(
      device, vrf_handle, iterator_fn);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "l3 route entries get by vrf failed on device %d vrf handle 0x%lx: "
        "v6 route entries get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_l3_route_entries_get_internal(
    const switch_device_t device, switch_l3_table_iterator_fn iterator_fn) {
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  FOR_EACH_HANDLE_BEGIN(device, SWITCH_HANDLE_TYPE_VRF, vrf_handle) {
    if (SWITCH_VRF_HANDLE(vrf_handle)) {
      status = switch_api_l3_route_entries_get_by_vrf(
          device, vrf_handle, iterator_fn);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "l3 route entries get by vrf failed on device %d vrf handle 0x%lx: "
            "route entries get failed(%s)\n",
            device,
            vrf_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }
  FOR_EACH_HANDLE_END();

  return status;
}

switch_status_t switch_api_l3_route_entries_get(
    const switch_device_t device, switch_l3_table_iterator_fn iterator_fn) {
  SWITCH_MT_WRAP(switch_api_l3_route_entries_get_internal(device, iterator_fn));
}
