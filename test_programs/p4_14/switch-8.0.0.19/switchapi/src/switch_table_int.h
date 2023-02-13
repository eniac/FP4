/*
Copyright 2015-present Barefoot Networks, Inc.
*/

#ifndef _SWITCH_TABLE_INT_H_
#define _SWITCH_TABLE_INT_H_

#define SWITCH_TABLE_ID_VALID(_table_id) \
  _table_id > SWITCH_TABLE_NONE &&_table_id < SWITCH_TABLE_MAX

static inline char *switch_table_id_to_string(switch_table_id_t table_id) {
  switch (table_id) {
    case SWITCH_TABLE_INGRESS_PORT_MAPPING:
      return "ingress port mapping";
    case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
      return "ingress port properties";
    case SWITCH_TABLE_EGRESS_PORT_MAPPING:
      return "egress port mapping";

    /* Rmac */
    case SWITCH_TABLE_OUTER_RMAC:
      return "outer rmac";
    case SWITCH_TABLE_INNER_RMAC:
      return "inner rmac";

    /* L2 */
    case SWITCH_TABLE_SMAC:
      return "smac";
    case SWITCH_TABLE_DMAC:
      return "dmac";

    /* FIB */
    case SWITCH_TABLE_IPV4_HOST:
      return "ipv4 host";
    case SWITCH_TABLE_IPV6_HOST:
      return "ipv6 host";
    case SWITCH_TABLE_IPV4_LPM:
      return "ipv4 lpm";
    case SWITCH_TABLE_IPV6_LPM:
      return "ipv6 lpm";
    case SWITCH_TABLE_SMAC_REWRITE:
      return "smac rewrite";
    case SWITCH_TABLE_MTU:
      return "mtu";
    case SWITCH_TABLE_URPF:
      return "urpf";

    /* Nexthop */
    case SWITCH_TABLE_NHOP:
      return "nexthop";
    case SWITCH_TABLE_ECMP_GROUP:
      return "ecmp group";
    case SWITCH_TABLE_ECMP_SELECT:
      return "ecmp select";

    /* Rewrite */
    case SWITCH_TABLE_REWRITE:
      return "rewrite";

    /* Tunnel */
    case SWITCH_TABLE_IPV4_SRC_VTEP:
      return "ipv4 src vtep";
    case SWITCH_TABLE_IPV4_DST_VTEP:
      return "ipv4 dst vtep";
    case SWITCH_TABLE_IPV6_SRC_VTEP:
      return "ipv6 src vtep";
    case SWITCH_TABLE_IPV6_DST_VTEP:
      return "ipv6 dst vtep";
    case SWITCH_TABLE_TUNNEL:
      return "tunnel";
    case SWITCH_TABLE_TUNNEL_REWRITE:
      return "tunnel rewrite";
    case SWITCH_TABLE_TUNNEL_DECAP:
      return "tunnel decap";
    case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
      return "tunnel smac rewrite";
    case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
      return "tunnel dmac rewrite";
    case SWITCH_TABLE_TUNNEL_DIP_REWRITE:
      return "tunnel dip rewrite";
    case SWITCH_TABLE_TUNNEL_MPLS:
      return "mpls";

    /* BD */
    case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
      return "port vlan bd mapping";
    case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
      return "port vlan ifindex mapping";
    case SWITCH_TABLE_BD:
      return "bd";
    case SWITCH_TABLE_BD_FLOOD:
      return "bd flood";
    case SWITCH_TABLE_INGRESS_BD_STATS:
      return "ingress bd stats";
    case SWITCH_TABLE_EGRESS_BD_STATS:
      return "egress bd stats";
    case SWITCH_TABLE_VLAN_DECAP:
      return "vlan decap";
    case SWITCH_TABLE_VLAN_XLATE:
      return "vlan xlate";
    case SWITCH_TABLE_EGRESS_BD:
      return "egress bd";

    /* ACL */
    case SWITCH_TABLE_IPV4_ACL:
      return "ipv4 acl";
    case SWITCH_TABLE_IPV6_ACL:
      return "ipv6 acl";
    case SWITCH_TABLE_IPV4_RACL:
      return "ipv4 racl";
    case SWITCH_TABLE_IPV6_RACL:
      return "ipv6 racl";
    case SWITCH_TABLE_SYSTEM_ACL:
      return "system acl";
    case SWITCH_TABLE_MAC_ACL:
      return "mac acl";
    case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
      return "egress system acl";
    case SWITCH_TABLE_ACL_STATS:
      return "acl stats";
    case SWITCH_TABLE_RACL_STATS:
      return "racl stats";
    case SWITCH_TABLE_EGRESS_ACL_STATS:
      return "egress_acl stats";

    /* Multicast */
    case SWITCH_TABLE_OUTER_MCAST_STAR_G:
      return "outer mcast star g";
    case SWITCH_TABLE_OUTER_MCAST_SG:
      return "outer mcast sg";
    case SWITCH_TABLE_IPV4_MCAST_S_G:
      return "ipv4 mcast sg";
    case SWITCH_TABLE_IPV4_MCAST_STAR_G:
      return "ipv4 mcast star g";
    case SWITCH_TABLE_IPV6_MCAST_S_G:
      return "ipv6 mcast sg";
    case SWITCH_TABLE_IPV6_MCAST_STAR_G:
      return "ipv6 mcast star g";
    case SWITCH_TABLE_OUTER_MCAST_RPF:
      return "outer mcast rpf";
    case SWITCH_TABLE_MCAST_RPF:
      return "mcast rpf";
    case SWITCH_TABLE_RID:
      return "rid";
    case SWITCH_TABLE_REPLICA_TYPE:
      return "replica type";

    /* STP */
    case SWITCH_TABLE_STP:
      return "stp";

    /* LAG */
    case SWITCH_TABLE_LAG_GROUP:
      return "lag group";
    case SWITCH_TABLE_LAG_SELECT:
      return "lag select";

    /* Mirror */
    case SWITCH_TABLE_MIRROR:
      return "mirror";

    /* Meter */
    case SWITCH_TABLE_METER_INDEX:
      return "meter index";
    case SWITCH_TABLE_METER_ACTION:
      return "meter action";

    /* Stats */
    case SWITCH_TABLE_DROP_STATS:
      return "drop stats";

    /* Nat */
    case SWITCH_TABLE_NAT_DST:
      return "nat dst";
    case SWITCH_TABLE_NAT_SRC:
      return "nat src";
    case SWITCH_TABLE_NAT_TWICE:
      return "nat twice";
    case SWITCH_TABLE_NAT_FLOW:
      return "nat flow";

    /* Qos */
    case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
      return "ingress qos map dscp";
    case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
      return "ingress qos map pcp";
    case SWITCH_TABLE_QUEUE:
      return "queue";
    case SWITCH_TABLE_EGRESS_QOS_MAP:
      return "egress qos map";

    default:
      return "unknown";
  }
}

switch_status_t switch_table_init(switch_device_t device,
                                  switch_size_t *table_sizes);

switch_status_t switch_table_free(switch_device_t device);

switch_status_t switch_table_default_sizes_get(switch_size_t *table_sizes);

switch_status_t switch_table_size_check(switch_device_t device,
                                        switch_table_id_t table_id,
                                        switch_size_t num_entries,
                                        bool *available);

switch_status_t switch_table_count_increment(switch_device_t device,
                                             switch_table_id_t table_id);

switch_status_t switch_table_count_decrement(switch_device_t device,
                                             switch_table_id_t table_id);

#endif /* _SWITCH_TABLE_INT_H_ */
