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

#ifndef __SWITCH_L3_INT_H__
#define __SWITCH_L3_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_ROUTE_HASH_KEY_SIZE \
  sizeof(switch_handle_t) + sizeof(switch_ip_addr_t) + sizeof(bool)

#define SWITCH_ROUTE_HASH_SEED 0x123456

#define SWITCH_IP_TYPE_NONE 0
#define SWITCH_IP_TYPE_IPv4 1
#define SWITCH_IP_TYPE_IPv6 2

#define SWITCH_IP_FORCE_HOST_IN_EXACT 0x1
#define SWITCH_IP_FORCE_HOST_IN_LPM 0x2
#define SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST 0x4

/** route handle wrappers */
#define switch_route_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_ROUTE, sizeof(switch_route_info_t))

#define switch_route_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ROUTE, _handle)

#define switch_route_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_ROUTE, _handle, (void **)_info)

/** mtu handle wrappers */
#define switch_mtu_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_MTU, sizeof(switch_mtu_info_t))

#define switch_mtu_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MTU, _handle)

#define switch_mtu_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_MTU, _handle, (void **)_info)

/** stores route information */
typedef struct switch_route_entry_s {
  /** vrf handle to identify the vrf id */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip;

  /** neighbor installed */
  bool neighbor_installed;
} switch_route_entry_t;

typedef enum switch_route_pd_entry_s {
  SWITCH_ROUTE_PD_ROUTE_ENTRY = (1 << 0),
  SWITCH_ROUTE_PD_URPF_ENTRY = (1 << 1)
} switch_route_pd_entry_t;

typedef enum switch_mtu_pd_entry_s {
  SWITCH_MTU_PD_IPV4_ENTRY = (1 << 0),
  SWITCH_MTU_PD_IPV6_ENTRY = (1 << 1),
} switch_mtu_pd_entry_t;

typedef struct switch_route_tunnel_info_s {
  /** mgid state for the route */
  switch_mgid_state_t mgid_state;

  /** List of tunnels using this route */
  Pvoid_t PJLarr_tunnels;

  /** Number of tunnels using the route */
  switch_uint32_t num_tunnels;
} switch_route_tunnel_info_t;

/** stores route info and associated hardware handles */
typedef struct switch_route_info_s {
  /**
   * route entry programmed by the application and
   * acts as the key for route hashtable. This should
   * be the first entry in this struct for hashing
   */
  switch_route_entry_t route_entry;

  /** route handle */
  switch_handle_t route_handle;

  /** nexhop handle */
  switch_handle_t nhop_handle;

  /** route hashtable node */
  switch_hashnode_t node;

  /** vrf array node */
  switch_node_t vrf_node;

  /** route hardware handle */
  switch_pd_hdl_t route_pd_hdl;

  /** urpf hardware handle */
  switch_pd_hdl_t urpf_pd_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

  /** tunnel info if this route is used for tunnel destination(s) */
  switch_route_tunnel_info_t tunnel_info;

} switch_route_info_t;

#define ROUTE_TUNNEL_MGID_TUNNEL_LIST(prinfo) prinfo->tunnel_info.PJLarr_tunnels

#define ROUTE_TUNNEL_MGID_NUM_TUNNELS(prinfo) prinfo->tunnel_info.num_tunnels

#define SET_ROUTE_TUNNEL_MGID_STATE(prinfo, state_fn) \
  prinfo->tunnel_info.mgid_state = state_fn

#define SEND_ROUTE_TUNNEL_MGID_EVENT(prinfo, dev, event, event_arg) \
  (prinfo)->tunnel_info.mgid_state(dev, (prinfo), event, event_arg)

typedef struct switch_mtu_info_s {
  /** mtu value */
  switch_mtu_t mtu;

  /** v4 hardware handle */
  switch_pd_hdl_t v4_pd_hdl;

  /** v6 hardware handle */
  switch_pd_hdl_t v6_pd_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

  /** route hashtable node */
  switch_hashnode_t node;

  /** mtu handle */
  switch_handle_t handle;

  /** L3 RIF count */
  switch_uint32_t l3intf_count;
} switch_mtu_info_t;

typedef struct switch_urpf_member_info_s {
  switch_node_t node;
  switch_handle_t intf_handle;
  switch_pd_hdl_t hw_entry;
} switch_urpf_member_info_t;

typedef struct switch_urpf_group_info_s {
  switch_list_t urpf_member_list;
} switch_urpf_group_info_t;

/* l3 device context */
typedef struct switch_l3_context_s {
  /* route hashtable */
  switch_hashtable_t route_hashtable;
  /* mtu hashtable */
  switch_hashtable_t mtu_hashtable;

} switch_l3_context_t;

#define SWITCH_L3_IP_TYPE(ip_info) ip_info.type

#define SWITCH_L3_IP_IPV4_ADDRESS(ip_info) ip_info->ip.v4addr

#define SWITCH_L3_IP_IPV6_ADDRESS(ip_info) ip_info->ip.v6addr

switch_status_t switch_l3_init(switch_device_t device);

switch_status_t switch_l3_free(switch_device_t device);

switch_status_t switch_l3_default_entries_add(switch_device_t device);

switch_status_t switch_l3_default_entries_delete(switch_device_t device);

switch_status_t switch_l3_default_route_entries_add(switch_device_t device,
                                                    switch_handle_t vrf_handle);

switch_status_t switch_l3_default_route_entries_delete(
    switch_device_t device, switch_handle_t vrf_handle);

switch_status_t switch_api_l3_route_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_l3_route_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_l3_route_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_l3_route_send_mgid_event(
    switch_device_t device,
    switch_handle_t route_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_l3_context_dump(const switch_device_t device,
                                           const void *cli_ctx);

switch_status_t switch_l3_hashtable_dump(const switch_device_t device,
                                         const switch_hashtable_type_t type,
                                         void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_L3_INT_H__ */
