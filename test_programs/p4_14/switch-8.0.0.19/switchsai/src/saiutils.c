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

#include "saiinternal.h"

// maps from SAI types to switchapi types

char *sai_status_to_string(_In_ const sai_status_t status) {
  switch (status) {
    case SWITCH_STATUS_SUCCESS:
      return "success";
    case SAI_STATUS_INVALID_PARAMETER:
      return "invalid parameter";
    case SAI_STATUS_NO_MEMORY:
      return "no memory";
    case SAI_STATUS_ITEM_ALREADY_EXISTS:
      return "item already exists";
    case SAI_STATUS_ITEM_NOT_FOUND:
      return "item not found";
    case SAI_STATUS_TABLE_FULL:
      return "table full";
    case SAI_STATUS_NOT_SUPPORTED:
      return "not supported";
    case SAI_STATUS_FAILURE:
      return "unknown failure";
    default:
      return "unknown failure";
  }
}

char *sai_object_type_to_string(_In_ sai_object_type_t object_type) {
  if (object_type > SAI_OBJECT_TYPE_MAX) {
    return "invalid object";
  }

  switch (object_type) {
    case SAI_OBJECT_TYPE_NULL:
      return "null object";
    case SAI_OBJECT_TYPE_PORT:
      return "port object";
    case SAI_OBJECT_TYPE_LAG:
      return "lag object";
    case SAI_OBJECT_TYPE_VIRTUAL_ROUTER:
      return "virtual router object";
    case SAI_OBJECT_TYPE_NEXT_HOP:
      return "nexthop object";
    case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
      return "nexthop group object";
    case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
      return "router interface object";
    case SAI_OBJECT_TYPE_ACL_TABLE:
      return "acl table object";
    case SAI_OBJECT_TYPE_ACL_ENTRY:
      return "acl entry object";
    case SAI_OBJECT_TYPE_ACL_COUNTER:
      return "acl counter object";
    case SAI_OBJECT_TYPE_HOSTIF:
      return "host interface object";
    case SAI_OBJECT_TYPE_MIRROR_SESSION:
      return "mirror object";
    case SAI_OBJECT_TYPE_SAMPLEPACKET:
      return "sample packet object";
    case SAI_OBJECT_TYPE_STP:
      return "stp instance object";
    case SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP:
      return "trap group object";
    case SAI_OBJECT_TYPE_ACL_TABLE_GROUP:
      return "acl table group object";
    case SAI_OBJECT_TYPE_POLICER:
      return "policer object";
    case SAI_OBJECT_TYPE_WRED:
      return "wred object";
    case SAI_OBJECT_TYPE_QOS_MAP:
      return "qos maps object";
    case SAI_OBJECT_TYPE_QUEUE:
      return "queue object";
    case SAI_OBJECT_TYPE_SCHEDULER:
      return "scheduler object";
    case SAI_OBJECT_TYPE_SWITCH:
      return "switch object";
    case SAI_OBJECT_TYPE_SCHEDULER_GROUP:
      return "scheduler group object";
    case SAI_OBJECT_TYPE_BRIDGE:
      return "SAI_OBJECT_TYPE_BRIDGE";
    case SAI_OBJECT_TYPE_VLAN:
      return "SAI_OBJECT_TYPE_VLAN";
    case SAI_OBJECT_TYPE_VLAN_MEMBER:
      return "SAI_OBJECT_TYPE_VLAN_MEMBER";
    case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
      return "ingress priority group object";
    case SAI_OBJECT_TYPE_DTEL:
      return "DTEL";
    case SAI_OBJECT_TYPE_DTEL_QUEUE_REPORT:
      return "DTEL queue report";
    case SAI_OBJECT_TYPE_DTEL_INT_SESSION:
      return "DTEL INT session";
    case SAI_OBJECT_TYPE_DTEL_REPORT_SESSION:
      return "DTEL report session";
    case SAI_OBJECT_TYPE_DTEL_EVENT:
      return "DTEL event";
    default:
      return "invalid object";
  }
}

sai_status_t sai_ipv4_prefix_length(_In_ sai_ip4_t ip4,
                                    _Out_ uint32_t *prefix_length) {
  int x = 0;
  *prefix_length = 0;
  while (ip4) {
    x = ip4 & 0x1;
    if (x) (*prefix_length)++;
    ip4 = ip4 >> 1;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipv6_prefix_length(_In_ const sai_ip6_t ip6,
                                    _Out_ uint32_t *prefix_length) {
  int i = 0, x = 0;
  sai_ip6_t ip6_temp;
  memcpy(ip6_temp, ip6, 16);
  *prefix_length = 0;
  for (i = 0; i < 16; i++) {
    if (ip6_temp[i] == 0xFF) {
      *prefix_length += 8;
    } else {
      while (ip6_temp[i]) {
        x = ip6_temp[i] & 0x1;
        if (x) (*prefix_length)++;
        ip6_temp[i] = ip6_temp[i] >> 1;
      }
    }
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ip_prefix_to_switch_ip_addr(
    const _In_ sai_ip_prefix_t *sai_ip_addr, _Out_ switch_ip_addr_t *ip_addr) {
  if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    ip_addr->type = SWITCH_API_IP_ADDR_V4;
    ip_addr->ip.v4addr = ntohl(sai_ip_addr->addr.ip4);
    sai_ipv4_prefix_length(ntohl(sai_ip_addr->mask.ip4), &ip_addr->prefix_len);
  } else if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    ip_addr->type = SWITCH_API_IP_ADDR_V6;
    memcpy(&ip_addr->ip.v6addr, sai_ip_addr->addr.ip6, 16);
    sai_ipv6_prefix_length(sai_ip_addr->mask.ip6, &ip_addr->prefix_len);
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ip_addr_to_switch_ip_addr(
    const _In_ sai_ip_address_t *sai_ip_addr, _Out_ switch_ip_addr_t *ip_addr) {
  if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    ip_addr->type = SWITCH_API_IP_ADDR_V4;
    ip_addr->ip.v4addr = ntohl(sai_ip_addr->addr.ip4);
    ip_addr->prefix_len = 32;
  } else if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    ip_addr->type = SWITCH_API_IP_ADDR_V6;
    memcpy(&ip_addr->ip.v6addr, sai_ip_addr->addr.ip6, 16);
    ip_addr->prefix_len = 128;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipv4_to_string(_In_ sai_ip4_t ip4,
                                _In_ uint32_t max_length,
                                _Out_ char *entry_string,
                                _Out_ int *entry_length) {
  inet_ntop(AF_INET, &ip4, entry_string, max_length);
  *entry_length = (int)strlen(entry_string);
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipv6_to_string(_In_ sai_ip6_t ip6,
                                _In_ uint32_t max_length,
                                _Out_ char *entry_string,
                                _Out_ int *entry_length) {
  inet_ntop(AF_INET6, &ip6, entry_string, max_length);
  *entry_length = (int)strlen(entry_string);
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipaddress_to_string(_In_ sai_ip_address_t ip_addr,
                                     _In_ uint32_t max_length,
                                     _Out_ char *entry_string,
                                     _Out_ int *entry_length) {
  if (ip_addr.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    sai_ipv4_to_string(
        ip_addr.addr.ip4, max_length, entry_string, entry_length);
  } else if (ip_addr.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    sai_ipv6_to_string(
        ip_addr.addr.ip6, max_length, entry_string, entry_length);
  } else {
    snprintf(entry_string,
             max_length,
             "Invalid addr family %d",
             ip_addr.addr_family);
    return SAI_STATUS_INVALID_PARAMETER;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipprefix_to_string(_In_ sai_ip_prefix_t ip_prefix,
                                    _In_ uint32_t max_length,
                                    _Out_ char *entry_string,
                                    _Out_ int *entry_length) {
  int len = 0;
  uint32_t pos = 0;

  if (ip_prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    sai_ipv4_to_string(ip_prefix.addr.ip4, max_length, entry_string, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    pos += snprintf(entry_string + pos, max_length - pos, "/");
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    sai_ipv4_to_string(
        ip_prefix.mask.ip4, max_length - pos, entry_string + pos, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
  } else if (ip_prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    sai_ipv6_to_string(ip_prefix.addr.ip6, max_length, entry_string, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    pos += snprintf(entry_string + pos, max_length - pos, "/");
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    sai_ipv6_to_string(
        ip_prefix.mask.ip6, max_length - pos, entry_string + pos, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
  } else {
    snprintf(entry_string,
             max_length,
             "Invalid addr family %d",
             ip_prefix.addr_family);
    return SAI_STATUS_INVALID_PARAMETER;
  }

  *entry_length = pos;
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_speed_to_switch_port_speed(
    uint32_t sai_port_speed, _Out_ switch_port_speed_t *switch_port_speed) {
  // speeds are in mbps
  switch (sai_port_speed) {
    case 10000:
      *switch_port_speed = SWITCH_PORT_SPEED_10G;
      break;
    case 25000:
      *switch_port_speed = SWITCH_PORT_SPEED_25G;
      break;
    case 40000:
      *switch_port_speed = SWITCH_PORT_SPEED_40G;
      break;
    case 50000:
      *switch_port_speed = SWITCH_PORT_SPEED_50G;
      break;
    case 100000:
      *switch_port_speed = SWITCH_PORT_SPEED_100G;
      break;
    default:
      return SAI_STATUS_INVALID_PARAMETER;
  }

  return SAI_STATUS_SUCCESS;
}

switch_acl_action_t sai_packet_action_to_switch_packet_action(
    _In_ sai_packet_action_t action) {
  switch (action) {
    case SAI_PACKET_ACTION_DROP:
      return SWITCH_ACL_ACTION_DROP;
    case SAI_PACKET_ACTION_FORWARD:
      return SWITCH_ACL_ACTION_PERMIT;
    case SAI_PACKET_ACTION_TRAP:
      return SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
    case SAI_PACKET_ACTION_COPY:
      return SWITCH_ACL_ACTION_COPY_TO_CPU;
    case SAI_PACKET_ACTION_LOG:
      return SWITCH_ACL_ACTION_LOG;
    default:
      return SWITCH_ACL_ACTION_NOP;
  }
}

sai_packet_action_t switch_packet_action_to_sai_packet_action(
    switch_acl_action_t acl_action) {
  switch (acl_action) {
    case SWITCH_ACL_ACTION_DROP:
      return SAI_PACKET_ACTION_DROP;
    case SWITCH_ACL_ACTION_PERMIT:
      return SAI_PACKET_ACTION_FORWARD;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
      return SAI_PACKET_ACTION_TRAP;
    case SWITCH_ACL_ACTION_LOG:
      return SAI_PACKET_ACTION_LOG;
    case SWITCH_ACL_ACTION_COPY_TO_CPU:
      return SAI_PACKET_ACTION_COPY;
    default:
      return SAI_PACKET_ACTION_FORWARD;
  }
}

// maps from switchapi types to SAI types

sai_status_t sai_switch_ip_addr_to_sai_ip_addr(
    _Out_ sai_ip_address_t *sai_ip_addr, const _In_ switch_ip_addr_t *ip_addr) {
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    sai_ip_addr->addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    sai_ip_addr->addr.ip4 = htonl(ip_addr->ip.v4addr);
  } else if (ip_addr->type == SWITCH_API_IP_ADDR_V6) {
    sai_ip_addr->addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(sai_ip_addr->addr.ip6, ip_addr->ip.v6addr.u.addr8, 16);
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_port_enabled_to_sai_oper_status(
    _In_ _Out_ sai_attribute_t *attr) {
  switch ((int)attr->value.booldata) {
    case 1:
      attr->value.u8 = SAI_PORT_OPER_STATUS_UP;
      break;
    case 0:
      attr->value.u8 = SAI_PORT_OPER_STATUS_DOWN;
      break;
  }

  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_status_to_sai_status(_In_ const switch_status_t
                                                 status) {
  switch (status) {
    case SWITCH_STATUS_SUCCESS:
      return SAI_STATUS_SUCCESS;
    case SWITCH_STATUS_FAILURE:
      return SWITCH_STATUS_FAILURE;
    case SWITCH_STATUS_INVALID_PARAMETER:
      return SAI_STATUS_INVALID_PARAMETER;
    case SWITCH_STATUS_NO_MEMORY:
      return SAI_STATUS_NO_MEMORY;
    case SWITCH_STATUS_ITEM_ALREADY_EXISTS:
      return SAI_STATUS_ITEM_ALREADY_EXISTS;
    case SWITCH_STATUS_ITEM_NOT_FOUND:
      return SAI_STATUS_ITEM_NOT_FOUND;
    case SWITCH_STATUS_TABLE_FULL:
      return SAI_STATUS_TABLE_FULL;
    case SWITCH_STATUS_NOT_SUPPORTED:
      return SAI_STATUS_NOT_SUPPORTED;
    default:
      return SAI_STATUS_FAILURE;
  }
}

const sai_attribute_t *get_attr_from_list(_In_ sai_attr_id_t attr_id,
                                          _In_ const sai_attribute_t *attr_list,
                                          _In_ uint32_t attr_count) {
  if (attr_list == NULL || attr_count == 0) {
    return NULL;
  }

  for (unsigned int index = 0; index < attr_count; index++) {
    if (attr_list[index].id == attr_id) {
      return &attr_list[index];
    }
  }

  return NULL;
}

sai_object_id_t sai_id_to_oid(_In_ uint32_t type, _In_ uint32_t id) {
  return (type << SWITCH_HANDLE_TYPE_SHIFT) | id;
}

sai_uint32_t sai_oid_to_id(_In_ sai_object_id_t oid) {
  return oid & ((1 << SWITCH_HANDLE_TYPE_SHIFT) - 1);
}

// -----------------------------------------------------------------------------
// Hash table utils
// -----------------------------------------------------------------------------

sai_uint32_t MurmurHash(const void *key,
                        sai_uint32_t length,
                        sai_uint32_t seed) {
// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.
#define m 0x5bd1e995
#define r 24

  // Initialize the hash to a 'random' value
  sai_uint32_t h = seed ^ (sai_uint32_t)length;

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

sai_status_t sai_hashtable_init(sai_hashtable_t *hashtable) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(hashtable != NULL);
  SAI_ASSERT(hashtable->size != 0);

  if (!hashtable || hashtable->size == 0) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("hashtable init failed(%s)\n", sai_status_to_string(status));
    return status;
  }
  tommy_hashtable_init(&hashtable->table, hashtable->size);
  hashtable->num_entries = 0;
  return SAI_STATUS_SUCCESS;
}

sai_size_t sai_hashtable_count(sai_hashtable_t *hashtable) {
  SAI_ASSERT(hashtable != NULL);
  if (!hashtable) {
    return 0;
  }
  return hashtable->num_entries;
}

sai_status_t sai_hashtable_insert(sai_hashtable_t *hashtable,
                                  sai_hashnode_t *node,
                                  void *key,
                                  void *data) {
  sai_uint8_t hash_key[SAI_LOG_BUFFER_SIZE];
  sai_uint32_t hash_length = 0;
  sai_uint32_t hash = 0;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(hashtable != NULL);
  SAI_ASSERT(node != NULL);
  SAI_ASSERT(key != NULL);
  SAI_ASSERT(data != NULL);

  if (!hashtable || !node || !key || !data) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("hashtable insert failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  SAI_MEMSET(hash_key, 0x0, SAI_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("hashtable insert failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  tommy_hashtable_insert(&hashtable->table, node, data, hash);
  hashtable->num_entries++;
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_hashtable_delete(sai_hashtable_t *hashtable,
                                  void *key,
                                  void **data) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_uint8_t hash_key[SAI_LOG_BUFFER_SIZE];
  sai_uint32_t hash_length = 0;
  sai_uint32_t hash = 0;

  SAI_ASSERT(hashtable != NULL);
  SAI_ASSERT(key != NULL);
  SAI_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("hashtable delete failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  SAI_MEMSET(hash_key, 0x0, SAI_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("hashtable delete failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void *)tommy_hashtable_remove(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("hashtable delete failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  hashtable->num_entries--;
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_hashtable_search(sai_hashtable_t *hashtable,
                                  void *key,
                                  void **data) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_uint8_t hash_key[SAI_LOG_BUFFER_SIZE];
  sai_uint32_t hash_length = 0;
  sai_uint32_t hash = 0;

  SAI_ASSERT(hashtable != NULL);
  SAI_ASSERT(key != NULL);
  SAI_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("hashtable search failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  SAI_MEMSET(hash_key, 0x0, SAI_LOG_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("hashtable search failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void *)tommy_hashtable_search(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("hashtable search failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  return status;
}

sai_status_t sai_hashtable_done(sai_hashtable_t *hashtable) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(hashtable != NULL);

  if (!hashtable) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("hashtable done failed(%s)\n", sai_status_to_string(status));
    return status;
  }
  tommy_hashtable_done(&hashtable->table);
  return SAI_STATUS_SUCCESS;
}

switch_uint32_t sai_acl_priority_to_switch_priority(
    sai_uint32_t sai_acl_priority) {
  if (sai_acl_priority < SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY ||
      sai_acl_priority > SWITCH_API_ACL_ENTRY_MAXIMUM_PRIORITY) {
    SAI_LOG_INFO("ACL entry priority is invalid");
  }
  // In driver, lower the priority_value, higher the priority.
  // So, convert the SAI priority value.
  if (sai_acl_priority <= SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY) {
    SAI_LOG_INFO(
        "ACL entry priority is less than min priority, return max value");
    return (SWITCH_API_ACL_ENTRY_MAXIMUM_PRIORITY +
            (SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY - sai_acl_priority));
  }
  if (sai_acl_priority >= SWITCH_API_ACL_ENTRY_MAXIMUM_PRIORITY) {
    SAI_LOG_INFO("ACL entry priority is maximum, return min value");
    return SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY;
  }
  return ((SWITCH_API_ACL_ENTRY_MAXIMUM_PRIORITY - sai_acl_priority) +
          SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY);
}
