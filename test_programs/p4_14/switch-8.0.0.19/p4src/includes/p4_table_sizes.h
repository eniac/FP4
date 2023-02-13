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
#ifndef _P4_TABLE_SIZES_H_
#define _P4_TABLE_SIZES_H_

// default undefs
#undef IPV4_LOCAL_HOST_TABLE_SIZE

// default table sizes
#define MIN_SRAM_TABLE_SIZE                    1024
#define MIN_TCAM_TABLE_SIZE                    512

#define PORT_VLAN_TABLE_SIZE                   4096
#define IPV4_SRC_TUNNEL_TABLE_SIZE             MIN_SRAM_TABLE_SIZE
#define IPV6_SRC_TUNNEL_TABLE_SIZE             MIN_SRAM_TABLE_SIZE
#define TUNNEL_DST_REWRITE_TABLE_SIZE          MIN_SRAM_TABLE_SIZE
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      MIN_SRAM_TABLE_SIZE
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         MIN_SRAM_TABLE_SIZE
#define VNID_MAPPING_TABLE_SIZE                MIN_SRAM_TABLE_SIZE
#define BD_TABLE_SIZE                          MIN_SRAM_TABLE_SIZE
#define EGRESS_OUTER_BD_MAPPING_TABLE_SIZE     MIN_SRAM_TABLE_SIZE
#define EGRESS_OUTER_BD_STATS_TABLE_SIZE       MIN_SRAM_TABLE_SIZE
#define CPU_BD_TABLE_SIZE                      MIN_SRAM_TABLE_SIZE
#define BD_FLOOD_TABLE_SIZE                    MIN_SRAM_TABLE_SIZE
#define BD_STATS_TABLE_SIZE                    MIN_SRAM_TABLE_SIZE
#define MAC_TABLE_SIZE                         MIN_SRAM_TABLE_SIZE
#define INGRESS_MAC_ACL_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define INGRESS_IP_ACL_TABLE_SIZE              MIN_TCAM_TABLE_SIZE
#define INGRESS_IPV6_ACL_TABLE_SIZE            MIN_TCAM_TABLE_SIZE
#define EGRESS_IP_ACL_TABLE_SIZE               MIN_TCAM_TABLE_SIZE
#define EGRESS_IPV6_ACL_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define INGRESS_IP_RACL_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define INGRESS_IPV6_RACL_TABLE_SIZE           MIN_TCAM_TABLE_SIZE
#define IPV4_LPM_TABLE_SIZE                    MIN_TCAM_TABLE_SIZE
#define IPV6_LPM_TABLE_SIZE                    MIN_TCAM_TABLE_SIZE
#define IPV4_HOST_TABLE_SIZE                   MIN_SRAM_TABLE_SIZE
#define IPV6_HOST_TABLE_SIZE                   MIN_SRAM_TABLE_SIZE
#define ECMP_GROUP_TABLE_SIZE                  MIN_SRAM_TABLE_SIZE
#define ECMP_SELECT_TABLE_SIZE                 MIN_SRAM_TABLE_SIZE
#define NEXTHOP_TABLE_SIZE                     MIN_SRAM_TABLE_SIZE
#define EGRESS_VNID_MAPPING_TABLE_SIZE         MIN_SRAM_TABLE_SIZE
#define EGRESS_BD_MAPPING_TABLE_SIZE           MIN_SRAM_TABLE_SIZE
#define EGRESS_BD_STATS_TABLE_SIZE             MIN_SRAM_TABLE_SIZE
#define RID_TABLE_SIZE                         MIN_SRAM_TABLE_SIZE
#define EGRESS_VLAN_XLATE_TABLE_SIZE           MIN_SRAM_TABLE_SIZE
#define SPANNING_TREE_TABLE_SIZE               MIN_SRAM_TABLE_SIZE
#define INGRESS_ACL_RANGE_TABLE_SIZE           MIN_TCAM_TABLE_SIZE
#define EGRESS_ACL_RANGE_TABLE_SIZE            MIN_TCAM_TABLE_SIZE
#define ACL_STATS_TABLE_SIZE                   MIN_SRAM_TABLE_SIZE
#define RACL_STATS_TABLE_SIZE                  MIN_SRAM_TABLE_SIZE
#define EGRESS_ACL_STATS_TABLE_SIZE            MIN_SRAM_TABLE_SIZE
#define MIRROR_ACL_STATS_TABLE_SIZE            MIN_SRAM_TABLE_SIZE

#define VALIDATE_PACKET_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define PORTMAP_TABLE_SIZE                     288

#define STORM_CONTROL_TABLE_SIZE               MIN_TCAM_TABLE_SIZE
#define STORM_CONTROL_METER_TABLE_SIZE         MIN_TCAM_TABLE_SIZE
#define STORM_CONTROL_STATS_TABLE_SIZE         MIN_SRAM_TABLE_SIZE
#define OUTER_ROUTER_MAC_TABLE_SIZE            MIN_TCAM_TABLE_SIZE
#define DEST_TUNNEL_TABLE_SIZE                 MIN_TCAM_TABLE_SIZE
#define VALIDATE_MPLS_TABLE_SIZE               MIN_TCAM_TABLE_SIZE
#define ROUTER_MAC_TABLE_SIZE                  MIN_TCAM_TABLE_SIZE
#define EGRESS_SYSTEM_ACL_TABLE_SIZE           MIN_TCAM_TABLE_SIZE
#define UPDATE_L4_CHECKSUM_TABLE_SIZE          MIN_TCAM_TABLE_SIZE
#define FWD_RESULT_TABLE_SIZE                  MIN_TCAM_TABLE_SIZE
#define LAG_GROUP_TABLE_SIZE                   MIN_SRAM_TABLE_SIZE
#define LAG_SELECT_TABLE_SIZE                  MIN_SRAM_TABLE_SIZE
#define SYSTEM_ACL_SIZE                        MIN_TCAM_TABLE_SIZE
#define INGRESS_ECN_ACL_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define INGRESS_MIRROR_ACL_TABLE_SIZE          MIN_TCAM_TABLE_SIZE
#define LEARN_NOTIFY_TABLE_SIZE                MIN_TCAM_TABLE_SIZE
#define MAC_REWRITE_TABLE_SIZE                 512
#define REPLICA_TYPE_TABLE_SIZE                MIN_TCAM_TABLE_SIZE
#define TUNNEL_DECAP_TABLE_SIZE                MIN_TCAM_TABLE_SIZE
#define L3_MTU_TABLE_SIZE                      MIN_TCAM_TABLE_SIZE
#define VLAN_DECAP_TABLE_SIZE                  MIN_TCAM_TABLE_SIZE
#define TUNNEL_HEADER_TABLE_SIZE               MIN_TCAM_TABLE_SIZE
#define TUNNEL_REWRITE_TABLE_SIZE              MIN_SRAM_TABLE_SIZE
#define TUNNEL_SMAC_REWRITE_TABLE_SIZE         MIN_SRAM_TABLE_SIZE
#define MIRROR_SESSIONS_TABLE_SIZE             MIN_SRAM_TABLE_SIZE
#define MIRROR_COALESCING_SESSIONS_TABLE_SIZE  8
#define DROP_STATS_TABLE_SIZE                  MIN_SRAM_TABLE_SIZE
#define METER_INDEX_TABLE_SIZE                 MIN_SRAM_TABLE_SIZE
#define METER_ACTION_TABLE_SIZE                MIN_SRAM_TABLE_SIZE
#define DTEL_HASH_WIDTH                        16
#define DTEL_MAX_MIRROR_SESSION_PER_GROUP      120
#define DTEL_BLOOM_FILTER_SIZE                 65536
#define DTEL_QUEUE_TABLE_SIZE                  1024
#define DTEL_FLOW_WATCHLIST_TABLE_SIZE         1024
#define DTEL_FLOW_WATCHLIST_RANGE_ENTRIES      16
#define DTEL_DROP_WATCHLIST_TABLE_SIZE         1024
#define DTEL_DROP_WATCHLIST_RANGE_ENTRIES      16
#define DTEL_CONFIG_SESSIONS                   256
#define DTEL_MIRROR_SESSION_SIZE               64
// DTEL_CONFIG_SESSIONS + 3
#define DTEL_CONFIG_SESSIONS_AND_L4            259
#define MIRROR_ON_DROP_ENCAP_TABLE_SIZE        16
#define INT_L45_MARKER_MAX_L4_PORTS            8
#define INGRESS_QOS_MAP_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define EGRESS_QOS_MAP_TABLE_SIZE              MIN_TCAM_TABLE_SIZE
#define QUEUE_TABLE_SIZE                       MIN_TCAM_TABLE_SIZE
#define DSCP_TO_TC_AND_COLOR_TABLE_SIZE        MIN_TCAM_TABLE_SIZE
#define PCP_TO_TC_AND_COLOR_TABLE_SIZE         MIN_TCAM_TABLE_SIZE
#define SFLOW_INGRESS_TABLE_SIZE               MIN_TCAM_TABLE_SIZE
#define MAX_SFLOW_SESSIONS                     16
#define MAX_BFD_SESSIONS                       512
#define MAX_BFD_SESSIONS_PER_PIPE              128
#define MAX_BFD_SESSIONS_PER_PIPE_2X           256
#define BFD_TX_TIMER_TABLE_SIZE                514 // max + 2
#define FLOWLET_MAP_SIZE                       8192
#define FLOWLET_MAP_WIDTH                      13
#define LAG_FAILOVER_TABLE_SIZE                512
#define ECMP_FAILOVER_TABLE_SIZE               65536
#define LAG_FAILOVER_REG_INSTANCE_COUNT        131072
#define ECMP_FAILOVER_REG_INSTANCE_COUNT       131072
#define COPP_TABLE_SIZE                        MIN_TCAM_TABLE_SIZE
#define EGRESS_PORT_LKP_FIELD_SIZE             4
#define ADJUST_PACKET_LENGTH_TABLE_SIZE        4
#define IP_NAT_TABLE_SIZE                      MIN_TCAM_TABLE_SIZE
#define IP_NAT_FLOW_TABLE_SIZE                 MIN_TCAM_TABLE_SIZE
#define EGRESS_NAT_TABLE_SIZE                  MIN_TCAM_TABLE_SIZE
#define WRED_TABLE_SIZE                        MIN_TCAM_TABLE_SIZE
#define IPSG_TABLE_SIZE                        8192
#define IPSG_PERMIT_SPECIAL_TABLE_SIZE         512
#define WCMP_GROUP_TABLE_SIZE                  6144
#define WRED_INDEX_TABLE_SIZE                  128
#define WRED_ACTION_TABLE_SIZE                 512
#define URPF_GROUP_TABLE_SIZE                  512
#define OUTER_MULTICAST_STAR_G_TABLE_SIZE      MIN_TCAM_TABLE_SIZE
#define OUTER_MULTICAST_S_G_TABLE_SIZE         1024
#define OUTER_MCAST_RPF_TABLE_SIZE             MIN_TCAM_TABLE_SIZE
#define MCAST_RPF_TABLE_SIZE                   MIN_TCAM_TABLE_SIZE
#define IPV4_MULTICAST_STAR_G_TABLE_SIZE       MIN_TCAM_TABLE_SIZE
#define IPV4_MULTICAST_S_G_TABLE_SIZE          MIN_TCAM_TABLE_SIZE
#define IPV6_MULTICAST_STAR_G_TABLE_SIZE       MIN_TCAM_TABLE_SIZE
#define IPV6_MULTICAST_S_G_TABLE_SIZE          MIN_TCAM_TABLE_SIZE

/******************************************************************************
 *  Min Table Size profile
 *****************************************************************************/
#if defined(MIN_TABLE_SIZES)

/******************************************************************************
 *  A typical profile for DC
 *****************************************************************************/
#elif defined(ENT_DC_GENERAL_TABLE_SIZES)

#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef SPANNING_TREE_TABLE_SIZE
#undef VNID_MAPPING_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef EGRESS_OUTER_BD_MAPPING_TABLE_SIZE
#undef EGRESS_OUTER_BD_STATS_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef IPV4_SRC_TUNNEL_TABLE_SIZE
#undef IPV6_SRC_TUNNEL_TABLE_SIZE
#undef TUNNEL_DST_REWRITE_TABLE_SIZE
#undef TUNNEL_TO_MGID_MAPPING_TABLE_SIZE
#undef OUTER_MULTICAST_S_G_TABLE_SIZE
#undef TUNNEL_DMAC_REWRITE_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_IPV6_ACL_TABLE_SIZE
#undef INGRESS_IP_RACL_TABLE_SIZE
#undef INGRESS_IPV6_RACL_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef RACL_STATS_TABLE_SIZE
#undef EGRESS_MAC_ACL_TABLE_SIZE
#undef EGRESS_IPV6_ACL_TABLE_SIZE
#undef EGRESS_ACL_TABLE_SIZE
#undef EGRESS_ACL_STATS_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef IPV6_HOST_TABLE_SIZE
#undef IPV6_LPM_TABLE_SIZE
#undef IPV4_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV4_MULTICAST_S_G_TABLE_SIZE
#undef IPV6_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV6_MULTICAST_S_G_TABLE_SIZE
#undef MCAST_RPF_TABLE_SIZE
#undef RID_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE

// 4K L2 vlans + 4K VXLAN
// 8K BDs
// 8K {port,vlan} <-> BD mappings
#define PORT_VLAN_TABLE_SIZE                   8192
#define BD_TABLE_SIZE                          8192
#define BD_FLOOD_TABLE_SIZE                   24576
#define BD_STATS_TABLE_SIZE                    8192
#define EGRESS_VLAN_XLATE_TABLE_SIZE           8192
#define EGRESS_VNID_MAPPING_TABLE_SIZE         4096
#define EGRESS_BD_MAPPING_TABLE_SIZE           8192
#define EGRESS_BD_STATS_TABLE_SIZE             8192
#define SPANNING_TREE_TABLE_SIZE               4096
#define VNID_MAPPING_TABLE_SIZE                4096
#define CPU_BD_TABLE_SIZE                      8192
#define EGRESS_OUTER_BD_MAPPING_TABLE_SIZE     8192
#define EGRESS_OUTER_BD_STATS_TABLE_SIZE       8192

   // 32K MACs
#define MAC_TABLE_SIZE                         32768

   // Tunnels - 4K IPv4 + 1K IPv6
#define IPV4_SRC_TUNNEL_TABLE_SIZE             4096
#define IPV6_SRC_TUNNEL_TABLE_SIZE             1024
#define TUNNEL_DST_REWRITE_TABLE_SIZE          4096
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      1024
#define OUTER_MULTICAST_S_G_TABLE_SIZE         1024
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         4096

   // Ingress ACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              1024
#define INGRESS_IPV6_ACL_TABLE_SIZE            512
#define INGRESS_IP_RACL_TABLE_SIZE             1024
#define INGRESS_IPV6_RACL_TABLE_SIZE           512
#define ACL_STATS_TABLE_SIZE                   2048
#define RACL_STATS_TABLE_SIZE                  2048

   // Egress ACLs
#define EGRESS_MAC_ACL_TABLE_SIZE              512
#define EGRESS_IPV6_ACL_TABLE_SIZE             512
#define EGRESS_ACL_TABLE_SIZE                  256
#define EGRESS_ACL_STATS_TABLE_SIZE            1536

    // IP Hosts/Routes
#define IPV4_HOST_TABLE_SIZE                   32768
#define IPV4_LPM_TABLE_SIZE                    16384
#define IPV6_HOST_TABLE_SIZE                   16384
#define IPV6_LPM_TABLE_SIZE                    16384

    // Multicast
#define IPV4_MULTICAST_STAR_G_TABLE_SIZE       2048
#define IPV4_MULTICAST_S_G_TABLE_SIZE          4096
#define IPV6_MULTICAST_STAR_G_TABLE_SIZE       512
#define IPV6_MULTICAST_S_G_TABLE_SIZE          512
#define MCAST_RPF_TABLE_SIZE                   8192
#define RID_TABLE_SIZE                         32768

   // ECMP/Nexthop
#define ECMP_GROUP_TABLE_SIZE                  1024
#define ECMP_SELECT_TABLE_SIZE                 16384
#define NEXTHOP_TABLE_SIZE                     32768

#if defined(DTEL_FIN_TABLE_SIZES)

#undef PORT_VLAN_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV4_MULTICAST_S_G_TABLE_SIZE
#undef MCAST_RPF_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef EGRESS_MAC_ACL_TABLE_SIZE
#undef EGRESS_IP_ACL_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef SPANNING_TREE_TABLE_SIZE

#define PORT_VLAN_TABLE_SIZE                   4096

#define IPV4_HOST_TABLE_SIZE                   32768
#define MAC_TABLE_SIZE                         32768
#define IPV4_LPM_TABLE_SIZE                    24576

#define IPV4_MULTICAST_STAR_G_TABLE_SIZE       4096
#define IPV4_MULTICAST_S_G_TABLE_SIZE          4096
#define MCAST_RPF_TABLE_SIZE                   16384

#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              2048
#define EGRESS_MAC_ACL_TABLE_SIZE              1024
#define EGRESS_IP_ACL_TABLE_SIZE               1024
#define INGRESS_IP_RACL_TABLE_SIZE             1024

#define NEXTHOP_TABLE_SIZE                     32768
#define SPANNING_TREE_TABLE_SIZE               2048

#endif /*defined(DTEL_FIN)*/

/******************************************************************************
 *  MSDC base and DTel Profiles                                               *
 *****************************************************************************/
#elif defined(MSDC_TABLE_SIZES)

#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef EGRESS_OUTER_BD_MAPPING_TABLE_SIZE
#undef EGRESS_OUTER_BD_STATS_TABLE_SIZE
#undef VNID_MAPPING_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef IPV4_SRC_TUNNEL_TABLE_SIZE
#undef IPV6_SRC_TUNNEL_TABLE_SIZE
#undef TUNNEL_DST_REWRITE_TABLE_SIZE
#undef TUNNEL_TO_MGID_MAPPING_TABLE_SIZE
#undef TUNNEL_REWRITE_TABLE_SIZE
#undef TUNNEL_DMAC_REWRITE_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_IPV6_ACL_TABLE_SIZE
#undef INGRESS_IP_RACL_TABLE_SIZE
#undef INGRESS_IPV6_RACL_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef RACL_STATS_TABLE_SIZE
#undef IPV4_LOCAL_HOST_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef IPV6_HOST_TABLE_SIZE
#undef IPV6_LPM_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef WRED_INDEX_TABLE_SIZE
#undef WRED_ACTION_TABLE_SIZE
#undef SPANNING_TREE_TABLE_SIZE
#undef RID_TABLE_SIZE

// 4K L2 vlans + 4K VXLANs
// 8K BDs
// 8K {port,vlan} <-> BD mappings
#define PORT_VLAN_TABLE_SIZE                   8192
#define BD_TABLE_SIZE                          8192
#define BD_FLOOD_TABLE_SIZE                    8192
#define BD_STATS_TABLE_SIZE                    8192
#define EGRESS_VLAN_XLATE_TABLE_SIZE           8192
#define EGRESS_VNID_MAPPING_TABLE_SIZE         8192
#define EGRESS_BD_MAPPING_TABLE_SIZE           8192
#define EGRESS_BD_STATS_TABLE_SIZE             8192
#define VNID_MAPPING_TABLE_SIZE                8192
#define CPU_BD_TABLE_SIZE                      8192
#define EGRESS_OUTER_BD_MAPPING_TABLE_SIZE     8192
#define EGRESS_OUTER_BD_STATS_TABLE_SIZE       8192
#define SPANNING_TREE_TABLE_SIZE               4096

// 16K MACs
#define MAC_TABLE_SIZE                         16384

// Tunnels - 4K IPv4 + 1K IPv6
#define IPV4_SRC_TUNNEL_TABLE_SIZE             4096
#define IPV6_SRC_TUNNEL_TABLE_SIZE             1024
#define TUNNEL_DST_REWRITE_TABLE_SIZE          4096
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      1024
#define TUNNEL_REWRITE_TABLE_SIZE              4096
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         4096

// Ingress ACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              1024
#define INGRESS_IPV6_ACL_TABLE_SIZE            512
#define INGRESS_IP_RACL_TABLE_SIZE             1024
#define INGRESS_IPV6_RACL_TABLE_SIZE           512
#define ACL_STATS_TABLE_SIZE                   2048
#define RACL_STATS_TABLE_SIZE                  2048

// IP Hosts/Routes
#define IPV4_LOCAL_HOST_TABLE_SIZE             8192
#define IPV4_HOST_TABLE_SIZE                   40960
#define IPV4_LPM_TABLE_SIZE                    28672
#define IPV6_HOST_TABLE_SIZE                   16384
#define IPV6_LPM_TABLE_SIZE                    16384

// ECMP/Nexthop
#define ECMP_GROUP_TABLE_SIZE                  1024
#define ECMP_SELECT_TABLE_SIZE                 16384
#define NEXTHOP_TABLE_SIZE                     32768

// WRED
#define WRED_INDEX_TABLE_SIZE                  4096
#define WRED_ACTION_TABLE_SIZE                 1536

#define RID_TABLE_SIZE                         32768

/* Keep MSDC DTEL configs after MSDC_TABLE_SIZES */
#if defined(MSDC_LEAF_DTEL_INT_PROFILE) || defined(MSDC_SPINE_DTEL_INT_PROFILE)

#undef DTEL_FLOW_WATCHLIST_TABLE_SIZE
#undef DTEL_DROP_WATCHLIST_TABLE_SIZE
#undef DTEL_FLOW_WATCHLIST_RANGE_ENTRIES
#undef DTEL_DROP_WATCHLIST_RANGE_ENTRIES

#define DTEL_FLOW_WATCHLIST_TABLE_SIZE         512
#define DTEL_DROP_WATCHLIST_TABLE_SIZE         256
#define DTEL_FLOW_WATCHLIST_RANGE_ENTRIES      32
#define DTEL_DROP_WATCHLIST_RANGE_ENTRIES      16

#endif // MSDC DTEL PROFILES

/******************************************************************************
 *  IPv4 only MSDC profile with large host and tunnel scale
 *****************************************************************************/

#elif defined(MSDC_IPV4_TABLE_SIZES)

#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef EGRESS_OUTER_BD_MAPPING_TABLE_SIZE
#undef EGRESS_OUTER_BD_STATS_TABLE_SIZE
#undef VNID_MAPPING_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef IPV4_SRC_TUNNEL_TABLE_SIZE
#undef TUNNEL_DST_REWRITE_TABLE_SIZE
#undef TUNNEL_TO_MGID_MAPPING_TABLE_SIZE
#undef TUNNEL_REWRITE_TABLE_SIZE
#undef TUNNEL_DMAC_REWRITE_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_IP_RACL_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef RACL_STATS_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef WRED_INDEX_TABLE_SIZE
#undef WRED_ACTION_TABLE_SIZE
#undef SPANNING_TREE_TABLE_SIZE
#undef RID_TABLE_SIZE


// 1K L2 vlans + 1K VXLANs
// 2K BDs
// 2K {port,vlan} <-> BD mappings
#define PORT_VLAN_TABLE_SIZE                   1024
#define BD_TABLE_SIZE                          1024
#define BD_FLOOD_TABLE_SIZE                    3072
#define BD_STATS_TABLE_SIZE                    4096
#define EGRESS_VLAN_XLATE_TABLE_SIZE           1024
#define EGRESS_VNID_MAPPING_TABLE_SIZE         1024
#define EGRESS_BD_MAPPING_TABLE_SIZE           2048
#define EGRESS_BD_STATS_TABLE_SIZE             4096
#define VNID_MAPPING_TABLE_SIZE                1024
#define CPU_BD_TABLE_SIZE                      2048
#define EGRESS_OUTER_BD_MAPPING_TABLE_SIZE     1024
#define EGRESS_OUTER_BD_STATS_TABLE_SIZE       4096
#define SPANNING_TREE_TABLE_SIZE               4096

// 4K MACs
#define MAC_TABLE_SIZE                         4096

// Ingress ACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              1024
#define INGRESS_IP_RACL_TABLE_SIZE             1024
#define ACL_STATS_TABLE_SIZE                   2048
#define RACL_STATS_TABLE_SIZE                  2048

// IP Hosts/Routes
#define NUM_HOSTS                              131072
#define IPV4_HOST_TABLE_SIZE                   NUM_HOSTS
#define IPV4_LPM_TABLE_SIZE                    4096

// ECMP/Nexthop
#define NUM_NEXTHOPS                           NUM_HOSTS
#define ECMP_GROUP_TABLE_SIZE                  1024
#define ECMP_SELECT_TABLE_SIZE                 16384
#define NEXTHOP_TABLE_SIZE                     NUM_NEXTHOPS

// Tunnels -
#define NUM_TUNNELS                            NUM_HOSTS
#define NUM_TUNNEL_NHOP_GROUPS                 1024
#define NUM_TUNNEL_NHOP                        4096
#define NUM_TUNNEL_ID                          1024
#define IPV4_SRC_TUNNEL_TABLE_SIZE             NUM_TUNNEL_ID
#define TUNNEL_DST_REWRITE_TABLE_SIZE          NUM_TUNNELS
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      NUM_TUNNELS
#define TUNNEL_REWRITE_TABLE_SIZE              NUM_TUNNEL_ID
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         NUM_TUNNEL_NHOP

// WRED
#define WRED_INDEX_TABLE_SIZE                  4096
#define WRED_ACTION_TABLE_SIZE                 1536

#define RID_TABLE_SIZE                         32768

/******************************************************************************
 *  L3 only MSDC profile
 *****************************************************************************/

#elif defined(MSDC_L3_TABLE_SIZES)

#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef EGRESS_OUTER_BD_MAPPING_TABLE_SIZE
#undef EGRESS_OUTER_BD_STATS_TABLE_SIZE
#undef VNID_MAPPING_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef IPV4_SRC_TUNNEL_TABLE_SIZE
#undef IPV6_SRC_TUNNEL_TABLE_SIZE
#undef TUNNEL_DST_REWRITE_TABLE_SIZE
#undef TUNNEL_TO_MGID_MAPPING_TABLE_SIZE
#undef TUNNEL_REWRITE_TABLE_SIZE
#undef TUNNEL_DMAC_REWRITE_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_IPV6_ACL_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef IPV6_HOST_TABLE_SIZE
#undef IPV6_LPM_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef WRED_INDEX_TABLE_SIZE
#undef WRED_ACTION_TABLE_SIZE
#undef RID_TABLE_SIZE

// 4K L3 interfaces
#define PORT_VLAN_TABLE_SIZE                   4096
#define BD_TABLE_SIZE                          4096
#define BD_FLOOD_TABLE_SIZE                    4096
#define BD_STATS_TABLE_SIZE                    4096
#define EGRESS_VLAN_XLATE_TABLE_SIZE           4096
#define EGRESS_VNID_MAPPING_TABLE_SIZE         4096
#define EGRESS_BD_MAPPING_TABLE_SIZE           4096
#define EGRESS_BD_STATS_TABLE_SIZE             4096
#define VNID_MAPPING_TABLE_SIZE                4096
#define CPU_BD_TABLE_SIZE                      4096
#define EGRESS_OUTER_BD_MAPPING_TABLE_SIZE     4096
#define EGRESS_OUTER_BD_STATS_TABLE_SIZE       4096

// 1K MACs
#define MAC_TABLE_SIZE                         1024

// Tunnels - 4K IPv4 + 1K IPv6
#define IPV4_SRC_TUNNEL_TABLE_SIZE             4096
#define IPV6_SRC_TUNNEL_TABLE_SIZE             1024
#define TUNNEL_DST_REWRITE_TABLE_SIZE          4096
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      1024
#define TUNNEL_REWRITE_TABLE_SIZE              4096
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         4096

// Ingress ACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              2048
#define INGRESS_IPV6_ACL_TABLE_SIZE            512
#define ACL_STATS_TABLE_SIZE                   2048

// IP Hosts/Routes
#define IPV4_HOST_TABLE_SIZE                   32768
#define IPV4_LPM_TABLE_SIZE                    65536
#define IPV6_HOST_TABLE_SIZE                   32768
#define IPV6_LPM_TABLE_SIZE                    16384

// ECMP/Nexthop
#define ECMP_GROUP_TABLE_SIZE                  4096
#define ECMP_SELECT_TABLE_SIZE                 32768
#define NEXTHOP_TABLE_SIZE                     32768

// WRED
#define WRED_INDEX_TABLE_SIZE                  4096
#define WRED_ACTION_TABLE_SIZE                 1536

#define RID_TABLE_SIZE                         32768

/******************************************************************************
 * INT_LEAF profiles for L3 fabric
 *****************************************************************************/
#elif defined(L3_HEAVY_INT_LEAF_TABLE_SIZES) || defined(GENERIC_INT_LEAF_TABLE_SIZES)
#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_ACL_RANGE_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef INGRESS_ECN_ACL_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef RID_TABLE_SIZE
#undef WRED_INDEX_TABLE_SIZE
#undef WRED_ACTION_TABLE_SIZE
#undef WRED_TABLE_SIZE
#undef EGRESS_IP_ACL_TABLE_SIZE
#undef DTEL_DROP_WATCHLIST_TABLE_SIZE
#undef DTEL_DROP_WATCHLIST_RANGE_ENTRIES

// DTEL
#define DTEL_DROP_WATCHLIST_TABLE_SIZE         256
#define DTEL_DROP_WATCHLIST_RANGE_ENTRIES      8

// WRED
#define WRED_INDEX_TABLE_SIZE                  4096
#define WRED_ACTION_TABLE_SIZE                 1536
#define WRED_TABLE_SIZE                        256

// ECMP
#define ECMP_GROUP_TABLE_SIZE                  1024
#define ECMP_SELECT_TABLE_SIZE                 16384

#if defined(L3_HEAVY_INT_LEAF_TABLE_SIZES)
#define PORT_VLAN_TABLE_SIZE                   2048
#define BD_TABLE_SIZE                          2048
#define BD_FLOOD_TABLE_SIZE                    6144
#define BD_STATS_TABLE_SIZE                    8192
#define EGRESS_VLAN_XLATE_TABLE_SIZE           2048
#define EGRESS_BD_MAPPING_TABLE_SIZE           2048
#define EGRESS_BD_STATS_TABLE_SIZE             8192
#define CPU_BD_TABLE_SIZE                      2048

// MACs
#define MAC_TABLE_SIZE                         4096

// ACLs
#define INGRESS_IP_ACL_TABLE_SIZE              1024 // Used for RACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_ACL_RANGE_TABLE_SIZE           256
#define ACL_STATS_TABLE_SIZE                   2048
#define EGRESS_IP_ACL_TABLE_SIZE               512
#define INGRESS_ECN_ACL_TABLE_SIZE             64

// IP Hosts/Routes
#define IPV4_HOST_TABLE_SIZE                   65536
#define IPV4_LPM_TABLE_SIZE                    65536

// Nexthop
#define NEXTHOP_TABLE_SIZE                     65536

// Multicast
#define RID_TABLE_SIZE                         4096

#elif defined(GENERIC_INT_LEAF_TABLE_SIZES)
#undef SPANNING_TREE_TABLE_SIZE
#undef IPV4_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV4_MULTICAST_S_G_TABLE_SIZE

#define PORT_VLAN_TABLE_SIZE                   4096
#define BD_TABLE_SIZE                          4096
#define BD_FLOOD_TABLE_SIZE                    12288
#define BD_STATS_TABLE_SIZE                    16384
#define EGRESS_VLAN_XLATE_TABLE_SIZE           4096
#define EGRESS_BD_MAPPING_TABLE_SIZE           4096
#define EGRESS_BD_STATS_TABLE_SIZE             16384
#define CPU_BD_TABLE_SIZE                      4096

#define SPANNING_TREE_TABLE_SIZE               4096

// MACs
#define MAC_TABLE_SIZE                         32768

// ACLs
#define INGRESS_IP_ACL_TABLE_SIZE              7168 // Used for RACLs
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_ACL_RANGE_TABLE_SIZE           256
#define ACL_STATS_TABLE_SIZE                   8192
#define EGRESS_IP_ACL_TABLE_SIZE               512
#define INGRESS_ECN_ACL_TABLE_SIZE             64

// IP Hosts/Routes
#define IPV4_HOST_TABLE_SIZE                   32768
#define IPV4_LPM_TABLE_SIZE                    8192

// Nexthop
#define NEXTHOP_TABLE_SIZE                     49152

// Multicast
#define IPV4_MULTICAST_STAR_G_TABLE_SIZE       4096
#define IPV4_MULTICAST_S_G_TABLE_SIZE          8192
#define RID_TABLE_SIZE                         32768

#endif

#else
/******************************************************************************
 *  Default Profile
 *****************************************************************************/
#undef PORT_VLAN_TABLE_SIZE
#undef BD_TABLE_SIZE
#undef BD_FLOOD_TABLE_SIZE
#undef BD_STATS_TABLE_SIZE
#undef EGRESS_VLAN_XLATE_TABLE_SIZE
#undef EGRESS_VNID_MAPPING_TABLE_SIZE
#undef EGRESS_BD_MAPPING_TABLE_SIZE
#undef EGRESS_BD_STATS_TABLE_SIZE
#undef VNID_MAPPING_TABLE_SIZE
#undef CPU_BD_TABLE_SIZE
#undef MAC_TABLE_SIZE
#undef IPV4_SRC_TUNNEL_TABLE_SIZE
#undef IPV6_SRC_TUNNEL_TABLE_SIZE
#undef TUNNEL_DST_REWRITE_TABLE_SIZE
#undef TUNNEL_TO_MGID_MAPPING_TABLE_SIZE
#undef TUNNEL_DMAC_REWRITE_TABLE_SIZE
#undef INGRESS_MAC_ACL_TABLE_SIZE
#undef INGRESS_IP_ACL_TABLE_SIZE
#undef INGRESS_IPV6_ACL_TABLE_SIZE
#undef INGRESS_IP_RACL_TABLE_SIZE
#undef INGRESS_IPV6_RACL_TABLE_SIZE
#undef ACL_STATS_TABLE_SIZE
#undef RACL_STATS_TABLE_SIZE
#undef EGRESS_ACL_STATS_TABLE_SIZE
#undef IPV4_HOST_TABLE_SIZE
#undef IPV4_LPM_TABLE_SIZE
#undef IPV6_HOST_TABLE_SIZE
#undef IPV6_LPM_TABLE_SIZE
#undef ECMP_GROUP_TABLE_SIZE
#undef ECMP_SELECT_TABLE_SIZE
#undef NEXTHOP_TABLE_SIZE
#undef WRED_INDEX_TABLE_SIZE
#undef WRED_ACTION_TABLE_SIZE
#undef IPV4_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV4_MULTICAST_S_G_TABLE_SIZE
#undef IPV6_MULTICAST_STAR_G_TABLE_SIZE
#undef IPV6_MULTICAST_S_G_TABLE_SIZE
#undef MCAST_RPF_TABLE_SIZE
#undef RID_TABLE_SIZE
#undef SPANNING_TREE_TABLE_SIZE

#define PORT_VLAN_TABLE_SIZE                   16384
#define IPV4_SRC_TUNNEL_TABLE_SIZE             16384
#define IPV6_SRC_TUNNEL_TABLE_SIZE             4096
#define TUNNEL_DST_REWRITE_TABLE_SIZE          16384
#define TUNNEL_TO_MGID_MAPPING_TABLE_SIZE      4096
#define OUTER_MULTICAST_S_G_TABLE_SIZE         1024
#define VNID_MAPPING_TABLE_SIZE                16384
#define BD_TABLE_SIZE                          16384
#define CPU_BD_TABLE_SIZE                      8192
#define BD_FLOOD_TABLE_SIZE                    49152
#define BD_STATS_TABLE_SIZE                    16384

#define MAC_TABLE_SIZE                         65536
#define INGRESS_MAC_ACL_TABLE_SIZE             512
#define INGRESS_IP_ACL_TABLE_SIZE              1024
#define INGRESS_IPV6_ACL_TABLE_SIZE            512
#define EGRESS_MAC_ACL_TABLE_SIZE              512
#define INGRESS_IP_RACL_TABLE_SIZE             1024
#define INGRESS_IPV6_RACL_TABLE_SIZE           512

#define IPV4_LPM_TABLE_SIZE                    32768
#define IPV6_LPM_TABLE_SIZE                    16384
#define IPV4_HOST_TABLE_SIZE                   65536
#define IPV6_HOST_TABLE_SIZE                   16384

#define IPV4_MULTICAST_STAR_G_TABLE_SIZE       2048
#define IPV4_MULTICAST_S_G_TABLE_SIZE          4096
#define IPV6_MULTICAST_STAR_G_TABLE_SIZE       512
#define IPV6_MULTICAST_S_G_TABLE_SIZE          512
#define MCAST_RPF_TABLE_SIZE                   32768

#define ECMP_GROUP_TABLE_SIZE                  1024
#define ECMP_SELECT_TABLE_SIZE                 16384
#define NEXTHOP_TABLE_SIZE                     49152

#define MAC_REWRITE_TABLE_SIZE                 512
#define EGRESS_VNID_MAPPING_TABLE_SIZE         16384
#define EGRESS_BD_MAPPING_TABLE_SIZE           16384
#define EGRESS_BD_STATS_TABLE_SIZE             16384
#define RID_TABLE_SIZE                         30720  // FIXME: 32768
#define EGRESS_VLAN_XLATE_TABLE_SIZE           16384
#define SPANNING_TREE_TABLE_SIZE               4096
#define EGRESS_ACL_TABLE_SIZE                  1024
#define TUNNEL_DMAC_REWRITE_TABLE_SIZE         4096

#define MIRROR_COALESCING_SESSIONS_TABLE_SIZE  8

#define ACL_STATS_TABLE_SIZE                   8192
#define RACL_STATS_TABLE_SIZE                  2048
#define EGRESS_ACL_STATS_TABLE_SIZE            2048

#define LAG_FAILOVER_TABLE_SIZE                512
#define ECMP_FAILOVER_TABLE_SIZE               65536
#define LAG_FAILOVER_REG_INSTANCE_COUNT        131072
#define ECMP_FAILOVER_REG_INSTANCE_COUNT       131072

#define EGRESS_PORT_LKP_FIELD_SIZE             4

#define ADJUST_PACKET_LENGTH_TABLE_SIZE        4

#define SRV6_LOCAL_SID_TABLE_SIZE              1024

#endif /* !MIN_TABLE_SIZES */

#endif /* _P4_TABLE_SIZES_H_ */

// override disable
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
#define URPF_DISABLE
#endif
