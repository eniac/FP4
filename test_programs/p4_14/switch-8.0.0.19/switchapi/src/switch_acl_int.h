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

#ifndef __SWITCH_ACL_INT_H__
#define __SWITCH_ACL_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** acl handle wrappers */
#define switch_acl_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_ACL, sizeof(switch_acl_info_t))

#define switch_acl_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ACL, _handle)

#define switch_acl_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_ACL, _handle, (void **)_info)

/** acl group handle wrappers */
#define switch_acl_group_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_ACL_GROUP, sizeof(switch_acl_group_info_t))

#define switch_acl_group_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ACL_GROUP, _handle)

#define switch_acl_group_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_ACL_GROUP, _handle, (void **)_info)

/** acl group member handle wrappers */
#define switch_acl_group_member_handle_create(_device)      \
  switch_handle_create(_device,                             \
                       SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER, \
                       sizeof(switch_acl_group_member_info_t))

#define switch_acl_group_member_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER, _handle)

#define switch_acl_group_member_get(_device, _handle, _info) \
  switch_handle_get(                                         \
      _device, SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER, _handle, (void **)_info)

/** ace handle wrappers */
#define switch_ace_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_ACE, sizeof(switch_ace_info_t))

#define switch_ace_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ACE, _handle)

#define switch_ace_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_ACE, _handle, (void **)_info)

/** acl range wrappers */
#define switch_range_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_RANGE, sizeof(switch_range_info_t))

#define switch_range_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RANGE, _handle)

#define switch_range_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_RANGE, _handle, (void **)_info)

#define SWITCH_ACL_LABEL_INVALID 0

#define SWITCH_DEFAULT_INTERNAL_MAX_ACL 100
#define SWITCH_DEFAULT_INERNAL_MOD_ACL_HIGH_PRIO_START 1
#define SWITCH_DEFAULT_INTERNAL_ACL_HIGH_PRIO_START \
  SWITCH_DEFAULT_INTERNAL_MAX_ACL +                 \
      SWITCH_DEFAULT_INERNAL_MOD_ACL_HIGH_PRIO_START

#define SWITCH_DEFAULT_INTERNAL_MOD_ACL_LOW_PRIO_START (1 << 15) + 1
#define SWITCH_DEFAULT_INTERNAL_ACL_LOW_PRIO_START \
  SWITCH_DEFAULT_INTERNAL_MOD_ACL_LOW_PRIO_START + \
      SWITCH_DEFAULT_INTERNAL_MAX_ACL

#define SWITCH_BIND_POINT_SUPPORTED(_bp_type) \
  ((_bp_type == SWITCH_HANDLE_TYPE_PORT) ||   \
   (_bp_type == SWITCH_HANDLE_TYPE_LAG) ||    \
   (_bp_type == SWITCH_HANDLE_TYPE_VLAN) ||   \
   (_bp_type == SWITCH_HANDLE_TYPE_RIF) ||    \
   (_bp_type == SWITCH_HANDLE_TYPE_NONE))

#define SWITCH_ACL_LABEL_GET(_bp_type, _label, _port_lag_label, _bd_label) \
  do {                                                                     \
    _port_lag_label = SWITCH_ACL_LABEL_INVALID;                            \
    _bd_label = SWITCH_ACL_LABEL_INVALID;                                  \
    switch (_bp_type) {                                                    \
      case SWITCH_HANDLE_TYPE_PORT:                                        \
      case SWITCH_HANDLE_TYPE_LAG:                                         \
        _port_lag_label = handle_to_id(_label);                            \
        break;                                                             \
      case SWITCH_HANDLE_TYPE_VLAN:                                        \
      case SWITCH_HANDLE_TYPE_BD:                                          \
      case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:                             \
      case SWITCH_HANDLE_TYPE_RIF:                                         \
        _bd_label = handle_to_id(_label);                                  \
        break;                                                             \
      case SWITCH_HANDLE_TYPE_NONE:                                        \
        _port_lag_label = 0;                                               \
        _bd_label = 0;                                                     \
        break;                                                             \
      default:                                                             \
        SWITCH_LOG_ERROR(                                                  \
            "acl label get failed for bp type %d: "                        \
            "invalid bp type:(invalid parameter)\n",                       \
            _bp_type);                                                     \
        return SWITCH_STATUS_INVALID_PARAMETER;                            \
    }                                                                      \
  } while (0);

#define SWITCH_ACL_DIRECTION_GET(_device, _acl_handle, _direction, _status)   \
  do {                                                                        \
    if (SWITCH_ACL_HANDLE(_acl_handle)) {                                     \
      switch_acl_info_t *_acl_info = NULL;                                    \
      _status = switch_acl_get(_device, _acl_handle, &_acl_info);             \
      if (_status == SWITCH_STATUS_SUCCESS) {                                 \
        _direction = _acl_info->direction;                                    \
      }                                                                       \
    } else if (SWITCH_ACL_GROUP_HANDLE(_acl_handle)) {                        \
      switch_acl_group_info_t *_acl_group_info = NULL;                        \
      _status = switch_acl_group_get(_device, _acl_handle, &_acl_group_info); \
      if (_status == SWITCH_STATUS_SUCCESS) {                                 \
        _direction = _acl_group_info->direction;                              \
      }                                                                       \
    } else {                                                                  \
      SWITCH_ASSERT(0);                                                       \
    }                                                                         \
  } while (0);

static inline char *switch_acl_type_to_string(switch_acl_type_t type) {
  switch (type) {
    case SWITCH_ACL_TYPE_IP:
      return "ipv4";
    case SWITCH_ACL_TYPE_MAC:
      return "mac";
    case SWITCH_ACL_TYPE_IPV6:
      return "ipv6";
    case SWITCH_ACL_TYPE_SYSTEM:
      return "system";
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      return "egress system";
    case SWITCH_ACL_TYPE_IP_RACL:
      return "ipv4 racl";
    case SWITCH_ACL_TYPE_IPV6_RACL:
      return "ipv6 racl";
    case SWITCH_ACL_TYPE_MAC_QOS:
      return "mac qos_acl";
    case SWITCH_ACL_TYPE_IP_QOS:
      return "ipv4 qos_acl";
    case SWITCH_ACL_TYPE_IPV6_QOS:
      return "ipv6 qos_acl";
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      return "ipv4 mirror_acl";
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      return "ipv6 mirror_acl";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_bp_type_to_string(switch_handle_type_t bp_type) {
  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      return "none";
    case SWITCH_HANDLE_TYPE_PORT:
      return "port";
    case SWITCH_HANDLE_TYPE_LAG:
      return "lag";
    case SWITCH_HANDLE_TYPE_VLAN:
      return "vlan";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_action_to_string(
    switch_acl_action_t acl_action) {
  switch (acl_action) {
    case SWITCH_ACL_ACTION_NOP:
      return "nop";
    case SWITCH_ACL_ACTION_DROP:
      return "drop";
    case SWITCH_ACL_ACTION_PERMIT:
      return "permit";
    case SWITCH_ACL_ACTION_LOG:
      return "log";
    case SWITCH_ACL_ACTION_REDIRECT:
      return "redirect";
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
      return "redirect to cpu";
    case SWITCH_ACL_ACTION_COPY_TO_CPU:
      return "copy to cpu";
    case SWITCH_ACL_ACTION_MIRROR_AND_DROP:
      return "mirror and drop";
    case SWITCH_ACL_ACTION_SET_MIRROR:
      return "mirror";
    case SWITCH_ACL_ACTION_FLOOD_TO_VLAN:
      return "flood to vlan";
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS:
      return "qos";
    default:
      return "unknown";
  }
}

static inline char *switch_color_to_string(switch_color_t color) {
  switch (color) {
    case SWITCH_COLOR_GREEN:
      return "green";
    case SWITCH_COLOR_YELLOW:
      return "yellow";
    case SWITCH_COLOR_RED:
      return "red";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_ipv4_field_to_string(
    switch_acl_ip_field_t field) {
  switch (field) {
    case SWITCH_ACL_IP_FIELD_IPV4_SRC:
      return "ipv4 src";
    case SWITCH_ACL_IP_FIELD_IPV4_DEST:
      return "ipv4 dst";
    case SWITCH_ACL_IP_FIELD_IP_PROTO:
      return "ip proto";
    case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
      return "l4 src port range";
    case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
      return "l4 dst port range";
    case SWITCH_ACL_IP_FIELD_ICMP_TYPE:
      return "icmp type";
    case SWITCH_ACL_IP_FIELD_ICMP_CODE:
      return "icmp code";
    case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
      return "tcp flags";
    case SWITCH_ACL_IP_FIELD_TTL:
      return "ipv4 ttl";
    case SWITCH_ACL_IP_FIELD_IP_FLAGS:
      return "ip flags";
    case SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL:
      return "port lag label";
    case SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL:
      return "vlan label";
    case SWITCH_ACL_IP_FIELD_IP_FRAGMENT:
      return "fragment";
    case SWITCH_ACL_IP_FIELD_ETH_TYPE:
      return "eth type";
    case SWITCH_ACL_IP_FIELD_RMAC_HIT:
      return "tcp flags";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_ipv6_field_to_string(
    switch_acl_ipv6_field_t field) {
  switch (field) {
    case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
      return "ipv6 src";
    case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
      return "ipv6 dst";
    case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
      return "ip proto";
    case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
      return "l4 src port range";
    case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
      return "l4 dst port range";
    case SWITCH_ACL_IPV6_FIELD_ICMP_TYPE:
      return "icmp type";
    case SWITCH_ACL_IPV6_FIELD_ICMP_CODE:
      return "icmp code";
    case SWITCH_ACL_IPV6_FIELD_TCP_FLAGS:
      return "tcp flags";
    case SWITCH_ACL_IPV6_FIELD_TTL:
      return "ip ttl";
    case SWITCH_ACL_IPV6_FIELD_FLOW_LABEL:
      return "flow label";
    case SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL:
      return "port lag label";
    case SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL:
      return "vlan label";
    case SWITCH_ACL_IPV6_FIELD_ETH_TYPE:
      return "eth type";
    case SWITCH_ACL_IPV6_FIELD_RMAC_HIT:
      return "tcp flags";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_mac_field_to_string(
    switch_acl_mac_field_t field) {
  switch (field) {
    case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
      return "eth type";
    case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
      return "src mac";
    case SWITCH_ACL_MAC_FIELD_DEST_MAC:
      return "dst mac";
    case SWITCH_ACL_MAC_FIELD_VLAN_PRI:
      return "vlan pri";
    case SWITCH_ACL_MAC_FIELD_VLAN_CFI:
      return "vlan cfi";
    case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
      return "port lag label";
    case SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL:
      return "vlan label";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_ecn_acl_field_to_string(
    switch_acl_ecn_field_t field) {
  switch (field) {
    case SWITCH_ACL_ECN_FIELD_DSCP:
      return "dscp";
    case SWITCH_ACL_ECN_FIELD_ECN:
      return "ecn";
    case SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL:
      return "port lag label";
    default:
      return "unknown";
  }
}
static inline char *switch_acl_system_field_to_string(
    switch_acl_system_field_t field) {
  switch (field) {
    case SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE:
      return "eth type";
    case SWITCH_ACL_SYSTEM_FIELD_SOURCE_MAC:
      return "src mac";
    case SWITCH_ACL_SYSTEM_FIELD_DEST_MAC:
      return "dst mac";
    case SWITCH_ACL_SYSTEM_FIELD_PORT_VLAN_MAPPING_MISS:
      return "port vlan mapping miss";
    case SWITCH_ACL_SYSTEM_FIELD_IPSG_CHECK:
      return "ipsg check";
    case SWITCH_ACL_SYSTEM_FIELD_ACL_DENY:
      return "acl deny";
    case SWITCH_ACL_SYSTEM_FIELD_RACL_DENY:
      return "racl deny";
    case SWITCH_ACL_SYSTEM_FIELD_URPF_CHECK:
      return "urpf check";
    case SWITCH_ACL_SYSTEM_FIELD_METER_DROP:
      return "meter drop";
    case SWITCH_ACL_SYSTEM_FIELD_L3_COPY:
      return "l3 copy";
    case SWITCH_ACL_SYSTEM_FIELD_DROP:
      return "drop";
    case SWITCH_ACL_SYSTEM_FIELD_ROUTED:
      return "routed";
    case SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL:
      return "link local";
    case SWITCH_ACL_SYSTEM_FIELD_NEXTHOP_GLEAN:
      return "glean";
    case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_HIT:
      return "mcast_route_hit";
    case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_S_G_HIT:
      return "mcast_route_s_g_hit";
    case SWITCH_ACL_SYSTEM_FIELD_MCAST_RPF_FAIL:
      return "mcast_rpf_fail";
    case SWITCH_ACL_SYSTEM_FIELD_MCAST_COPY_TO_CPU:
      return "mcast_copy_to_cpu";
    case SWITCH_ACL_SYSTEM_FIELD_BD_CHECK:
      return "bd check";
    case SWITCH_ACL_SYSTEM_FIELD_TTL:
      return "ttl";
    case SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX:
      return "egerss ifindex";
    case SWITCH_ACL_SYSTEM_FIELD_STP_STATE:
      return "stp state";
    case SWITCH_ACL_SYSTEM_FIELD_CONTROL_FRAME:
      return "control plane";
    case SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED:
      return "ipv4 enabled";
    case SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED:
      return "ipv6_enabled";
    case SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT:
      return "rmac hit";
    case SWITCH_ACL_SYSTEM_FIELD_IF_CHECK:
      return "if check";
    case SWITCH_ACL_SYSTEM_FIELD_TUNNEL_IF_CHECK:
      return "tunnel if check";
    case SWITCH_ACL_SYSTEM_FIELD_REASON_CODE:
      return "reason code";
    case SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP:
      return "mirror on drop";
    case SWITCH_ACL_SYSTEM_FIELD_DROP_CTL:
      return "drop control";
    case SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL:
      return "port lag label";
    case SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL:
      return "vlan label";
    case SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR:
      return "storm control color";
    case SWITCH_ACL_SYSTEM_FIELD_L2_DST_MISS:
      return "l2 dst miss";
    case SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE:
      return "packet type";
    case SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE:
      return "encoded arp opcode";
    case SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP:
      return "fib hit myip";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_egress_field_to_string(
    switch_acl_egress_system_field_t field) {
  switch (field) {
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT:
      return "egress dest port";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_SRC_PORT:
      return "ingress dest port";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT:
      return "egress deflect";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK:
      return "l3 mtu check";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_ACL_DENY:
      return "acl deny";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_REASON_CODE:
      return "reason code";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP:
      return "mirror on drop";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE:
      return "queue dod enable";
    case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DROP_CTL:
      return "drop control";
    default:
      return "unknown";
  }
}

static inline char *switch_range_type_to_string(
    switch_range_type_t range_type) {
  switch (range_type) {
    case SWITCH_RANGE_TYPE_NONE:
      return "none";
    case SWITCH_RANGE_TYPE_SRC_PORT:
      return "src port";
    case SWITCH_RANGE_TYPE_DST_PORT:
      return "dst port";
    case SWITCH_RANGE_TYPE_VLAN:
      return "vlan";
    case SWITCH_RANGE_TYPE_PACKET_LENGTH:
      return "pkt length";
    default:
      return "unknown";
  }
}

static inline char *switch_acl_drop_stats_id_to_string(
    switch_int32_t drop_reason) {
  switch (drop_reason) {
    case DROP_UNKNOWN:
      return "unknown";
    case DROP_OUTER_SRC_MAC_ZERO:
      return "outer src mac zero";
    case DROP_OUTER_SRC_MAC_MULTICAST:
      return "outer src mac multicast";
    case DROP_OUTER_DST_MAC_ZERO:
      return "outer dst mac zero";
    case DROP_OUTER_ETHERNET_MISS:
      return "outer ethernet miss";
    case DROP_SRC_MAC_ZERO:
      return "src mac zero";
    case DROP_SRC_MAC_MULTICAST:
      return "src mac multicast";
    case DROP_DST_MAC_ZERO:
      return "dst mac zero";
    case DROP_OUTER_IP_VERSION_INVALID:
      return "outer ip version invalid";
    case DROP_OUTER_IP_TTL_ZERO:
      return "outer ip ttl zero";
    case DROP_OUTER_IP_SRC_MULTICAST:
      return "outer ip src multicast";
    case DROP_OUTER_IP_SRC_LOOPBACK:
      return "outer ip src loopback";
    case DROP_OUTER_IP_MISS:
      return "outer ip miss";
    case DROP_OUTER_IP_IHL_INVALID:
      return "outer ip ihl invalid";
    case DROP_IP_VERSION_INVALID:
      return "ip version invalid";
    case DROP_IP_TTL_ZERO:
      return "ip ttl zero";
    case DROP_IP_SRC_MULTICAST:
      return "ip src multicast";
    case DROP_IP_SRC_LOOPBACK:
      return "ip src loopback";
    case DROP_IP_IHL_INVALID:
      return "ip ihl invalid";
    case DROP_PORT_VLAN_MAPPING_MISS:
      return "pv mapping miss";
    case DROP_STP_STATE_LEARNING:
      return "stp state learning";
    case DROP_STP_STATE_BLOCKING:
      return "stp state blocking";
    case DROP_SAME_IFINDEX:
      return "same ifindex";
    case DROP_MULTICAST_SNOOPING_ENABLED:
      return "multicast snooping enabled";
    case DROP_MTU_CHECK_FAIL:
      return "mtu check fail";
    case DROP_TRAFFIC_MANAGER:
      return "tm";
    case DROP_METER:
      return "meter";
    case DROP_ACL_DENY:
      return "acl deny";
    case DROP_RACL_DENY:
      return "racl deny";
    case DROP_URPF_CHECK_FAIL:
      return "urpf check fail";
    case DROP_IPSG_MISS:
      return "ipsg miss";
    case DROP_IFINDEX:
      return "ifindex";
    case DROP_CPU_COLOR_YELLOW:
      return "cpu yellow";
    case DROP_CPU_COLOR_RED:
      return "cpu red";
    case DROP_STORM_CONTROL_COLOR_YELLOW:
      return "storm_control yellow";
    case DROP_STORM_CONTROL_COLOR_RED:
      return "storm_control red";
    case DROP_L2_MISS_UNICAST:
      return "l2_miss unicast";
    case DROP_L2_MISS_BROADCAST:
      return "l2_miss broadcast";
    case DROP_L2_MISS_MULTICAST:
      return "l2_miss multicast";
    case DROP_EGRESS_ACL_DENY:
      return "egress acl deny";
    case DROP_OTHERS_INGRESS:
      return "others ingress";
    case DROP_OTHERS_EGRESS:
      return "others egress";
    case DROP_NHOP:
      return "nhop";
    default:
      return "unknown";
  }
}

#define SWITCH_DROP_REASON_VALID(_drop_reason) \
  ((_drop_reason >= DROP_OUTER_SRC_MAC_ZERO) && (_drop_reason <= DROP_NHOP))

typedef struct switch_acl_default_info_s {
  /** flag to skip programming the acl */
  bool program_acl;

  /** flag to skip programming mod acl */
  bool program_mod_acl;

} switch_acl_default_info_t;

/** acl default entry enums */
typedef enum switch_acl_default_type_s {
  SWITCH_ACL_DROP = 1,
  SWITCH_ACL_PV_MISS = 2,
  SWITCH_ACL_STP_BLOCKED_DROP = 3,
  SWITCH_ACL_STP_LEARN_DROP = 4,
  SWITCH_ACL_DENY_DROP = 5,
  SWITCH_ACL_URPF_FAIL_DROP = 6,
  SWITCH_ACL_RACL_DENY_DROP = 7,
  SWITCH_ACL_METER_DROP = 8,
  SWITCH_ACL_SAME_IF_CHECK_DROP = 9,
  SWITCH_ACL_TTL_1_TO_CPU = 10,
  SWITCH_ACL_TTL_1_REDIRECT_TO_CPU = 11,
  SWITCH_ACL_IPV6_LINK_LOCAL_TO_CPU = 12,
  SWITCH_ACL_IPV6_LINK_LOCAL_REDIRECT_TO_CPU = 13,
  SWITCH_ACL_GLEAN = 14,
  SWITCH_ACL_SAME_BD_CHECK = 15,
  SWITCH_ACL_L3_COPY_TO_CPU = 16,
  SWITCH_ACL_DROP_STORM_CONTROL_COLOR_YELLOW = 17,
  SWITCH_ACL_DROP_STORM_CONTROL_COLOR_RED = 18,
  SWITCH_ACL_EGRESS_DEFLECT_QUEUE_DOD = 19,
  SWITCH_ACL_EGRESS_DEFLECT_MOD_WATCHLIST = 20,
  SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_DOD = 21,
  SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_NODOD = 22,
  SWITCH_ACL_L3_MTU_CHECK = 23,
  SWITCH_ACL_EGRESS_ACL_DENY = 24,
  SWITCH_ACL_EGRESS_DROP_CPU_COLOR_YELLOW = 25,
  SWITCH_ACL_EGRESS_DROP_CPU_COLOR_RED = 26,
  /* DROP_OTHERS and EGRESS_DROP_OTHERS must be the last two */
  SWITCH_ACL_DROP_OTHERS = 27,
  SWITCH_ACL_EGRESS_DROP_OTHERS = 28,
  /* Do not add any other ACL after EGRESS_DROP_OTHERS */
  SWITCH_ACL_DEFAULT_MAX
} switch_acl_default_type_t;

static inline void switch_acl_default_check(
    switch_acl_default_type_t acl_type,
    switch_acl_default_info_t *default_info) {
  switch_pd_feature_t *pd_feature = switch_pd_feature_get();
  /* mod_check set to true will cause a duplicate of this default rule
   * to be created, adding match criteria mod_watchlist_hit = 1 and
   * setting the action to mirror_and_drop. This should only be set
   * for default rules with action other than drop, since
   * switch_api_acl_rule_create already adds mod rules when the action
   * is drop. */
  bool mod_check = FALSE;

  default_info->program_acl = TRUE;
  default_info->program_mod_acl =
      pd_feature->mirror_on_drop | pd_feature->dtel_stateless_sup;

  switch (acl_type) {
    case SWITCH_ACL_DROP:
    case SWITCH_ACL_PV_MISS:
    case SWITCH_ACL_STP_BLOCKED_DROP:
    case SWITCH_ACL_STP_LEARN_DROP:
    case SWITCH_ACL_DENY_DROP:
      break;
    case SWITCH_ACL_URPF_FAIL_DROP:
      default_info->program_acl &= pd_feature->urpf;
      break;
    case SWITCH_ACL_RACL_DENY_DROP:
      default_info->program_acl &= pd_feature->racl;
      break;
    case SWITCH_ACL_METER_DROP:
      default_info->program_acl &= pd_feature->qos_metering;
      break;
    case SWITCH_ACL_SAME_IF_CHECK_DROP:
      default_info->program_acl &=
          !(pd_feature->ingress_uc_self_fwd_check_disable);
    case SWITCH_ACL_TTL_1_TO_CPU:
    case SWITCH_ACL_TTL_1_REDIRECT_TO_CPU:
    case SWITCH_ACL_IPV6_LINK_LOCAL_TO_CPU:
    case SWITCH_ACL_IPV6_LINK_LOCAL_REDIRECT_TO_CPU:
    case SWITCH_ACL_L3_COPY_TO_CPU:
    case SWITCH_ACL_GLEAN:
      break;
    case SWITCH_ACL_SAME_BD_CHECK:
      default_info->program_acl &=
          !(pd_feature->ingress_uc_self_fwd_check_disable);
      break;
    case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_YELLOW:
    case SWITCH_ACL_DROP_STORM_CONTROL_COLOR_RED:
      default_info->program_acl &= pd_feature->storm_control;
      break;
    case SWITCH_ACL_EGRESS_DEFLECT_QUEUE_DOD:
      default_info->program_acl &= pd_feature->dtel_stateless_sup;
      break;
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_WATCHLIST:
      default_info->program_acl &= pd_feature->mirror_on_drop;
      break;
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_DOD:
    case SWITCH_ACL_EGRESS_DEFLECT_MOD_AND_NODOD:
      default_info->program_acl =
          pd_feature->mirror_on_drop & pd_feature->dtel_stateless_sup;
      break;
    case SWITCH_ACL_L3_MTU_CHECK:
    case SWITCH_ACL_EGRESS_ACL_DENY:
      break;
    case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_YELLOW:
    case SWITCH_ACL_EGRESS_DROP_CPU_COLOR_RED:
      default_info->program_acl &= pd_feature->copp_color_drop;
      break;
    case SWITCH_ACL_DROP_OTHERS:
    case SWITCH_ACL_EGRESS_DROP_OTHERS:
      default_info->program_acl &= pd_feature->mirror_on_drop;
      break;
    default:
      break;
  }

  default_info->program_mod_acl &= default_info->program_acl;
  default_info->program_mod_acl &= mod_check;

  return;
}

/** Acl info struct */
typedef struct switch_acl_info_s {
  /** acl direction - ingress/egress */
  switch_direction_t direction;

  /** acl type */
  switch_acl_type_t type;

  /** acl bind point type */
  switch_handle_type_t bp_type;

  /** acl default group member */
  switch_handle_t default_group_member;

  /** acl default group */
  switch_handle_t default_group;

  /** list of acl groups */
  switch_list_t group_list;

  /** set of ace rules */
  switch_array_t rules;

  /** set of ace pd handles */
  switch_array_t pd_hdl_array;

} switch_acl_info_t;

/** Acl group info struct */
typedef struct switch_acl_group_info_s {
  /** acl direction - ingress/egress */
  switch_direction_t direction;

  /** list of bindpoints */
  switch_list_t handle_list;

  /** list of acl group members */
  switch_list_t acl_member_list;

  /** bind point type */
  switch_handle_type_t bp_type;

} switch_acl_group_info_t;

/** Acl group member info struct */
typedef struct switch_acl_group_member_info_s {
  /** acl group handle */
  switch_handle_type_t acl_group_handle;

  /** acl list handle */
  switch_handle_type_t acl_handle;

} switch_acl_group_member_info_t;

/**
 * access control entry info
 * identified by ace handle
 */
typedef struct switch_ace_info_s {
  /** acl parent handle */
  switch_handle_t acl_handle;

  /** ace priority */
  switch_uint32_t priority;

  /** number of tlvs */
  switch_uint16_t field_count;

  /** actual tlvs */
  void *fields;

  /** acl action - permit/deny/copy/redirect */
  switch_acl_action_t action;

  /** acl action parameters */
  switch_acl_action_params_t action_params;

  /** acl optional action parameters */
  switch_acl_opt_action_params_t opt_action_params;

  /** ace handle for corresponding MoD-specific hidden ace */
  switch_handle_t mod_ace_handle;

} switch_ace_info_t;

typedef switch_ace_info_t switch_acl_rule_t;

/** acl range info */
typedef struct switch_range_info_s {
  switch_uint32_t ref_count;

  /** range type - vlan/src port/dst port */
  switch_range_type_t range_type;

  /** direction - ingress/egress */
  switch_direction_t direction;

  /** range min/max value */
  switch_range_t range;

  /** hardware handle */
  switch_pd_hdl_t hw_entry[SWITCH_API_DIRECTION_EGRESS];

} switch_range_info_t;

typedef struct switch_acl_group_member_s {
  /** list node */
  switch_node_t node;

  /** acl list handle */
  switch_handle_t acl_handle;

} switch_acl_group_member_t;

typedef struct switch_acl_ref_group_s {
  /** list node */
  switch_node_t node;

  /** acl group handle */
  switch_handle_t acl_group_handle;

  /** list of pd handles */
  switch_array_t pd_hdl_array;

} switch_acl_ref_group_t;

typedef struct switch_acl_handle_s {
  /** list node */
  switch_node_t node;

  /** list of reference handles */
  switch_handle_t handle;

} switch_acl_handle_t;

/** acl device context */
typedef struct switch_acl_context_s {
  /** counter id allocator */
  switch_id_allocator_t *counter_index;

  /** default system acl handles */
  switch_handle_t acl_handle[SWITCH_ACL_DEFAULT_MAX];

  /** default system ace handles */
  switch_handle_t ace_handle[SWITCH_ACL_DEFAULT_MAX];

  /** default system ace handles for mirror on drop actions */
  switch_handle_t mod_ace_handle[SWITCH_ACL_DEFAULT_MAX];

} switch_acl_context_t;

switch_status_t switch_acl_init(switch_device_t device);

switch_status_t switch_acl_free(switch_device_t device);

switch_status_t switch_acl_default_entries_add(switch_device_t device);

switch_status_t switch_acl_default_entries_delete(switch_device_t device);

switch_status_t switch_system_acl_default_entries_delete(
    switch_device_t device);

switch_status_t switch_system_acl_default_entries_add(switch_device_t device);

switch_status_t switch_acl_print_kvp(switch_acl_type_t acl_type,
                                     void *acl_kvp,
                                     switch_uint32_t acl_kvp_count,
                                     char *buffer,
                                     switch_int32_t buffer_size);

switch_status_t switch_acl_drop_stats_dump(const switch_device_t device,
                                           const void *cli_ctx);

#define SWITCH_API_L2_FDB_MISS_ACL_PRIORITY \
  SWITCH_DEFAULT_INTERNAL_ACL_LOW_PRIO_START + SWITCH_DEFAULT_INTERNAL_MAX_ACL
#define SWITCH_API_L2_MISS_BURST_SIZE 100000
#define SWITCH_API_L2_MISS_RATE_BPS 100000

#define SWITCH_ACL_FIELD_CHECK(payload, type, _field, count) \
  {                                                          \
    type *acl = (type *)payload;                             \
    for (int i = 0; i < count; i++) {                        \
      if (acl[i].field == _field) {                          \
        return SWITCH_STATUS_INVALID_PARAMETER;              \
      }                                                      \
    }                                                        \
  }

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_ACL_INT_H__ */
