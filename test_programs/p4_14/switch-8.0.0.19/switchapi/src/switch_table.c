
/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_table_default_sizes_get(switch_size_t *table_sizes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;

  if (!table_sizes) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("table sizes get failed: %s",
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    switch (index) {
      case SWITCH_TABLE_NONE:
        table_sizes[index] = 0;
        break;

      /* Ingress Port */
      case SWITCH_TABLE_INGRESS_PORT_MAPPING:
        table_sizes[index] = PORTMAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
        table_sizes[index] = PORTMAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_PORT_MAPPING:
        table_sizes[index] = PORTMAP_TABLE_SIZE;
        break;

      /* Rmac */
      case SWITCH_TABLE_OUTER_RMAC:
        table_sizes[index] = OUTER_ROUTER_MAC_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INNER_RMAC:
        table_sizes[index] = ROUTER_MAC_TABLE_SIZE;
        break;

      /* L2 */
      case SWITCH_TABLE_SMAC:
        table_sizes[index] = MAC_TABLE_SIZE;
        break;

      case SWITCH_TABLE_DMAC:
        table_sizes[index] = MAC_TABLE_SIZE;
        break;

      /* FIB */
      case SWITCH_TABLE_IPV4_HOST:
        table_sizes[index] = IPV4_HOST_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_HOST:
        table_sizes[index] = IPV6_HOST_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_LPM:
        table_sizes[index] = IPV4_LPM_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_LPM:
        table_sizes[index] = IPV6_LPM_TABLE_SIZE;
        break;

      case SWITCH_TABLE_SMAC_REWRITE:
        table_sizes[index] = MAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_MTU:
        table_sizes[index] = L3_MTU_TABLE_SIZE;
        break;

      case SWITCH_TABLE_URPF:
        table_sizes[index] = URPF_GROUP_TABLE_SIZE;
        break;

      /* Nexthop */
      case SWITCH_TABLE_NHOP:
        table_sizes[index] = NEXTHOP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ECMP_GROUP:
        table_sizes[index] = ECMP_GROUP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ECMP_SELECT:
        table_sizes[index] = ECMP_SELECT_TABLE_SIZE;
        break;

      /* Rewrite */
      case SWITCH_TABLE_REWRITE:
        table_sizes[index] = ECMP_SELECT_TABLE_SIZE;
        break;

      /* Tunnel */
      case SWITCH_TABLE_IPV4_SRC_VTEP:
        table_sizes[index] = IPV4_SRC_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_DST_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_SRC_VTEP:
        table_sizes[index] = IPV6_SRC_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_DST_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL:
        table_sizes[index] = VNID_MAPPING_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_REWRITE:
        table_sizes[index] = VNID_MAPPING_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_DECAP:
        table_sizes[index] = TUNNEL_DECAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
        table_sizes[index] = TUNNEL_SMAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
        table_sizes[index] = TUNNEL_DMAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_DIP_REWRITE:
        table_sizes[index] = TUNNEL_DST_REWRITE_TABLE_SIZE;
        break;

      /* BD */
      case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
        table_sizes[index] = PORT_VLAN_TABLE_SIZE;
        break;

      case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
        table_sizes[index] = PORT_VLAN_TABLE_SIZE;
        break;

      case SWITCH_TABLE_BD:
        table_sizes[index] = BD_TABLE_SIZE;
        break;

      case SWITCH_TABLE_BD_FLOOD:
        table_sizes[index] = BD_FLOOD_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_BD_STATS:
        table_sizes[index] = BD_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_BD_STATS:
        table_sizes[index] = EGRESS_BD_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_VLAN_DECAP:
        table_sizes[index] = VLAN_DECAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_VLAN_XLATE:
        table_sizes[index] = EGRESS_VLAN_XLATE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_BD:
        table_sizes[index] = EGRESS_BD_MAPPING_TABLE_SIZE;
        break;

      /* ACL */
      case SWITCH_TABLE_IPV4_ACL:
        table_sizes[index] = INGRESS_IP_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_ACL:
        table_sizes[index] = INGRESS_IPV6_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_RACL:
        table_sizes[index] = INGRESS_IP_RACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_RACL:
        table_sizes[index] = INGRESS_IPV6_RACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_SYSTEM_ACL:
        table_sizes[index] = SYSTEM_ACL_SIZE;
        break;

      case SWITCH_TABLE_MAC_ACL:
        table_sizes[index] = INGRESS_MAC_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
        table_sizes[index] = EGRESS_SYSTEM_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ACL_STATS:
        table_sizes[index] = ACL_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_RACL_STATS:
        table_sizes[index] = RACL_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_ACL_STATS:
        table_sizes[index] = EGRESS_ACL_STATS_TABLE_SIZE;
        break;

      /* Multicast */
      case SWITCH_TABLE_OUTER_MCAST_STAR_G:
        table_sizes[index] = OUTER_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_OUTER_MCAST_SG:
        table_sizes[index] = OUTER_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_OUTER_MCAST_RPF:
        table_sizes[index] = OUTER_MCAST_RPF_TABLE_SIZE;
        break;

      case SWITCH_TABLE_MCAST_RPF:
        table_sizes[index] = MCAST_RPF_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_MCAST_S_G:
        table_sizes[index] = IPV4_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_MCAST_STAR_G:
        table_sizes[index] = IPV4_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_MCAST_S_G:
        table_sizes[index] = IPV4_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_MCAST_STAR_G:
        table_sizes[index] = IPV4_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_RID:
        table_sizes[index] = RID_TABLE_SIZE;
        break;

      case SWITCH_TABLE_REPLICA_TYPE:
        table_sizes[index] = REPLICA_TYPE_TABLE_SIZE;
        break;

      /* STP */
      case SWITCH_TABLE_STP:
        table_sizes[index] = SPANNING_TREE_TABLE_SIZE;
        break;

      /* LAG */
      case SWITCH_TABLE_LAG_GROUP:
        table_sizes[index] = LAG_GROUP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_LAG_SELECT:
        table_sizes[index] = LAG_SELECT_TABLE_SIZE;
        break;

      /* Mirror */
      case SWITCH_TABLE_MIRROR:
        table_sizes[index] = MIRROR_SESSIONS_TABLE_SIZE;
        break;

      /* Meter */
      case SWITCH_TABLE_METER_INDEX:
        table_sizes[index] = METER_INDEX_TABLE_SIZE;
        break;

      case SWITCH_TABLE_METER_ACTION:
        table_sizes[index] = METER_ACTION_TABLE_SIZE;
        break;

      /* Stats */
      case SWITCH_TABLE_DROP_STATS:
        table_sizes[index] = DROP_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_DST:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_SRC:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_TWICE:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
        table_sizes[index] = DSCP_TO_TC_AND_COLOR_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
        table_sizes[index] = PCP_TO_TC_AND_COLOR_TABLE_SIZE;
        break;

      case SWITCH_TABLE_QUEUE:
        table_sizes[index] = QUEUE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP:
        table_sizes[index] = INGRESS_QOS_MAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_QOS_MAP:
        table_sizes[index] = EGRESS_QOS_MAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_WRED:
        table_sizes[index] = WRED_TABLE_SIZE;
        break;

      default:
        table_sizes[index] = 0;
        break;
    }
  }

  return status;
}

switch_status_t switch_table_init(switch_device_t device,
                                  switch_size_t *table_sizes) {
  switch_table_t *table_info = NULL;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_char_t *table_str = NULL;

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    table_info[index].table_size = table_sizes[index];
    table_info[index].direction = switch_table_id_to_direction(index);
    table_info[index].num_entries = 0;
    if (table_sizes[index]) {
      table_info[index].valid = TRUE;
      table_str = switch_table_id_to_string(index);
      SWITCH_MEMCPY(
          &table_info[index].table_name, table_str, strlen(table_str));
    }
  }

  return status;
}

switch_status_t switch_table_free(switch_device_t device) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(table_info, 0, SWITCH_TABLE_MAX * sizeof(switch_table_t));
  return status;
}

switch_status_t switch_table_count_increment(switch_device_t device,
                                             switch_table_id_t table_id) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  table_info[table_id].num_entries++;

  SWITCH_LOG_DEBUG(
      "table entry incremented on device %d "
      "table name %s table id %d "
      "table size %s num entries %d",
      device,
      table_info[table_id].table_name,
      table_id,
      table_info[table_id].table_size,
      table_info[table_id].num_entries);

  return status;
}

switch_status_t switch_table_count_decrement(switch_device_t device,
                                             switch_table_id_t table_id) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  table_info[table_id].num_entries--;

  SWITCH_LOG_DEBUG(
      "table entry incremented on device %d "
      "table name %s table id %d "
      "table size %s num entries %d",
      device,
      table_info[table_id].table_name,
      table_id,
      table_info[table_id].table_size,
      table_info[table_id].num_entries);

  return status;
}

switch_status_t switch_table_size_check(switch_device_t device,
                                        switch_table_id_t table_id,
                                        switch_size_t num_entries,
                                        bool *available) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *available = false;

  if ((table_info[table_id].num_entries + num_entries) <=
      table_info[table_id].table_size) {
    *available = true;
  }

  return status;
}

switch_status_t switch_api_table_get_internal(switch_device_t device,
                                              switch_table_id_t table_id,
                                              switch_table_t *api_table_info) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));
  SWITCH_ASSERT(api_table_info != NULL);

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(api_table_info, &table_info[table_id], sizeof(switch_table_t));

  SWITCH_LOG_DEBUG(
      "table info get on device %d"
      "table name %s table id "
      "table size %d num_entries %d",
      device,
      table_info[table_id].table_name,
      table_id,
      table_info[table_id].table_size,
      table_info[table_id].num_entries);

  SWITCH_LOG_DEBUG("table info get on device %d for table %s",
                   device,
                   table_info[table_id].table_name);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_table_size_get_internal(switch_device_t device,
                                                   switch_table_id_t table_id,
                                                   switch_size_t *table_size) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));
  SWITCH_ASSERT(table_size != NULL);

  if (!table_size) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *table_size = table_info[table_id].table_size;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_table_all_get_internal(
    switch_device_t device,
    switch_size_t *num_entries,
    switch_table_t *api_table_info) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_table_info != NULL);
  SWITCH_ASSERT(num_entries != NULL);

  if (!api_table_info || !num_entries) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("failed to get table on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      api_table_info, table_info, SWITCH_TABLE_MAX * sizeof(switch_table_t));

  *num_entries = SWITCH_TABLE_MAX;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_table_entry_count_get_internal(
    switch_device_t device,
    switch_table_id_t table_id,
    switch_uint32_t *num_entries) {
  switch_table_t *table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (table_id > SWITCH_TABLE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "table entry count get failed on device %d table id %d: "
        "table id invalid:(%s)",
        device,
        table_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "table entry count get failed on device %d table id %d: "
        "table get failed:(%s)",
        device,
        table_id,
        switch_error_to_string(status));
    return status;
  }

  if (!(table_info[table_id].valid)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "table entry count get failed on device %d table id %d: "
        "table is not valid:(%s)",
        device,
        table_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_table_entry_count_get(device, table_id, num_entries);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "table entry count get failed on device %d table id %d: "
        "pd table entry count get failed:(%s)",
        device,
        table_id,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_table_size_get(switch_device_t device,
                                          switch_table_id_t table_id,
                                          switch_size_t *table_size) {
  SWITCH_MT_WRAP(
      switch_api_table_size_get_internal(device, table_id, table_size))
}

switch_status_t switch_api_table_get(switch_device_t device,
                                     switch_table_id_t table_id,
                                     switch_table_t *api_table_info) {
  SWITCH_MT_WRAP(
      switch_api_table_get_internal(device, table_id, api_table_info))
}

switch_status_t switch_api_table_all_get(switch_device_t device,
                                         switch_size_t *num_entries,
                                         switch_table_t *api_table_info) {
  SWITCH_MT_WRAP(
      switch_api_table_all_get_internal(device, num_entries, api_table_info))
}

switch_status_t switch_api_table_entry_count_get(switch_device_t device,
                                                 switch_table_id_t table_id,
                                                 switch_uint32_t *num_entries) {
  SWITCH_MT_WRAP(
      switch_api_table_entry_count_get_internal(device, table_id, num_entries))
}
