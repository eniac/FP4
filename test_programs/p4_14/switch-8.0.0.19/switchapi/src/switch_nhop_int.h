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

#ifndef __SWITCH_NHOP_INT_H__
#define __SWITCH_NHOP_INT_H__

#define MAX_ECMP_GROUP_SIZE (64)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_NHOP_HASH_SEED 0x12345678

#define MAX_WCMP_WEIGHT 256.0

#define switch_nhop_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_NHOP, sizeof(switch_nhop_info_t))

#define switch_nhop_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_NHOP, _handle)

#define switch_nhop_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_NHOP, _handle, (void **)_info)

#define switch_ecmp_member_handle_create(_device) \
  switch_handle_create(                           \
      _device, SWITCH_HANDLE_TYPE_ECMP_MEMBER, sizeof(switch_ecmp_member_t))

#define switch_ecmp_member_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ECMP_MEMBER, _handle)

#define switch_ecmp_member_get(_device, _handle, _info) \
  switch_handle_get(                                    \
      _device, SWITCH_HANDLE_TYPE_ECMP_MEMBER, _handle, (void **)_info)

#define switch_ecmp_handle_create switch_nhop_handle_create
#define switch_ecmp_handle_delete switch_nhop_handle_delete
#define switch_ecmp_get switch_nhop_get

#define switch_wcmp_handle_create switch_nhop_handle_create
#define switch_wcmp_handle_delete switch_nhop_handle_delete
#define switch_wcmp_get switch_nhop_get

#define SWITCH_ECMP_MEMBER_INIT(_m)                         \
  do {                                                      \
    if (_m) {                                               \
      SWITCH_MEMSET(_m, 0x0, sizeof(switch_ecmp_member_t)); \
      _m->nhop_handle = SWITCH_API_INVALID_HANDLE;          \
      _m->urpf_pd_hdl = SWITCH_PD_INVALID_HANDLE;           \
      _m->mbr_hdl = SWITCH_PD_INVALID_HANDLE;               \
    }                                                       \
  } while (0);

#define SWITCH_NHOP_HASH_KEY_SIZE sizeof(switch_nhop_key_t)

#define SWITCH_NHOP_KEY_GET(_api_nhop_info, _nhop_key)                    \
  do {                                                                    \
    SWITCH_MEMSET(&_nhop_key, 0x0, sizeof(switch_nhop_key_t));            \
    if (_api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_IP) {               \
      SWITCH_ASSERT(SWITCH_RIF_HANDLE(_api_nhop_info->rif_handle));       \
      _nhop_key.handle = _api_nhop_info->rif_handle;                      \
    } else if (_api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_TUNNEL) {    \
      SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(_api_nhop_info->tunnel_handle)); \
      _nhop_key.handle = _api_nhop_info->tunnel_handle;                   \
    } else if (_api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_MPLS) {      \
      _nhop_key.handle = _api_nhop_info->mpls_handle;                     \
    } else {                                                              \
      _nhop_key.handle = _api_nhop_info->intf_handle;                     \
    }                                                                     \
    SWITCH_MEMCPY(&_nhop_key.ip_addr,                                     \
                  &api_nhop_info->ip_addr,                                \
                  sizeof(switch_ip_addr_t));                              \
  } while (0);

#define SWITCH_NHOP_TUNNEL_BD_HANDLE_GET(_api_nhop_info, _bd_handle, _status) \
  do {                                                                        \
    switch_handle_t _handle = SWITCH_API_INVALID_HANDLE;                      \
    _status = SWITCH_STATUS_SUCCESS;                                          \
    if ((_api_nhop_info->nhop_tunnel_type == SWITCH_NHOP_TUNNEL_TYPE_VLAN) || \
        (_api_nhop_info->nhop_tunnel_type == SWITCH_NHOP_TUNNEL_TYPE_LN)) {   \
      _handle = _api_nhop_info->network_handle;                               \
    } else if (_api_nhop_info->nhop_tunnel_type ==                            \
               SWITCH_NHOP_TUNNEL_TYPE_VRF) {                                 \
      _handle = _api_nhop_info->vrf_handle;                                   \
    }                                                                         \
    if (_handle != SWITCH_API_INVALID_HANDLE) {                               \
      _status = switch_bd_handle_get(device, _handle, &_bd_handle);           \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                        \
      SWITCH_ASSERT(SWITCH_BD_HANDLE(_bd_handle));                            \
    }                                                                         \
  } while (0);

typedef enum switch_nhop_pd_action_s {
  SWITCH_NHOP_PD_ACTION_NON_TUNNEL = 0x1,
  SWITCH_NHOP_PD_ACTION_TUNNEL = 0x2,
  SWITCH_NHOP_PD_ACTION_MGID_TUNNEL = 0x3,
  SWITCH_NHOP_PD_ACTION_FLOOD = 0x4,
  SWITCH_NHOP_PD_ACTION_GLEAN = 0x5,
  SWITCH_NHOP_PD_ACTION_DROP = 0x6,
  SWITCH_NHOP_PD_ACTION_MAX
} switch_nhop_pd_action_t;

static inline char *switch_nhop_type_to_string(switch_nhop_type_t nhop_type) {
  switch (nhop_type) {
    case SWITCH_NHOP_TYPE_NONE:
      return "none";
    case SWITCH_NHOP_TYPE_IP:
      return "ip";
    case SWITCH_NHOP_TYPE_TUNNEL:
      return "tunnel";
    case SWITCH_NHOP_TYPE_MPLS:
      return "mpls";
    case SWITCH_NHOP_TYPE_GLEAN:
      return "glean";
    default:
      return "unknown";
  }
}

static inline char *switch_nhop_rewrite_type_to_string(
    switch_nhop_tunnel_rewrite_type_t rw_type) {
  switch (rw_type) {
    case SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2:
      return "l2";
    case SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3:
      return "l3";
    case SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR:
      return "l2-mirror";
    case SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3_VNI:
      return "l3-vni";
    default:
      return "unknown";
  }
}

static inline char *switch_nhop_tunnel_type_to_string(
    switch_nhop_tunnel_type_t tunnel_type) {
  switch (tunnel_type) {
    case SWITCH_NHOP_TUNNEL_TYPE_NONE:
      return "none";
    case SWITCH_NHOP_TUNNEL_TYPE_VLAN:
      return "vlan";
    case SWITCH_NHOP_TUNNEL_TYPE_LN:
      return "ln";
    case SWITCH_NHOP_TUNNEL_TYPE_VRF:
      return "vrf";
    default:
      return "unknown";
  }
}

/** ecmp member struct */
typedef struct switch_ecmp_member_s {
  /** ecmp member handle */
  switch_handle_t member_handle;

  /** ecmp group handle */
  switch_handle_t ecmp_handle;

  /** list node */
  switch_node_t node;

  /** ecmp member nhop handle */
  switch_handle_t nhop_handle;

  /** member active bit */
  bool active;

  /** urpf hardware handle */
  switch_pd_hdl_t urpf_pd_hdl;

  /** ecmp member hardware handle */
  switch_pd_mbr_hdl_t mbr_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_ecmp_member_t;

/** wcmp member struct */
typedef struct switch_wcmp_member_s {
  /** list node */
  switch_node_t node;

  /** wcmp member nhop handle */
  switch_handle_t nhop_handle;

  /** member weight */
  uint8_t weight;

  /** wcmp hardware handle */
  switch_pd_hdl_t hw_entry;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_wcmp_member_t;

/** multipath nexthop struct */
typedef struct switch_mpath_info_s {
  /** ecmp/wcmp members */

  switch_list_t members;

  /** ecmp hardware handle */
  switch_pd_hdl_t hw_entry;

  /** ecmp group hardware handle */
  switch_pd_grp_hdl_t pd_group_hdl;

  /** wcmp member handle */
  switch_pd_mbr_hdl_t mbr_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_mpath_info_t;

/** single path nexthop struct */
typedef struct switch_spath_info_s {
  /**
   * nhop key programmed by the application and
   * acts as the key for nhop hashtable. This should
   * be the first entry in this struct for hashing
   */
  switch_nhop_key_t nhop_key;

  /** nhop application info */
  switch_api_nhop_info_t api_nhop_info;

  /** nhop handle - self pointer */
  switch_handle_t nhop_handle;

  /** neighbor handle */
  switch_handle_t neighbor_handle;

  /** interface ifindex */
  switch_ifindex_t ifindex;

  /** interface port lag index */
  switch_port_lag_index_t port_lag_index;

  /** bd handle */
  switch_handle_t bd_handle;

  /** is tunnel */
  bool tunnel;

  /** hashtable node */
  switch_hashnode_t node;

  /** nhop hardware handle */
  switch_pd_hdl_t hw_entry;

  /** urpf hardware handle */
  switch_pd_hdl_t urpf_pd_hdl;

  /** replication ID */
  switch_rid_t rid;

  /** tunnel destination ip index */
  switch_id_t tunnel_dst_index;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_spath_info_t;

typedef struct switch_nhop_tunnel_info_s {
  /** mgid handle for next hop to tunnel destination */
  switch_handle_t mgid_handle;

  /** mgid state for the nexthop */
  switch_mgid_state_t mgid_state;

  /** List of tunnels using this nexthop */
  Pvoid_t PJLarr_tunnels;

  /** Number of tunnels using this nh */
  switch_uint32_t num_tunnels;

  /** List of routes using this nexthop */
  Pvoid_t PJLarr_routes;

  /** Number of routes using this nh */
  switch_uint32_t num_routes;
} switch_nhop_tunnel_info_t;

typedef struct switch_nhop_info_s {
  /** nhop type - nhop/wcmp/ecmp */
  switch_nhop_id_type_t id_type;

  /** nhop handle - self pointer */
  switch_handle_t nhop_handle;

  /** nhop type is single path */
  switch_spath_info_t spath;

  /** nhop type is multi path */
  switch_mpath_info_t mpath;

  /** Reference count to track how many routes use this nexthop */
  switch_uint32_t routes_refcount;

  /** tunnel info if the nexthop is used by tunnel dest(s) */
  switch_nhop_tunnel_info_t tunnel_info;

  /** number of ecmp references */
  switch_uint32_t ecmp_ref_count;

  /** List of ecmp-members(handles) referring this nexthop */
  Pvoid_t PJLarr_ecmp_members;

  /** nexthop reference count */
  switch_handle_t nhop_ref_count;

  /** tunnel encapsulation handle */
  switch_handle_t tunnel_encap_handle;

} switch_nhop_info_t;

/** ecmp info struct */
typedef switch_nhop_info_t switch_ecmp_info_t;

/** wcmp info struct */
typedef switch_nhop_info_t switch_wcmp_info_t;

/** nhop device context */
typedef struct switch_nhop_context_s {
  /** nexthop hashtable */
  switch_hashtable_t nhop_hashtable;

} switch_nhop_context_t;

#define SWITCH_NHOP_SPATH_INFO(nhop) nhop->spath

#define SWITCH_ECMP_MPATH_INFO(nhop) nhop->mpath

#define SWITCH_WCMP_MPATH_INFO(nhop) nhop->mpath

#define SWITCH_NHOP_ID_TYPE_ECMP(nhop)          \
  (nhop->id_type == SWITCH_NHOP_ID_TYPE_ECMP || \
   nhop->id_type == SWITCH_NHOP_ID_TYPE_WCMP)

#define NHOP_TUNNEL_MGID_ROUTE_LIST(pninfo) pninfo->tunnel_info.PJLarr_routes

#define NHOP_TUNNEL_MGID_TUNNEL_LIST(pninfo) pninfo->tunnel_info.PJLarr_tunnels

#define NHOP_TUNNEL_MGID_HANDLE(pninfo) pninfo->tunnel_info.mgid_handle

#define NHOP_TUNNEL_NUM_TUNNELS(pninfo) pninfo->tunnel_info.num_tunnels

#define NHOP_TUNNEL_NUM_ROUTES(pninfo) pninfo->tunnel_info.num_routes

#define SET_NHOP_TUNNEL_MGID_STATE(pninfo, state_fn) \
  pninfo->tunnel_info.mgid_state = state_fn

#define SEND_NHOP_TUNNEL_MGID_EVENT(pninfo, dev, event, event_arg) \
  (pninfo)->tunnel_info.mgid_state(dev, (pninfo), event, event_arg)

#define SWITCH_NHOP_NUM_ECMP_MEMBER_REF(nhop) nhop->ecmp_ref_count

#define SWITCH_NHOP_ECMP_MEMBER_REF_LIST(nhop) nhop->PJLarr_ecmp_members

#define SWITCH_NHOP_TYPE(_nhop_info) _nhop_info->spath.api_nhop_info.nhop_type

switch_status_t switch_nhop_init(switch_device_t device);

switch_status_t switch_nhop_free(switch_device_t device);

switch_status_t switch_nhop_default_entries_add(switch_device_t device);

switch_status_t switch_nhop_default_entries_delete(switch_device_t device);

switch_status_t switch_api_nhop_update(switch_device_t device,
                                       switch_handle_t nhop_handle);

switch_status_t switch_nhop_l3_vlan_interface_resolve(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t bd_handle,
    switch_mac_addr_t *mac_addr,
    bool neighbor_deleted);

switch_status_t switch_api_nhop_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_nhop_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_nhop_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_nhop_send_mgid_event(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_ecmp_member_activate(switch_device_t device,
                                            switch_handle_t ecmp_handle,
                                            switch_uint16_t num_nhops,
                                            switch_handle_t *nhop_handles,
                                            bool activate);

switch_status_t switch_nhop_ecmp_members_deactivate(
    switch_device_t device, switch_nhop_info_t *nhop_info);

switch_status_t switch_api_nhop_context_dump(const switch_device_t device,
                                             const void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_NHOP_INT_H__ */
