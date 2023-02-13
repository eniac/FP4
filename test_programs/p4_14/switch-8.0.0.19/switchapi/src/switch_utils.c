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

#include "switchapi/switch_utils.h"
#include "switch_ver.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_uint32_t MurmurHash(const void *key,
                           switch_uint32_t length,
                           switch_uint32_t seed) {
// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.
#define m 0x5bd1e995
#define r 24

  // Initialize the hash to a 'random' value
  switch_uint32_t h = seed ^ (switch_uint32_t)length;

  // Mix 4 bytes at a time into the hash
  const unsigned char *data = (const unsigned char *)key;

  while (length >= 4) {
    uint32_t k = *(uint32_t *)data;

    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;

    data += 4;
    length -= 4;
  }

  // Handle the last few bytes of the input array

  switch (length) {
    case 3:
      h ^= data[2] << 16;
    case 2:
      h ^= data[1] << 8;
    case 1:
      h ^= data[0];
      h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}

const char *switch_get_version(void) { return SWITCH_VER; }

const char *switch_get_internal_version(void) { return SWITCH_INTERNAL_VER; }

switch_status_t SWITCH_ARRAY_INIT(switch_array_t *array) {
  SWITCH_ASSERT(array != NULL);
  array->array = NULL;
  array->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_uint32_t SWITCH_ARRAY_COUNT(switch_array_t *array) {
  SWITCH_ASSERT(array != NULL);
  return array->num_entries;
}

switch_status_t SWITCH_ARRAY_INSERT(switch_array_t *array,
                                    switch_uint64_t index,
                                    void *data) {
  SWITCH_ASSERT(array != NULL);
  SWITCH_ASSERT(data != NULL);

  void *p = NULL;
  JLI(p, array->array, (switch_uint64_t)index);
  if (p) {
    *(switch_uint64_t *)p = (switch_uint64_t)data;
    array->num_entries++;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_NO_MEMORY;
  }
}

switch_status_t SWITCH_ARRAY_GET(switch_array_t *array,
                                 switch_uint64_t index,
                                 void **data) {
  SWITCH_ASSERT(array != NULL);
  SWITCH_ASSERT(data != NULL);

  void *p = NULL;
  JLG(p, array->array, (switch_uint64_t)index);
  if (p) {
    *(switch_uint64_t *)data = *(switch_uint64_t *)p;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }
}

switch_status_t SWITCH_ARRAY_DELETE(switch_array_t *array,
                                    switch_uint64_t index) {
  SWITCH_ASSERT(array != NULL);

  switch_int32_t rc = 0;
  JLD(rc, array->array, (switch_uint64_t)index);
  if (rc == 1) {
    array->num_entries--;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }
}

switch_status_t SWITCH_LIST_INIT(switch_list_t *list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("list insert failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_list_init(&list->list);
  list->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_LIST_SORT(switch_list_t *list,
                                 switch_list_compare_func_t compare_func) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("list insert failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_list_sort(&list->list, compare_func);
  return SWITCH_STATUS_SUCCESS;
}

bool SWITCH_LIST_EMPTY(switch_list_t *list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool is_empty = false;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("list empty get failed(%s)\n",
                     switch_error_to_string(status));
    return FALSE;
  }

  is_empty = tommy_list_empty(&list->list);
  return is_empty;
}

switch_size_t SWITCH_LIST_COUNT(switch_list_t *list) {
  SWITCH_ASSERT(list != NULL);

  if (!list) {
    return 0;
  }

  return list->num_entries;
}

switch_status_t SWITCH_LIST_INSERT(switch_list_t *list,
                                   switch_node_t *node,
                                   void *data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);
  SWITCH_ASSERT(node != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!list || !node || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("list insert failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_list_insert_head(&list->list, node, data);
  list->num_entries++;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_LIST_DELETE(switch_list_t *list, switch_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);
  SWITCH_ASSERT(node != NULL);

  if (!list || !node) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("list delete failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_list_remove_existing(&list->list, node);
  list->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_INIT(switch_hashtable_t *hashtable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(hashtable->size != 0);

  if (!hashtable || hashtable->size == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable init failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_init(&hashtable->table, hashtable->size);
  hashtable->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_size_t SWITCH_HASHTABLE_COUNT(switch_hashtable_t *hashtable) {
  SWITCH_ASSERT(hashtable != NULL);

  if (!hashtable) {
    return 0;
  }

  return hashtable->num_entries;
}

switch_status_t SWITCH_HASHTABLE_INSERT(switch_hashtable_t *hashtable,
                                        switch_hashnode_t *node,
                                        void *key,
                                        void *data) {
  switch_uint8_t hash_key[SWITCH_LOG_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(node != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !node || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable insert failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hashtable insert failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  tommy_hashtable_insert(&hashtable->table, node, data, hash);
  hashtable->num_entries++;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DELETE(switch_hashtable_t *hashtable,
                                        void *key,
                                        void **data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t hash_key[SWITCH_LOG_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable delete failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hashtable delete failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void *)tommy_hashtable_remove(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("hashtable delete failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  hashtable->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DELETE_NODE(switch_hashtable_t *hashtable,
                                             switch_hashnode_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(node != NULL);

  if (!hashtable || !node) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable delete node failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_remove_existing(&hashtable->table, node);
  hashtable->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_SEARCH(switch_hashtable_t *hashtable,
                                        void *key,
                                        void **data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t hash_key[SWITCH_LOG_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable search failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hashtable search failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void *)tommy_hashtable_search(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_DEBUG("hashtable search failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t SWITCH_HASHTABLE_FOREACH_ARG(switch_hashtable_t *hashtable,
                                             void *func,
                                             void *arg) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(func != NULL);
  SWITCH_ASSERT(arg != NULL);

  if (!hashtable || !func || !arg) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable foreach arg failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_foreach_arg(&hashtable->table, func, arg);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DONE(switch_hashtable_t *hashtable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);

  if (!hashtable) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("hashtable done failed(%s)\n",
                     switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_done(&hashtable->table);
  return SWITCH_STATUS_SUCCESS;
}

char *switch_error_to_string(switch_status_t status) {
  switch (status) {
    case SWITCH_STATUS_ITEM_NOT_FOUND:
      return "err: entry not found";
    case SWITCH_STATUS_FAILURE:
      return "err: general failure";
    case SWITCH_STATUS_NO_MEMORY:
      return "err: no memory";
    case SWITCH_STATUS_INSUFFICIENT_RESOURCES:
      return "err: invalid parameter";
    case SWITCH_STATUS_ITEM_ALREADY_EXISTS:
      return "err: item already exists";
    case SWITCH_STATUS_BUFFER_OVERFLOW:
      return "err: buffer overflow";
    case SWITCH_STATUS_INVALID_PORT_NUMBER:
      return "err: invalid port number";
    case SWITCH_STATUS_INVALID_PORT_MEMBER:
      return "err: invalid port member";
    case SWITCH_STATUS_UNINITIALIZED:
      return "err: uninitialized";
    case SWITCH_STATUS_TABLE_FULL:
      return "err: table full";
    case SWITCH_STATUS_INVALID_VLAN_ID:
      return "err: invalid vlan id";
    case SWITCH_STATUS_INVALID_ATTRIBUTE:
      return "err: invalid attribute";
    case SWITCH_STATUS_INVALID_INTERFACE:
      return "err: invalid interface";
    case SWITCH_STATUS_PORT_IN_USE:
      return "err: port in use";
    case SWITCH_STATUS_NOT_IMPLEMENTED:
      return "err: not implemented";
    case SWITCH_STATUS_INVALID_HANDLE:
      return "err: invalid handle";
    case SWITCH_STATUS_PD_FAILURE:
      return "err: pd failure";
    case SWITCH_STATUS_INVALID_PARAMETER:
      return "err: invalid parameter";
    default:
      return "err: unknown failure";
  }
}

char *switch_handle_type_to_string(switch_handle_type_t handle_type) {
  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      return "none";
    case SWITCH_HANDLE_TYPE_PORT:
      return "port";
    case SWITCH_HANDLE_TYPE_LAG:
      return "lag";
    case SWITCH_HANDLE_TYPE_LAG_MEMBER:
      return "lag member";
    case SWITCH_HANDLE_TYPE_INTERFACE:
      return "interface";
    case SWITCH_HANDLE_TYPE_VRF:
      return "vrf";
    case SWITCH_HANDLE_TYPE_BD:
      return "bd";
    case SWITCH_HANDLE_TYPE_NHOP:
      return "nexthop";
    case SWITCH_HANDLE_TYPE_NEIGHBOR:
      return "neighbor";
    case SWITCH_HANDLE_TYPE_RMAC:
      return "rmac";
    case SWITCH_HANDLE_TYPE_VLAN:
      return "vlan";
    case SWITCH_HANDLE_TYPE_STP:
      return "stp";
    case SWITCH_HANDLE_TYPE_MGID:
      return "mgid";
    case SWITCH_HANDLE_TYPE_ACL:
      return "acl";
    case SWITCH_HANDLE_TYPE_MGID_ECMP:
      return "mgid ecmp";
    case SWITCH_HANDLE_TYPE_URPF:
      return "urpf";
    case SWITCH_HANDLE_TYPE_HOSTIF_GROUP:
      return "hostif group";
    case SWITCH_HANDLE_TYPE_HOSTIF:
      return "hostif";
    case SWITCH_HANDLE_TYPE_ACE:
      return "ace";
    case SWITCH_HANDLE_TYPE_MIRROR:
      return "mirror";
    case SWITCH_HANDLE_TYPE_METER:
      return "meter";
    case SWITCH_HANDLE_TYPE_SFLOW:
      return "sflow";
    case SWITCH_HANDLE_TYPE_SFLOW_ACE:
      return "sflow ace";
    case SWITCH_HANDLE_TYPE_ACL_COUNTER:
      return "acl counter";
    case SWITCH_HANDLE_TYPE_RACL_COUNTER:
      return "racl counter";
    case SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER:
      return "egress_acl counter";
    case SWITCH_HANDLE_TYPE_QOS_MAP:
      return "qos map";
    case SWITCH_HANDLE_TYPE_PRIORITY_GROUP:
      return "priority group";
    case SWITCH_HANDLE_TYPE_QUEUE:
      return "queue";
    case SWITCH_HANDLE_TYPE_SCHEDULER:
      return "scheduler";
    case SWITCH_HANDLE_TYPE_BUFFER_POOL:
      return "buffer pool";
    case SWITCH_HANDLE_TYPE_BUFFER_PROFILE:
      return "buffer profile";
    case SWITCH_HANDLE_TYPE_LABEL:
      return "label";
    case SWITCH_HANDLE_TYPE_BD_MEMBER:
      return "bd member";
    case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:
      return "logical network";
    case SWITCH_HANDLE_TYPE_BFD:
      return "bfd";
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER:
      return "tunnel mapper";
    case SWITCH_HANDLE_TYPE_HASH:
      return "hash";
    case SWITCH_HANDLE_TYPE_WRED:
      return "wred";
    case SWITCH_HANDLE_TYPE_RANGE:
      return "range";
    case SWITCH_HANDLE_TYPE_ECMP_MEMBER:
      return "ecmp member";
    case SWITCH_HANDLE_TYPE_STP_PORT:
      return "stp port";
    case SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE:
      return "hostif reason code";
    case SWITCH_HANDLE_TYPE_RPF_GROUP:
      return "mrpf";
    case SWITCH_HANDLE_TYPE_RIF:
      return "rif";
    case SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER:
      return "hostif rx filter";
    case SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER:
      return "hostif tx filter";
    case SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER:
      return "pktdriver rx filter";
    case SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER:
      return "pktdriver tx filter";
    default:
      return "invalid";
  }
}

char *switch_direction_to_string(switch_direction_t direction) {
  switch (direction) {
    case SWITCH_API_DIRECTION_INGRESS:
      return "ingress";
    case SWITCH_API_DIRECTION_EGRESS:
      return "egress";
    case SWITCH_API_DIRECTION_BOTH:
      return "ingress/egress";
    default:
      return "unknown direction";
  }
}

bool switch_l3_host_entry(const switch_ip_addr_t *ip_addr) {
  SWITCH_ASSERT(ip_addr != NULL);

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    return ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH_IN_BITS ? TRUE
                                                                    : FALSE;
  } else {
    return ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS ? TRUE
                                                                    : FALSE;
  }
}

switch_status_t switch_ipv4_to_string(switch_ip4_t ip4,
                                      char *buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t *length) {
  SWITCH_ASSERT(buffer != NULL);

  char tmp_buffer[SWITCH_LOG_BUFFER_SIZE];
  inet_ntop(AF_INET, &ip4, tmp_buffer, SWITCH_LOG_BUFFER_SIZE);
  *length = (switch_int32_t)strlen(tmp_buffer);
  SWITCH_MEMCPY(buffer, tmp_buffer, *length);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_ipv6_to_string(switch_ip6_t ip6,
                                      char *buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t *length) {
  SWITCH_ASSERT(buffer != NULL);

  char tmp_buffer[SWITCH_LOG_BUFFER_SIZE];
  inet_ntop(AF_INET6, &ip6, tmp_buffer, SWITCH_LOG_BUFFER_SIZE);
  *length = (switch_int32_t)strlen(tmp_buffer);
  SWITCH_MEMCPY(buffer, tmp_buffer, *length);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_mac_to_string(switch_mac_addr_t *mac,
                                     char *buffer,
                                     switch_int32_t buffer_size,
                                     switch_int32_t *length_out) {
  SWITCH_ASSERT(buffer != NULL);
  SWITCH_ASSERT(mac != NULL);

  switch_int32_t length = snprintf(buffer,
                                   buffer_size,
                                   "%02x:%02x:%02x:%02x:%02x:%02x",
                                   mac->mac_addr[0],
                                   mac->mac_addr[1],
                                   mac->mac_addr[2],
                                   mac->mac_addr[3],
                                   mac->mac_addr[4],
                                   mac->mac_addr[5]);
  if (length_out) {
    *length_out = length;
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_ipv4_prefix_to_mask(switch_uint32_t prefix,
                                           switch_uint32_t *mask) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!mask) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *mask = 0;
  if (prefix) {
    *mask = (0xFFFFFFFF << (SWITCH_IPV4_PREFIX_LENGTH_IN_BITS - prefix));
    *mask = *mask & 0xFFFFFFFF;
  }
  return status;
}

switch_status_t switch_ipv6_prefix_to_mask(switch_uint32_t prefix,
                                           switch_uint8_t *mask) {
  switch_uint32_t prefix_bytes = 0;
  switch_uint32_t index = 0;

  SWITCH_MEMSET(mask, 0xFF, SWITCH_IPV6_PREFIX_LENGTH);

  if (prefix == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS) {
    return SWITCH_STATUS_SUCCESS;
  }

  prefix_bytes = prefix / SWITCH_IPV6_PREFIX_LENGTH;
  for (index = 0; index < prefix_bytes; index++) {
    mask[index] = 0xFF;
  }

  mask[index] = (0xFF << (SWITCH_IPV6_PREFIX_LENGTH_IN_BITS - prefix));
  mask[index] = mask[index] & 0xFF;
  return SWITCH_STATUS_SUCCESS;
}

switch_direction_t switch_table_id_to_direction(switch_table_id_t table_id) {
  switch (table_id) {
    case SWITCH_TABLE_NONE:
      return 0;

    case SWITCH_TABLE_INGRESS_PORT_MAPPING:
    case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
    case SWITCH_TABLE_OUTER_RMAC:
    case SWITCH_TABLE_INNER_RMAC:
    case SWITCH_TABLE_SMAC:
    case SWITCH_TABLE_DMAC:
    case SWITCH_TABLE_IPV4_HOST:
    case SWITCH_TABLE_IPV6_HOST:
    case SWITCH_TABLE_IPV4_LPM:
    case SWITCH_TABLE_IPV6_LPM:
    case SWITCH_TABLE_URPF:
    case SWITCH_TABLE_NHOP:
    case SWITCH_TABLE_ECMP_GROUP:
    case SWITCH_TABLE_ECMP_SELECT:
    case SWITCH_TABLE_IPV4_SRC_VTEP:
    case SWITCH_TABLE_IPV4_DST_VTEP:
    case SWITCH_TABLE_IPV6_SRC_VTEP:
    case SWITCH_TABLE_IPV6_DST_VTEP:
    case SWITCH_TABLE_TUNNEL:
    case SWITCH_TABLE_TUNNEL_MPLS:
    case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
    case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
    case SWITCH_TABLE_BD:
    case SWITCH_TABLE_BD_FLOOD:
    case SWITCH_TABLE_INGRESS_BD_STATS:
    case SWITCH_TABLE_IPV4_ACL:
    case SWITCH_TABLE_IPV6_ACL:
    case SWITCH_TABLE_IPV4_RACL:
    case SWITCH_TABLE_IPV6_RACL:
    case SWITCH_TABLE_SYSTEM_ACL:
    case SWITCH_TABLE_MAC_ACL:
    case SWITCH_TABLE_ACL_STATS:
    case SWITCH_TABLE_RACL_STATS:
    case SWITCH_TABLE_OUTER_MCAST_STAR_G:
    case SWITCH_TABLE_OUTER_MCAST_SG:
    case SWITCH_TABLE_OUTER_MCAST_RPF:
    case SWITCH_TABLE_MCAST_RPF:
    case SWITCH_TABLE_IPV4_MCAST_S_G:
    case SWITCH_TABLE_IPV4_MCAST_STAR_G:
    case SWITCH_TABLE_IPV6_MCAST_S_G:
    case SWITCH_TABLE_IPV6_MCAST_STAR_G:
    case SWITCH_TABLE_STP:
    case SWITCH_TABLE_LAG_GROUP:
    case SWITCH_TABLE_LAG_SELECT:
    case SWITCH_TABLE_MIRROR:
    case SWITCH_TABLE_METER_INDEX:
    case SWITCH_TABLE_METER_ACTION:
    case SWITCH_TABLE_DROP_STATS:
      return SWITCH_API_DIRECTION_INGRESS;

    case SWITCH_TABLE_EGRESS_PORT_MAPPING:
    case SWITCH_TABLE_EGRESS_BD_STATS:
    case SWITCH_TABLE_SMAC_REWRITE:
    case SWITCH_TABLE_MTU:
    case SWITCH_TABLE_REWRITE:
    case SWITCH_TABLE_TUNNEL_REWRITE:
    case SWITCH_TABLE_TUNNEL_DECAP:
    case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
    case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
    case SWITCH_TABLE_TUNNEL_DIP_REWRITE:
    case SWITCH_TABLE_VLAN_DECAP:
    case SWITCH_TABLE_VLAN_XLATE:
    case SWITCH_TABLE_EGRESS_BD:
    case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
    case SWITCH_TABLE_EGRESS_ACL_STATS:
    case SWITCH_TABLE_RID:
    case SWITCH_TABLE_REPLICA_TYPE:
      return SWITCH_API_DIRECTION_EGRESS;

    default:
      return 0;
  }
}

char *switch_api_type_to_string(switch_api_type_t api_type) {
  switch (api_type) {
    case SWITCH_API_TYPE_PORT:
      return "port";
    case SWITCH_API_TYPE_L2:
      return "l2";
    case SWITCH_API_TYPE_BD:
      return "bd";
    case SWITCH_API_TYPE_VRF:
      return "vrf";
    case SWITCH_API_TYPE_L3:
      return "l3";
    case SWITCH_API_TYPE_RMAC:
      return "rmac";
    case SWITCH_API_TYPE_INTERFACE:
      return "intf";
    case SWITCH_API_TYPE_LAG:
      return "lag";
    case SWITCH_API_TYPE_NHOP:
      return "nhop";
    case SWITCH_API_TYPE_NEIGHBOR:
      return "neighbor";
    case SWITCH_API_TYPE_TUNNEL:
      return "tunnel";
    case SWITCH_API_TYPE_MCAST:
      return "mcast";
    case SWITCH_API_TYPE_ACL:
      return "acl";
    case SWITCH_API_TYPE_MIRROR:
      return "mirror";
    case SWITCH_API_TYPE_METER:
      return "meter";
    case SWITCH_API_TYPE_SFLOW:
      return "sflow";
    case SWITCH_API_TYPE_HOSTIF:
      return "hostif";
    case SWITCH_API_TYPE_STP:
      return "stp";
    case SWITCH_API_TYPE_VLAN:
      return "vlan";
    case SWITCH_API_TYPE_QOS:
      return "qos";
    case SWITCH_API_TYPE_QUEUE:
      return "queue";
    case SWITCH_API_TYPE_LOGICAL_NETWORK:
      return "ln";
    case SWITCH_API_TYPE_NAT:
      return "nat";
    case SWITCH_API_TYPE_BUFFER:
      return "buffer";
    case SWITCH_API_TYPE_BFD:
      return "bfd";
    case SWITCH_API_TYPE_WRED:
      return "wred";
    case SWITCH_API_TYPE_HASH:
      return "hash";
    case SWITCH_API_TYPE_ILA:
      return "ila";
    case SWITCH_API_TYPE_FAILOVER:
      return "failover";
    case SWITCH_API_TYPE_LABEL:
      return "label";
    case SWITCH_API_TYPE_RPF:
      return "rpf";
    case SWITCH_API_TYPE_DTEL:
      return "dtel";
    case SWITCH_API_TYPE_DEVICE:
      return "device";
    case SWITCH_API_TYPE_PACKET_DRIVER:
      return "pktdriver";
    case SWITCH_API_TYPE_SCHEDULER:
      return "scheduler";
    default:
      return "unknown";
  }
}

char *switch_packet_type_to_string(switch_packet_type_t packet_type) {
  switch (packet_type) {
    case SWITCH_PACKET_TYPE_UNICAST:
      return "unicast";
    case SWITCH_PACKET_TYPE_MULTICAST:
      return "multicast";
    case SWITCH_PACKET_TYPE_BROADCAST:
      return "broadcast";
    default:
      return "unknown";
  }
}

switch_status_t switch_api_hashtable_dump(const switch_device_t device,
                                          const switch_hashtable_type_t type,
                                          void *cli_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_PRINT(
      cli_ctx, "\n\tHashtable: %s\n", switch_hashtable_type_to_string(type));

  switch (type) {
    case SWITCH_HASHTABLE_TYPE_MAC:
      status = switch_l2_hashtable_dump(device, type, cli_ctx);
      break;
    case SWITCH_HASHTABLE_TYPE_ROUTE:
      status = switch_l3_hashtable_dump(device, type, cli_ctx);
      break;
    case SWITCH_HASHTABLE_TYPE_TUNNEL_INGRESS_VNI:
    case SWITCH_HASHTABLE_TYPE_TUNNEL_EGRESS_VNI:
    case SWITCH_HASHTABLE_TYPE_TUNNEL_SRC_IP:
    case SWITCH_HASHTABLE_TYPE_TUNNEL_DST_IP:
      break;
    default:
      break;
  }

  return status;
}

#ifdef __cplusplus
}
#endif
