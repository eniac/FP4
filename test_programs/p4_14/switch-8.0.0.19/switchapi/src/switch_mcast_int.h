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

#ifndef __SWITCH_MCAST_INT_H__
#define __SWITCH_MCAST_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __TARGET_TOFINO__
typedef bf_mc_ecmp_hdl_t mc_ecmp_hdl_t;
typedef bf_mc_node_hdl_t mc_node_hdl_t;
#endif

#define SWITCH_MGID_TABLE_SIZE 64 * 1024
#define SWITCH_MGID_ECMP_TABLE_SIZE 64 * 1024

#define SWITCH_MCAST_GROUP_HASH_KEY_SIZE sizeof(switch_mcast_group_key_t)

typedef uint32_t mc_mgrp_hdl_t;

#define SWITCH_API_MAX_PORTS 288

#define SWITCH_RID_INVALID 0
#define SWITCH_MCAST_GLOBAL_RID 0xFFFF

#define SWITCH_PORT_ARRAY_SIZE ((SWITCH_API_MAX_PORTS + 7) / 8)
typedef uint8_t switch_mc_port_map_t[SWITCH_PORT_ARRAY_SIZE];
#define SWITCH_LAG_ARRAY_SIZE ((SWITCH_API_MAX_LAG + 7) / 8)
typedef uint8_t switch_mc_lag_map_t[SWITCH_LAG_ARRAY_SIZE];

#define SWITCH_DEV_PORT_TO_PIPE(_dp) (((_dp) >> 7) & 0x3)
#define SWITCH_DEV_PORT_TO_LOCAL_PORT(_dp) ((_dp)&0x7F)

#ifdef __TARGET_TOFINO__
#define SWITCH_DEV_PORT_TO_BIT_IDX(_dp) \
  (72 * SWITCH_DEV_PORT_TO_PIPE(_dp) + SWITCH_DEV_PORT_TO_LOCAL_PORT(_dp))
#else
#define SWITCH_DEV_PORT_TO_BIT_IDX(_dp) _dp
#endif

#define SWITCH_MC_PORT_MAP_CLEAR(pm, port)          \
  do {                                              \
    int _port_p = SWITCH_DEV_PORT_TO_BIT_IDX(port); \
    switch_mc_port_map_t *_port_pm = &(pm);         \
    if (_port_p >= SWITCH_API_MAX_PORTS) break;     \
    size_t _port_i = (_port_p) / 8;                 \
    unsigned int _port_j = (_port_p) % 8;           \
    (*_port_pm)[_port_i] &= ~(1 << _port_j);        \
  } while (0);

#define SWITCH_MC_PORT_MAP_SET(pm, port)            \
  do {                                              \
    int _port_p = SWITCH_DEV_PORT_TO_BIT_IDX(port); \
    switch_mc_port_map_t *_port_pm = &(pm);         \
    if (_port_p >= SWITCH_API_MAX_PORTS) break;     \
    size_t _port_i = (_port_p) / 8;                 \
    unsigned int _port_j = (_port_p) % 8;           \
    (*_port_pm)[_port_i] |= (1 << _port_j);         \
  } while (0);

#define SWITCH_MC_LAG_MAP_CLEAR(pm, lag)     \
  do {                                       \
    int _lag_p = (lag);                      \
    switch_mc_lag_map_t *_lag_pm = &(pm);    \
    if (_lag_p >= SWITCH_API_MAX_LAG) break; \
    size_t _lag_i = (_lag_p) / 8;            \
    unsigned int _lag_j = (_lag_p) % 8;      \
    (*_lag_pm)[_lag_i] &= ~(1 << _lag_j);    \
  } while (0);

#define SWITCH_MC_LAG_MAP_SET(pm, lag)       \
  do {                                       \
    int _lag_p = (lag);                      \
    switch_mc_lag_map_t *_lag_pm = &(pm);    \
    if (_lag_p >= SWITCH_API_MAX_LAG) break; \
    size_t _lag_i = (_lag_p) / 8;            \
    unsigned int _lag_j = (_lag_p) % 8;      \
    (*_lag_pm)[_lag_i] |= (1 << _lag_j);     \
  } while (0);

/** multicast index handle wrappers */
#define switch_mgid_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_MGID, sizeof(switch_mcast_info_t))

#define switch_mgid_handle_delete(_device, _handle) \
  switch_handle_delete(device, SWITCH_HANDLE_TYPE_MGID, _handle)

#define switch_mgid_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_MGID, _handle, (void **)_info)

/** multicast ecmp index handle wrappers */
#define switch_mgid_ecmp_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_MGID_ECMP, sizeof(switch_ace_info_t))

#define switch_mgid_ecmp_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MGID_ECMP, _handle)

#define switch_mgid_ecmp_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_MGID_ECMP, _handle, (void **)_info)

#define SWITCH_MCAST_IPV4_TO_MAC(_ip, _mac)          \
  do {                                               \
    _mac.mac_addr[0] = 0x01;                         \
    _mac.mac_addr[1] = 0x00;                         \
    _mac.mac_addr[2] = 0x5E;                         \
    _mac.mac_addr[3] = (_ip.ip.v4addr >> 16) & 0xFF; \
    _mac.mac_addr[4] = (_ip.ip.v4addr >> 8) & 0xFF;  \
    _mac.mac_addr[5] = (_ip.ip.v4addr) & 0xFF;       \
  } while (0);

/** multicast node type */
typedef enum switch_mcast_node_type_s {
  SWITCH_NODE_TYPE_SINGLE = 0,
  SWITCH_NODE_TYPE_ECMP = 1
} switch_mcast_node_type_t;

typedef enum switch_rid_pd_flags_s {
  SWITCH_RID_PD_ENTRY = (1 << 0),
  SWITCH_RID_IFINDEX_PD_ENTRY = (1 << 1),
} switch_rid_pd_flags_t;

typedef enum switch_mgid_type_s {
  SWITCH_MGID_TYPE_MULTICAST = 0x00,
  SWITCH_MGID_TYPE_UNICAST = 0x01,
} switch_mgid_type_t;

typedef enum switch_rid_type_s {
  SWITCH_RID_TYPE_UNICAST = 0x0,
  SWITCH_RID_TYPE_INNER_REPLICA = 0x1,
  SWITCH_RID_TYPE_OUTER_REPLICA = 0x2,
} switch_rid_type_t;

typedef enum switch_mcast_key_type_s {
  SWITCH_MCAST_KEY_TYPE_BD,
  SWITCH_MCAST_KEY_TYPE_VRF
} switch_mcast_key_type_t;

/** multicast node representing l1/l2 node in pre */
typedef struct switch_mcast_node_info_s {
  /** replication id */
  switch_rid_t rid;

  /** port bitmap */
  switch_mc_port_map_t port_map;

  /** lag bitmap */
  switch_mc_lag_map_t lag_map;

  /** pd handle */
  switch_pd_hdl_t hw_entry;

} switch_mcast_node_info_t;

/** replication id info */
typedef struct switch_rid_info_s {
  /** replication id */
  switch_rid_t rid;

  /** reference count */
  switch_uint16_t ref_count;

  /** rid table pd handle */
  switch_pd_hdl_t rid_pd_hdl;

  /** rid egress ifindex pd handle */
  switch_pd_hdl_t mcast_egress_ifindex_pd_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_rid_info_t;

typedef struct switch_mcast_ecmp_info_s {
  /** list of l1/l2 nodes */
  switch_list_t node_list;

  /** pd handle */
  switch_pd_hdl_t hw_entry;

  switch_handle_t handle;

} switch_mcast_ecmp_info_t;

/** multicast node representing l1 node in pre */
typedef struct switch_mcast_node_s {
  /** list node */
  switch_node_t node;

  /** level 1 exclusion id */
  switch_xid_t xid;

  /** node type - one path/ecmp */
  switch_mcast_node_type_t node_type;

  /** union based on node type */
  union {
    switch_mcast_ecmp_info_t ecmp_info;
    switch_mcast_node_info_t node_info;
  } u;
} switch_mcast_node_t;

/** multicast info representing a mgid tree */
typedef struct switch_mcast_info_s {
  /** list of l1/l2 nodes */
  switch_list_t node_list;

  /** multicast tree pd handle */
  switch_pd_hdl_t mgrp_hdl;

  /** multicast node type - unicast/multicast */
  switch_mgid_type_t type;

  /** maximum member count */
  switch_uint16_t mbr_count_max;

  /** member count */
  switch_uint16_t mbr_count;

  /** list of multicast members */
  switch_vlan_interface_t *mbrs;

} switch_mcast_info_t;

/** (S, G) or (*, G)  multicast group key */
typedef struct __attribute__((__packed__)) switch_mcast_group_key_s {
  /** vrf or vlan/ln handle */
  switch_handle_t handle;

  /** source ip address */
  switch_ip_addr_t src_ip;

  /** multicast ip address */
  switch_ip_addr_t grp_ip;

  /** (S, G) or (*, G) */
  bool sg_entry;

} switch_mcast_group_key_t;

/** multicast group info */
typedef struct switch_mcast_group_info_s {
  /**
   * multicast group key. The group key is hashed
   * to identify the corresponding entry. This struct
   * should always be on top for hash comparison
   */
  switch_mcast_group_key_t group_key;

  /** hashtable node */
  switch_hashnode_t node;

  /** multicast tree handle */
  switch_handle_t mgid_handle;

  /** rpf handle */
  switch_handle_t rpf_handle;

  /** outer multicast hw entry */
  switch_pd_hdl_t outer_hw_entry;

  /** inner multicast hw entry */
  switch_pd_hdl_t inner_hw_entry;

} switch_mcast_group_info_t;

typedef struct switch_mcast_member_info_s {
  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** interface handle */
  switch_handle_t intf_handle;

} switch_mcast_member_info_t;

/** multicast context */
typedef struct switch_mcast_context_s {
  /** replication id hashtable */
  switch_hashtable_t rid_hashtable;

  /** multicast group hashtable */
  switch_hashtable_t mcast_group_hashtable;

  /** replicate id allocator */
  switch_id_allocator_t *rid_allocator;

  /** rid info array indexed by rid */
  switch_array_t rid_array;

} switch_mcast_context_t;

#define SWITCH_MCAST_NODE_RID(node) node->u.node_info.rid

#define SWITCH_MCAST_NODE_RID_HW_ENTRY(node) node->u.node_info.rid_hw_entry

#define SWITCH_MCAST_GROUP_IPV4_SRC_IP(group_key) group_key->src_ip.ip.v4addr

#define SWITCH_MCAST_GROUP_IPV6_SRC_IP(group_key) group_key->src_ip.ip.v6addr

#define SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key) group_key->grp_ip.ip.v4addr

#define SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key) group_key->grp_ip.ip.v6addr

#define SWITCH_MCAST_GROUP_IP_TYPE(group_key) group_key->grp_ip.type

#define SWITCH_MCAST_NODE_INFO_HW_ENTRY(node) node->u.node_info.hw_entry

#define SWITCH_MCAST_NODE_INFO_PORT_MAP(node) node->u.node_info.port_map

#define SWITCH_MCAST_NODE_INFO_LAG_MAP(node) node->u.node_info.lag_map

#define SWITCH_MCAST_ECMP_INFO_HW_ENTRY(node) node->u.ecmp_info.hw_entry

#define SWITCH_MCAST_ECMP_INFO_NODE_LIST(node) node->u.ecmp_info.node_list

#define SWITCH_MCAST_ECMP_INFO_HDL(node) node->u.ecmp_info.handle

/* MCAST Internal API's */

switch_status_t switch_mcast_init(switch_device_t device);

switch_status_t switch_mcast_free(switch_device_t device);

switch_status_t switch_mcast_default_entries_add(switch_device_t device);

switch_status_t switch_mcast_default_entries_delete(switch_device_t device);

switch_status_t switch_mcast_bd_member_rid_allocate(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t intf_handle);

switch_status_t switch_mcast_bd_member_rid_free(switch_device_t device,
                                                switch_handle_t bd_handle,
                                                switch_handle_t intf_handle);

switch_status_t switch_mcast_rid_allocate(switch_device_t device,
                                          switch_rid_t *rid);

switch_status_t switch_mcast_rid_release(switch_device_t device,
                                         switch_rid_t rid);

switch_status_t switch_multicast_nhop_member_add(switch_device_t device,
                                                 switch_handle_t mgid_handle,
                                                 switch_handle_t nhop_handle);

switch_status_t switch_multicast_nhop_member_delete(
    switch_device_t device,
    switch_handle_t mgid_handle,
    switch_handle_t nhop_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_MCAST_INT_H__ */
