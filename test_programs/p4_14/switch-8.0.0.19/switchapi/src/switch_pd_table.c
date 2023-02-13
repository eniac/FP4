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
#include "switchapi/switch_table.h"
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef switch_pd_status_t (*switch_pd_table_entry_count_get_fn)(
    switch_pd_sess_hdl_t sess_hdl,
    switch_pd_target_t dev_tgt,
    switch_uint32_t *count);

typedef struct switch_pd_table_s {
  switch_pd_table_entry_count_get_fn entry_count_get[SWITCH_TABLE_MAX];
} switch_pd_table_t;

switch_pd_table_t pd_table_info;

switch_pd_status_t switch_pd_table_entry_count_get_stub(
    switch_pd_sess_hdl_t sess_hdl,
    switch_pd_target_t dev_tgt,
    switch_uint32_t *count) {
  *count = 0;
  return SWITCH_PD_STATUS_SUCCESS;
}

switch_status_t switch_pd_table_init() {
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_MEMSET(&pd_table_info, 0x0, sizeof(pd_table_info));

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    pd_table_info.entry_count_get[index] = switch_pd_table_entry_count_get_stub;
  }

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    switch (index) {
      case SWITCH_TABLE_NONE:
        break;

      /* Ingress Port */
      case SWITCH_TABLE_INGRESS_PORT_MAPPING:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ingress_port_mapping_get_entry_count;
        break;

      case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ingress_port_properties_get_entry_count;
        break;

      case SWITCH_TABLE_EGRESS_PORT_MAPPING:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_egress_port_mapping_get_entry_count;
        break;

      /* Rmac */
      case SWITCH_TABLE_OUTER_RMAC:
#ifndef P4_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_outer_rmac_get_entry_count;
#endif /* P4_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_INNER_RMAC:
#if !defined(P4_L2_DISABLE) || !defined(P4_L2_MULTICAST_DISABLE) || \
    !defined(P4_L3_MULTICAST_DISABLE)
        pd_table_info.entry_count_get[index] = p4_pd_dc_rmac_get_entry_count;
#endif /* !P4_L2_DISABLE || !P4_L2_MULTICAST_DISABLE || \
          P4_L3_MULTICAST_DISABLE */
        break;

      /* L2 */
      case SWITCH_TABLE_SMAC:
#ifndef P4_L2_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_smac_get_entry_count;
#endif /* P4_L2_DISABLE */
        break;

      case SWITCH_TABLE_DMAC:
#ifndef P4_L2_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_dmac_get_entry_count;
#endif /* P4_L2_DISABLE */
        break;

      /* FIB */
      case SWITCH_TABLE_IPV4_HOST:
#ifndef P4_L3_DISABLE
#ifndef P4_IPV4_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv4_fib_get_entry_count;
#endif /* P4_IPV4_DISABLE */
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_HOST:
#ifndef P4_L3_DISABLE
#ifndef P4_IPV6_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_fib_get_entry_count;
#endif /* P4_IPV6_DISABLE */
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_IPV4_LPM:
#ifndef P4_L3_DISABLE
#ifndef P4_IPV4_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv4_fib_lpm_get_entry_count;
#endif /* P4_IPV4_DISABLE */
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_LPM:
#ifndef P4_L3_DISABLE
#ifndef P4_IPV6_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_fib_lpm_get_entry_count;
#endif /* P4_IPV6_DISABLE */
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_SMAC_REWRITE:
#ifndef P4_L3_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_smac_rewrite_get_entry_count;
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_MTU:
#ifndef P4_L3_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_mtu_get_entry_count;
#endif /* P4_L3_DISABLE */
        break;

      case SWITCH_TABLE_URPF:
        break;

      /* Nexthop */
      case SWITCH_TABLE_NHOP:
        pd_table_info.entry_count_get[index] = p4_pd_dc_nexthop_get_entry_count;
        break;

      case SWITCH_TABLE_ECMP_GROUP:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ecmp_group_get_entry_count;
        break;

      case SWITCH_TABLE_ECMP_SELECT:
        break;

      /* Rewrite */
      case SWITCH_TABLE_REWRITE:
        pd_table_info.entry_count_get[index] = p4_pd_dc_rewrite_get_entry_count;
        break;

      /* Tunnel */
      case SWITCH_TABLE_IPV4_SRC_VTEP:
#ifndef P4_IPV4_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv4_src_vtep_get_entry_count;
#endif /* P4_IPV4_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_IPV4_DST_VTEP:
#ifndef P4_IPV4_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv4_dest_vtep_get_entry_count;
#endif /* P4_IPV4_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_SRC_VTEP:
#ifndef P4_IPV6_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_src_vtep_get_entry_count;
#endif /* P4_IPV6_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_DST_VTEP:
#ifndef P4_IPV6_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_dest_vtep_get_entry_count;
#endif /* P4_IPV6_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_TUNNEL:
#ifndef P4_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_tunnel_get_entry_count;
#endif /* P4_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_TUNNEL_REWRITE:
#ifndef P4_TUNNEL_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_tunnel_rewrite_get_entry_count;
#endif /* P4_TUNNEL_DISABLE */
        break;

      case SWITCH_TABLE_TUNNEL_DECAP:
        break;

      case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_tunnel_smac_rewrite_get_entry_count;
#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
        break;

      case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE) || \
    defined(P4_DTEL_MIRROR_NEXTHOP_ENABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_tunnel_dmac_rewrite_get_entry_count;
#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
        break;

      case SWITCH_TABLE_TUNNEL_DIP_REWRITE:
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_tunnel_dst_rewrite_get_entry_count;
#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
        break;

      /* BD */
      case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_port_vlan_to_bd_mapping_get_entry_count;
        break;

      case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_port_vlan_to_ifindex_mapping_get_entry_count;
        break;

      case SWITCH_TABLE_BD:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_bd_flood_get_entry_count;
        break;

      case SWITCH_TABLE_BD_FLOOD:
        break;

      case SWITCH_TABLE_INGRESS_BD_STATS:
#ifndef P4_STATS_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ingress_bd_stats_get_entry_count;
#endif /* P4_STATS_DISABLE */
        break;

      case SWITCH_TABLE_EGRESS_BD_STATS:
#ifndef P4_STATS_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_egress_bd_stats_get_entry_count;
#endif /* P4_STATS_DISABLE */
        break;

      case SWITCH_TABLE_VLAN_DECAP:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_vlan_decap_get_entry_count;
        break;

      case SWITCH_TABLE_VLAN_XLATE:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_egress_vlan_xlate_get_entry_count;
        break;

      case SWITCH_TABLE_EGRESS_BD:
#ifndef P4_L3_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_egress_bd_map_get_entry_count;
#endif /* P4_L3_DISABLE */
        break;

      /* ACL */
      case SWITCH_TABLE_IPV4_ACL:
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV4_DISABLE)
        pd_table_info.entry_count_get[index] = p4_pd_dc_ip_acl_get_entry_count;
#endif /* P4_RACL_DISABLE && P4_IPV4_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_ACL:
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV6_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_acl_get_entry_count;
#endif /* P4_RACL_DISABLE && P4_IPV6_DISABLE */
        break;

      case SWITCH_TABLE_IPV4_RACL:
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV4_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv4_racl_get_entry_count;
#endif /* P4_RACL_DISABLE && P4_IPV4_DISABLE */
        break;

      case SWITCH_TABLE_IPV6_RACL:
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV6_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_ipv6_racl_get_entry_count;
#endif /* P4_RACL_DISABLE && P4_IPV6_DISABLE */
        break;

      case SWITCH_TABLE_SYSTEM_ACL:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_system_acl_get_entry_count;
        break;

      case SWITCH_TABLE_MAC_ACL:
#if !defined(P4_L2_DISABLE)
#if !defined(P4_INGRESS_MAC_ACL_DISABLE)
        pd_table_info.entry_count_get[index] = p4_pd_dc_mac_acl_get_entry_count;
#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
        break;

      case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_egress_system_acl_get_entry_count;
        break;

      case SWITCH_TABLE_ACL_STATS:
#ifndef P4_STATS_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_acl_stats_get_entry_count;
#endif /* P4_STATS_DISABLE */
        break;

      case SWITCH_TABLE_RACL_STATS:
#ifdef P4_RACL_STATS_ENABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_racl_stats_get_entry_count;
#endif /* P4_RACL_STATS_ENABLE */
        break;

      case SWITCH_TABLE_EGRESS_ACL_STATS:
        break;

      /* Multicast */
      case SWITCH_TABLE_OUTER_MCAST_STAR_G:
        break;

      case SWITCH_TABLE_OUTER_MCAST_SG:
        break;

      case SWITCH_TABLE_OUTER_MCAST_RPF:
        break;

      case SWITCH_TABLE_MCAST_RPF:
        break;

      case SWITCH_TABLE_IPV4_MCAST_S_G:
        break;

      case SWITCH_TABLE_IPV4_MCAST_STAR_G:
        break;

      case SWITCH_TABLE_IPV6_MCAST_S_G:
        break;

      case SWITCH_TABLE_IPV6_MCAST_STAR_G:
        break;

      case SWITCH_TABLE_RID:
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_MIRROR_NEXTHOP_ENABLE)
        pd_table_info.entry_count_get[index] = p4_pd_dc_rid_get_entry_count;
#endif /* P4_MULTICAST_DISABLE || P4_TUNNEL_NEXTHOP_ENABLE || \
          P4_DTEL_MIRROR_NEXTHOP_ENABLE */
        break;

      case SWITCH_TABLE_REPLICA_TYPE:
#ifndef P4_L3_MULTICAST_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_replica_type_get_entry_count;
#endif /* P4_L3_MULTICAST_DISABLE */
        break;

      /* STP */
      case SWITCH_TABLE_STP:
#if !defined(P4_L2_DISABLE) && !defined(P4_STP_DISABLE)
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_spanning_tree_get_entry_count;
#endif /* P4_L2_DISABLE && P4_STP_DISABLE */
        break;

      /* LAG */
      case SWITCH_TABLE_LAG_GROUP:
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_lag_group_get_entry_count;
        break;

      case SWITCH_TABLE_LAG_SELECT:
        break;

      /* Mirror */
      case SWITCH_TABLE_MIRROR:
#ifndef P4_MIRROR_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_mirror_get_entry_count;
#endif /* P4_MIRROR_DISABLE */
        break;

      /* Meter */
      case SWITCH_TABLE_METER_INDEX:
#ifdef P4_QOS_METERING_ENABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_meter_index_get_entry_count;
#endif /* P4_QOS_METERING_ENABLE */
        break;

      case SWITCH_TABLE_METER_ACTION:
#ifdef P4_QOS_METERING_ENABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_meter_action_get_entry_count;
#endif /* P4_QOS_METERING_ENABLE */
        break;

      /* Stats */
      case SWITCH_TABLE_DROP_STATS:
        break;

      case SWITCH_TABLE_NAT_DST:
#ifndef P4_NAT_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_nat_dst_get_entry_count;
#endif /* P4_NAT_DISABLE */
        break;

      case SWITCH_TABLE_NAT_SRC:
#ifndef P4_NAT_DISABLE
        pd_table_info.entry_count_get[index] = p4_pd_dc_nat_src_get_entry_count;
#endif /* P4_NAT_DISABLE */
        break;

      case SWITCH_TABLE_NAT_TWICE:
#ifndef P4_NAT_DISABLE
        pd_table_info.entry_count_get[index] =
            p4_pd_dc_nat_twice_get_entry_count;
#endif /* P4_NAT_DISABLE */
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
        break;

      case SWITCH_TABLE_QUEUE:
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP:
        break;

      case SWITCH_TABLE_EGRESS_QOS_MAP:
        break;

      case SWITCH_TABLE_WRED:
        break;

      default:
        break;
    }
  }
#endif /* defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO) */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_table_entry_count_get(switch_device_t device,
                                                switch_table_id_t table_id,
                                                switch_uint32_t *num_entries) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = pd_table_info.entry_count_get[table_id](
      switch_cfg_sess_hdl, p4_pd_device, num_entries);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "table entry count get failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(table_id),
        switch_pd_action_id_to_string(0));
  }

  status = switch_pd_status_to_status(pd_status);

#endif /* SWITCH_PD */

  return status;
}

#ifdef __cplusplus
}
#endif
