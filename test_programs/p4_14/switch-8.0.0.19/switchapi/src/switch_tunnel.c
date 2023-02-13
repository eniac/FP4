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

#include "switchapi/switch_tunnel.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

switch_status_t switch_tunnel_ip_hash_key_init(void *args,
                                               switch_uint8_t *key,
                                               switch_uint32_t *len) {
  switch_tunnel_ip_key_t *ip_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  ip_key = (switch_tunnel_ip_key_t *)args;

  SWITCH_MEMCPY(key + *len, &ip_key->ip_type, sizeof(switch_tunnel_ip_type_t));
  *len += sizeof(switch_tunnel_ip_type_t);

  SWITCH_MEMCPY(key + *len, &ip_key->ip_addr, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_ASSERT(*len == SWITCH_TUNNEL_IP_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_tunnel_ip_hash_compare(const void *key1,
                                             const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_TUNNEL_IP_HASH_KEY_SIZE);
}

switch_status_t switch_tunnel_vtep_hash_key_init(void *args,
                                                 switch_uint8_t *key,
                                                 switch_uint32_t *len) {
  switch_tunnel_vtep_key_t *vtep_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  vtep_key = (switch_tunnel_vtep_key_t *)args;

  SWITCH_MEMCPY(
      key + *len, &vtep_key->ip_type, sizeof(switch_tunnel_ip_type_t));
  *len += sizeof(switch_tunnel_ip_type_t);

  SWITCH_MEMCPY(
      key + *len, &vtep_key->tunnel_type, sizeof(switch_tunnel_type_t));
  *len += sizeof(switch_tunnel_type_t);

  SWITCH_MEMCPY(key + *len, &vtep_key->vrf_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY(key + *len, &vtep_key->ip_addr, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_ASSERT(*len == SWITCH_TUNNEL_IP_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_tunnel_vtep_hash_compare(const void *key1,
                                               const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_TUNNEL_VTEP_HASH_KEY_SIZE);
}

switch_status_t switch_tunnel_ingress_vni_hash_key_init(void *args,
                                                        switch_uint8_t *key,
                                                        switch_uint32_t *len) {
  switch_tunnel_vni_ingress_key_t *ingress_vni_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  ingress_vni_key = (switch_tunnel_vni_ingress_key_t *)args;

  SWITCH_MEMCPY(key + *len, &ingress_vni_key->tunnel_vni, sizeof(switch_vni_t));
  *len += sizeof(switch_vni_t);

  SWITCH_ASSERT(*len == SWITCH_TUNNEL_INGRESS_VNI_HASH_KEY_SIZE);

  return status;
}

switch_status_t switch_tunnel_ingress_vni_hash_compare(const void *key1,
                                                       const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_TUNNEL_INGRESS_VNI_HASH_KEY_SIZE);
}

switch_status_t switch_tunnel_egress_vni_hash_key_init(void *args,
                                                       switch_uint8_t *key,
                                                       switch_uint32_t *len) {
  switch_tunnel_vni_egress_key_t *egress_vni_key = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  egress_vni_key = (switch_tunnel_vni_egress_key_t *)args;

  SWITCH_MEMCPY(
      key + *len, &egress_vni_key->bd_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_ASSERT(*len == SWITCH_TUNNEL_EGRESS_VNI_HASH_KEY_SIZE);

  return status;
}

switch_status_t switch_tunnel_egress_vni_hash_compare(const void *key1,
                                                      const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_TUNNEL_EGRESS_VNI_HASH_KEY_SIZE);
}

switch_status_t switch_tunnel_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_src_vtep_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dest_vtep_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_bd_map_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_smac_rewrite_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_dmac_rewrite_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_rewrite_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_decap_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_encap_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_validate_mpls_packet_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel default entry add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_tunnel_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_tunnel_type_ingress_get(
    switch_tunnel_type_t tunnel_type,
    bool ipv4,
    switch_tunnel_type_ingress_t *ingress_tunnel_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(tunnel_type);
  UNUSED(ipv4);
  UNUSED(ingress_tunnel_type);

  SWITCH_ASSERT(tunnel_type < SWITCH_TUNNEL_TYPE_MAX);
  SWITCH_ASSERT(ingress_tunnel_type != NULL);
  *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_NONE;

  switch (tunnel_type) {
    case SWITCH_TUNNEL_TYPE_VXLAN:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_VXLAN;
      break;

    case SWITCH_TUNNEL_TYPE_NVGRE:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_NVGRE;
      break;

    case SWITCH_TUNNEL_TYPE_GENEVE:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_GENEVE;
      break;

    case SWITCH_TUNNEL_TYPE_IPIP:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_IPIP;
      break;

    case SWITCH_TUNNEL_TYPE_SRV6:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_SRV6;
      break;

    case SWITCH_TUNNEL_TYPE_GRE:
    case SWITCH_TUNNEL_TYPE_ERSPAN_T3:
    case SWITCH_TUNNEL_TYPE_DTEL_REPORT:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_GRE;
      break;

    default:
      *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_NONE;
      status = SWITCH_STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}

switch_status_t switch_tunnel_type_egress_get(
    switch_tunnel_type_t tunnel_type,
    bool ipv4,
    switch_tunnel_type_egress_t *egress_tunnel_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(tunnel_type);
  UNUSED(ipv4);
  UNUSED(egress_tunnel_type);

  SWITCH_ASSERT(tunnel_type < SWITCH_TUNNEL_TYPE_MAX);
  SWITCH_ASSERT(egress_tunnel_type != NULL);
  *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_NONE;

  switch (tunnel_type) {
    case SWITCH_TUNNEL_TYPE_VXLAN:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_VXLAN;
      }
      break;

    case SWITCH_TUNNEL_TYPE_NVGRE:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_NVGRE;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_NVGRE;
      }
      break;

    case SWITCH_TUNNEL_TYPE_GENEVE:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GENEVE;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GENEVE;
      }
      break;

    case SWITCH_TUNNEL_TYPE_IPIP:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_IP;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_IP;
      }
      break;

    case SWITCH_TUNNEL_TYPE_SRV6:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_NONE;
        status = SWITCH_STATUS_NOT_SUPPORTED;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_SRV6;
      }
      break;

    case SWITCH_TUNNEL_TYPE_GRE:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GRE;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GRE;
      }
      break;

    case SWITCH_TUNNEL_TYPE_ERSPAN_T3:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_ERSPAN_T3;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_ERSPAN_T3;
      }
      break;

    case SWITCH_TUNNEL_TYPE_DTEL_REPORT:
      if (ipv4) {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV4_DTEL_REPORT;
      } else {
        *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_IPV6_DTEL_REPORT;
      }
      break;

    default:
      *egress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_NONE;
      status = SWITCH_STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}

switch_status_t switch_tunnel_init(switch_device_t device) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_size_t src_vtep_table_size = 0;
  switch_size_t dst_vtep_table_size = 0;
  switch_size_t dip_rewrite_table_size = 0;
  switch_size_t tunnel_table_size = 0;
  switch_size_t egress_vni_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  tunnel_ctx = SWITCH_MALLOC(device, sizeof(switch_tunnel_context_t), 0x1);
  if (!tunnel_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel context malloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(tunnel_ctx, 0x0, sizeof(switch_tunnel_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_TUNNEL, (void *)tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel context set failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_IPV4_SRC_VTEP, &src_vtep_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_IPV4_DST_VTEP, &dst_vtep_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_TUNNEL, &tunnel_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_EGRESS_BD, &egress_vni_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_TUNNEL_DIP_REWRITE, &dip_rewrite_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  tunnel_ctx->dst_ip_hashtable.size = dip_rewrite_table_size;
  tunnel_ctx->dst_ip_hashtable.compare_func = switch_tunnel_ip_hash_compare;
  tunnel_ctx->dst_ip_hashtable.key_func = switch_tunnel_ip_hash_key_init;
  tunnel_ctx->dst_ip_hashtable.hash_seed = SWITCH_TUNNEL_IP_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&tunnel_ctx->dst_ip_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_id_allocator_new(
      device, dip_rewrite_table_size, FALSE, &tunnel_ctx->dst_ip_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "dst ip id allocator failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  tunnel_ctx->src_vtep_hashtable.size = src_vtep_table_size;
  tunnel_ctx->src_vtep_hashtable.compare_func = switch_tunnel_vtep_hash_compare;
  tunnel_ctx->src_vtep_hashtable.key_func = switch_tunnel_vtep_hash_key_init;
  tunnel_ctx->src_vtep_hashtable.hash_seed = SWITCH_TUNNEL_VTEP_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&tunnel_ctx->src_vtep_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  tunnel_ctx->dst_vtep_hashtable.size = dst_vtep_table_size;
  tunnel_ctx->dst_vtep_hashtable.compare_func = switch_tunnel_vtep_hash_compare;
  tunnel_ctx->dst_vtep_hashtable.key_func = switch_tunnel_vtep_hash_key_init;
  tunnel_ctx->dst_vtep_hashtable.hash_seed = SWITCH_TUNNEL_VTEP_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&tunnel_ctx->dst_vtep_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_id_allocator_new(device,
                                       SWITCH_TUNNEL_VNI_ALLOCATOR_SIZE,
                                       FALSE,
                                       &tunnel_ctx->tunnel_vni_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel vni allocation failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY,
                                   tunnel_table_size + egress_vni_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel mapper entry handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_TUNNEL_MAPPER,
                                   SWITCH_TUNNEL_MAPPER_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel mapper handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_TUNNEL, SWITCH_TUNNEL_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, tunnel_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel term handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_TUNNEL_ENCAP, SWITCH_TUNNEL_ENCAP_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel init failed on device %d: "
        "tunnel encap handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  tunnel_ctx->ingress_tunnel_vni_hashtable.size = tunnel_table_size;
  tunnel_ctx->ingress_tunnel_vni_hashtable.compare_func =
      switch_tunnel_ingress_vni_hash_compare;
  tunnel_ctx->ingress_tunnel_vni_hashtable.key_func =
      switch_tunnel_ingress_vni_hash_key_init;
  tunnel_ctx->ingress_tunnel_vni_hashtable.hash_seed =
      SWITCH_TUNNEL_INGRESS_VNI_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&tunnel_ctx->ingress_tunnel_vni_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  tunnel_ctx->egress_tunnel_vni_hashtable.size = egress_vni_table_size;
  tunnel_ctx->egress_tunnel_vni_hashtable.compare_func =
      switch_tunnel_egress_vni_hash_compare;
  tunnel_ctx->egress_tunnel_vni_hashtable.key_func =
      switch_tunnel_egress_vni_hash_key_init;
  tunnel_ctx->egress_tunnel_vni_hashtable.hash_seed =
      SWITCH_TUNNEL_EGRESS_VNI_HASH_SEED;
  status = SWITCH_HASHTABLE_INIT(&tunnel_ctx->egress_tunnel_vni_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  tunnel_ctx->PJLarr_tunnel_dest = (Pvoid_t)NULL;

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  tmp_status = switch_tunnel_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_tunnel_free(switch_device_t device) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->src_ip_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->dst_ip_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->src_vtep_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->dst_vtep_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->ingress_tunnel_vni_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_HASHTABLE_DONE(&tunnel_ctx->egress_tunnel_vni_hashtable);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status =
      switch_api_id_allocator_destroy(device, tunnel_ctx->src_ip_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "src ip allocator free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status =
      switch_api_id_allocator_destroy(device, tunnel_ctx->dst_ip_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "dst ip allocator free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status =
      switch_api_id_allocator_destroy(device, tunnel_ctx->tunnel_vni_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel vni allocator free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel mapper handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status =
      switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel mapper handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL_TERM);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel term handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL_ENCAP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel free failed on device %d: "
        "tunnel encap handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, tunnel_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_TUNNEL, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_tunnel_ip_rewrite_table_add(
    const switch_device_t device,
    const switch_tunnel_ip_key_t *ip_key,
    switch_id_t *ip_id) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_ip_entry_t *ip_entry = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_id_allocator_t *allocator = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash insert failed on device %d: "
        "tunnel context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (ip_key->ip_type == SWITCH_TUNNEL_IP_TYPE_SRC) {
    hashtable = &tunnel_ctx->src_ip_hashtable;
    allocator = tunnel_ctx->src_ip_id_allocator;
  } else {
    hashtable = &tunnel_ctx->dst_ip_hashtable;
    allocator = tunnel_ctx->dst_ip_id_allocator;
  }

  status =
      SWITCH_HASHTABLE_SEARCH(hashtable, (void *)ip_key, (void **)&ip_entry);

  if (status == SWITCH_STATUS_SUCCESS) {
    ip_entry->ref_count++;
    *ip_id = ip_entry->ip_id;
    return status;
  }

  ip_entry = SWITCH_MALLOC(device, sizeof(switch_tunnel_ip_entry_t), 0x1);
  if (!ip_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel ip hash insert failed on device %d: "
        "ip entry alloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(ip_entry, 0x0, sizeof(switch_tunnel_ip_entry_t));

  status = switch_api_id_allocator_allocate(device, allocator, ip_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash insert failed on device %d: "
        "ip id alloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&ip_entry->ip_key, ip_key, sizeof(switch_tunnel_ip_key_t));
  ip_entry->ip_id = *ip_id;
  ip_entry->ref_count++;

  status = SWITCH_HASHTABLE_INSERT(
      hashtable, &(ip_entry->node), (void *)ip_key, (void *)ip_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash insert failed on device %d: "
        "hashtable insert failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (ip_key->ip_type == SWITCH_TUNNEL_IP_TYPE_SRC) {
    SWITCH_ASSERT(0);
  } else {
    status = switch_pd_tunnel_ip_dst_rewrite_table_entry_add(
        device, (*ip_id), &ip_key->ip_addr, &ip_entry->rw_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel ip hash insert failed on device %d: "
          "tunnel ip dst rewrite failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_tunnel_ip_rewrite_table_delete(
    const switch_device_t device, const switch_tunnel_ip_key_t *ip_key) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_ip_entry_t *ip_entry = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_id_allocator_t *allocator = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash delete failed on device %d: "
        "tunnel context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (ip_key->ip_type == SWITCH_TUNNEL_IP_TYPE_SRC) {
    hashtable = &tunnel_ctx->src_ip_hashtable;
    allocator = tunnel_ctx->src_ip_id_allocator;
  } else {
    hashtable = &tunnel_ctx->dst_ip_hashtable;
    allocator = tunnel_ctx->dst_ip_id_allocator;
  }

  status =
      SWITCH_HASHTABLE_SEARCH(hashtable, (void *)ip_key, (void **)&ip_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash delete failed on device %d: "
        "tunnel ip hashtable search failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  ip_entry->ref_count--;
  if (ip_entry->ref_count > 0) {
    return status;
  }

  if (ip_key->ip_type == SWITCH_TUNNEL_IP_TYPE_SRC) {
    SWITCH_ASSERT(0);
  } else {
    status = switch_pd_tunnel_ip_dst_rewrite_table_entry_delete(
        device, ip_entry->rw_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel ip hash delete failed on device %d: "
          "tunnel ip dst rewrite failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status =
      SWITCH_HASHTABLE_DELETE(hashtable, (void *)ip_key, (void **)&ip_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash delete failed on device %d: "
        "tunnel hashtable delte failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_release(device, allocator, ip_entry->ip_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip hash delete failed on device %d: "
        "ip id release failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, ip_entry);

  return status;
}

switch_status_t switch_tunnel_ip_index_get(switch_device_t device,
                                           switch_tunnel_ip_key_t *ip_key,
                                           switch_id_t *ip_id) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_hashtable_t *hashtable = NULL;
  switch_tunnel_ip_entry_t *ip_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(ip_key != NULL);
  SWITCH_ASSERT(ip_id != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip index get failed on device: %d "
        "tunnel context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (ip_key->ip_type == SWITCH_TUNNEL_IP_TYPE_SRC) {
    hashtable = &tunnel_ctx->src_ip_hashtable;
  } else {
    hashtable = &tunnel_ctx->dst_ip_hashtable;
  }

  *ip_id = 0;

  status =
      SWITCH_HASHTABLE_SEARCH(hashtable, (void *)ip_key, (void **)&ip_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel ip index get failed on device: %d "
        "tunnel hashtable get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *ip_id = ip_entry->ip_id;

  return status;
}

switch_status_t switch_api_tunnel_create_internal(
    const switch_device_t device,
    const switch_api_tunnel_info_t *api_tunnel_info,
    switch_handle_t *tunnel_handle) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  bool ip_type = TRUE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_tunnel_info);
  SWITCH_ASSERT(api_tunnel_info->tunnel_type <= SWITCH_TUNNEL_TYPE_MAX);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel create failed on device %d: "
        "tunnel context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_tunnel_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel create failed on device %d: "
        "tunnel handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel create failed on device %d: "
        "tunnel get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&tunnel_info->api_tunnel_info,
                api_tunnel_info,
                sizeof(switch_api_tunnel_info_t));
  tunnel_info->tunnel_type = api_tunnel_info->tunnel_type;
  SWITCH_ARRAY_INIT(&tunnel_info->tunnel_term_array);

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(api_tunnel_info->underlay_rif_handle));
  if ((!SWITCH_RIF_HANDLE(api_tunnel_info->underlay_rif_handle))) {
    SWITCH_LOG_ERROR(
        "tunnel create failed on device %d: "
        "tunnel underlay rif handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_rif_vrf_handle_get(device,
                                         api_tunnel_info->underlay_rif_handle,
                                         &tunnel_info->underlay_vrf_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_VRF_HANDLE(tunnel_info->underlay_vrf_handle));

  ip_type = (api_tunnel_info->ip_type == SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4)
                ? TRUE
                : FALSE;
  switch_tunnel_type_ingress_get(
      tunnel_info->tunnel_type, ip_type, &tunnel_info->ingress_tunnel_type);
  switch_tunnel_type_egress_get(
      tunnel_info->tunnel_type, ip_type, &tunnel_info->egress_tunnel_type);

  if ((tunnel_info->tunnel_type == SWITCH_TUNNEL_TYPE_IPIP) ||
      (tunnel_info->tunnel_type == SWITCH_TUNNEL_TYPE_GRE)) {
    status = switch_api_id_allocator_allocate(
        device, tunnel_ctx->tunnel_vni_allocator, &tunnel_info->tunnel_vni);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel create failed on device %d: "
          "tunnel vni allocation failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    tunnel_info->tunnel_vni += SWITCH_TUNNEL_VNI_OFFSET;

    status = switch_api_rif_vrf_handle_get(
        device, api_tunnel_info->overlay_rif_handle, &vrf_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
    status = switch_bd_handle_get(device, vrf_handle, &bd_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel create failed on device %d vrf handle 0x%lx: "
          "tunnel bd get failed:(%s)\n",
          device,
          vrf_handle,
          switch_error_to_string(status));
      return status;
    }
    status = switch_bd_get(device, bd_handle, &bd_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    status =
        switch_pd_tunnel_table_entry_add(device,
                                         handle_to_id(bd_handle),
                                         tunnel_info->tunnel_vni,
                                         0x0,
                                         SWITCH_TUNNEL_PD_TYPE_IP,
                                         bd_info,
                                         tunnel_info->ingress_tunnel_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel create failed on device %d vrf handle 0x%lx: "
          "tunnel table ingress add failed:(%s)\n",
          device,
          vrf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_TUNNEL_TYPE_IP(tunnel_info)) {
    status = switch_pd_tunnel_rewrite_table_entry_add(
        device,
        handle_to_id(handle),
        &api_tunnel_info->src_ip,
        &tunnel_info->src_ip_rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel create failed on device %d: "
          "tunnel src ip rewrite table add failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  *tunnel_handle = handle;

  SWITCH_LOG_DEBUG(
      "tunnel created on device %d tunnel handle 0x%lx\n", device, handle);

  return status;
}

switch_status_t switch_api_tunnel_delete_internal(
    const switch_device_t device, const switch_handle_t tunnel_handle) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if (!SWITCH_TUNNEL_HANDLE(tunnel_handle)) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel handle invalid:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_ARRAY_COUNT(&tunnel_info->tunnel_term_array)) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel term array is still used:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_TUNNEL_TYPE_IP(tunnel_info)) {
    status = switch_pd_tunnel_rewrite_table_entry_delete(
        device, tunnel_info->src_ip_rewrite_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel delete failed on device %d tunnel handle 0x%lx: "
          "tunnel src ip rewrite delete failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if ((tunnel_info->tunnel_type == SWITCH_TUNNEL_TYPE_IPIP) ||
      (tunnel_info->tunnel_type == SWITCH_TUNNEL_TYPE_GRE)) {
    tunnel_info->tunnel_vni -= SWITCH_TUNNEL_VNI_OFFSET;

    status = switch_pd_tunnel_table_entry_delete(
        device, tunnel_info->ingress_tunnel_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel delete failed on device %d tunnel handle 0x%lx: "
          "tunnel table ingress delete failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_api_id_allocator_release(
        device, tunnel_ctx->tunnel_vni_allocator, tunnel_info->tunnel_vni);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel delete failed on device %d tunnel handle 0x%lx: "
          "tunnel id allocator release failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
    tunnel_info->tunnel_vni = 0;
  }

  status = switch_tunnel_handle_delete(device, tunnel_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("tunnel deleted on device %d tunnel handle 0x%lx\n",
                   device,
                   tunnel_handle);

  return status;
}

switch_status_t switch_api_tunnel_info_get_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    switch_api_tunnel_info_t *api_tunnel_info) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel get failed on device %d tunnel handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if (!SWITCH_TUNNEL_HANDLE(tunnel_handle)) {
    SWITCH_LOG_ERROR(
        "tunnel get failed on device %d tunnel handle 0x%lx: "
        "tunnel handle invalid:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel get failed on device %d tunnel handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  memcpy(api_tunnel_info,
         &tunnel_info->api_tunnel_info,
         sizeof(switch_api_tunnel_info_t));

  return status;
}

switch_status_t switch_api_tunnel_interface_get_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    switch_handle_t *intf_handle) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if (!SWITCH_TUNNEL_HANDLE(tunnel_handle)) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel handle invalid:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel delete failed on device %d tunnel handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  *intf_handle = tunnel_info->intf_handle;
  return status;
}

switch_status_t switch_api_tunnel_term_create_internal(
    const switch_device_t device,
    const switch_api_tunnel_term_info_t *api_tunnel_term_info,
    switch_handle_t *tunnel_term_handle) {
  switch_tunnel_term_info_t *tunnel_term_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_vni_t tunnel_vni = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_tunnel_term_info);

  SWITCH_LOG_DEBUG(
      "tunnel term create for tunnel type %s: "
      "vrf handle: 0x%lx",
      switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type),
      api_tunnel_term_info->vrf_handle);

  handle = switch_tunnel_term_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d: "
        "handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  tunnel_handle = api_tunnel_term_info->tunnel_handle;

  status = switch_tunnel_term_get(device, handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d: "
        "tunnel term get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if (!SWITCH_TUNNEL_HANDLE(tunnel_handle)) {
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d tunnel handle 0x%lx: "
        "tunnel handle invalid:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d tunnel handle 0x%lx"
        "tunnel get failed(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_INSERT(
      &tunnel_info->tunnel_term_array, handle, (void *)tunnel_term_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if ((api_tunnel_term_info->tunnel_type == SWITCH_TUNNEL_TYPE_IPIP) ||
      (api_tunnel_term_info->tunnel_type == SWITCH_TUNNEL_TYPE_GRE)) {
    tunnel_vni = tunnel_info->tunnel_vni;
  }

  status = switch_pd_tunnel_ip_dst_table_entry_add(
      device,
      handle_to_id(api_tunnel_term_info->vrf_handle),
      &api_tunnel_term_info->dst_ip,
      tunnel_info->ingress_tunnel_type,
      api_tunnel_term_info->term_entry_type,
      tunnel_vni,
      &tunnel_term_info->dst_vtep_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d tunnel handle 0x%lx "
        "dst_vtep hw program failed for tunnel type %s:(%s)\n",
        device,
        tunnel_handle,
        switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type),
        switch_error_to_string(status));
  }

  if (api_tunnel_term_info->term_entry_type ==
      SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P) {
    status = switch_pd_tunnel_ip_src_table_entry_add(
        device,
        handle_to_id(api_tunnel_term_info->vrf_handle),
        &api_tunnel_term_info->src_ip,
        tunnel_info->ingress_tunnel_type,
        0x0,
        &tunnel_term_info->src_vtep_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel term create failed on device %d, tunnel_handle 0x%lx",
          "src_vtep hw add failed: %s",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_MEMCPY(&tunnel_term_info->api_tunnel_term_info,
                api_tunnel_term_info,
                sizeof(switch_api_tunnel_term_info_t));

  *tunnel_term_handle = handle;

  return status;
}

switch_status_t switch_api_tunnel_term_get_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_term_handle,
    switch_api_tunnel_term_info_t *api_term_info) {
  switch_tunnel_term_info_t *tunnel_term_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_TERM_HANDLE(tunnel_term_handle));
  status =
      switch_tunnel_term_get(device, tunnel_term_handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term get failed on device %d: "
        "tunnel term get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(api_term_info,
                &tunnel_term_info->api_tunnel_term_info,
                sizeof(switch_api_tunnel_term_info_t));
  return status;
}

switch_status_t switch_api_tunnel_term_delete_internal(
    const switch_device_t device, switch_handle_t tunnel_term_handle) {
  switch_tunnel_term_info_t *tunnel_term_info = NULL;
  switch_api_tunnel_term_info_t *api_tunnel_term_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_TERM_HANDLE(tunnel_term_handle));
  status =
      switch_tunnel_term_get(device, tunnel_term_handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term delete failed on device %d: "
        "tunnel term get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  api_tunnel_term_info = &tunnel_term_info->api_tunnel_term_info;
  tunnel_handle = api_tunnel_term_info->tunnel_handle;

  SWITCH_LOG_DEBUG(
      "tunnel term delete on device %d for tunnel type %s: "
      "vrf handle: 0x%lx\n",
      device,
      switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type),
      api_tunnel_term_info->vrf_handle);

  status = switch_pd_tunnel_ip_dst_table_entry_delete(
      device, &api_tunnel_term_info->dst_ip, tunnel_term_info->dst_vtep_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term delete failed on device %d: "
        "dst_vtep hw program failed for tunnel type %s:(%s)\n",
        device,
        switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type),
        switch_error_to_string(status));
    return status;
  }

  if (api_tunnel_term_info->term_entry_type ==
      SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P) {
    status = switch_pd_tunnel_ip_src_table_entry_delete(
        device,
        &api_tunnel_term_info->src_ip,
        tunnel_term_info->src_vtep_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel term delete failed on device %d: "
          "dst_vtep hw program failed for tunnel type %s:(%s)\n",
          device,
          switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type),
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_MEMCPY(&tunnel_term_info->api_tunnel_term_info,
                api_tunnel_term_info,
                sizeof(switch_api_tunnel_term_info_t));
  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term create failed on device %d tunnel handle 0x%lx"
        "tunnel get failed(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_ARRAY_DELETE(&tunnel_info->tunnel_term_array, tunnel_term_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_tunnel_term_handle_delete(device, tunnel_term_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_tunnel_member_add(switch_device_t device,
                                         switch_direction_t direction,
                                         switch_handle_t bd_handle,
                                         switch_handle_t tunnel_handle,
                                         switch_uint64_t flags) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_handle_t member_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(flags);

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if ((!SWITCH_BD_HANDLE(bd_handle)) ||
      (!SWITCH_TUNNEL_HANDLE(tunnel_handle))) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tunnel member add failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member add failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member add failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "bd get failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, tunnel_handle, &bd_member);
  if (status == SWITCH_STATUS_SUCCESS) {
    return status;
  }

  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    status = switch_bd_member_add(device, bd_handle, &member_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel member add failed on device %d tunnel handle 0x%lx "
          "bd handle 0x%lx: "
          "bd member add failed:(%s)\n",
          device,
          tunnel_handle,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_bd_member_get(device, member_handle, &bd_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel member add failed on device %d tunnel handle 0x%lx "
          "bd handle 0x%lx: "
          "bd member get failed:(%s)\n",
          device,
          tunnel_handle,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }

    bd_member->handle = tunnel_handle;
    bd_member->member_handle = member_handle;
    bd_member->bd_handle = bd_handle;

    status =
        switch_mcast_bd_member_rid_allocate(device, bd_handle, tunnel_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel member add failed on device %d tunnel handle 0x%lx "
          "bd handle 0x%lx: "
          "mcast bd member rid allocation failed:(%s)\n",
          device,
          tunnel_handle,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_ASSERT(bd_member != NULL);

  return status;
}

switch_status_t switch_tunnel_member_delete(switch_device_t device,
                                            switch_direction_t direction,
                                            switch_handle_t bd_handle,
                                            switch_handle_t tunnel_handle,
                                            switch_uint64_t flags) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(flags);

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if ((!SWITCH_BD_HANDLE(bd_handle)) ||
      (!SWITCH_TUNNEL_HANDLE(tunnel_handle))) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "bd get failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, tunnel_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "bd member find failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mcast_bd_member_rid_free(device, bd_handle, tunnel_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "rid free failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_delete(
      device, bd_member->bd_handle, bd_member->member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel member delete failed on device %d tunnel handle 0x%lx "
        "bd handle 0x%lx: "
        "rid free failed:(%s)\n",
        device,
        tunnel_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_tunnel_mapper_create_internal(
    const switch_device_t device,
    const switch_api_tunnel_mapper_t *tunnel_mapper,
    switch_handle_t *tunnel_mapper_handle) {
  switch_tunnel_mapper_info_t *tunnel_mapper_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  handle = switch_tunnel_mapper_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel mapper create failed on device %d: "
        "tunnel mapper handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_mapper_get(device, handle, &tunnel_mapper_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper create failed on device %d: "
        "tunnel mapper get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  tunnel_mapper_info->tunnel_map_type = tunnel_mapper->tunnel_map_type;
  *tunnel_mapper_handle = handle;

  SWITCH_LOG_DEBUG(
      "tunnel mapper handle created on device %d "
      "handle 0x%lx map type %s\n",
      device,
      handle,
      switch_tunnel_map_type_to_string(tunnel_mapper_info->tunnel_map_type));

  return status;
}

switch_status_t switch_api_tunnel_mapper_delete_internal(
    const switch_device_t device, const switch_handle_t tunnel_mapper_handle) {
  switch_tunnel_mapper_info_t *tunnel_mapper_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_HANDLE(tunnel_mapper_handle));
  if (!SWITCH_TUNNEL_MAPPER_HANDLE(tunnel_mapper_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tunnel mapper delete failed on device %d mapper handle 0x%lx: "
        "tunnel mapper hang ndle invalid:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_mapper_get(
      device, tunnel_mapper_handle, &tunnel_mapper_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper delete failed on device %d mapper handle 0x%lx: "
        "tunnel mapper get failed:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_ARRAY_COUNT(&tunnel_mapper_info->mapper_array)) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "tunnel mapper delete failed on device %d mapper handle 0x%lx: "
        "tunnel mapper is still referenced:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_mapper_handle_delete(device, tunnel_mapper_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("tunnel mapper deleted on device %d handle 0x%lx\n",
                   device,
                   tunnel_mapper_handle);

  return status;
}

switch_status_t switch_tunnel_decap_mapper_entry_add(
    switch_device_t device,
    switch_tunnel_vni_ingress_key_t *ingress_vni_key,
    switch_handle_t bd_handle) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_vni_ingress_entry_t *ingress_vni_entry = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(ingress_vni_key);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry add failed on device %d: "
        "vni %d bd handle 0x%lx: "
        "tunnel context get failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&tunnel_ctx->ingress_tunnel_vni_hashtable,
                                   (void *)ingress_vni_key,
                                   (void **)&ingress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry add failed on device %d: "
        "vni %d bd handle 0x%lx: "
        "tunnel hashtable search failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    ingress_vni_entry->ref_count++;
    return status;
  }

  ingress_vni_entry =
      SWITCH_MALLOC(device, sizeof(switch_tunnel_vni_ingress_entry_t), 0x1);
  if (!ingress_vni_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry add failed on device %d: "
        "vni %d bd handle 0x%lx: "
        "tunnel hashtable search failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(
      ingress_vni_entry, 0x0, sizeof(switch_tunnel_vni_ingress_entry_t));
  SWITCH_MEMCPY(&ingress_vni_entry->vni_key,
                ingress_vni_key,
                sizeof(switch_tunnel_vni_ingress_key_t));

  ingress_vni_entry->bd_handle = bd_handle;
  ingress_vni_entry->ref_count = 1;

  status = SWITCH_HASHTABLE_INSERT(&tunnel_ctx->ingress_tunnel_vni_hashtable,
                                   &(ingress_vni_entry->node),
                                   (void *)ingress_vni_key,
                                   (void *)ingress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry add failed on device %d: "
        "vni %d bd handle 0x%lx: "
        "tunnel hashtable insert failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        bd_handle,
        switch_error_to_string(status));
    SWITCH_FREE(device, ingress_vni_entry);
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_pd_tunnel_table_entry_add(device,
                                            handle_to_id(bd_handle),
                                            ingress_vni_key->tunnel_vni,
                                            0x0,
                                            SWITCH_TUNNEL_PD_TYPE_NON_IP,
                                            bd_info,
                                            ingress_vni_entry->tunnel_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry add failed on device %d: "
        "vni %d bd handle 0x%lx: "
        "tunnel table add failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_tunnel_decap_mapper_entry_delete(
    switch_device_t device, switch_tunnel_vni_ingress_key_t *ingress_vni_key) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_vni_ingress_entry_t *ingress_vni_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(ingress_vni_key);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry delete failed on device %d vni %d:  "
        "tunnel context get failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&tunnel_ctx->ingress_tunnel_vni_hashtable,
                                   (void *)ingress_vni_key,
                                   (void **)&ingress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry delete failed on device %d vni %d: "
        "tunnel hashtable search failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  ingress_vni_entry->ref_count--;
  if (ingress_vni_entry->ref_count > 0) {
    return status;
  }

  status = switch_pd_tunnel_table_entry_delete(
      device, ingress_vni_entry->tunnel_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ingress tunnel mapper entry delete failed on device %d vni %d: "
        "tunnel table entry delete failed:(%s)\n",
        device,
        ingress_vni_key->tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(&tunnel_ctx->ingress_tunnel_vni_hashtable,
                                   (void *)ingress_vni_key,
                                   (void *)ingress_vni_entry);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_FREE(device, ingress_vni_entry);

  return status;
}

switch_status_t switch_tunnel_encap_mapper_entry_add(
    switch_device_t device,
    switch_tunnel_vni_egress_key_t *egress_vni_key,
    switch_vni_t tunnel_vni) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_vni_egress_entry_t *egress_vni_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_VNI_VALID(tunnel_vni));
  SWITCH_ASSERT(egress_vni_key);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry add failed on device %d: "
        "bd handle 0x%lx tunnel vni %d: "
        "tunnel context get failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&tunnel_ctx->egress_tunnel_vni_hashtable,
                                   (void *)egress_vni_key,
                                   (void **)&egress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry add failed on device %d: "
        "bd handle 0x%lx tunnel vni %d: "
        "tunnel hashtable search failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    egress_vni_entry->ref_count++;
    return status;
  }

  egress_vni_entry =
      SWITCH_MALLOC(device, sizeof(switch_tunnel_vni_egress_entry_t), 0x1);
  if (!egress_vni_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry add failed on device %d: "
        "bd handle 0x%lx tunnel vni %d: "
        "egress vni malloc failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(
      egress_vni_entry, 0x0, sizeof(switch_tunnel_vni_egress_entry_t));
  SWITCH_MEMCPY(&egress_vni_entry->vni_key,
                egress_vni_key,
                sizeof(switch_tunnel_vni_egress_key_t));

  egress_vni_entry->tunnel_vni = tunnel_vni;
  egress_vni_entry->ref_count = 1;

  status = SWITCH_HASHTABLE_INSERT(&tunnel_ctx->egress_tunnel_vni_hashtable,
                                   &(egress_vni_entry->node),
                                   (void *)egress_vni_key,
                                   (void *)egress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry add failed on device %d: "
        "bd handle 0x%lx tunnel vni %d: "
        "hashtable insert failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        tunnel_vni,
        switch_error_to_string(status));
    SWITCH_FREE(device, egress_vni_entry);
    return status;
  }

  status = switch_pd_egress_vni_table_entry_add(
      device,
      handle_to_id(egress_vni_key->bd_handle),
      tunnel_vni,
      SWITCH_TUNNEL_PD_TYPE_NON_IP,
      &egress_vni_entry->tunnel_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry add failed on device %d: "
        "bd handle 0x%lx tunnel vni %d: "
        "egress vni table add failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_tunnel_encap_mapper_entry_delete(
    switch_device_t device, switch_tunnel_vni_egress_key_t *egress_vni_key) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_tunnel_vni_egress_entry_t *egress_vni_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(egress_vni_key);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry delete failed on device %d: "
        "bd handle 0x%lx:  "
        "tunnel context get failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&tunnel_ctx->egress_tunnel_vni_hashtable,
                                   (void *)egress_vni_key,
                                   (void **)&egress_vni_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry delete failed on device %d: "
        "bd handle 0x%lx: "
        "tunnel hashtable search failed:(%s)\n",
        device,
        egress_vni_key->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  egress_vni_entry->ref_count--;
  if (egress_vni_entry->ref_count > 0) {
    return status;
  }

  status = switch_pd_egress_vni_table_entry_delete(
      device, egress_vni_entry->tunnel_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "egress tunnel mapper entry delete failed on device %d: "
        "tunnel vni %d: "
        "egress vni table entry delete failed:(%s)\n",
        device,
        egress_vni_entry->tunnel_vni,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(&tunnel_ctx->egress_tunnel_vni_hashtable,
                                   (void *)egress_vni_key,
                                   (void *)egress_vni_entry);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_FREE(device, egress_vni_entry);

  return status;
}

switch_status_t switch_api_tunnel_mapper_entry_create_internal(
    const switch_device_t device,
    const switch_api_tunnel_mapper_entry_t *tunnel_mapper_entry,
    switch_handle_t *tunnel_mapper_entry_handle) {
  switch_tunnel_mapper_entry_info_t *tunnel_mapper_entry_info = NULL;
  switch_tunnel_mapper_info_t *tunnel_mapper_info = NULL;
  switch_tunnel_vni_ingress_key_t ingress_vni_key = {0};
  switch_tunnel_vni_egress_key_t egress_vni_key = {0};
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_mapper_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(tunnel_mapper_entry != NULL);
  tunnel_mapper_handle = tunnel_mapper_entry->tunnel_mapper_handle;

  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_HANDLE(tunnel_mapper_handle));
  status = switch_tunnel_mapper_get(
      device, tunnel_mapper_handle, &tunnel_mapper_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper entry create failed on device %d "
        "mapper handle 0x%lx: "
        "tunnel mapper get failed:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  if (tunnel_mapper_info->tunnel_map_type !=
      tunnel_mapper_entry->tunnel_map_type) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tunnel mapper entry create failed on device %d "
        "mapper handle 0x%lx: "
        "tunnel mapper map type mismatch:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (tunnel_mapper_entry->tunnel_map_type) {
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
      if (!SWITCH_VLAN_HANDLE(tunnel_mapper_entry->vlan_handle)) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry create failed on device %d vlan handle 0x%lx: "
            "vlan handle invalid:(%s)\n",
            device,
            tunnel_mapper_entry->vlan_handle,
            switch_error_to_string(status));
        return status;
      }
      status = switch_bd_handle_get(
          device, tunnel_mapper_entry->vlan_handle, &bd_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI:
      if (!SWITCH_LN_HANDLE(tunnel_mapper_entry->ln_handle)) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry create failed on device %d ln handle 0x%lx: "
            "ln handle invalid:(%s)\n",
            device,
            tunnel_mapper_entry->ln_handle,
            switch_error_to_string(status));
        return status;
      }
      status = switch_bd_handle_get(
          device, tunnel_mapper_entry->ln_handle, &bd_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE:
      if (!SWITCH_VRF_HANDLE(tunnel_mapper_entry->vrf_handle)) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry create failed on device %d vrf handle 0x%lx: "
            "vrf handle invalid:(%s)\n",
            device,
            tunnel_mapper_entry->vrf_handle,
            switch_error_to_string(status));
        return status;
      }
      status = switch_bd_handle_get(
          device, tunnel_mapper_entry->vrf_handle, &bd_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      break;

    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "tunnel mapper entry create failed on device %d: "
          "tunnel map type invalid:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
      break;
  }

  switch (tunnel_mapper_entry->tunnel_map_type) {
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE:
      SWITCH_MEMSET(&ingress_vni_key, 0x0, sizeof(ingress_vni_key));
      ingress_vni_key.tunnel_vni = tunnel_mapper_entry->tunnel_vni;
      status = switch_tunnel_decap_mapper_entry_add(
          device, &ingress_vni_key, bd_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry create failed on device %d:  "
            "tunnel ingress mapper entry add failed:(%s)\n",
            device,
            tunnel_mapper_entry->vrf_handle,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
    case SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI:
    case SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
      SWITCH_MEMSET(&egress_vni_key, 0x0, sizeof(egress_vni_key));
      egress_vni_key.bd_handle = bd_handle;
      status = switch_tunnel_encap_mapper_entry_add(
          device, &egress_vni_key, tunnel_mapper_entry->tunnel_vni);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry create failed on device %d:  "
            "tunnel egress mapper entry add failed:(%s)\n",
            device,
            tunnel_mapper_entry->vrf_handle,
            switch_error_to_string(status));
        return status;
      }
      break;
  }

  handle = switch_tunnel_mapper_entry_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "tunnel mapper entry create failed on device %d: "
        "tunnel mapper entry handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_tunnel_mapper_entry_get(device, handle, &tunnel_mapper_entry_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper entry create failed on device %d: "
        "tunnel mapper entry get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&tunnel_mapper_entry_info->api_tunnel_mapper_entry,
                tunnel_mapper_entry,
                sizeof(switch_api_tunnel_mapper_entry_t));
  *tunnel_mapper_entry_handle = handle;

  SWITCH_LOG_DEBUG(
      "tunnel mapper entry handle created on device %d "
      "tunnel map type %s vlan handle 0x%lx ln handle 0x%lx "
      "vrf handle 0x%lx vni %d\n",
      device,
      switch_tunnel_map_type_to_string(tunnel_mapper_entry->tunnel_map_type),
      tunnel_mapper_entry->vlan_handle,
      tunnel_mapper_entry->ln_handle,
      tunnel_mapper_entry->vrf_handle,
      tunnel_mapper_entry->tunnel_vni);

  return status;
}

switch_status_t switch_api_tunnel_mapper_entry_delete_internal(
    switch_device_t device, switch_handle_t tunnel_mapper_entry_handle) {
  switch_tunnel_mapper_info_t *tunnel_mapper_info = NULL;
  switch_tunnel_mapper_entry_info_t *tunnel_mapper_entry_info = NULL;
  switch_api_tunnel_mapper_entry_t *tunnel_mapper_entry = NULL;
  switch_tunnel_vni_ingress_key_t ingress_vni_key = {0};
  switch_tunnel_vni_egress_key_t egress_vni_key = {0};
  switch_handle_t tunnel_mapper_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_ENTRY_HANDLE(tunnel_mapper_entry_handle));
  if (!SWITCH_TUNNEL_MAPPER_ENTRY_HANDLE(tunnel_mapper_entry_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "tunnel mapper entry delete failed on device %d: "
        "mapper entry handle 0x%lx: "
        "tunnel mapper entry handle invalid:(%s)\n",
        device,
        tunnel_mapper_entry_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_mapper_entry_get(
      device, tunnel_mapper_entry_handle, &tunnel_mapper_entry_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper entry delete failed on device %d: "
        "mapper entry handle 0x%lx: "
        "tunnel mapper entry get failed:(%s)\n",
        device,
        tunnel_mapper_entry_handle,
        switch_error_to_string(status));
    return status;
  }

  tunnel_mapper_entry = &tunnel_mapper_entry_info->api_tunnel_mapper_entry;
  tunnel_mapper_handle = tunnel_mapper_entry->tunnel_mapper_handle;
  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_HANDLE(tunnel_mapper_handle));
  status = switch_tunnel_mapper_get(
      device, tunnel_mapper_handle, &tunnel_mapper_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper entry create failed on device %d "
        "mapper handle 0x%lx: "
        "tunnel mapper get failed:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (tunnel_mapper_entry->tunnel_map_type) {
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
      handle = tunnel_mapper_entry->vlan_handle;
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI:
      handle = tunnel_mapper_entry->ln_handle;
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
      handle = tunnel_mapper_entry->vrf_handle;
      break;
  }

  switch (tunnel_mapper_entry->tunnel_map_type) {
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE:
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE:
      SWITCH_MEMSET(&ingress_vni_key, 0x0, sizeof(ingress_vni_key));
      ingress_vni_key.tunnel_vni = tunnel_mapper_entry->tunnel_vni;
      status =
          switch_tunnel_decap_mapper_entry_delete(device, &ingress_vni_key);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry delete failed on device %d:  "
            "tunnel ingress mapper entry delete failed:(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
    case SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI:
    case SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
      SWITCH_MEMSET(&egress_vni_key, 0x0, sizeof(egress_vni_key));
      status = switch_bd_handle_get(device, handle, &egress_vni_key.bd_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      status = switch_tunnel_encap_mapper_entry_delete(device, &egress_vni_key);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mapper entry delete failed on device %d:  "
            "tunnel egress mapper entry delete failed:(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      break;
  }

  status = switch_tunnel_mapper_entry_handle_delete(device,
                                                    tunnel_mapper_entry_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "tunnel mapper entry handle deleted on device %d "
      "mapper entry handle 0x%lx\n",
      device,
      tunnel_mapper_entry_handle);

  return status;
}

switch_status_t switch_api_tunnel_dest_list_add(
    switch_device_t device, switch_handle_t tunnel_encap_handle) {
  switch_tunnel_context_t *tunnel_ctx;
  switch_status_t status;
  PWord_t PValue;

  SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_encap_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel dest list add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  JLI(PValue, tunnel_ctx->PJLarr_tunnel_dest, tunnel_encap_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR("tunnel dest list add failed on device %d\n", device);
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_dest_list_remove(
    switch_device_t device, switch_handle_t tunnel_encap_handle) {
  switch_tunnel_context_t *tunnel_ctx;
  switch_status_t status;
  int Rc_int;

  SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_encap_handle));

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel dest list add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  JLD(Rc_int, tunnel_ctx->PJLarr_tunnel_dest, tunnel_encap_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR("tunnel dest list add failed on device %d\n", device);
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_encap_create(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t *tunnel_encap_handle) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_tunnel_ip_key_t ip_key = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d nhop handle 0x%lx: "
        "nhop get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  api_nhop_info = &nhop_info->spath.api_nhop_info;
  tunnel_handle = api_nhop_info->tunnel_handle;
  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d nhop handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  vrf_handle = tunnel_info->underlay_vrf_handle;
  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));

  handle = switch_tunnel_encap_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d nhop handle 0x%lx: "
        "tunnel encap handle allocate failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_encap_get(device, handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d nhop handle 0x%lx: "
        "tunnel encap get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_TUNNEL_TYPE_IP(tunnel_info)) {
    SWITCH_MEMSET(&ip_key, 0x0, sizeof(ip_key));
    ip_key.ip_type = SWITCH_TUNNEL_IP_TYPE_DST;
    SWITCH_MEMCPY(
        &ip_key.ip_addr, &api_nhop_info->ip_addr, sizeof(switch_ip_addr_t));
    status = switch_tunnel_ip_rewrite_table_add(
        device, &ip_key, &tunnel_encap_info->tunnel_dip_index);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel encap create failed on device %d tunnel handle 0x%lx: "
          "tunnel dst ip rewrite table add failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  tunnel_encap_info->nhop_handle = nhop_handle;
  tunnel_encap_info->tunnel_handle = tunnel_handle;
  tunnel_encap_info->vrf_handle = vrf_handle;
  SWITCH_MEMCPY(&tunnel_encap_info->dst_ip,
                &api_nhop_info->ip_addr,
                sizeof(switch_ip_addr_t));

  tunnel_encap_info->mgid_info.tunnel_encap_handle = handle;
  SET_TUNNEL_MGID_STATE(tunnel_encap_info, switch_api_tunnel_mgid_state_init);

  status = switch_api_tunnel_dest_list_add(device, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d tunnel handle 0x%lx: "
        "tunnel dest list add failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SEND_TUNNEL_MGID_EVENT(
      tunnel_encap_info, device, SWITCH_TUNNEL_CREATE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap create failed on device %d nhop handle 0x%lx: "
        "tunnel mgid tunnel create send event failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  *tunnel_encap_handle = handle;

  return status;
}

switch_status_t switch_api_tunnel_encap_delete(
    switch_device_t device, switch_handle_t tunnel_encap_handle) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_tunnel_ip_key_t ip_key = {0};
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_encap_handle));
  status =
      switch_tunnel_encap_get(device, tunnel_encap_handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap delete failed on device %d tunnel encap handle 0x%lx: "
        "tunnel encap get failed:(%s)\n",
        device,
        tunnel_encap_handle,
        switch_error_to_string(status));
    return status;
  }

  nhop_handle = tunnel_encap_info->nhop_handle;
  tunnel_handle = tunnel_encap_info->tunnel_handle;

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap delete failed on device %d nhop handle 0x%lx: "
        "nhop get failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  api_nhop_info = &nhop_info->spath.api_nhop_info;

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap delete failed on device %d tunnel handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_TUNNEL_TYPE_IP(tunnel_info)) {
    SWITCH_MEMSET(&ip_key, 0x0, sizeof(ip_key));
    ip_key.ip_type = SWITCH_TUNNEL_IP_TYPE_DST;
    SWITCH_MEMCPY(
        &ip_key.ip_addr, &api_nhop_info->ip_addr, sizeof(switch_ip_addr_t));
    status = switch_tunnel_ip_rewrite_table_delete(device, &ip_key);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel encap delete failed on device %d tunnel handle 0x%lx: "
          "tunnel dst ip rewrite table delete failed:(%s)\n",
          device,
          tunnel_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = SEND_TUNNEL_MGID_EVENT(
      tunnel_encap_info, device, SWITCH_TUNNEL_DELETE, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap delete failed on device %d nhop handle 0x%lx: "
        "tunnel mgid tunnel delete event send failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_tunnel_dest_list_remove(device, tunnel_encap_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap delete failed on device %d tunnel handle 0x%lx: "
        "tunnel dest list add failed:(%s)\n",
        device,
        tunnel_encap_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_encap_handle_delete(device, tunnel_encap_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_tunnel_mapper_create(
    const switch_device_t device,
    const switch_api_tunnel_mapper_t *tunnel_mapper,
    switch_handle_t *tunnel_mapper_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_mapper_create_internal(
      device, tunnel_mapper, tunnel_mapper_handle));
}

switch_status_t switch_api_tunnel_mapper_delete(
    const switch_device_t device, const switch_handle_t tunnel_mapper_handle) {
  SWITCH_MT_WRAP(
      switch_api_tunnel_mapper_delete_internal(device, tunnel_mapper_handle));
}

switch_status_t switch_api_tunnel_mapper_entry_create(
    const switch_device_t device,
    const switch_api_tunnel_mapper_entry_t *tunnel_mapper_entry,
    switch_handle_t *tunnel_mapper_entry_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_mapper_entry_create_internal(
      device, tunnel_mapper_entry, tunnel_mapper_entry_handle));
}

switch_status_t switch_api_tunnel_mapper_entry_delete(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_entry_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_mapper_entry_delete_internal(
      device, tunnel_mapper_entry_handle));
}

switch_status_t switch_api_tunnel_create(
    const switch_device_t device,
    const switch_api_tunnel_info_t *api_tunnel_info,
    switch_handle_t *tunnel_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_create_internal(
      device, api_tunnel_info, tunnel_handle));
}

switch_status_t switch_api_tunnel_delete(const switch_device_t device,
                                         const switch_handle_t tunnel_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_delete_internal(device, tunnel_handle));
}

switch_status_t switch_api_tunnel_info_get(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    switch_api_tunnel_info_t *tunnel_info) {
  SWITCH_MT_WRAP(
      switch_api_tunnel_info_get_internal(device, tunnel_handle, tunnel_info));
}

switch_status_t switch_api_tunnel_interface_get(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_interface_get_internal(
      device, tunnel_handle, intf_handle));
}

switch_status_t switch_api_tunnel_term_create(
    const switch_device_t device,
    const switch_api_tunnel_term_info_t *api_term_info,
    switch_handle_t *term_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_term_create_internal(
      device, api_term_info, term_handle));
}

switch_status_t switch_api_tunnel_term_delete(const switch_device_t device,
                                              switch_handle_t term_handle) {
  SWITCH_MT_WRAP(switch_api_tunnel_term_delete_internal(device, term_handle));
}

switch_status_t switch_api_tunnel_term_get(
    const switch_device_t device,
    const switch_handle_t term_handle,
    switch_api_tunnel_term_info_t *api_term_info) {
  SWITCH_MT_WRAP(
      switch_api_tunnel_term_get_internal(device, term_handle, api_term_info));
}

switch_status_t switch_api_tunnel_dest_list_get(
    switch_device_t device,
    switch_handle_t **tunnel_handle_list,
    switch_uint32_t *tunnel_handle_count,
    switch_handle_t route_handle) {
  switch_tunnel_context_t *tunnel_ctx;
  int Rc_int, count;
  switch_status_t status;
  PWord_t PValue;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel dest list add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  JLC(Rc_int, tunnel_ctx->PJLarr_tunnel_dest, 0, -1);
  *tunnel_handle_count = Rc_int;

  if (Rc_int) {
    *tunnel_handle_list =
        SWITCH_MALLOC(device, sizeof(switch_handle_t), Rc_int);

    for (count = 1; count <= Rc_int; count++) {
      JLBC(PValue,
           tunnel_ctx->PJLarr_tunnel_dest,
           count,
           (*tunnel_handle_list)[count - 1]);
    }
  }

  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_tunnel_add_reachability(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t route_handle,
    switch_tunnel_encap_info_t *tunnel_encap_info) {
  switch_status_t status;

  status = switch_api_nhop_send_mgid_event(
      device,
      nhop_handle,
      SWITCH_TUNNEL_CREATE,
      (void *)TUNNEL_MGID_TUNNEL_HANDLE(tunnel_encap_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel add reachability failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_l3_route_send_mgid_event(
      device,
      route_handle,
      SWITCH_TUNNEL_CREATE,
      (void *)TUNNEL_MGID_TUNNEL_HANDLE(tunnel_encap_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel add reachability failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

static switch_status_t switch_api_tunnel_remove_reachability(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t route_handle,
    switch_tunnel_encap_info_t *tunnel_encap_info) {
  switch_status_t status;

  status = switch_api_nhop_send_mgid_event(
      device,
      nhop_handle,
      SWITCH_TUNNEL_DELETE,
      (void *)TUNNEL_MGID_TUNNEL_HANDLE(tunnel_encap_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel remove reachability failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_l3_route_send_mgid_event(
      device,
      route_handle,
      SWITCH_TUNNEL_DELETE,
      (void *)TUNNEL_MGID_TUNNEL_HANDLE(tunnel_encap_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel remove reachability failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

static switch_status_t switch_api_tunnel_mgid_add(
    switch_device_t device,
    switch_handle_t mgid_handle,
    switch_tunnel_encap_info_t *tunnel_encap_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t mirror_handle = SWITCH_API_INVALID_HANDLE;

  status =
      switch_pd_tunnel_mgid_entry_add(device,
                                      tunnel_encap_info->tunnel_dip_index,
                                      handle_to_id(mgid_handle),
                                      &tunnel_encap_info->tunnel_mgid_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mgid add failed on device %d"
        "intf info %lx: , mgid handle %lx: ",
        device,
        tunnel_encap_info,
        mgid_handle);
    return status;
  }

  /* Loop through all the mirror sessions using this
  tunnel and update the mgid associated with them */
  JLF(PValue, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  while (PValue != NULL) {
    status = switch_api_mirror_session_update_mgid(
        device, mirror_handle, mgid_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel mgid add %d"
          "intf info %lx: "
          "with status (%s)\n",
          device,
          tunnel_encap_info,
          switch_error_to_string(status));
      return status;
    }

    JLN(PValue, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  }

  return status;
}

static switch_status_t switch_api_tunnel_mgid_remove(
    switch_device_t device, switch_tunnel_encap_info_t *tunnel_encap_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t *PValue = NULL;
  switch_handle_t mirror_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_pd_tunnel_mgid_entry_delete(
      device, tunnel_encap_info->tunnel_mgid_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mgid remove %d"
        "intf info %lx: "
        "with status (%s)\n",
        device,
        tunnel_encap_info,
        switch_error_to_string(status));
  }

  /* Loop through all the mirror sessions using this
  tunnel and update the mgid associated with them */
  JLF(PValue, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  while (PValue != NULL) {
    status = switch_api_mirror_session_update_mgid(
        device, mirror_handle, SWITCH_API_INVALID_HANDLE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "tunnel mgid remove %d"
          "intf info %lx: "
          "with status (%s)\n",
          device,
          tunnel_encap_info,
          switch_error_to_string(status));
      return status;
    }

    JLN(PValue, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  }

  return status;
}

switch_status_t switch_api_tunnel_mgid_state_dont_handle(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_api_route_entry_t route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mgid_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status;

  tunnel_encap_info = (switch_tunnel_encap_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_CREATE:
      /* Look up tunnel destination */
      SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
      route_entry.nhop_handle = nhop_handle;
      route_entry.vrf_handle = tunnel_encap_info->vrf_handle;
      SWITCH_MEMCPY(&route_entry.ip_address,
                    &(tunnel_encap_info->dst_ip),
                    sizeof(switch_ip_addr_t));

      status = switch_api_l3_route_lookup(device, &route_entry, &nhop_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        /* Tunnel route is not reachable */
        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                              switch_api_tunnel_mgid_state_no_mgid);
        return SWITCH_STATUS_SUCCESS;
      }

      TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = nhop_handle;

      status = switch_api_l3_route_handle_lookup(
          device, &route_entry, &route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("tunnel mgid state init failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }

      TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = route_handle;

      /* Tunnel route is reachable. Update the nhop and route objects */
      status = switch_api_tunnel_add_reachability(
          device, nhop_handle, route_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("tunnel mgid state init failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        /* NHOP not found for SVI/L3 tunnel. Reset state to no mgid */
        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                              switch_api_tunnel_mgid_state_no_mgid);
        return SWITCH_STATUS_SUCCESS;
      }

      SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                            switch_api_tunnel_mgid_state_mgid_associated);
      break;

    case SWITCH_MGID_ADD:
      mgid_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));

      status =
          switch_api_tunnel_mgid_add(device, mgid_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("tunnel mgid state init failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }

      break;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_api_route_entry_t route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status;
  switch_handle_t mgid_handle = SWITCH_API_INVALID_HANDLE;

  tunnel_encap_info = (switch_tunnel_encap_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_DELETE:
      SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                            switch_api_tunnel_mgid_state_init);
      break;

    case SWITCH_ROUTE_ADD:
      /* Look up tunnel destination */
      SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
      route_entry.nhop_handle = nhop_handle;
      route_entry.vrf_handle = tunnel_encap_info->vrf_handle;
      SWITCH_MEMCPY(&route_entry.ip_address,
                    &(tunnel_encap_info->dst_ip),
                    sizeof(switch_ip_addr_t));

      status = switch_api_l3_route_lookup(device, &route_entry, &nhop_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        return SWITCH_STATUS_SUCCESS;
      }

      TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = nhop_handle;

      status = switch_api_l3_route_handle_lookup(
          device, &route_entry, &route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("tunnel mgid state no mgid failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }

      TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = route_handle;

      /* Tunnel route is reachable. Update the nhop and route objects */
      status = switch_api_tunnel_add_reachability(
          device, nhop_handle, route_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        SWITCH_LOG_DEBUG(
            "tunnel no mgid state on device %d nhop handle 0x%lx "
            "setting tunnel mgid state to no mgid\n",
            device,
            nhop_handle);
        return SWITCH_STATUS_SUCCESS;
      }

      SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                            switch_api_tunnel_mgid_state_mgid_associated);
      break;

    case SWITCH_MGID_ADD:
      mgid_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));

      status =
          switch_api_tunnel_mgid_add(device, mgid_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("tunnel mgid state init failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      break;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_api_route_entry_t route_entry;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mgid_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status;

  tunnel_encap_info = (switch_tunnel_encap_info_t *)info;

  switch (event) {
    case SWITCH_TUNNEL_DELETE:
      status = switch_api_tunnel_remove_reachability(
          device,
          TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info),
          TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info),
          tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                            switch_api_tunnel_mgid_state_init);
      break;

    case SWITCH_ROUTE_ADD:
      /* Check if reachability has changed */
      SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
      route_entry.nhop_handle = nhop_handle;
      route_entry.vrf_handle = tunnel_encap_info->vrf_handle;
      SWITCH_MEMCPY(&route_entry.ip_address,
                    &(tunnel_encap_info->dst_ip),
                    sizeof(switch_ip_addr_t));

      status = switch_api_l3_route_handle_lookup(
          device, &route_entry, &route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      if (route_handle != TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info)) {
        /* Better route to reach the tunnel, use this route and nexthop */
        status = switch_api_tunnel_remove_reachability(
            device,
            TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info),
            TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info),
            tunnel_encap_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "tunnel mgid state mgid associated failed on device %d: %s\n",
              device,
              switch_error_to_string(status));
          return status;
        }

        status = switch_api_l3_route_lookup(device, &route_entry, &nhop_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "tunnel mgid state mgid associated failed on device %d: %s\n",
              device,
              switch_error_to_string(status));
          return status;
        }

        status = switch_api_tunnel_add_reachability(
            device, nhop_handle, route_handle, tunnel_encap_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "tunnel mgid state mgid associated failed on device %d: %s\n",
              device,
              switch_error_to_string(status));
          return status;
        }

        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = nhop_handle;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = route_handle;
      }
      break;

    case SWITCH_ROUTE_REMOVE:
      /* Remove the current tunnel to <route, nhop> association */
      status = switch_api_tunnel_remove_reachability(
          device,
          TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info),
          TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info),
          tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      /* Check if there is still reachability */
      SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
      route_entry.nhop_handle = nhop_handle;
      route_entry.vrf_handle = tunnel_encap_info->vrf_handle;
      SWITCH_MEMCPY(&route_entry.ip_address,
                    &(tunnel_encap_info->dst_ip),
                    sizeof(switch_ip_addr_t));

      status = switch_api_l3_route_lookup(device, &route_entry, &nhop_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                              switch_api_tunnel_mgid_state_no_mgid);
        return SWITCH_STATUS_SUCCESS;
      }

      if (nhop_handle == TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info)) {
        TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = SWITCH_API_INVALID_HANDLE;
        SET_TUNNEL_MGID_STATE(tunnel_encap_info,
                              switch_api_tunnel_mgid_state_no_mgid);
        SWITCH_LOG_DEBUG(
            "tunnel mgid associated on device %d nhop handle 0x%lx "
            "setting tunnel mgid state to no mgid\n",
            device,
            nhop_handle);
        return SWITCH_STATUS_SUCCESS;
      }

      TUNNEL_MGID_NHOP_HANDLE(tunnel_encap_info) = nhop_handle;

      status = switch_api_l3_route_handle_lookup(
          device, &route_entry, &route_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }

      TUNNEL_MGID_ROUTE_HANDLE(tunnel_encap_info) = route_handle;

      status = switch_api_tunnel_add_reachability(
          device, nhop_handle, route_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_MGID_ADD:
      mgid_handle = (switch_handle_t)event_arg;

      SWITCH_ASSERT(SWITCH_MGID_HANDLE(mgid_handle));

      status =
          switch_api_tunnel_mgid_add(device, mgid_handle, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_MGID_REMOVE:
      status = switch_api_tunnel_mgid_remove(device, tunnel_encap_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "tunnel mgid state mgid associated failed on device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      break;

    default:
      SWITCH_ASSERT(FALSE);
  };

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_send_mgid_event(
    switch_device_t device,
    switch_handle_t tunnel_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_status_t status;

  SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_handle));
  status = switch_tunnel_encap_get(device, tunnel_handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("tunnel send mgid event failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SEND_TUNNEL_MGID_EVENT(tunnel_encap_info, device, event, event_arg);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel send mgid event failed on device %d "
        "with error: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_mirror_list_add(
    switch_device_t device,
    switch_handle_t tunnel_encap_handle,
    switch_handle_t mirror_handle) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  PWord_t PValue;

  status =
      switch_tunnel_encap_get(device, tunnel_encap_handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel add mirror failed on device %d"
        "intf info %lx: , mirror handle %lx: ",
        device,
        tunnel_encap_info,
        mirror_handle);
    return SWITCH_STATUS_FAILURE;
  }

  JLI(PValue, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  if (PValue == PJERR) {
    SWITCH_LOG_ERROR(
        "tunnel add mirror failed on device %d"
        "intf info %lx: , mirror handle %lx: ",
        device,
        tunnel_encap_info,
        mirror_handle);
    return SWITCH_STATUS_FAILURE;
  }

  TUNNEL_NUM_MIRRORS(tunnel_encap_info) += 1;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_mirror_list_remove(
    switch_device_t device,
    switch_handle_t tunnel_encap_handle,
    switch_handle_t mirror_handle) {
  int Rc_int;
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status =
      switch_tunnel_encap_get(device, tunnel_encap_handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel remove mirror failed on device %d"
        "intf info %lx: , mirror handle %lx: ",
        device,
        tunnel_encap_info,
        mirror_handle);
    return SWITCH_STATUS_FAILURE;
  }

  JLD(Rc_int, TUNNEL_MIRROR_LIST(tunnel_encap_info), mirror_handle);
  if (Rc_int != 1) {
    SWITCH_LOG_ERROR(
        "tunnel remove mirror failed on device %d"
        "intf info %lx: , mirror handle %lx: ",
        device,
        tunnel_encap_info,
        mirror_handle);
    return SWITCH_STATUS_FAILURE;
  }

  TUNNEL_NUM_MIRRORS(tunnel_encap_info) -= 1;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_tunnel_underlay_vrf_handle_get(
    switch_device_t device,
    switch_handle_t tunnel_handle,
    switch_handle_t *vrf_handle) {
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel underlay vrf get failed on device %d tunnel handle 0x%lx",
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  *vrf_handle = tunnel_info->underlay_vrf_handle;
  return status;
}
