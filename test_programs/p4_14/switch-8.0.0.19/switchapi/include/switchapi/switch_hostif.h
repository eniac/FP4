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
#ifndef __SWITCH_HOSTIF_H__
#define __SWITCH_HOSTIF_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_acl.h"
#include "switch_meter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup HostInterface Host Interface API
 *  API functions define and manipulate host interfaces
 *  @{
 */  // begin of Host Interface API

/** switch hostif reason code */
typedef enum switch_hostif_reason_code_s {
  /*
   * Reason code groups must start on power of 2 boundary since
   * rx_net_filters are setup to use masks
   */
  /* generic reason codes 0x0-0x0FF */
  SWITCH_HOSTIF_REASON_CODE_NONE = 0x0,
  SWITCH_HOSTIF_REASON_CODE_CUSTOM = 0x1,
  SWITCH_HOSTIF_REASON_CODE_DROP = 0x2,
  SWITCH_HOSTIF_REASON_CODE_NULL_DROP = 0x3,
  SWITCH_HOSTIF_REASON_CODE_SFLOW_SAMPLE = 0x4,
  SWITCH_HOSTIF_REASON_CODE_BFD_RX = 0x5,
  SWITCH_HOSTIF_REASON_CODE_BFD_EVENT = 0x6,
  SWITCH_HOSTIF_REASON_CODE_ACL_LOG = 0x7,
  SWITCH_HOSTIF_REASON_CODE_PTP = 0x8,

  /* L2 reason codes 0x100 - 0x1FF */
  SWITCH_HOSTIF_REASON_CODE_L2_START = 0x100,
  SWITCH_HOSTIF_REASON_CODE_STP = SWITCH_HOSTIF_REASON_CODE_L2_START,
  SWITCH_HOSTIF_REASON_CODE_LACP,                /* 0x101 */
  SWITCH_HOSTIF_REASON_CODE_EAPOL,               /* 0x102 */
  SWITCH_HOSTIF_REASON_CODE_LLDP,                /* 0x103 */
  SWITCH_HOSTIF_REASON_CODE_PVRST,               /* 0x104 */
  SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_QUERY,     /* 0x105 */
  SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_LEAVE,     /* 0x106 */
  SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V1_REPORT, /* 0x107 */
  SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V2_REPORT, /* 0x108 */
  SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V3_REPORT, /* 0x109 */
  /* Currently IGMP packet is not parsed in switch.p4, hence igmp reason code by
  type as above are not supported.
  Only below reason code for IGMP (all igmp packets) is currently supported */
  SWITCH_HOSTIF_REASON_CODE_IGMP,              /* 0x10A */
  SWITCH_HOSTIF_REASON_CODE_L2_MISS_UNICAST,   /* 0x10B */
  SWITCH_HOSTIF_REASON_CODE_L2_MISS_MULTICAST, /* 0x10C */
  SWITCH_HOSTIF_REASON_CODE_L2_MISS_BROADCAST, /* 0x10D */
  SWITCH_HOSTIF_REASON_CODE_L2_END,

  /* L3 reason codes 0x200-0x2FF */
  SWITCH_HOSTIF_REASON_CODE_L3_START = 0x200,
  SWITCH_HOSTIF_REASON_CODE_SAMPLEPACKET = SWITCH_HOSTIF_REASON_CODE_L3_START,
  SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST = 0x201,
  SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE = 0x202,
  SWITCH_HOSTIF_REASON_CODE_DHCP = 0x203,
  SWITCH_HOSTIF_REASON_CODE_OSPF = 0x204,
  SWITCH_HOSTIF_REASON_CODE_PIM = 0x205,
  SWITCH_HOSTIF_REASON_CODE_VRRP = 0x206,
  SWITCH_HOSTIF_REASON_CODE_DHCPV6 = 0x208,
  SWITCH_HOSTIF_REASON_CODE_OSPFV6 = 0x209,
  SWITCH_HOSTIF_REASON_CODE_VRRPV6 = 0x20a,
  SWITCH_HOSTIF_REASON_CODE_IPV6_NEIGHBOR_DISCOVERY = 0x20c,
  SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_V2 = 0x20d,
  SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_REPORT = 0x20e,
  SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_DONE = 0x20f,
  SWITCH_HOSTIF_REASON_CODE_MLD_V2_REPORT = 0x210,
  SWITCH_HOSTIF_REASON_CODE_L3_MTU_ERROR = 0x211,
  SWITCH_HOSTIF_REASON_CODE_TTL_ERROR = 0x212,
  SWITCH_HOSTIF_REASON_CODE_GLEAN = 0x213,
  SWITCH_HOSTIF_REASON_CODE_ICMP_REDIRECT = 0x215,
  SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL = 0x216,
  SWITCH_HOSTIF_REASON_CODE_L3_REDIRECT = 0x217,
  SWITCH_HOSTIF_REASON_CODE_BROADCAST = 0x218,
  SWITCH_HOSTIF_REASON_CODE_SSH = 0x219,
  SWITCH_HOSTIF_REASON_CODE_SNMP = 0x21a,
  SWITCH_HOSTIF_REASON_CODE_L3_END,

  /* Local IP reason codes 0x400-0x4FF */
  SWITCH_HOSTIF_REASON_CODE_LOCAL_IP_START = 0x400,
  SWITCH_HOSTIF_REASON_CODE_MYIP = SWITCH_HOSTIF_REASON_CODE_LOCAL_IP_START,
  SWITCH_HOSTIF_REASON_CODE_BGP,   /* 0x401 */
  SWITCH_HOSTIF_REASON_CODE_BGPV6, /* 0x402 */
  SWITCH_HOSTIF_REASON_CODE_LOCAL_IP_END,

  SWITCH_HOSTIF_REASON_CODE_MAX,
} switch_hostif_reason_code_t;

typedef enum switch_hostif_channel_s {
  SWITCH_HOSTIF_CHANNEL_CB,
  SWITCH_HOSTIF_CHANNEL_FD,
  SWITCH_HOSTIF_CHANNEL_NETDEV,
} switch_hostif_channel_t;

typedef enum switch_tx_bypass_flags_s {
  SWITCH_BYPASS_NONE = 0x0,
  SWITCH_BYPASS_L2 = (1 << 0),
  SWITCH_BYPASS_L3 = (1 << 1),
  SWITCH_BYPASS_ACL = (1 << 2),
  SWITCH_BYPASS_QOS = (1 << 3),
  SWITCH_BYPASS_METER = (1 << 4),
  SWITCH_BYPASS_SYSTEM_ACL = (1 << 5),
  SWITCH_BYPASS_ALL = 0xFFFF
} switch_tx_bypass_flags_t;

typedef enum switch_hostif_attr_s {
  SWITCH_HOSTIF_ATTR_HANDLE = (1 << 0),
  SWITCH_HOSTIF_ATTR_MAC_ADDRESS = (1 << 1),
  SWITCH_HOSTIF_ATTR_IPV4_ADDRESS = (1 << 2),
  SWITCH_HOSTIF_ATTR_IPV6_ADDRESS = (1 << 3),
  SWITCH_HOSTIF_ATTR_INTERFACE_NAME = (1 << 4),
  SWITCH_HOSTIF_ATTR_ADMIN_STATE = (1 << 5),
  SWITCH_HOSTIF_ATTR_OPER_STATUS = (1 << 6),
  SWITCH_HOSTIF_ATTR_VLAN_ACTION = (1 << 7)
} switch_hostif_attr_t;

/** hostif vlan action */
typedef enum switch_hostif_vlan_action_s {
  SWITCH_HOSTIF_VLAN_ACTION_NONE = 0x0,
  SWITCH_HOSTIF_VLAN_ACTION_ADD = 0x1,
  SWITCH_HOSTIF_VLAN_ACTION_REMOVE = 0x2,
} switch_hostif_vlan_action_t;

typedef enum switch_hostif_rx_filter_attr_s {
  SWITCH_HOSTIF_RX_FILTER_ATTR_PORT_HANDLE = (1 << 0),
  SWITCH_HOSTIF_RX_FILTER_ATTR_LAG_HANDLE = (1 << 1),
  SWITCH_HOSTIF_RX_FILTER_ATTR_INTF_HANDLE = (1 << 2),
  SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE = (1 << 3),
  SWITCH_HOSTIF_RX_FILTER_ATTR_REASON_CODE = (1 << 4),
  SWITCH_HOSTIF_RX_FILTER_ATTR_ETHER_TYPE = (1 << 5),
  SWITCH_HOSTIF_RX_FILTER_ATTR_GLOBAL = (1 << 6)
} switch_hostif_rx_filter_attr_t;

typedef enum switch_hostif_tx_filter_attr_s {
  SWITCH_HOSTIF_TX_FILTER_ATTR_HOSTIF_HANDLE = (1 << 0),
  SWITCH_HOSTIF_TX_FILTER_ATTR_VLAN_ID = (1 << 1),
} switch_hostif_tx_filter_attr_t;

typedef enum switch_hostif_rx_filter_priority_s {
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_PORT = 0x1,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_LAG = 0x2,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_INTERFACE = 0x3,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_VLAN = 0x4,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_LN = 0x5,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_RIF = 0x6,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO1 = 0x10,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO2 = 0x11,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO3 = 0x12,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO4 = 0x13,
  SWITCH_HOSTIF_RX_FILTER_PRIORITY_MAX = 0x14
} switch_hostif_rx_filter_priority_t;

typedef enum switch_hostif_tx_filter_priority_s {
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_HOSTIF = 0x1,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO1 = 0x4,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO2 = 0x5,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO3 = 0x6,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO4 = 0x7,
  SWITCH_HOSTIF_TX_FILTER_PRIORITY_MAX = 0x8
} switch_hostif_tx_filter_priority_t;

typedef enum switch_hostif_rcode_attr_s {
  SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE = (1 << 0),
  SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION = (1 << 1),
  SWITCH_HOSTIF_RCODE_ATTR_PRIORITY = (1 << 2),
  SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP = (1 << 3)
} switch_hostif_rcode_attr_t;

/** switch hostif group */
typedef struct switch_hostif_group_s {
  /** queue handle */
  switch_handle_t queue_handle;

  /** policer handle */
  switch_handle_t policer_handle;

} switch_hostif_group_t;

/** switch hostif reason code info */
typedef struct switch_api_hostif_rcode_info_s {
  /** reason code */
  switch_hostif_reason_code_t reason_code;

  /** packet action */
  switch_acl_action_t action;

  /** priority */
  switch_uint32_t priority;

  /** hostif group id */
  switch_handle_t hostif_group_id;

} switch_api_hostif_rcode_info_t;

/** hostif tx/rx packet info */
typedef struct switch_hostif_packet_s {
  /** device id */
  switch_device_t device;

  /** reason code */
  switch_hostif_reason_code_t reason_code;

  /** port handle */
  switch_handle_t handle;

  /** lag handle */
  switch_handle_t lag_handle;

  /* vlan/ln interface handle */
  switch_handle_t network_handle;

  /** interface handle */
  switch_handle_t intf_handle;

  /** bypass flags */
  switch_uint16_t bypass_flags;

  /** packet */
  void *pkt;

  /** packet size */
  switch_size_t pkt_size;

  /** port number */
  switch_port_t port;

  /** arrival time */
  switch_arrival_time_t arrival_time;

  /** app cookie */
  void *cookie;

} switch_hostif_packet_t;

/** host interface */
typedef struct switch_hostif_s {
  /** netdev interface name */
  switch_char_t intf_name[SWITCH_HOSTIF_NAME_SIZE];

  /** handle - port/interface/vlan/ln/rif */
  switch_handle_t handle;

  /** hostif mac address */
  switch_mac_addr_t mac;

  /** hostif v4 ip address */
  switch_ip_addr_t v4addr;

  /** hostif v6 ip address */
  switch_ip_addr_t v6addr;

  /** vlan action */
  switch_hostif_vlan_action_t vlan_action;

  /** oper status */
  bool operstatus;

  /** admin state */
  bool admin_state;

  /** netlink socket get cb */
  struct nl_sock *(*nl_sock_get_fn)(const char *ifname,
                                    switch_handle_t hostif_handle);

} switch_hostif_t;

/** rx key for net filter */
typedef struct switch_hostif_rx_filter_key_s {
  /** port handle */
  switch_handle_t port_handle;

  /** lag handle */
  switch_handle_t lag_handle;

  /** interface handle */
  switch_handle_t intf_handle;

  /** handle - vlan/ln/interface */
  switch_handle_t handle;

  /** reason code */
  switch_hostif_reason_code_t reason_code;

  /** reason code mask */
  switch_uint32_t reason_code_mask;

} switch_hostif_rx_filter_key_t;

/** rx action for net filter */
typedef struct switch_hostif_rx_filter_action_s {
  /** channel type */
  switch_hostif_channel_t channel_type;

  /** hostif handle */
  switch_handle_t hostif_handle;

  /** vlan action */
  switch_hostif_vlan_action_t vlan_action;

} switch_hostif_rx_filter_action_t;

/** tx key for net filter */
typedef struct switch_hostif_tx_filter_key_s {
  /** hostif handle */
  switch_handle_t hostif_handle;

  /** vlan id */
  switch_vlan_t vlan_id;

} switch_hostif_tx_filter_key_t;

/** tx net filter ation */
typedef struct switch_packet_tx_filter_action_s {
  /** tx bypass flags */
  switch_uint16_t bypass_flags;

  /** handle - port/interface/vlan/ln */
  switch_handle_t handle;

} switch_hostif_tx_filter_action_t;

/** CPU Rx Callback */
typedef void (*switch_hostif_rx_callback_fn)(
    switch_hostif_packet_t *hostif_packet);

/**
Register for callback on reception of packets qualified by reason
@param device device to register callback
@param cb_fn callback function pointer
*/
switch_status_t switch_api_hostif_rx_callback_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn,
    void *cookie);

/**
Deregister for callback on reception of packets qualified by reason
@param device device to register callback
@param cb_fn callback function pointer
*/
switch_status_t switch_api_hostif_rx_callback_deregister(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn);

/**
Allocate packe memory to transmit
@param device device
@param hostif_packet packet info
*/
switch_status_t switch_api_hostif_tx_packet(
    switch_hostif_packet_t *hostif_packet);

/**
 Create a hostif profile to be shared across multiple reason codes
 @param device device
 @param hostif_group hostif group info
 */
switch_status_t switch_api_hostif_group_create(
    const switch_device_t device,
    const switch_hostif_group_t *hif_group,
    switch_handle_t *hif_group_handle);

/**
 Delete a hostif profile that is shared across multiple reason codes
 @param device device
 @param hostif_group_id hostif group id
 */
switch_status_t switch_api_hostif_group_delete(
    const switch_device_t device, const switch_handle_t hif_group_handle);

/**
 Add a hostif reason code to trap/forward the packet to cpu
 @param device device
 @param rcode_api_info reason code info
 */
switch_status_t switch_api_hostif_reason_code_create(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info,
    switch_handle_t *hostif_rcode_handle);

/**
 Update a hostif reason code to trap/forward the packet to cpu
 @param device device
 @param rcode_api_info reason code info
 */
switch_status_t switch_api_hostif_reason_code_update(
    const switch_device_t device,
    const switch_handle_t hostif_rcode_handle,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info);

/**
 Remove a reason code to trap/forward the packet to cpu
 @param device device
 @param reason_code reason code
 */
switch_status_t switch_api_hostif_reason_code_delete(
    const switch_device_t device, const switch_handle_t hostif_rcode_handle);

/**
 Create host interface
 @param device device
 @param hostif host interface
 */
switch_status_t switch_api_hostif_create(const switch_device_t device,
                                         const switch_uint64_t flags,
                                         const switch_hostif_t *hostif,
                                         switch_handle_t *hostif_handle);

switch_status_t switch_api_hostif_update(const switch_device_t device,
                                         const switch_handle_t hostif_handle,
                                         const switch_uint64_t flags,
                                         switch_hostif_t *hostif);
/**
 Delete host interface
 @param device device
 @param hostif_handle hostif handle
 */
switch_status_t switch_api_hostif_delete(const switch_device_t device,
                                         const switch_handle_t hostif_handle);

/**
 Return nexthop based on reason code
 @param rcode Reason code
 */
switch_status_t switch_api_hostif_nhop_get(switch_device_t device,
                                           switch_hostif_reason_code_t rcode,
                                           switch_handle_t *nhop_handle);

/**
 Create tx net filter
 @param device device
 @param tx_key tx net filter key
 @param tx_action tx net filter action
 */
switch_status_t switch_api_hostif_tx_filter_create(
    const switch_device_t device,
    const switch_hostif_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_tx_filter_key_t *tx_key,
    const switch_hostif_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle);

/**
 Delete tx net filter
 @param device device
 @param tx_key tx net filter key
 */
switch_status_t switch_api_hostif_tx_filter_delete(
    const switch_device_t device, const switch_handle_t tx_filter_handle);

/**
 Create rx net filter
 @param device device
 @param rx_key rx net filter key
 @param rx_action rx net filter action
 */
switch_status_t switch_api_hostif_rx_filter_create(
    const switch_device_t device,
    const switch_hostif_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_rx_filter_key_t *rx_key,
    const switch_hostif_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle);

/**
 Delete rx net filter
 @param device device
 @param rx_key rx net filter key
 */
switch_status_t switch_api_hostif_rx_filter_delete(
    const switch_device_t device, const switch_handle_t rx_filter_handle);

/**
 create a meter for control plane policing
 @param device device
 @param api_meter_info meter struct
 @param meter_handle return meter handle
 */
switch_status_t switch_api_hostif_meter_create(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle);

/**
 delete meter for control plane policing
 @param device device
 @param meter_handle meter handle
*/
switch_status_t switch_api_hostif_meter_delete(
    const switch_device_t device, const switch_handle_t meter_handle);

switch_status_t switch_api_hostif_handle_get(const switch_device_t device,
                                             const char *intf_name,
                                             switch_handle_t *hostif_handle);

switch_status_t switch_api_hostif_group_get(
    const switch_device_t device,
    const switch_handle_t hostif_group_handle,
    switch_hostif_group_t *hostif_group);

switch_status_t switch_api_hostif_group_meter_set(const switch_device_t device,
                                                  switch_handle_t handle,
                                                  switch_handle_t meter_handle);

switch_status_t switch_api_hostif_oper_state_set(switch_device_t device,
                                                 switch_handle_t hostif_handle,
                                                 bool oper_state);

switch_status_t switch_api_hostif_oper_state_get(switch_device_t device,
                                                 switch_handle_t hostif_handle,
                                                 bool *oper_state);
/** @} */  // end of Host Interface API

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_HOSTIF_H__ */
