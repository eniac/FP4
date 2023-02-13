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

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_TUNNEL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_tunnel_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx) {
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_api_tunnel_info_t *api_tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel dump failed on device %d tunnel handle 0x%lx: "
        "tunnel get failed:(%s)\n",
        device,
        tunnel_handle,
        switch_error_to_string(status));
    return status;
  }

  api_tunnel_info = &tunnel_info->api_tunnel_info;

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx, "\ttunnel handle: 0x%lx\n", tunnel_handle);
  SWITCH_PRINT(cli_ctx, "\t\tentry type: %d\n", api_tunnel_info->entry_type);
  SWITCH_PRINT(cli_ctx, "\t\tip type: %d\n", api_tunnel_info->ip_type);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tip address %s\n",
               switch_ipaddress_to_string(&api_tunnel_info->src_ip));
  SWITCH_PRINT(cli_ctx,
               "\t\tdecap mapper handle: 0x%lx\n",
               api_tunnel_info->decap_mapper_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tencaps mapper handle: 0x%lx\n",
               api_tunnel_info->encap_mapper_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tunderlay rif handle: 0x%lx\n",
               api_tunnel_info->underlay_rif_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\toverlay rif handle: 0x%lx\n",
               api_tunnel_info->overlay_rif_handle);

  SWITCH_PRINT(
      cli_ctx, "\t\tunderlay vrf: 0x%lx\n", tunnel_info->underlay_vrf_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\ttunnel type: %s\n",
               switch_tunnel_type_to_string(tunnel_info->tunnel_type));
  SWITCH_PRINT(cli_ctx, "\t\tsip index: %d\n", tunnel_info->sip_index);
  SWITCH_PRINT(cli_ctx, "\t\ttunnel vni: %d\n", tunnel_info->tunnel_vni);
  SWITCH_PRINT(cli_ctx, "\t\tintf handle: 0x%lx\n", tunnel_info->intf_handle);
  SWITCH_PRINT(
      cli_ctx,
      "\t\tingress tunnel type: %s\n",
      switch_tunnel_ingress_type_to_string(tunnel_info->ingress_tunnel_type));
  SWITCH_PRINT(
      cli_ctx,
      "\t\tegress tunnel type: %s\n",
      switch_tunnel_egress_type_to_string(tunnel_info->egress_tunnel_type));
  SWITCH_PRINT(cli_ctx, "\n");
  return status;
}

switch_status_t switch_api_tunnel_term_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_term_handle,
    const void *cli_ctx) {
  switch_tunnel_term_info_t *tunnel_term_info = NULL;
  switch_api_tunnel_term_info_t *api_tunnel_term_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_TERM_HANDLE(tunnel_term_handle));
  status =
      switch_tunnel_term_get(device, tunnel_term_handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel term dump failed on device %d tunnel handle 0x%lx: "
        "tunnel term get failed:(%s)\n",
        device,
        tunnel_term_handle,
        switch_error_to_string(status));
    return status;
  }

  api_tunnel_term_info = &tunnel_term_info->api_tunnel_term_info;

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx, "\ttunnel term handle: 0x%lx\n", tunnel_term_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle: 0x%lx\n", api_tunnel_term_info->vrf_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tsrc ip: %s\n",
               switch_ipaddress_to_string(&api_tunnel_term_info->src_ip));
  SWITCH_PRINT(cli_ctx,
               "\t\t\tdst ip: %s\n",
               switch_ipaddress_to_string(&api_tunnel_term_info->dst_ip));
  SWITCH_PRINT(cli_ctx,
               "\t\ttunnel type: %s\n",
               switch_tunnel_type_to_string(api_tunnel_term_info->tunnel_type));
  return status;
}

switch_status_t switch_api_tunnel_mapper_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_handle,
    const void *cli_ctx) {
  switch_tunnel_mapper_info_t *tunnel_mapper_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_HANDLE(tunnel_mapper_handle));
  status = switch_tunnel_mapper_get(
      device, tunnel_mapper_handle, &tunnel_mapper_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper dump failed on device %d mapper handle 0x%lx: "
        "tunnel mapper get failed:(%s)\n",
        device,
        tunnel_mapper_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(
      cli_ctx, "\ttunnel mapper handle: 0x%lx\n", tunnel_mapper_handle);
  SWITCH_PRINT(
      cli_ctx,
      "\t\ttunnel map type: %s\n",
      switch_tunnel_map_type_to_string(tunnel_mapper_info->tunnel_map_type));
  SWITCH_PRINT(cli_ctx, "\n");

  return status;
}

switch_status_t switch_api_tunnel_mapper_entry_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_entry_handle,
    const void *cli_ctx) {
  switch_api_tunnel_mapper_entry_t *tunnel_mapper_entry = NULL;
  switch_tunnel_mapper_entry_info_t *tunnel_mapper_entry_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_MAPPER_ENTRY_HANDLE(tunnel_mapper_entry_handle));
  status = switch_tunnel_mapper_get(
      device, tunnel_mapper_entry_handle, &tunnel_mapper_entry_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel mapper entry dump failed on device %d mapper handle 0x%lx: "
        "tunnel mapper entry get failed:(%s)\n",
        device,
        tunnel_mapper_entry_handle,
        switch_error_to_string(status));
    return status;
  }

  tunnel_mapper_entry = &tunnel_mapper_entry_info->api_tunnel_mapper_entry;
  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx,
               "\ttunnel mapper entry handle: 0x%lx\n",
               tunnel_mapper_entry_handle);
  SWITCH_PRINT(
      cli_ctx,
      "\t\ttunnel map type: %s\n",
      switch_tunnel_map_type_to_string(tunnel_mapper_entry->tunnel_map_type));
  SWITCH_PRINT(
      cli_ctx, "\t\tvlan handle 0x%lx\n", tunnel_mapper_entry->vlan_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tln handle 0x%lx\n", tunnel_mapper_entry->ln_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle 0x%lx\n", tunnel_mapper_entry->vrf_handle);
  SWITCH_PRINT(cli_ctx, "\t\tvni %d\n", tunnel_mapper_entry->tunnel_vni);
  SWITCH_PRINT(cli_ctx, "\n");
  return status;
}

switch_status_t switch_api_tunnel_encap_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t tunnel_encap_handle,
    const void *cli_ctx) {
  switch_tunnel_encap_info_t *tunnel_encap_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_ENCAP_HANDLE(tunnel_encap_handle));
  status =
      switch_tunnel_encap_get(device, tunnel_encap_handle, &tunnel_encap_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel encap dump failed on device %d encap handle 0x%lx: "
        "tunnel mapper entry get failed:(%s)\n",
        device,
        tunnel_encap_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx, "\ttunnel encap handle: 0x%lx\n", tunnel_encap_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tnhop handle: 0x%lx\n", tunnel_encap_info->nhop_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\ttunnel handle: 0x%lx\n", tunnel_encap_info->tunnel_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tvrf handle: 0x%lx\n", tunnel_encap_info->vrf_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\ttunnel dip index: %d\n",
               tunnel_encap_info->tunnel_dip_index);
  SWITCH_PRINT(cli_ctx, "\n");
  return status;
}

switch_status_t switch_api_tunnel_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_tunnel_context_t *tunnel_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_TUNNEL, (void **)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "tunnel context dump failed on device %d: "
        "tunnel context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tTunnel Context:\n");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->src_ip_hashtable, "Source IP");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->dst_ip_hashtable, "Destination IP");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->ingress_tunnel_vni_hashtable, "Ingress VNI");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->egress_tunnel_vni_hashtable, "Egress VNI");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->src_vtep_hashtable, "Source Vtep");
  SWITCH_CLI_HASHTABLE_PRINT(
      cli_ctx, tunnel_ctx->dst_vtep_hashtable, "Destination Vtep");

  SWITCH_PRINT(cli_ctx, "\n");

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_tunnel_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_tunnel_handle_dump_internal(device, tunnel_handle, cli_ctx));
}

switch_status_t switch_api_tunnel_term_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_term_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_tunnel_term_handle_dump_internal(
      device, tunnel_term_handle, cli_ctx));
}

switch_status_t switch_api_tunnel_mapper_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_tunnel_mapper_handle_dump_internal(
      device, tunnel_mapper_handle, cli_ctx));
}

switch_status_t switch_api_tunnel_mapper_entry_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_entry_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_tunnel_mapper_entry_handle_dump_internal(
      device, tunnel_mapper_entry_handle, cli_ctx));
}

switch_status_t switch_api_tunnel_encap_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_encap_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_tunnel_encap_handle_dump_internal(
      device, tunnel_encap_handle, cli_ctx));
}

switch_status_t switch_api_tunnel_context_dump(const switch_device_t device,
                                               const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_tunnel_context_dump_internal(device, cli_ctx));
}
