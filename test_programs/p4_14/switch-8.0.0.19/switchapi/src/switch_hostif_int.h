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

#ifndef __SWITCH_HOSTIF_INT_H__
#define __SWITCH_HOSTIF_INT_H__

#include "switch_packet_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_CPU_PORT_ID 64

#define SWITCH_HOSTIF_GROUP_SIZE 64

#define SWITCH_HOSTIF_RANGE_HANDLE_MAX 8

#define SWITCH_HOSTIF_SIZE 1024

#define SWITCH_HOSTIF_HASHTABLE_SIZE SWITCH_HOSTIF_SIZE

#define SWITCH_HOSTIF_RX_FILTER_SIZE 4096

#define SWITCH_HOSTIF_TX_FILTER_SIZE 4096

#define SWITCH_MAX_RX_CALLBACK 32

#define SWITCH_HOSTIF_IP_PROTO_TCP 6

#define SWITCH_HOSTIF_IP_PROTO_UDP 17

#define SWITCH_HOSTIF_IP_PROTO_UDP 17

#define SWITCH_HOSTIF_IP_PROTO_VRRP 112

#define SWITCH_HOSTIF_BGP_PORT 179

#define SWITCH_HOSTIF_SSH_PORT 22

#define SWITCH_HOSTIF_SNMP_PORT 161

#define SWITCH_HOSTIF_DHCP_PORT1 67

#define SWITCH_HOSTIF_DHCP_PORT2 68

#define SWITCH_HOSTIF_BFD_DST_PORT 3784

#define SWITCH_HOSTIF_PTP_DST_PORT1 319
#define SWITCH_HOSTIF_PTP_DST_PORT2 320

/** Hostif hashtable random seed value */
#define SWITCH_HOSTIF_HASH_SEED 0x123456

#define SWITCH_HOSTIF_HASH_KEY_SIZE SWITCH_HOSTIF_NAME_SIZE

#define switch_hostif_group_handle_create(_device)      \
  switch_handle_create(_device,                         \
                       SWITCH_HANDLE_TYPE_HOSTIF_GROUP, \
                       sizeof(switch_hostif_group_info_t))

#define switch_hostif_group_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF_GROUP, _handle)

#define switch_hostif_group_get(_device, _handle, _info) \
  switch_handle_get(                                     \
      _device, SWITCH_HANDLE_TYPE_HOSTIF_GROUP, _handle, (void **)_info)

#define switch_hostif_handle_create(_device) \
  switch_handle_create(                      \
      _device, SWITCH_HANDLE_TYPE_HOSTIF, sizeof(switch_hostif_info_t))

#define switch_hostif_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF, _handle)

#define switch_hostif_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_HOSTIF, _handle, (void **)_info)

#define switch_hostif_table_entry_handle_create(_device)      \
  switch_handle_create(_device,                               \
                       SWITCH_HANDLE_TYPE_HOSTIF_TABLE_ENTRY, \
                       sizeof(switch_hostif_table_entry_info_t))

#define switch_hostif_table_entry_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF_TABLE_ENTRY, _handle)

#define switch_hostif_table_entry_get(_device, _handle, _info) \
  switch_handle_get(                                           \
      _device, SWITCH_HANDLE_TYPE_HOSTIF_TABLE_ENTRY, _handle, (void **)_info)

#define switch_hostif_rx_filter_handle_create(_device)      \
  switch_handle_create(_device,                             \
                       SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER, \
                       sizeof(switch_hostif_rx_filter_info_t))

#define switch_hostif_rx_filter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER, _handle)

#define switch_hostif_rx_filter_get(_device, _handle, _info) \
  switch_handle_get(                                         \
      _device, SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER, _handle, (void **)_info)

#define switch_hostif_tx_filter_handle_create(_device)      \
  switch_handle_create(_device,                             \
                       SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER, \
                       sizeof(switch_hostif_tx_filter_info_t))

#define switch_hostif_tx_filter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER, _handle)

#define switch_hostif_tx_filter_get(_device, _handle, _info) \
  switch_handle_get(                                         \
      _device, SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER, _handle, (void **)_info)

#define switch_hostif_rcode_handle_create(_device)            \
  switch_handle_create(_device,                               \
                       SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE, \
                       sizeof(switch_hostif_rcode_info_t))

#define switch_hostif_rcode_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE, _handle)

#define switch_hostif_rcode_get(_device, _handle, _info) \
  switch_handle_get(                                     \
      _device, SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE, _handle, (void **)_info)

#define SWITCH_HOSTIF_HANDLE_GET(_device, _handle, _hif_handle, _status) \
  do {                                                                   \
    __hif_handle = SWITCH_API_INVALID_HANDLE;                            \
    if (SWITCH_PORT_HANDLE(_handle)) {                                   \
      switch_port_info_t *_port_info = NULL;                             \
      _status = switch_port_get(_device, _handle, &_port_info);          \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _hif_handle = _port_info->hostif_handle;                           \
    } else if (SWITCH_LAG_HANDLE(_handle)) {                             \
      switch_lag_info_t *_lag_info = NULL;                               \
      _status = switch_lag_get(_device, _handle, &_lag_info);            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      __hif_handle = _lag_info->hostif_handle;                           \
    } else if (SWITCH_INTERFACE_HANDLE(_handle)) {                       \
      switch_interface_info_t *_intf_info = NULL;                        \
      _status = switch_interface_get(_device, _handle, &_intf_info);     \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _hif_handle = _intf_info->hostif_handle;                           \
    } else if (SWITCH_VLAN_HANDLE(_handle)) {                            \
      switch_vlan_info_t *_vlan_info = NULL;                             \
      _status = switch_vlan_get(_device, _handle, &_vlan_info);          \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _hif_handle = vlan_info->hostif_handle;                            \
    } else if (SWITCH_LN_HANDLE(_handle)) {                              \
      switch_ln_info_t *_ln_info = NULL;                                 \
      _status = switch_ln_get(_device, _handle, &_ln_info);              \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _hif_handle = ln_info->hostif_handle;                              \
    } else if (SWITCH_RIF_HANDLE(_handle)) {                             \
      switch_rif_info_t *_rif_info = NULL;                               \
      _status = switch_rif_get(_device, _handle, &_rif_info);            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _hif_handle = rif_info->hostif_handle;                             \
    } else {                                                             \
      SWITCH_ASSERT(0);                                                  \
    }                                                                    \
  } while (0);

#define SWITCH_HOSTIF_HANDLE_SET(_device, _handle, _hif_handle, _status) \
  do {                                                                   \
    if (SWITCH_PORT_HANDLE(_handle)) {                                   \
      switch_port_info_t *_port_info = NULL;                             \
      _status = switch_port_get(_device, _handle, &_port_info);          \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _port_info->hostif_handle = _hif_handle;                           \
    } else if (SWITCH_LAG_HANDLE(_handle)) {                             \
      switch_lag_info_t *_lag_info = NULL;                               \
      _status = switch_lag_get(_device, _handle, &_lag_info);            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _lag_info->hostif_handle = _hif_handle;                            \
    } else if (SWITCH_INTERFACE_HANDLE(_handle)) {                       \
      switch_interface_info_t *_intf_info = NULL;                        \
      _status = switch_interface_get(_device, _handle, &_intf_info);     \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _intf_info->hostif_handle = _hif_handle;                           \
    } else if (SWITCH_VLAN_HANDLE(_handle)) {                            \
      switch_vlan_info_t *_vlan_info = NULL;                             \
      _status = switch_vlan_get(_device, _handle, &_vlan_info);          \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _vlan_info->hostif_handle = _hif_handle;                           \
    } else if (SWITCH_LN_HANDLE(_handle)) {                              \
      switch_ln_info_t *_ln_info = NULL;                                 \
      _status = switch_ln_get(_device, _handle, &_ln_info);              \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _ln_info->hostif_handle = _hif_handle;                             \
    } else if (SWITCH_RIF_HANDLE(_handle)) {                             \
      switch_rif_info_t *_rif_info = NULL;                               \
      _status = switch_rif_get(_device, _handle, &_rif_info);            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      _rif_info->hostif_handle = _hif_handle;                            \
    } else {                                                             \
      SWITCH_ASSERT(0);                                                  \
    }                                                                    \
  } while (0);

#define SWITCH_HOSTIF_REASON_CODE_VALID(_rc)           \
  ((_rc <= SWITCH_HOSTIF_REASON_CODE_PTP) ||           \
   (_rc >= SWITCH_HOSTIF_REASON_CODE_L2_START &&       \
    _rc < SWITCH_HOSTIF_REASON_CODE_L2_END) ||         \
   (_rc >= SWITCH_HOSTIF_REASON_CODE_L3_START &&       \
    _rc < SWITCH_HOSTIF_REASON_CODE_L3_END) ||         \
   (_rc >= SWITCH_HOSTIF_REASON_CODE_LOCAL_IP_START && \
    _rc < SWITCH_HOSTIF_REASON_CODE_LOCAL_IP_END))

static inline const char *switch_hostif_code_to_string(
    switch_hostif_reason_code_t reason_code) {
  switch (reason_code) {
    case SWITCH_HOSTIF_REASON_CODE_STP:
      return "stp";
    case SWITCH_HOSTIF_REASON_CODE_LACP:
      return "lacp";
    case SWITCH_HOSTIF_REASON_CODE_LLDP:
      return "lldp";
    case SWITCH_HOSTIF_REASON_CODE_PVRST:
      return "pvrst";
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_QUERY:
      return "igmp query";
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_LEAVE:
      return "igmp leave";
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V1_REPORT:
      return "igmp v1 report";
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V2_REPORT:
      return "igmp v2 report";
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V3_REPORT:
      return "igmp v3 report";
    case SWITCH_HOSTIF_REASON_CODE_IGMP:
      return "igmp";
    case SWITCH_HOSTIF_REASON_CODE_SAMPLEPACKET:
      return "sflow";
    case SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST:
      return "arp-request";
    case SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE:
      return "arp-response";
    case SWITCH_HOSTIF_REASON_CODE_DHCP:
      return "dhcp";
    case SWITCH_HOSTIF_REASON_CODE_OSPF:
      return "ospf";
    case SWITCH_HOSTIF_REASON_CODE_PIM:
      return "pim";
    case SWITCH_HOSTIF_REASON_CODE_VRRP:
      return "vrrp";
    case SWITCH_HOSTIF_REASON_CODE_BGP:
      return "bgp";
    case SWITCH_HOSTIF_REASON_CODE_BGPV6:
      return "bgpv6";
    case SWITCH_HOSTIF_REASON_CODE_BFD_RX:
      return "bfd_rx";
    case SWITCH_HOSTIF_REASON_CODE_BFD_EVENT:
      return "bfd_event";
    case SWITCH_HOSTIF_REASON_CODE_DHCPV6:
      return "dhcpv6";
    case SWITCH_HOSTIF_REASON_CODE_OSPFV6:
      return "ospf6";
    case SWITCH_HOSTIF_REASON_CODE_VRRPV6:
      return "vrrpv6";
    case SWITCH_HOSTIF_REASON_CODE_IPV6_NEIGHBOR_DISCOVERY:
      return "ipv6nd";
    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_V2:
      return "mld v1 v2";
    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_REPORT:
      return "mpd v1 report";
    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_DONE:
      return "mpd v1 done";
    case SWITCH_HOSTIF_REASON_CODE_MLD_V2_REPORT:
      return "mld v2 report";
    case SWITCH_HOSTIF_REASON_CODE_L3_MTU_ERROR:
      return "l3 mtu error";
    case SWITCH_HOSTIF_REASON_CODE_TTL_ERROR:
      return "ttl error";
    case SWITCH_HOSTIF_REASON_CODE_GLEAN:
      return "glean";
    case SWITCH_HOSTIF_REASON_CODE_MYIP:
      return "myip";
    case SWITCH_HOSTIF_REASON_CODE_ICMP_REDIRECT:
      return "icmp redirect";
    case SWITCH_HOSTIF_REASON_CODE_SRC_IS_LINK_LOCAL:
      return "link-local";
    case SWITCH_HOSTIF_REASON_CODE_BROADCAST:
      return "broadcast";
    case SWITCH_HOSTIF_REASON_CODE_SSH:
      return "ssh";
    case SWITCH_HOSTIF_REASON_CODE_SNMP:
      return "snmp";
    case SWITCH_HOSTIF_REASON_CODE_SFLOW_SAMPLE:
      return "sflow-sample";
    case SWITCH_HOSTIF_REASON_CODE_L2_MISS_UNICAST:
      return "l2-miss-unicast";
    case SWITCH_HOSTIF_REASON_CODE_L2_MISS_MULTICAST:
      return "l2-miss-multicast";
    case SWITCH_HOSTIF_REASON_CODE_L2_MISS_BROADCAST:
      return "l2-miss-broadcast";
    case SWITCH_HOSTIF_REASON_CODE_PTP:
      return "ptp";
    default:
      return "unknown";
  }
}

static inline switch_pktdriver_vlan_action_t
switch_hostif_vlan_action_to_pktdriver_vlan_action(
    switch_hostif_vlan_action_t vlan_action) {
  switch (vlan_action) {
    case SWITCH_HOSTIF_VLAN_ACTION_NONE:
      return SWITCH_PACKET_VLAN_ACTION_NONE;
    case SWITCH_HOSTIF_VLAN_ACTION_ADD:
      return SWITCH_PACKET_VLAN_ACTION_ADD;
    case SWITCH_HOSTIF_VLAN_ACTION_REMOVE:
      return SWITCH_PACKET_VLAN_ACTION_REMOVE;
    default:
      return SWITCH_PACKET_VLAN_ACTION_NONE;
  }
}

static inline switch_uint64_t switch_hostif_rx_flags_to_pktdriver_rx_flags(
    switch_uint64_t flags) {
  switch_uint64_t tmp_flags = 0;

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_PORT_HANDLE) {
    tmp_flags |= SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_INTF_HANDLE) {
    tmp_flags |= SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE) {
    tmp_flags |= SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_REASON_CODE) {
    tmp_flags |= SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_ETHER_TYPE) {
    tmp_flags |= SWITCH_PKTDRIVER_RX_FILTER_ATTR_ETHER_TYPE;
  }

  return tmp_flags;
}

static inline switch_uint64_t switch_hostif_tx_flags_to_pktdriver_tx_flags(
    switch_uint64_t flags) {
  switch_uint64_t tmp_flags = 0;

  if (flags & SWITCH_HOSTIF_TX_FILTER_ATTR_HOSTIF_HANDLE) {
    tmp_flags |= SWITCH_PKTDRIVER_TX_FILTER_ATTR_HOSTIF_FD;
  }

  if (flags & SWITCH_HOSTIF_TX_FILTER_ATTR_VLAN_ID) {
    tmp_flags |= SWITCH_PKTDRIVER_TX_FILTER_ATTR_VLAN_ID;
  }

  return tmp_flags;
}

static inline switch_pktdriver_rx_filter_priority_t
switch_hostif_rx_priority_to_pktdriver_rx_priority(
    switch_hostif_rx_filter_priority_t rx_priority) {
  switch (rx_priority) {
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_MIN:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MIN;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_PORT:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PORT;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_INTERFACE:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_INTERFACE;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_VLAN:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_VLAN;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_LN:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_LN;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_RIF:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_RIF;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO1:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO1;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO2:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO2;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO3:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO3;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_PRIO4:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO4;
    case SWITCH_HOSTIF_RX_FILTER_PRIORITY_MAX:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MAX;
    default:
      return SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MIN;
  }
}

static inline switch_pktdriver_tx_filter_priority_t
switch_hostif_tx_priority_to_pktdriver_tx_priority(
    switch_hostif_tx_filter_priority_t tx_priority) {
  switch (tx_priority) {
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_MIN:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MIN;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_HOSTIF:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_HOSTIF;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO1:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO1;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO2:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO2;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO3:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO3;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_PRIO4:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO4;
    case SWITCH_HOSTIF_TX_FILTER_PRIORITY_MAX:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MAX;
    default:
      return SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MIN;
  }
}

static inline char *switch_hostif_vlan_action_to_string(
    switch_hostif_vlan_action_t vlan_action) {
  switch (vlan_action) {
    case SWITCH_HOSTIF_VLAN_ACTION_NONE:
      return "none";
    case SWITCH_HOSTIF_VLAN_ACTION_ADD:
      return "add";
    case SWITCH_HOSTIF_VLAN_ACTION_REMOVE:
      return "remove";
    default:
      return "none";
  }
}

#define SWITCH_HOSTIF_TX_FILTER_DEFAULT(                                    \
    _tx_key, _tx_action, _priority, _flags, _handle, _hif_handle, _filter)  \
  do {                                                                      \
    _filter = TRUE;                                                         \
    switch_handle_t _tmp_handle = SWITCH_API_INVALID_HANDLE;                \
    switch_handle_t _tmp_intf_handle = SWITCH_API_INVALID_HANDLE;           \
    switch_status_t _status = SWITCH_STATUS_SUCCESS;                        \
    switch (switch_handle_type_get(_handle)) {                              \
      case SWITCH_HANDLE_TYPE_PORT:                                         \
        _tx_action.bypass_flags = SWITCH_BYPASS_ALL;                        \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_INTERFACE:                                    \
        _status =                                                           \
            switch_api_interface_handle_get(device, _handle, &_tmp_handle); \
        if (_status == SWITCH_STATUS_SUCCESS &&                             \
            SWITCH_LAG_HANDLE(_tmp_handle)) {                               \
          _filter = FALSE;                                                  \
        }                                                                   \
        _tx_action.bypass_flags = SWITCH_BYPASS_ALL;                        \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_RIF: {                                        \
        switch_rif_info_t *_rif_info = NULL;                                \
        _tx_action.bypass_flags = SWITCH_BYPASS_ALL;                        \
        _status = switch_rif_get(device, _handle, &_rif_info);              \
        if (_status == SWITCH_STATUS_SUCCESS) {                             \
          _tx_action.bypass_flags =                                         \
              (_rif_info->api_rif_info.rif_type == SWITCH_RIF_TYPE_INTF)    \
                  ? SWITCH_BYPASS_ALL                                       \
                  : SWITCH_BYPASS_NONE | SWITCH_BYPASS_SYSTEM_ACL;          \
        }                                                                   \
        _tmp_intf_handle = _rif_info->api_rif_info.intf_handle;             \
        _status = switch_api_interface_handle_get(                          \
            device, _tmp_intf_handle, &_tmp_handle);                        \
        if (_status == SWITCH_STATUS_SUCCESS &&                             \
            SWITCH_LAG_HANDLE(_tmp_handle)) {                               \
          _filter = FALSE;                                                  \
        }                                                                   \
      } break;                                                              \
      case SWITCH_HANDLE_TYPE_LAG:                                          \
        _filter = FALSE;                                                    \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_VLAN:                                         \
      case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:                              \
        _tx_action.bypass_flags =                                           \
            SWITCH_BYPASS_NONE | SWITCH_BYPASS_SYSTEM_ACL;                  \
        break;                                                              \
      default:                                                              \
        break;                                                              \
    }                                                                       \
    _tx_action.handle = _handle;                                            \
    _tx_key.hostif_handle = _hif_handle;                                    \
    _priority = SWITCH_HOSTIF_TX_FILTER_PRIORITY_HOSTIF;                    \
    _flags = SWITCH_HOSTIF_TX_FILTER_ATTR_HOSTIF_HANDLE;                    \
  } while (0);

#define SWITCH_HOSTIF_RX_FILTER_DEFAULT(                                    \
    _rx_key, _rx_action, _priority, _flags, _handle, _hif_handle, _filter)  \
  do {                                                                      \
    _filter = TRUE;                                                         \
    switch_handle_t _tmp_handle = SWITCH_API_INVALID_HANDLE;                \
    switch_handle_t _tmp_intf_handle = SWITCH_API_INVALID_HANDLE;           \
    switch_status_t _status = SWITCH_STATUS_SUCCESS;                        \
    switch (switch_handle_type_get(_handle)) {                              \
      case SWITCH_HANDLE_TYPE_PORT:                                         \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_PORT;                  \
        _flags = SWITCH_HOSTIF_RX_FILTER_ATTR_PORT_HANDLE;                  \
        _rx_key.port_handle = _handle;                                      \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_INTERFACE:                                    \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_INTERFACE;             \
        _flags = SWITCH_HOSTIF_RX_FILTER_ATTR_INTF_HANDLE;                  \
        _rx_key.intf_handle = _handle;                                      \
        _status =                                                           \
            switch_api_interface_handle_get(device, _handle, &_tmp_handle); \
        if (_status == SWITCH_STATUS_SUCCESS &&                             \
            SWITCH_LAG_HANDLE(_tmp_handle)) {                               \
          _filter = FALSE;                                                  \
        }                                                                   \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_LAG:                                          \
        _filter = FALSE;                                                    \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_VLAN:                                         \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_VLAN;                  \
        _flags = SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE;                       \
        _rx_key.handle = _handle;                                           \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:                              \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_LN;                    \
        _flags = SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE;                       \
        _rx_key.handle = _handle;                                           \
        break;                                                              \
      case SWITCH_HANDLE_TYPE_RIF: {                                        \
        switch_rif_info_t *_rif_info = NULL;                                \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_RIF;                   \
        _flags = SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE;                       \
        _rx_key.handle = _handle;                                           \
        _status = switch_rif_get(device, _handle, &_rif_info);              \
        if (_status == SWITCH_STATUS_SUCCESS) {                             \
          _tmp_intf_handle = _rif_info->api_rif_info.intf_handle;           \
          _status = switch_api_interface_handle_get(                        \
              device, _tmp_intf_handle, &_tmp_handle);                      \
          if (_status == SWITCH_STATUS_SUCCESS &&                           \
              SWITCH_LAG_HANDLE(_tmp_handle)) {                             \
            _filter = FALSE;                                                \
          }                                                                 \
        }                                                                   \
      } break;                                                              \
      default:                                                              \
        _priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_MIN;                   \
        _flags = 0;                                                         \
        break;                                                              \
    }                                                                       \
    _rx_action.hostif_handle = _hif_handle;                                 \
    _rx_action.channel_type = SWITCH_HOSTIF_CHANNEL_NETDEV;                 \
  } while (0);

typedef struct switch_hostif_rcode_info_s {
  /** acl handle */
  switch_handle_t acl_handle;

  /** counter handle */
  switch_handle_t counter_handle;

  /** system acl handle */
  switch_handle_t system_acl_handle;

  /** system acl counter handle */
  switch_handle_t system_counter_handle;

  /** range handles */
  switch_handle_t range_handles[SWITCH_HOSTIF_RANGE_HANDLE_MAX];

  /** app reason code info */
  switch_api_hostif_rcode_info_t rcode_api_info;

} switch_hostif_rcode_info_t;

typedef struct switch_hostif_info_s {
  /** app hostif info */
  switch_hostif_t hostif;

  /** valid flags */
  switch_uint64_t flags;

  /** hashtable node */
  switch_hashnode_t node;

  /** netdev fd */
  switch_fd_t hostif_fd;

  /** hostif handle - self */
  switch_handle_t hostif_handle;

  /** knet hostif handle - used with kernel packet processing */
  switch_knet_hostif_t knet_hostif_handle;

  /** rx filter */
  switch_handle_t rx_filter_handle;

  /** KNET RX filter handle*/
  switch_knet_filter_t knet_rx_filter_handle;

  /** tx filter */
  switch_handle_t tx_filter_handle;

} switch_hostif_info_t;

typedef struct switch_hostif_group_info_s {
  /** app hostif group */
  switch_hostif_group_t hif_group;

  /** reference count */
  switch_uint16_t ref_count;

} switch_hostif_group_info_t;

/** rx filter */
typedef struct switch_hostif_rx_filter_info_s {
  /** rx filter flags */
  switch_uint64_t flags;

  /** rx key */
  switch_hostif_rx_filter_key_t rx_key;

  /** rx action */
  switch_hostif_rx_filter_action_t rx_action;

  /** rx filter packet handle */
  switch_handle_t filter_handle;

  /** KNET Rx filter handle */
  switch_knet_filter_t knet_filter_handle;

  /** rx filter priority */
  switch_hostif_rx_filter_priority_t priority;

  /** list node */
  switch_node_t node;

  /** Rx filter keys used by packet driver */

  /** port number */
  switch_dev_port_t dev_port;

  /** ifindex */
  switch_ifindex_t ifindex;

  /** bridge domain */
  switch_bd_t bd;

  /** reason code */
  switch_hostif_reason_code_t reason_code;

  /** reason code mask */
  switch_uint32_t reason_code_mask;

  /** Rx filter data used by packet driver */

  /** netdev fd */
  switch_fd_t hostif_fd;

  /** Rx filter data used by KNET driver */

  /** knet hostif handle - used with kernel packet processing */
  switch_knet_hostif_t knet_hostif_handle;

} switch_hostif_rx_filter_info_t;

/** tx filter */
typedef struct switch_hostif_tx_filter_info_s {
  /** tx filter flags */
  switch_uint64_t flags;

  /** tx key */
  switch_hostif_tx_filter_key_t tx_key;

  /** tx action */
  switch_hostif_tx_filter_action_t tx_action;

  /** tx filter packet handle */
  switch_handle_t filter_handle;

  /** tx filter priority */
  switch_hostif_tx_filter_priority_t priority;

  /** list node */
  switch_node_t node;

  /** tx filter keys used by packet driver */

  /** netdev fd */
  switch_fd_t hostif_fd;

  /** KNET hostif interface handle */
  switch_knet_hostif_t knet_hostif_handle;

  /** tx filter data used by packet driver */

  /** bridge domain if tx bypass is false */
  switch_bd_t bd;

  /** port if tx bypass is true */
  switch_dev_port_t dev_port;

} switch_hostif_tx_filter_info_t;

typedef struct switch_hostif_cb_info_s {
  /* valid cb entry */
  bool valid;

  /** app id */
  switch_app_id_t app_id;

  /** cb fn */
  switch_hostif_rx_callback_fn cb_fn;

  /* app data */
  void *cookie;

} switch_hostif_cb_info_t;

typedef struct switch_hostif_context_s {
  /** hostif hashtable hased upon interface name */
  switch_hashtable_t hostif_hashtable;

  /** cpu interface handle */
  switch_handle_t intf_handle;

  /** cpu mirror session handle */
  switch_handle_t mirror_handle;

  /** array of reason code to rcode handle mapping */
  switch_handle_t rcode_handles[SWITCH_HOSTIF_REASON_CODE_MAX];

  /** list of rx callbacks */
  switch_hostif_cb_info_t rx_cb_list[SWITCH_MAX_RX_CALLBACK];

  /** ingress fabric pd handle */
  switch_pd_hdl_t ing_pd_hdl;

  /** rewrite pd handle */
  switch_pd_hdl_t rw_pd_hdl;

  /** tunnel rewrite pd handle */
  switch_pd_hdl_t tunnel_rw_pd_hdl;

  /** array of reason code to nhop handle mapping */
  switch_handle_t nhop_handles[SWITCH_HOSTIF_REASON_CODE_MAX];

} switch_hostif_context_t;

/*
 * Internal API's
 */
switch_status_t switch_hostif_init(switch_device_t device);

switch_status_t switch_hostif_free(switch_device_t device);

switch_status_t switch_hostif_default_entries_add(switch_device_t device);

switch_status_t switch_hostif_default_entries_delete(switch_device_t device);

switch_status_t switch_hostif_callback_rx(switch_packet_info_t *pkt_info);

switch_status_t switch_api_hostif_cpu_intf_handle_get(
    switch_device_t device, switch_handle_t *intf_handle);

switch_status_t switch_api_hostif_cpu_intf_info_get(
    switch_device_t device, switch_interface_info_t **intf_info);

switch_status_t switch_api_hostif_handle_dump(
    const switch_device_t device,
    const switch_handle_t hostif_handle,
    const void *cli_ctx);

switch_status_t switch_api_hostif_group_handle_dump(
    const switch_device_t device,
    const switch_handle_t hostif_group_handle,
    const void *cli_ctx);

switch_status_t switch_api_hostif_rcode_handle_dump(
    const switch_device_t device,
    const switch_handle_t rcode_handle,
    const void *cli_ctx);

switch_status_t switch_api_hostif_rx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t rx_filter_handle,
    const void *cli_ctx);

switch_status_t switch_api_hostif_tx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t tx_filter_handle,
    const void *cli_ctx);

switch_status_t switch_api_hostif_by_name_dump(const switch_device_t device,
                                               const char *intf_name,
                                               const void *cli_ctx);

#define SWITCH_HOSTIF_DEFAULT_POLICER_RATE 100000000000UL
#ifdef __cplusplus
}
#endif

#endif /* SWITCH_HOSTIF_INT_H__ */
