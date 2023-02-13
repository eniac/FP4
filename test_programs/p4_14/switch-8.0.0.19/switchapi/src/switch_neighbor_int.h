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

#ifndef __SWITCH_NEIGHBOR_INT_H__
#define __SWITCH_NEIGHBOR_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_TUNNEL_DMAC_REWRITE_HASH_SEED 0x12345678

#define SWITCH_NEIGHBOR_HASH_SEED 0x12345678

#define switch_neighbor_handle_create(_device) \
  switch_handle_create(                        \
      _device, SWITCH_HANDLE_TYPE_NEIGHBOR, sizeof(switch_neighbor_info_t))

#define switch_neighbor_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_NEIGHBOR, _handle)

#define switch_neighbor_get(_device, _handle, _info) \
  switch_handle_get(                                 \
      _device, SWITCH_HANDLE_TYPE_NEIGHBOR, _handle, (void **)_info)

#define SWITCH_NEIGHBOR_TABLE_HASH_KEY_SIZE \
  sizeof(switch_handle_t) + sizeof(switch_mac_addr_t)

#define SWITCH_DMAC_REWRITE_HASH_KEY_SIZE sizeof(switch_mac_addr_t)

#define SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS(neigh_type)                        \
  neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_L3VPN ||              \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_L3VPN || \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_L3VPN || \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_PUSH_L3VPN ||     \
      neigh_type ==                                                         \
          SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_PUSH_L3VPN ||      \
      neigh_type ==                                                         \
          SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_PUSH_L3VPN ||      \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN ||          \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L2VPN || \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L2VPN || \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN ||          \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L3VPN || \
      neigh_type == SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L3VPN

static inline char *switch_neighbor_type_to_string(
    switch_neighbor_type_t neigh_type) {
  switch (neigh_type) {
    case SWITCH_NEIGHBOR_TYPE_IP:
      return "ip";
    case SWITCH_NEIGHBOR_TYPE_NHOP:
      return "nhop";
    default:
      return "none";
  }
}

static inline char *switch_neighbor_tunnel_type_to_string(
    switch_neighbor_tunnel_type_t neigh_tunnel_type) {
  switch (neigh_tunnel_type) {
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_L3VPN:
      return "mpls swap l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_PUSH_L3VPN:
      return "mpls swap push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN:
      return "mpls swap l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN:
      return "mpls push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_IPV4_TUNNEL:
      return "ipv4 tunnel";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_IPV6_TUNNEL:
      return "ipv6 tunnel";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L2VPN:
      return "mpls ipv4 udp push l2vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L3VPN:
      return "mpls ipv4 udp push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_PUSH_L3VPN:
      return "mpls ipv4 udp swap push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_L3VPN:
      return "mpls ipv4 udp swap l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L2VPN:
      return "mpls ipv4 udp swap push l2vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L3VPN:
      return "mpls ipv4 udp push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_PUSH_L3VPN:
      return "mpls ipv4 udp swap push l3vpn";
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_L3VPN:
      return "mpls ipv4 udp swap l3vpn";
    default:
      return "none";
  }
}

static inline char *switch_neighbor_rewrite_type_to_string(
    switch_neighbor_rw_type_t rw_type) {
  switch (rw_type) {
    case SWITCH_API_NEIGHBOR_RW_TYPE_L2:
      return "l2";
    case SWITCH_API_NEIGHBOR_RW_TYPE_L3:
      return "l3";
    default:
      return "unknown";
  }
}

typedef struct switch_neighbor_dmac_entry_s {
  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** neighbor mac address */
  switch_mac_addr_t mac;

} switch_neighbor_dmac_entry_t;

typedef struct switch_neighbor_nhop_list_s {
  /** bridge domain handle */
  switch_handle_t handle;

  /** neighbor mac address */
  switch_mac_addr_t mac;

  /** dmac hashtable node */
  switch_hashnode_t node;

  /** dmac hashtable node */
  switch_list_t list;

} switch_neighbor_nhop_list_t;

/** neighbor struct */
typedef struct switch_neighbor_info_s {
  /** dmac list node */
  switch_node_t node;

  /** api neighbor info */
  switch_api_neighbor_info_t api_neighbor_info;

  /** bridge domain handle */
  switch_handle_t handle;

  /** neighbor mac address */
  switch_mac_addr_t mac;

  /** nhop handle */
  switch_handle_t nhop_handle;

  /** neighbor handle - self pointer */
  switch_handle_t neighbor_handle;

  /** rewrite tunnel mac index */
  switch_id_t tunnel_dmac_index;

  /** rewrite pd handle */
  switch_pd_hdl_t rewrite_pd_hdl;

  /** tunnel rewrite hardware handle */
  switch_pd_hdl_t tunnel_rewrite_pd_hdl;

} switch_neighbor_info_t;

/** neighbor tunnel dmac struct */
typedef struct switch_tunnel_dmac_rewrite_s {
  /** destination rewrite mac */
  switch_mac_addr_t mac;

  /** hashtable node */
  switch_hashnode_t node;

  /** dmac rewrite index */
  switch_id_t index;

  /** reference count */
  switch_uint16_t ref_count;

  /** rewrite hardware handle */
  switch_pd_hdl_t rewrite_pd_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_tunnel_dmac_rewrite_t;

/** neighbor device context */
typedef struct switch_neighbor_context_s {
  /** tunnel dmac rewrite hashtable */
  switch_hashtable_t tunnel_dmac_rewrite_hashtable;

  /** dmac rewrite hashtable */
  switch_hashtable_t neighbor_dmac_hashtable;

  /** tunnel dmac rewrite index allocator */
  switch_id_allocator_t *dmac_rewrite_index;

} switch_neighbor_context_t;

switch_status_t switch_neighbor_init(switch_device_t device);

switch_status_t switch_neighbor_free(switch_device_t device);

switch_status_t switch_neighbor_default_entries_add(switch_device_t device);

switch_status_t switch_neighbor_default_entries_delete(switch_device_t device);

switch_status_t switch_neighbor_entry_nhop_list_get(
    switch_device_t device,
    switch_neighbor_dmac_entry_t *neighbor_entry,
    switch_neighbor_nhop_list_t **nhop_list);

switch_status_t switch_neighbor_tunnel_dmac_rewrite_delete(
    switch_device_t device, switch_mac_addr_t *mac);

switch_status_t switch_neighbor_tunnel_dmac_rewrite_add(
    switch_device_t device, switch_mac_addr_t *mac, switch_id_t *dmac_index);

switch_status_t switch_api_neighbor_context_dump(const switch_device_t device,
                                                 const void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* _switch_neighbor_int_h_ */
