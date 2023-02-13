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

#ifndef _switch_acl_h_
#define _switch_acl_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_id.h"
#include "switch_nat.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup ACL ACL API
 *  API functions define and manipulate Access lists
 *  @{
 */  // begin of ACL API

/** ACL Types */
typedef enum switch_acl_type_ {
  SWITCH_ACL_TYPE_IP,              /**< IPv4 ACL */
  SWITCH_ACL_TYPE_MAC,             /**< MAC ACL */
  SWITCH_ACL_TYPE_IPV6,            /**< IPv6 ACL */
  SWITCH_ACL_TYPE_MAC_QOS,         /**< QoS ACL */
  SWITCH_ACL_TYPE_IP_QOS,          /**< QoS ACL */
  SWITCH_ACL_TYPE_IPV6_QOS,        /**< QoS ACL */
  SWITCH_ACL_TYPE_SYSTEM,          /**< Ingress System ACL */
  SWITCH_ACL_TYPE_EGRESS_SYSTEM,   /**< Egress System ACL */
  SWITCH_ACL_TYPE_EGRESS_IP_ACL,   /**< EGRESS IPv4 ACL */
  SWITCH_ACL_TYPE_EGRESS_IPV6_ACL, /**< EGRESS IPv6 ACL */
  SWITCH_ACL_TYPE_IP_RACL,         /**< IPv4 Route ACL */
  SWITCH_ACL_TYPE_IPV6_RACL,       /**< IPv6 Route ACL */
  SWITCH_ACL_TYPE_IP_MIRROR_ACL,   /**< IPv4 Mirror ACL */
  SWITCH_ACL_TYPE_IPV6_MIRROR_ACL, /**< IPv6 Mirror ACL */
  SWITCH_ACL_TYPE_ECN,             /**< ECN ACL */
  SWITCH_ACL_TYPE_MAX
} switch_acl_type_t;

/** Ingress/Egress Acl IP field enum */
typedef enum switch_acl_ip_field_ {
  SWITCH_ACL_IP_FIELD_IPV4_SRC,             /**< IPv4 Source address */
  SWITCH_ACL_IP_FIELD_IPV4_DEST,            /**< IPv4 Dest address */
  SWITCH_ACL_IP_FIELD_IP_PROTO,             /**< IP Protocol */
  SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port UDP/TCP */
  SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE, /**< L4 dest port range for UDP/TCP */
  SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT, /**< L4 source port value for UDP/TCP */
  SWITCH_ACL_IP_FIELD_L4_DEST_PORT,   /**< L4 source port value for UDP/TCP */
  SWITCH_ACL_IP_FIELD_ICMP_TYPE,      /**< ICMP type */
  SWITCH_ACL_IP_FIELD_ICMP_CODE,      /**< ICMP code */
  SWITCH_ACL_IP_FIELD_TCP_FLAGS,      /**< TCP flags */
  SWITCH_ACL_IP_FIELD_TTL,            /**< TTL */
  SWITCH_ACL_IP_FIELD_IP_FLAGS,       /**< IP flags */
  SWITCH_ACL_IP_FIELD_IP_FRAGMENT,    /**< IP FRAG */
  SWITCH_ACL_IP_FIELD_IP_DSCP,        /**< IP Diffserv */
  SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL, /**< Port/LAG label */
  SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL, /**< VLAN RIF label */
  SWITCH_ACL_IP_FIELD_ETH_TYPE,       /**< Ether type */
  SWITCH_ACL_IP_FIELD_RMAC_HIT,       /**< router mac hit */

  SWITCH_ACL_IP_FIELD_MAX
} switch_acl_ip_field_t;

/** Ingress/Egress Acl IPv6 field enum */
typedef enum switch_acl_ipv6_field_ {
  SWITCH_ACL_IPV6_FIELD_IPV6_SRC,             /**< IPv6 Source address */
  SWITCH_ACL_IPV6_FIELD_IPV6_DEST,            /**< IPv6 Destination address */
  SWITCH_ACL_IPV6_FIELD_IP_PROTO,             /**< IP protocol */
  SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port (UDP/TCP) */
  SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE,   /**< L4 Dest port (UDP/TCP) */
  SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT, /**< L4 source port value for UDP/TCP */
  SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT,   /**< L4 dest port value for UDP/TCP */
  SWITCH_ACL_IPV6_FIELD_ICMP_TYPE,      /**< ICMP type */
  SWITCH_ACL_IPV6_FIELD_ICMP_CODE,      /**< ICMP code */
  SWITCH_ACL_IPV6_FIELD_TCP_FLAGS,      /**< TCP flags */
  SWITCH_ACL_IPV6_FIELD_TTL,            /**< TTL */
  SWITCH_ACL_IPV6_FIELD_FLOW_LABEL,     /**< Flow Label */
  SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL, /**< Port/LAG label */
  SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL, /**< VLAN RIF label */
  SWITCH_ACL_IPV6_FIELD_ETH_TYPE,       /**< Ether type */
  SWITCH_ACL_IPV6_FIELD_RMAC_HIT,       /**< router mac hit */

  SWITCH_ACL_IPV6_FIELD_MAX
} switch_acl_ipv6_field_t;

/** Ingress/Egress Acl IP field list */
typedef union switch_acl_ip_value_ {
  unsigned int ipv4_source;           /**< v4 source IP */
  unsigned int ipv4_dest;             /**< v4 destination IP */
  unsigned char ip_proto;             /**< protocol */
  unsigned short l4_source_port;      /**< source port */
  unsigned short l4_dest_port;        /**< destination port */
  unsigned char icmp_type;            /**< icmp type */
  unsigned char icmp_code;            /**< icmp code */
  unsigned char tcp_flags;            /**< tcp flags */
  unsigned char ttl;                  /**< time to live */
  unsigned char dscp;                 /**< DSCP */
  unsigned char ip_flags;             /**< IP flags */
  unsigned char tos;                  /**< TOS */
  unsigned char ip_frag;              /**< IP FRAG */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  uint16_t vlan_rif_label;            /**< vlan label */
  unsigned short eth_type;            /**< ethernet type */
  unsigned char rmac_hit;             /**< router mac hit */

} switch_acl_ip_value;

/** Ingress Acl IPv6 field list */
typedef union switch_acl_ipv6_value_ {
  switch_ip6_t ipv6_source;           /**< v6 souce IP */
  switch_ip6_t ipv6_dest;             /**< v6 destination IP */
  unsigned char ip_proto;             /**< protocol */
  unsigned short l4_source_port;      /**< source port */
  unsigned short l4_dest_port;        /**< destination port */
  unsigned char icmp_type;            /**< icmp type */
  unsigned char icmp_code;            /**< icmp code */
  unsigned char tcp_flags;            /**< tcp flags */
  unsigned char ttl;                  /**< time to live */
  uint32_t flow_label;                /**< flow label */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  uint16_t vlan_rif_label;            /**< vlan label */
  unsigned short eth_type;            /**< ethernet type */
  unsigned char rmac_hit;             /**< router mac hit */

} switch_acl_ipv6_value;

/** Ingress/Egress Acl IP mask */
typedef union switch_acl_ip_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint32_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< ip mask union */
} switch_acl_ip_mask;

/** Ingress/Egress Acl IPV6 mask */
typedef union switch_acl_ipv6_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint16_t mask16;
    switch_uint128_t mask;   /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< ipv6 mask union */
} switch_acl_ipv6_mask;

/** Ingress/Egress Acl IP key value pair */
typedef struct switch_acl_ip_key_value_pair_ {
  switch_acl_ip_field_t field; /**< acl ip field type */
  switch_acl_ip_value value;   /**< acl ip field value */
  switch_acl_ip_mask mask;     /**< acl ip field mask */
} switch_acl_ip_key_value_pair_t;

/** Ingress/Egress Acl IPv6 key value pair */
typedef struct {
  switch_acl_ipv6_field_t field; /**< acl ip field type */
  switch_acl_ipv6_value value;   /**< acl ip field value */
  switch_acl_ipv6_mask mask;     /**< acl ip field mask */
} switch_acl_ipv6_key_value_pair_t;

/** Acl action */
typedef enum switch_acl_action_ {
  SWITCH_ACL_ACTION_NOP,             /**< Do nothing action */
  SWITCH_ACL_ACTION_DROP,            /**< Drop the packet */
  SWITCH_ACL_ACTION_PERMIT,          /**< Permit */
  SWITCH_ACL_ACTION_LOG,             /**< Log packet by sending to CPU */
  SWITCH_ACL_ACTION_REDIRECT,        /**< Redirect packet to new destination */
  SWITCH_ACL_ACTION_REDIRECT_TO_CPU, /**< Redirect packet to CPU */
  SWITCH_ACL_ACTION_COPY_TO_CPU,     /**< Send Copy of packet to CPU */
  SWITCH_ACL_ACTION_MIRROR_AND_DROP, /**< Mirror on drop to defined target */
  SWITCH_ACL_ACTION_SET_MIRROR,      /**< Set mirror session */
  SWITCH_ACL_ACTION_FLOOD_TO_VLAN,   /**< Flood to all members of BD */
  SWITCH_ACL_ACTION_SET_QOS_FIELDS,  /**< Set QoS fields */
  SWITCH_ACL_ACTION_TC_AND_COLOR,    /**< Set Traffic class and color */

  SWITCH_ACL_ACTION_MAX
} switch_acl_action_t;

/** Ingress Acl Mac field enum */
typedef enum switch_acl_mac_field_ {
  SWITCH_ACL_MAC_FIELD_ETH_TYPE,       /**< Ether type */
  SWITCH_ACL_MAC_FIELD_SOURCE_MAC,     /**< Source MAC address */
  SWITCH_ACL_MAC_FIELD_DEST_MAC,       /**< Destination MAC address */
  SWITCH_ACL_MAC_FIELD_VLAN_PRI,       /**< VLAN priority */
  SWITCH_ACL_MAC_FIELD_VLAN_CFI,       /**< VLAN CFI */
  SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL, /**< Port/LAG label */
  SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL, /**< VLAN RIF label */

  SWITCH_ACL_MAC_FIELD_MAX
} switch_acl_mac_field_t;

/** Ingress Acl mac field list */
typedef union switch_acl_mac_value_ {
  unsigned short eth_type;      /**< ethernet type */
  switch_mac_addr_t source_mac; /**< source mac */
  switch_mac_addr_t dest_mac;   /**< destionation mac */
  uint8_t vlan_pri;             /**< VLAN priority */
  uint8_t vlan_cfi;             /**< drop eligible */
  uint16_t port_lag_label;      /**< port label */
  uint16_t vlan_rif_label;      /**< vlan label */

} switch_acl_mac_value;

/** Ingress Acl mac mask */
typedef union switch_acl_mac_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint64_t mask;
    uint16_t mask16;         /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< mac mask union */
} switch_acl_mac_mask;

/** Ingress Acl mac key value pair */
typedef struct switch_acl_mac_key_value_pair_ {
  switch_acl_mac_field_t field; /**< acl mac field type */
  switch_acl_mac_value value;   /**< acl mac field value */
  switch_acl_mac_mask mask;     /**< acl mac field mask */
} switch_acl_mac_key_value_pair_t;

/** ACL ip racl field enum */
typedef enum switch_acl_ip_racl_field_ {
  SWITCH_ACL_IP_RACL_FIELD_IPV4_SRC,             /**< IPv4 Source address */
  SWITCH_ACL_IP_RACL_FIELD_IPV4_DEST,            /**< IPv4 Dest address */
  SWITCH_ACL_IP_RACL_FIELD_IP_PROTO,             /**< IP protocol (TCP/UDP) */
  SWITCH_ACL_IP_RACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port */
  SWITCH_ACL_IP_RACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port */
  SWITCH_ACL_IP_RACL_FIELD_TCP_FLAGS,            /**< TCP flags */
  SWITCH_ACL_IP_RACL_FIELD_TTL,                  /**< TTL */
  SWITCH_ACL_IP_RACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */
  SWITCH_ACL_IP_RACL_FIELD_VLAN_RIF_LABEL,       /**< VLAN RIF label */

  SWITCH_ACL_IP_RACL_FIELD_MAX
} switch_acl_ip_racl_field_t;

/** ACL ipv6 racl field enum */
typedef enum switch_acl_ipv6_racl_field_ {
  SWITCH_ACL_IPV6_RACL_FIELD_IPV6_SRC,             /**< IPv6 source address */
  SWITCH_ACL_IPV6_RACL_FIELD_IPV6_DEST,            /**< IPv6 dest address */
  SWITCH_ACL_IPV6_RACL_FIELD_IP_PROTO,             /**< IPv6 protocol */
  SWITCH_ACL_IPV6_RACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port */
  SWITCH_ACL_IPV6_RACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port */
  SWITCH_ACL_IPV6_RACL_FIELD_TCP_FLAGS,            /**< TCP flags */
  SWITCH_ACL_IPV6_RACL_FIELD_TTL,                  /**< TTL */
  SWITCH_ACL_IPV6_RACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */
  SWITCH_ACL_IPV6_RACL_FIELD_VLAN_RIF_LABEL,       /**< VLAN RIF label */

  SWITCH_ACL_IPV6_RACL_FIELD_MAX
} switch_acl_ipv6_racl_field_t;

/** Acl ip racl field list */
typedef union switch_acl_ip_racl_value_ {
  unsigned int ipv4_source;           /**< v4 source IP */
  unsigned int ipv4_dest;             /**< v4 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned char tcp_flags;            /**< tcp flags */
  unsigned char ttl;                  /**< time to live */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  uint16_t vlan_rif_label;            /**< vlan label */
} switch_acl_ip_racl_value;

/** Acl ipv6 racl field list */
typedef union switch_acl_ipv6_racl_value_ {
  switch_ip6_t ipv6_source;           /**< v6 source IP */
  switch_ip6_t ipv6_dest;             /**< v6 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned char ttl;                  /**< time to live */
  unsigned char tcp_flags;            /**< tcp flags */
  unsigned short l4_source_port;      /**< source port */
  unsigned short l4_dest_port;        /**< destination port */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  uint16_t vlan_rif_label;            /**< vlan label */
} switch_acl_ipv6_racl_value;

/** Acl ip racl mask */
typedef union switch_acl_ip_racl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint32_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< ip racl mask union */
} switch_acl_ip_racl_mask;

/** Acl ipv6 racl mask */
typedef union switch_acl_ipv6_racl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint16_t mask16;
    switch_uint128_t mask;   /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< ipv6 racl mask union */
} switch_acl_ipv6_racl_mask;

/** Acl ip racl key value pair */
typedef struct switch_acl_ip_racl_key_value_pair_ {
  switch_acl_ip_racl_field_t field; /**< acl ip racl field type */
  switch_acl_ip_racl_value value;   /**< acl ip racl field value */
  switch_acl_ip_racl_mask mask;     /**< acl ip racl field mask */
} switch_acl_ip_racl_key_value_pair_t;

/** Acl ipv6 racl key value pair */
typedef struct switch_acl_ipv6_racl_key_value_pair_ {
  switch_acl_ipv6_racl_field_t field; /**< acl ip racl field type */
  switch_acl_ipv6_racl_value value;   /**< acl ip racl field value */
  switch_acl_ipv6_racl_mask mask;     /**< acl ip racl field mask */
} switch_acl_ipv6_racl_key_value_pair_t;

/** Acl ip mirror_acl field enum */
typedef enum switch_acl_ip_mirror_acl_field_ {
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_SRC,  /**< IPv4 source address */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_DEST, /**< IPv4 dest address */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_PROTO,  /**< IP protocol */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_DSCP,   /**< IP Diffserv */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_TCP_FLAGS, /**< TCP flags */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_OPCODE,        /**< BTH Header Opcode */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_ACK_REQ_RSVD,  /**< BTH Header Ack
                                                          Request bit + 7-bits of
                                                          Reserved
                                                          Field */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_DST_QP, /**< BTH Header Destination
                                                   Queue Pair Field */

  SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT, /** <L4 source port> */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT,   /** <L4 destination port> */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_ETH_TYPE,       /**< Ether type */
  SWITCH_ACL_IP_MIRROR_ACL_FIELD_MAX
} switch_acl_ip_mirror_acl_field_t;

/** Acl ipv6 mirror_acl field enum */
typedef enum switch_acl_ipv6_mirror_acl_field_ {
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_SRC,  /**< IPv6 source address */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_DEST, /**< IPv6 dest address */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IP_PROTO,  /**< IP protocol */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IP_DSCP,   /**< IP Diffserv */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_TCP_FLAGS, /**< TCP flags */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port
                                                            range handle */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port range
                                                            handle */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT,       /**< L4 source port */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT,         /** <L4 dest port */
  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_ETH_TYPE,             /**< Ether type */

  SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_MAX
} switch_acl_ipv6_mirror_acl_field_t;

/** Acl ip mirror_acl field list */
typedef union switch_acl_ip_mirror_acl_value_ {
  unsigned int ipv4_source;           /**< v4 source IP */
  unsigned int ipv4_dest;             /**< v4 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned short ip_dscp;             /**< diffserv */
  unsigned char tcp_flags;            /**< tcp flags */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  uint8_t rocev2_opcode;              /**< RoCEv2 BTH Header Opcode */
  uint8_t rocev2_ack_req_rsvd; /**< RoCEv2 BTH Header Ack Req bit + 7-bits of
                                  reserved field */
  unsigned int rocev2_dst_qp;  /**< RoCEv2 BTH Header destination queue-pair
                                  field (24-bits)*/
  unsigned short eth_type;     /**< ethernet type */
} switch_acl_ip_mirror_acl_value;

/** Acl ipv6 mirror_acl field list */
typedef union switch_acl_ipv6_mirror_acl_value_ {
  switch_ip6_t ipv6_source;           /**< v6 souce IP */
  switch_ip6_t ipv6_dest;             /**< v6 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned short ip_dscp;             /**< diffserv */
  unsigned char tcp_flags;            /**< tcp flags */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
  unsigned short eth_type;            /**< ethernet type */
} switch_acl_ipv6_mirror_acl_value;

/** Acl ip mirror_acl mask */
typedef union switch_acl_ip_mirror_acl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint32_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< mirror_acl mask union */
} switch_acl_ip_mirror_acl_mask;

/** Acl ip mirror_acl mask */
typedef union switch_acl_ipv6_mirror_acl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint16_t mask16;
    switch_uint128_t mask;   /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< mirror_acl mask union */
} switch_acl_ipv6_mirror_acl_mask;

/** Acl ip mirror_acl key value pair */
typedef struct switch_acl_ip_mirror_acl_key_value_pair_ {
  switch_acl_ip_mirror_acl_field_t field; /**< ipv4 acl mirror_acl field type */
  switch_acl_ip_mirror_acl_value value; /**< ipv4 acl mirror_acl field value */
  switch_acl_ip_mirror_acl_mask mask;   /**< ipv4 acl mirror_acl field mask */
} switch_acl_ip_mirror_acl_key_value_pair_t;

/** Acl ipv6 mirror_acl key value pair */
typedef struct switch_acl_ipv6_mirror_acl_key_value_pair_ {
  switch_acl_ipv6_mirror_acl_field_t
      field; /**< ipv6 acl mirror_acl field type */
  switch_acl_ipv6_mirror_acl_value
      value;                            /**< ipv6 acl mirror_acl field value */
  switch_acl_ipv6_mirror_acl_mask mask; /**< ipv6 acl mirror_acl field mask */
} switch_acl_ipv6_mirror_acl_key_value_pair_t;

/** QoS Acl Mac field enum */
typedef enum switch_acl_mac_qos_acl_field_ {
  SWITCH_ACL_MAC_QOS_ACL_FIELD_ETH_TYPE,       /**< Ether type */
  SWITCH_ACL_MAC_QOS_ACL_FIELD_SOURCE_MAC,     /**< Source MAC address */
  SWITCH_ACL_MAC_QOS_ACL_FIELD_DEST_MAC,       /**< Destination MAC address */
  SWITCH_ACL_MAC_QOS_ACL_FIELD_VLAN_PRI,       /**< VLAN priority */
  SWITCH_ACL_MAC_QOS_ACL_FIELD_VLAN_CFI,       /**< VLAN CFI */
  SWITCH_ACL_MAC_QOS_ACL_FIELD_PORT_LAG_LABEL, /**< Port/LAG label */

  SWITCH_ACL_MAC_QOS_ACL_FIELD_MAX
} switch_acl_mac_qos_acl_field_t;

/** QoS Acl mac field list */
typedef union switch_acl_mac_qos_acl_value_ {
  unsigned short eth_type;      /**< ethernet type */
  switch_mac_addr_t source_mac; /**< source mac */
  switch_mac_addr_t dest_mac;   /**< destionation mac */
  uint8_t vlan_pri;             /**< VLAN priority */
  uint8_t vlan_cfi;             /**< drop eligible */
  uint16_t port_lag_label;      /**< port label */
} switch_acl_mac_qos_acl_value;

/** QoS Acl mac mask */
typedef union switch_acl_mac_qos_acl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint64_t mask;
    uint16_t mask16;         /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< mac mask union */
} switch_acl_mac_qos_acl_mask;

/** QoS Acl mac key value pair */
typedef struct switch_acl_mac_qos_acl_key_value_pair_ {
  switch_acl_mac_qos_acl_field_t field; /**< acl mac field type */
  switch_acl_mac_qos_acl_value value;   /**< acl mac field value */
  switch_acl_mac_qos_acl_mask mask;     /**< acl mac field mask */
} switch_acl_mac_qos_acl_key_value_pair_t;

/** QoS Acl ip qos_acl field enum */
typedef enum switch_acl_ip_qos_acl_field_ {
  SWITCH_ACL_IP_QOS_ACL_FIELD_IPV4_SRC,             /**< IPv4 source address */
  SWITCH_ACL_IP_QOS_ACL_FIELD_IPV4_DEST,            /**< IPv4 dest address */
  SWITCH_ACL_IP_QOS_ACL_FIELD_IP_PROTO,             /**< IP protocol */
  SWITCH_ACL_IP_QOS_ACL_FIELD_IP_DSCP,              /**< IP Diffserv */
  SWITCH_ACL_IP_QOS_ACL_FIELD_TCP_FLAGS,            /**< TCP flags */
  SWITCH_ACL_IP_QOS_ACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port */
  SWITCH_ACL_IP_QOS_ACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port */
  SWITCH_ACL_IP_QOS_ACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */

  SWITCH_ACL_IP_QOS_ACL_FIELD_MAX
} switch_acl_ip_qos_acl_field_t;

/** QoS Acl ipv6 qos_acl field enum */
typedef enum switch_acl_ipv6_qos_acl_field_ {
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_SRC,  /**< IPv6 source address */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_DEST, /**< IPv6 dest address */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_IP_PROTO,  /**< IP protocol */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_IP_DSCP,   /**< IP Diffserv */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_TCP_FLAGS, /**< TCP flags */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_L4_SOURCE_PORT_RANGE, /**< L4 source port */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_L4_DEST_PORT_RANGE,   /**< L4 dest port */
  SWITCH_ACL_IPV6_QOS_ACL_FIELD_PORT_LAG_LABEL,       /**< Port/LAG label */

  SWITCH_ACL_IPV6_QOS_ACL_FIELD_MAX
} switch_acl_ipv6_qos_acl_field_t;

/** Acl ip qos_acl field list */
typedef union switch_acl_ip_qos_acl_value_ {
  unsigned int ipv4_source;           /**< v4 source IP */
  unsigned int ipv4_dest;             /**< v4 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned short ip_dscp;             /**< diffserv */
  unsigned char tcp_flags;            /**< tcp flags */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
} switch_acl_ip_qos_acl_value;

/** Acl ipv6 qos_acl field list */
typedef union switch_acl_ipv6_qos_acl_value_ {
  switch_ip6_t ipv6_source;           /**< v6 souce IP */
  switch_ip6_t ipv6_dest;             /**< v6 destination IP */
  unsigned short ip_proto;            /**< protocol */
  unsigned short ip_dscp;             /**< diffserv */
  unsigned char tcp_flags;            /**< tcp flags */
  switch_handle_t sport_range_handle; /**< sport range handle */
  switch_handle_t dport_range_handle; /**< dport range handle */
  uint16_t port_lag_label;            /**< port label */
} switch_acl_ipv6_qos_acl_value;

/** Acl ip qos_acl mask */
typedef union switch_acl_ip_qos_acl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint32_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< qos_acl mask union */
} switch_acl_ip_qos_acl_mask;

/** Acl ip qos_acl mask */
typedef union switch_acl_ipv6_qos_acl_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint16_t mask16;
    switch_uint128_t mask;   /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< qos_acl mask union */
} switch_acl_ipv6_qos_acl_mask;

/** Acl ip qos_acl key value pair */
typedef struct switch_acl_ip_qos_acl_key_value_pair_ {
  switch_acl_ip_qos_acl_field_t field; /**< ipv4 acl qos_acl field type */
  switch_acl_ip_qos_acl_value value;   /**< ipv4 acl qos_acl field value */
  switch_acl_ip_qos_acl_mask mask;     /**< ipv4 acl qos_acl field mask */
} switch_acl_ip_qos_acl_key_value_pair_t;

/** Acl ipv6 qos_acl key value pair */
typedef struct switch_acl_ipv6_qos_acl_key_value_pair_ {
  switch_acl_ipv6_qos_acl_field_t field; /**< ipv6 acl qos_acl field type */
  switch_acl_ipv6_qos_acl_value value;   /**< ipv6 acl qos_acl field value */
  switch_acl_ipv6_qos_acl_mask mask;     /**< ipv6 acl qos_acl field mask */
} switch_acl_ipv6_qos_acl_key_value_pair_t;

/** Acl system field enum */
typedef enum switch_acl_system_field_ {
  SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE,               /**< Ether type */
  SWITCH_ACL_SYSTEM_FIELD_SOURCE_MAC,             /**< Source MAC address */
  SWITCH_ACL_SYSTEM_FIELD_DEST_MAC,               /**< Dest MAC address */
  SWITCH_ACL_SYSTEM_FIELD_PORT_VLAN_MAPPING_MISS, /**< Port/vlan miss*/
  SWITCH_ACL_SYSTEM_FIELD_IPSG_CHECK,             /**< IP sourceguard check */
  SWITCH_ACL_SYSTEM_FIELD_ACL_DENY,               /**< ACL deny */
  SWITCH_ACL_SYSTEM_FIELD_RACL_DENY,              /**< Route ACL deny check */
  SWITCH_ACL_SYSTEM_FIELD_URPF_CHECK,             /**< URPF check */
  SWITCH_ACL_SYSTEM_FIELD_DROP,                   /**< Dropped packet */
  SWITCH_ACL_SYSTEM_FIELD_METER_DROP,             /**< Meter drop */
  SWITCH_ACL_SYSTEM_FIELD_L3_COPY,                /**< L3 copy */
  SWITCH_ACL_SYSTEM_FIELD_ROUTED,                 /**< Routed packet check */
  SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL,          /**< Link local address (IPv6) */
  SWITCH_ACL_SYSTEM_FIELD_NEXTHOP_GLEAN,       /**< glean adjacency */
  SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_HIT,     /**< multicast Route Hit */
  SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_S_G_HIT, /**< multicast {S, G} Route Hit
                                                  */
  SWITCH_ACL_SYSTEM_FIELD_MCAST_RPF_FAIL,    /**< multicast RPF check failed */
  SWITCH_ACL_SYSTEM_FIELD_MCAST_COPY_TO_CPU, /**< copy to cpu flag set on mroute
                                                */
  SWITCH_ACL_SYSTEM_FIELD_BD_CHECK,          /**< Bridge domain check */
  SWITCH_ACL_SYSTEM_FIELD_TTL,               /**< TTL */
  SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX,    /**< Egress ifindex */
  SWITCH_ACL_SYSTEM_FIELD_STP_STATE,         /**< STP state */
  SWITCH_ACL_SYSTEM_FIELD_CONTROL_FRAME,     /**< Control frame */
  SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED,      /**< IPv4 enabled on BD */
  SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED,      /**< IPv6 enabled on BD */
  SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT,          /**< Rmac hit */
  SWITCH_ACL_SYSTEM_FIELD_IF_CHECK,          /**< Same intf check */
  SWITCH_ACL_SYSTEM_FIELD_TUNNEL_IF_CHECK,   /**< Tunnel intf check */
  SWITCH_ACL_SYSTEM_FIELD_REASON_CODE,       /**< hostif reason code */
  SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP,    /**< Mirror on drop enable flag */
  SWITCH_ACL_SYSTEM_FIELD_DROP_CTL,          /**< Ingress drop ctl */

  SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR, /**< Storm control policer color
                                                  */
  SWITCH_ACL_SYSTEM_FIELD_L2_DST_MISS,         /**< L2 DST miss */
  SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE, /** Broadcast/Unicast/Multicast */
  SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE, /**< Encoded opcode field from ARP/RARP */
  SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL,  /**< Port/LAG label */
  SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL,  /**< VLAN RIF label */
  SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP,    /**< FIB Hit on router IP */
  SWITCH_ACL_SYSTEM_FIELD_INGRESS_IFINDEX, /**< Ingress ifindex */

  SWITCH_ACL_SYSTEM_FIELD_MAX
} switch_acl_system_field_t;

/** Maximum Acl fields */
#define SWITCH_ACL_FIELD_MAX SWITCH_ACL_SYSTEM_FIELD_MAX

/** Acl system field list */
typedef union switch_acl_system_value_ {
  unsigned short eth_type;        /**< ethernet type */
  switch_mac_addr_t source_mac;   /**< source mac */
  switch_mac_addr_t dest_mac;     /**< destination mac */
  unsigned ipsg_check : 1,        /**< ip sourceguard check */
      acl_deny : 1,               /**< acl deny */
      acl_copy : 1,               /**< acl copy */
      racl_deny : 1,              /**< racl deny */
      urpf_check_fail : 1,        /**< urpf check fail */
      port_vlan_mapping_miss : 1, /**< port vlan mapping miss */
      meter_drop : 1,             /**< meter drop */
      drop_flag : 1,              /**< drop flag */
      l3_copy : 1,                /**< l3 copy */
      routed : 1,                 /**< routed */
      src_is_link_local : 1,      /**< link local source ip */
      nexthop_glean : 1,          /**< glean adjacency */
      mcast_route_hit,            /**< multicast Route Hit */
      mcast_route_s_g_hit,        /**< multicast {S, G} Route Hit */
      mcast_rpf_fail,             /**< multicast RPF check failed */
      mcast_copy_to_cpu,          /**< copy to cpu flag set on mroute */
      tunnel_if_check : 1,        /**< tunnel if check */
      control_frame : 1,          /**< control frame */
      ipv4_enabled : 1,           /**< IPv4 enabled on BD */
      ipv6_enabled : 1,           /**< IPv6 enabled on BD */
      rmac_hit : 1,               /**< rmac hit */
      mirror_on_drop : 1,         /**< mirror on drop enable */
      drop_ctl : 3,               /**< drop control */
      l2_dst_miss : 1,            /**< l2 dst miss */
      packet_type : 3,            /**< l2 packet type */
      arp_opcode : 2,             /**< encoded arp opcode */
      fib_hit_myip : 1;           /**< myip flag set on route entry */

  unsigned short if_check : 16;       /**< same if check */
  unsigned short bd_check : 16;       /**< same bd check */
  unsigned char ttl;                  /**< time to live */
  unsigned short out_ifindex;         /**< egress ifindex */
  unsigned char stp_state;            /**< spanning tree port state */
  uint16_t reason_code;               /**< hostif reason code */
  switch_color_t storm_control_color; /**< Storm control policer color */
  uint16_t port_lag_label;            /**< port label */
  uint16_t vlan_rif_label;            /**< vlan label */
  unsigned short ingress_ifindex;     /**< ingress ifindex */
} switch_acl_system_value;

/** Acl system mask */
typedef union switch_acl_system_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint64_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< system acl mask union */
} switch_acl_system_mask;

/** Acl system key value pair */
typedef struct switch_acl_system_key_value_pair_ {
  switch_acl_system_field_t field; /**< acl system field type */
  switch_acl_system_value value;   /**< acl system field value */
  switch_acl_system_mask mask;     /**< acl system field mask */
} switch_acl_system_key_value_pair_t;

/** Acl action parameters */
typedef union switch_acl_action_params_ {
  struct {
    switch_handle_t handle; /**< port/nexthop handle */
  } redirect;               /**< port redirect struct */
  struct {
    uint16_t reason_code; /**< cpu reason code */
  } cpu_redirect;         /**< cpu redirect struct */
  struct {
    uint8_t reason_code; /**< drop reason code */
  } drop;                /**< drop struct */
} switch_acl_action_params_t;

/** Acl optional action parameters */
typedef struct switch_acl_opt_action_params_ {
  bool copy_to_cpu;               /**< generate a cpu copy */
  switch_handle_t mirror_handle;  /**< mirror session handle */
  unsigned int switch_id;         /**< mirror switch id */
  switch_handle_t meter_handle;   /**< meter handle */
  switch_handle_t counter_handle; /**< counter handle */
  switch_nat_mode_t nat_mode;     /**< nat mode */
  uint16_t tc;                    /**< traffic class */
  switch_color_t color;           /**< packet color */
  uint8_t ingress_cos;            /**< ingress cos */
  switch_qid_t queue_id;          /**< queue id */
} switch_acl_opt_action_params_t;

/** Egress System ACL field enum */
typedef enum switch_acl_egress_system_field_ {
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_ACL_DENY,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_REASON_CODE,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_PACKET_COLOR,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_DROP_CTL, /**< Egress drop ctl */
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_SRC_PORT,
  SWITCH_ACL_EGRESS_SYSTEM_FIELD_MAX
} switch_acl_egress_system_field_t;

/** Egress System ACL match value */
typedef union switch_acl_egress_system_value_ {
  switch_handle_t egr_port;    /**< egress port */
  bool deflection_flag;        /**< deflection flag */
  unsigned short l3_mtu_check; /**< L3 MTU check */
  bool acl_deny;               /**< acl deny */
  uint16_t reason_code;        /**< drop or hostif reason code */
  unsigned mirror_on_drop : 2; /**< mirror on drop enable */
  bool queue_dod_enable;       /**< queue dod enable */
  switch_handle_t ing_port;    /**< ingress port */
  switch_color_t packet_color; /**< packet color */
  unsigned drop_ctl : 3;       /**< drop control */
} switch_acl_egress_system_value_t;

/** Egress System ACL match mask */
typedef union switch_acl_egress_system_mask_ {
  unsigned type : 1; /**< acl mask type */
  union {
    uint64_t mask;           /**< mask value */
    unsigned int start, end; /**< mask range */
  } u;                       /**< mask union */
} switch_acl_egress_system_mask_t;

/** Egress System acl key value pair */
typedef struct switch_acl_egress_system_key_value_pair_ {
  switch_acl_egress_system_field_t field; /**< acl ip field type */
  switch_acl_egress_system_value_t value; /**< acl ip field value */
  switch_acl_egress_system_mask_t mask;   /**< acl ip field mask */
} switch_acl_egress_system_key_value_pair_t;

/** Egress System acl port action */
typedef enum switch_acl_egress_system_action_ {
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_NOP,             /**< Do nothing action */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_SET_MIRROR,      /**< Set mirror session */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU, /**< redirect to cpu */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP, /**< mirror on drop */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP,            /**< drop packets */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_PERMIT,          /**< permit packets */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_INSERT_CPU_TIMESTAMP,  /**< insert timestamp
                                                  header after
                                                  cpu header */
  SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP_QALERT /**< mirror on drop
                                                          * with qalert
                                               */
} switch_acl_egress_system_action_t;

typedef enum switch_acl_ecn_field_s {
  SWITCH_ACL_ECN_FIELD_ECN,
  SWITCH_ACL_ECN_FIELD_DSCP,
  SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL, /**< Port/LAG label */
  SWITCH_ACL_ECN_FIELD_MAX
} switch_acl_ecn_field_t;

typedef union switch_acl_ecn_value_s {
  /** dscp value */
  uint8_t dscp;

  /** ecn value */
  uint8_t ecn;

  /**< port label */
  uint16_t port_lag_label;

} switch_acl_ecn_value_t;

/** Acl ecn acl mask */
typedef union switch_acl_ecn_mask_s {
  union {
    /** 8 bit mask */
    switch_uint8_t mask;
  } u;
} switch_acl_ecn_mask_t;

/** Acl ecn key value pair */
typedef struct switch_acl_ecn_key_value_pair_s {
  /** ecn field */
  switch_acl_ecn_field_t field;

  /** ecn value */
  switch_acl_ecn_value_t value;

  /** ecn field mask */
  switch_acl_ecn_mask_t mask;

} switch_acl_ecn_key_value_pair_t;

typedef switch_acl_action_t switch_acl_ip_action_t;   /**< acl action */
typedef switch_acl_action_t switch_acl_ipv6_action_t; /**< IPv6 acl action */
typedef switch_acl_action_t switch_acl_mac_action_t;  /**< mac acl action */
typedef switch_acl_action_t switch_acl_ecn_action_t;  /**< ecn acl action */
typedef switch_acl_action_t
    switch_acl_system_action_t; /**< system acl action */

typedef enum switch_range_type_ {
  SWITCH_RANGE_TYPE_NONE = 0x0,
  SWITCH_RANGE_TYPE_SRC_PORT = 0x1,
  SWITCH_RANGE_TYPE_DST_PORT = 0x2,
  SWITCH_RANGE_TYPE_VLAN = 0x3,
  SWITCH_RANGE_TYPE_PACKET_LENGTH = 0x4
} switch_range_type_t;

typedef struct switch_range_ {
  uint32_t start_value;
  uint32_t end_value;
} switch_range_t;
/** ACL group apis
 *
 */
switch_handle_t switch_api_acl_list_group_create(
    switch_device_t device,
    switch_direction_t direction,
    switch_handle_type_t bp_type,
    switch_handle_t *acl_group_handle);

switch_status_t switch_api_acl_list_group_delete(
    switch_device_t device, switch_handle_t acl_group_handle);

switch_handle_t switch_api_acl_group_member_create(
    switch_device_t device,
    switch_handle_t acl_group_handle,
    switch_handle_t acl_handle,
    switch_handle_t *acl_group_member_handle);

switch_status_t switch_api_acl_group_member_delete(
    switch_device_t device, switch_handle_t acl_group_member_handle);

/**
 ACL Key list create
 @param device device
 @param type - acl type
*/
switch_status_t switch_api_acl_list_create(switch_device_t device,
                                           switch_direction_t direction,
                                           switch_acl_type_t type,
                                           switch_handle_type_t bp_type,
                                           switch_handle_t *acl_handle);

/**
 ACL Key list update
 @param device device
 @param acl_handle handle of created ACL
 @param type - acl type
*/
switch_status_t switch_api_acl_list_update(switch_device_t device,
                                           switch_handle_t acl_handle,
                                           switch_acl_type_t type);

/**
 Delete the ACL key list
 @param device device
 @param acl_handle handle of created ACL
*/
switch_status_t switch_api_acl_list_delete(switch_device_t device,
                                           switch_handle_t acl_handle);

/**
 Get the ACL type for a given ACL
 @param - device - device
 @param - acl_handle - handle of created ACL
 @param - acl_type - Type from switch_acl_type_t
*/
switch_status_t switch_api_acl_type_get(switch_device_t device,
                                        switch_handle_t acl_handle,
                                        switch_acl_type_t *acl_type);

/**
 Set the ACL type for a given ACL
 @param - device - device
 @param - acl_handle - handle of created ACL
 @param - acl_type - Type from switch_acl_type_t
*/
switch_status_t switch_api_acl_type_set(switch_device_t device,
                                        switch_handle_t acl_handle,
                                        switch_acl_type_t acl_type);
/**
 Create ACL Rules
 @param device device
 @param acl_handle - Acl handle
 @param priority - priority of Acl
 @param key_value_count - key value pair count
 @param acl_kvp - pointer to multiple key value pair
 @param action - Acl action (permit/drop/redirect to cpu)
 @param action_params - action parameters
 @param opt_action_params - optional action parameters
 @param ace_handle - returned handle for the rule
*/
switch_status_t switch_api_acl_rule_create(
    switch_device_t device,
    switch_handle_t acl_handle,
    unsigned int priority,
    unsigned int key_value_count,
    void *acl_kvp,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_handle_t *ace_handle);

/**
 Delete ACL Rules
 @param device device
 @param acl_handle - Acl handle
 @param ace_handle - handle obtained from create_rule
*/
switch_status_t switch_api_acl_rule_delete(switch_device_t device,
                                           switch_handle_t acl_handle,
                                           switch_handle_t ace_handle);

/**
 Renumber ACL Rules
 @param device device
 @param acl_handle - Acl handle
 @param increment_priority - priority to reorder the acl rule
*/
switch_status_t switch_api_acl_renumber(switch_device_t device,
                                        switch_handle_t acl_handle,
                                        int increment_priority);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_ingress_acl_reference(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_handle_t interface_handle);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_ingress_acl_dereference(switch_device_t device,
                                                   switch_handle_t acl_handle,
                                                   switch_handle_t handle);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_egress_acl_reference(
    switch_device_t device,
    switch_handle_t acl_handle,
    switch_handle_t interface_handle);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_egress_acl_dereference(switch_device_t device,
                                                  switch_handle_t acl_handle,
                                                  switch_handle_t handle);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_acl_reference(switch_device_t device,
                                         switch_handle_t acl_handle,
                                         switch_handle_t interface_handle);

/**
 Apply ACLs on interfaces, VLANs, etc.
 @param device device
 @param acl_handle handle created with list_create
 @param interface_handle - Interface handle
*/
switch_status_t switch_api_acl_dereference(switch_device_t device,
                                           switch_handle_t acl_handle,
                                           switch_handle_t handle);
/**
 Get drop statistics
 @param device device
 @param num_counters number of counters
 @param counters pointer to counter array
*/
switch_status_t switch_api_drop_stats_get(switch_device_t device,
                                          int *num_counters,
                                          switch_uint64_t **counters);

/**
 create acl counter handle
 @param device device
*/
switch_status_t switch_api_acl_counter_create(switch_device_t device,
                                              switch_handle_t *counter_handle);

/**
 delete acl counter handle
 @param device device
 @param counter_handle acl counter handle
*/
switch_status_t switch_api_acl_counter_delete(switch_device_t device,
                                              switch_handle_t counter_handle);

/**
 get acl statistics
 @param device device
 @param counter_handle acl counter handle
 @param counter counter value
*/
switch_status_t switch_api_acl_counter_get(switch_device_t device,
                                           switch_handle_t counter_handle,
                                           switch_counter_t *counter);

switch_status_t switch_api_acl_counter_clear(switch_device_t device,
                                             switch_handle_t counter_handle);
/**
 create racl counter handle
 @param device device
*/
switch_status_t switch_api_racl_counter_create(switch_device_t device,
                                               switch_handle_t *counter_handle);

/**
 delete racl counter handle
 @param device device
 @param counter_handle acl counter handle
*/
switch_status_t switch_api_racl_counter_delete(switch_device_t device,
                                               switch_handle_t counter_handle);

/**
 get racl statistics
 @param device device
 @param counter_handle acl counter handle
 @param counter counter value
*/
switch_status_t switch_api_racl_counter_get(switch_device_t device,
                                            switch_handle_t counter_handle,
                                            switch_counter_t *counter);

switch_status_t switch_api_racl_counter_clear(switch_device_t device,
                                              switch_handle_t counter_handle);
/**
 create egress acl counter handle
 @param device device
*/
switch_status_t switch_api_egress_acl_counter_create(
    switch_device_t device, switch_handle_t *counter_handle);

/**
 delete egres acl counter handle
 @param device device
 @param counter_handle acl counter handle
*/
switch_status_t switch_api_egress_acl_counter_delete(
    switch_device_t device, switch_handle_t counter_handle);

/**
 get egress acl statistics
 @param device device
 @param counter_handle acl counter handle
 @param counter counter value
*/
switch_status_t switch_api_egress_acl_counter_get(
    switch_device_t device,
    switch_handle_t counter_handle,
    switch_counter_t *counter);

switch_status_t switch_api_egress_acl_counter_clear(
    switch_device_t device, switch_handle_t counter_handle);
/**
 get acl type
 @param device device
 @param acl_handle acl handle
*/
switch_acl_type_t switch_acl_type_get(switch_device_t device,
                                      switch_handle_t acl_handle);

switch_status_t switch_api_acl_range_create(switch_device_t device,
                                            switch_direction_t direction,
                                            switch_range_type_t range_type,
                                            switch_range_t *range,
                                            switch_handle_t *range_handle);

switch_status_t switch_api_acl_range_update(switch_device_t device,
                                            switch_handle_t range_handle,
                                            switch_range_t *range);

switch_status_t switch_api_acl_range_type_get(switch_device_t device,
                                              switch_handle_t range_handle,
                                              switch_range_type_t *range_type);

switch_status_t switch_api_acl_range_get(switch_device_t device,
                                         switch_handle_t range_handle,
                                         switch_range_t *range);

switch_status_t switch_api_acl_range_delete(switch_device_t device,
                                            switch_handle_t range_handle);

switch_status_t switch_api_acl_handle_dump(const switch_device_t device,
                                           const switch_handle_t acl_handle,
                                           const void *cli_ctx);

switch_status_t switch_api_ace_handle_dump(const switch_device_t device,
                                           const switch_handle_t ace_handle,
                                           const void *cli_ctx);

switch_status_t switch_api_acl_group_handle_dump(
    const switch_device_t device,
    const switch_handle_t acl_group_handle,
    const void *cli_ctx);

switch_status_t switch_api_acl_group_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t acl_group_member_handle,
    const void *cli_ctx);

switch_status_t switch_api_acl_range_handle_dump(
    const switch_device_t device,
    const switch_handle_t acl_range_handle,
    const void *cli_ctx);

switch_status_t switch_api_acl_entry_action_set(
    switch_device_t device,
    switch_handle_t ace_handle,
    unsigned int priority,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params);

/**
 Get ACL action and action parameters
 @param - device
 @param - ace_handle - ACL entry handle
 @param - action
 @param - action_params
 @param - opt_action_params
*/
switch_status_t switch_api_acl_entry_action_get(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_acl_action_t *action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params);

/**
 Returns the field count in each rule
 @param - device
 @param - ace_handle - ACL entry handle
 @param - count - Field count
*/
switch_status_t switch_api_acl_entry_rules_count_get(
    switch_device_t device,
    switch_handle_t ace_handle,
    switch_uint16_t *rules_count);

switch_status_t switch_api_acl_entry_rules_get(switch_device_t device,
                                               switch_handle_t ace_handle,
                                               void *kvp);

/**
 Return the ACL table handle given the ACL entry handle
 @param - device
 @param - acl_entry_handle - ACL entry handle
 @param - acl_table_handle - ALC table handle
*/
switch_status_t switch_api_acl_entry_acl_table_get(
    switch_device_t device,
    switch_handle_t acl_entry_handle,
    switch_handle_t *acl_table_handle);

/**
Get the ACL direction for a given ACL
@param - device - device
@param - acl_handle - handle of created ACL
@param - direction
*/
switch_status_t switch_api_acl_direction_get(switch_device_t device,
                                             switch_handle_t acl_handle,
                                             switch_direction_t *direction);

#define SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY 200
#define SWITCH_API_ACL_ENTRY_MAXIMUM_PRIORITY (1 << 14)

/** @} */  // end of ACL API

#ifdef __cplusplus
}
#endif

#endif
