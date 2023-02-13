/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef __SWITCH_TABLE_H__
#define __SWITCH_TABLE_H__

#include "switch_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_table_id_ {

  SWITCH_TABLE_NONE = 0,

  /* Ingress Port */
  SWITCH_TABLE_INGRESS_PORT_MAPPING = 1,
  SWITCH_TABLE_EGRESS_PORT_MAPPING = 2,
  SWITCH_TABLE_INGRESS_PORT_PROPERTIES = 3,

  /* Rmac */
  SWITCH_TABLE_OUTER_RMAC = 21,
  SWITCH_TABLE_INNER_RMAC = 22,

  /* L2 */
  SWITCH_TABLE_SMAC = 41,
  SWITCH_TABLE_DMAC = 42,

  /* FIB */
  SWITCH_TABLE_IPV4_HOST = 61,
  SWITCH_TABLE_IPV6_HOST = 62,
  SWITCH_TABLE_IPV4_LPM = 63,
  SWITCH_TABLE_IPV6_LPM = 64,
  SWITCH_TABLE_SMAC_REWRITE = 65,
  SWITCH_TABLE_MTU = 66,
  SWITCH_TABLE_URPF = 67,

  /* Nexthop */
  SWITCH_TABLE_NHOP = 81,
  SWITCH_TABLE_ECMP_GROUP = 82,
  SWITCH_TABLE_ECMP_SELECT = 83,

  /* Rewrite */
  SWITCH_TABLE_REWRITE = 101,

  /* Tunnel */
  SWITCH_TABLE_IPV4_SRC_VTEP = 121,
  SWITCH_TABLE_IPV4_DST_VTEP = 122,
  SWITCH_TABLE_IPV6_SRC_VTEP = 123,
  SWITCH_TABLE_IPV6_DST_VTEP = 124,
  SWITCH_TABLE_TUNNEL = 125,
  SWITCH_TABLE_TUNNEL_REWRITE = 126,
  SWITCH_TABLE_TUNNEL_DECAP = 127,
  SWITCH_TABLE_TUNNEL_SMAC_REWRITE = 128,
  SWITCH_TABLE_TUNNEL_DMAC_REWRITE = 129,
  SWITCH_TABLE_TUNNEL_SIP_REWRITE = 130,
  SWITCH_TABLE_TUNNEL_DIP_REWRITE = 131,
  SWITCH_TABLE_TUNNEL_MPLS = 132,

  /* BD */
  SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING = 141,
  SWITCH_TABLE_BD = 142,
  SWITCH_TABLE_BD_FLOOD = 143,
  SWITCH_TABLE_INGRESS_BD_STATS = 144,
  SWITCH_TABLE_VLAN_DECAP = 145,
  SWITCH_TABLE_VLAN_XLATE = 146,
  SWITCH_TABLE_EGRESS_BD = 147,
  SWITCH_TABLE_EGRESS_BD_STATS = 148,
  SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING = 149,

  /* ACL */
  SWITCH_TABLE_IPV4_ACL = 161,
  SWITCH_TABLE_IPV6_ACL = 162,
  SWITCH_TABLE_IPV4_RACL = 163,
  SWITCH_TABLE_IPV6_RACL = 164,
  SWITCH_TABLE_SYSTEM_ACL = 165,
  SWITCH_TABLE_MAC_ACL = 166,
  SWITCH_TABLE_EGRESS_SYSTEM_ACL = 167,
  SWITCH_TABLE_ACL_STATS = 168,
  SWITCH_TABLE_RACL_STATS = 169,
  SWITCH_TABLE_EGRESS_ACL_STATS = 170,

  /* Multicast */
  SWITCH_TABLE_OUTER_MCAST_STAR_G = 181,
  SWITCH_TABLE_OUTER_MCAST_SG = 182,
  SWITCH_TABLE_IPV4_MCAST_S_G = 185,
  SWITCH_TABLE_IPV4_MCAST_STAR_G = 186,
  SWITCH_TABLE_IPV6_MCAST_S_G = 187,
  SWITCH_TABLE_IPV6_MCAST_STAR_G = 188,
  SWITCH_TABLE_OUTER_MCAST_RPF = 183,
  SWITCH_TABLE_MCAST_RPF = 184,
  SWITCH_TABLE_RID = 189,
  SWITCH_TABLE_REPLICA_TYPE = 190,

  /* STP */
  SWITCH_TABLE_STP = 201,

  /* LAG */
  SWITCH_TABLE_LAG_GROUP = 221,
  SWITCH_TABLE_LAG_SELECT = 222,

  /* Mirror */
  SWITCH_TABLE_MIRROR = 241,

  /* Meter */
  SWITCH_TABLE_METER_INDEX = 261,
  SWITCH_TABLE_METER_ACTION = 262,

  /* Stats */
  SWITCH_TABLE_DROP_STATS = 281,

  /* Nat */
  SWITCH_TABLE_NAT_DST = 301,
  SWITCH_TABLE_NAT_SRC = 302,
  SWITCH_TABLE_NAT_TWICE = 303,
  SWITCH_TABLE_NAT_FLOW = 304,

  /* Qos */
  SWITCH_TABLE_INGRESS_QOS_MAP = 320,
  SWITCH_TABLE_INGRESS_QOS_MAP_DSCP = 321,
  SWITCH_TABLE_INGRESS_QOS_MAP_PCP = 322,
  SWITCH_TABLE_QUEUE = 323,
  SWITCH_TABLE_EGRESS_QOS_MAP = 324,

  /* Wred */
  SWITCH_TABLE_WRED = 341,

  SWITCH_TABLE_MAX = 512

} switch_table_id_t;

typedef struct switch_table_s {
  bool valid;
  switch_size_t table_size;
  switch_size_t num_entries;
  switch_direction_t direction;
  switch_uint8_t table_name[SWITCH_MAX_STRING_SIZE];
} switch_table_t;

switch_status_t switch_api_table_get(switch_device_t device,
                                     switch_table_id_t table_id,
                                     switch_table_t *api_table_info);

switch_status_t switch_api_table_size_get(switch_device_t device,
                                          switch_table_id_t table_id,
                                          switch_size_t *table_size);

switch_status_t switch_api_table_all_get(switch_device_t device,
                                         switch_size_t *num_entries,
                                         switch_table_t *api_table_info);

switch_status_t switch_api_table_sizes_dump(const switch_device_t device,
                                            const void *cli_ctx);

switch_status_t switch_api_table_entry_count_get(switch_device_t device,
                                                 switch_table_id_t table_id,
                                                 switch_uint32_t *num_entries);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_TABLE_H__ */
