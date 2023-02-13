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

#include <saiacl.h>
#include "saiinternal.h"
#include <switchapi/switch_handle.h>
#include <switchapi/switch_acl.h>
#include <switchapi/switch_dtel.h>

static sai_api_t api_id = SAI_API_ACL;

// declare DTEL watchlist acl table fucntions
bool is_dtel_acl(uint32_t attr_count, const sai_attribute_t *attr_list);
sai_status_t sai_create_dtel_watchlist_table(
    _Out_ sai_object_id_t *acl_table_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list);
sai_status_t sai_remove_dtel_watchlist_table(_In_ sai_object_id_t acl_table_id);
sai_status_t sai_create_dtel_watchlist_entry(
    _Out_ sai_object_id_t *acl_entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list);
sai_status_t sai_remove_dtel_watchlist_entry(_In_ sai_object_id_t acl_entry_id);
sai_status_t sai_set_dtel_watchlist_entry(_In_ sai_object_id_t acl_entry_id,
                                          _In_ const sai_attribute_t *attr);
sai_status_t sai_get_dtel_watchlist_entry(_In_ sai_object_id_t acl_entry_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list);

/*
Note: SAI ACL action processing implementation changes in the future
This is an interim solution to handling actions for the ACL in a more
static way. In a future implementation a dynamic action composiitng
scheme will allow for having multiple actions be speicifed in any
combination in response to a match
*/

typedef struct sai_handle_node_ {
  tommy_node node;
  switch_handle_t handle;
} sai_handle_node_t;

#define SAI_ACL_FIELD_NOT_SUPPORTED -1
int switch_acl[SWITCH_ACL_TYPE_MAX][SAI_ACL_TABLE_ATTR_FIELD_END -
                                    SAI_ACL_TABLE_ATTR_FIELD_START + 1];

void sai_acl_qualifiers_load() {
  switch_acl_type_t acl_type = SWITCH_ACL_TYPE_IP;
  sai_acl_table_attr_t acl_table_field = SAI_ACL_TABLE_ATTR_FIELD_START;
  int acl_attr_index = 0;

  for (acl_type = SWITCH_ACL_TYPE_IP; acl_type < SWITCH_ACL_TYPE_MAX;
       acl_type++) {
    acl_attr_index = 0;
    for (acl_table_field = SAI_ACL_TABLE_ATTR_FIELD_START;
         acl_table_field < SAI_ACL_TABLE_ATTR_FIELD_END;
         acl_table_field++, acl_attr_index++) {
      switch (acl_type) {
        case SWITCH_ACL_TYPE_IP: {
          switch (acl_table_field) {
            case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_IPV4_SRC;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_IPV4_DEST;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_IP_PROTO;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_TTL:
              switch_acl[acl_type][acl_attr_index] = SWITCH_ACL_IP_FIELD_TTL;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_IP_FLAGS;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_TCP_FLAGS;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_IP_DSCP;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_ETH_TYPE;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IP_FIELD_L4_DEST_PORT;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE:
            case SAI_ACL_TABLE_ATTR_FIELD_TC:  // Unsupported
              switch_acl[acl_type][acl_attr_index] = -2;
              break;

            default:
              switch_acl[acl_type][acl_attr_index] =
                  SAI_ACL_FIELD_NOT_SUPPORTED;
              break;
          }
        } break;

        case SWITCH_ACL_TYPE_IPV6: {
          switch (acl_table_field) {
            case SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_IPV6_SRC;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_IPV6_DEST;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_IP_PROTO;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_TTL:
              switch_acl[acl_type][acl_attr_index] = SWITCH_ACL_IPV6_FIELD_TTL;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_TCP_FLAGS;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_FLOW_LABEL;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_ETH_TYPE;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT;
              break;
            case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
              switch_acl[acl_type][acl_attr_index] = -2;
              break;

            default:
              switch_acl[acl_type][acl_attr_index] =
                  SAI_ACL_FIELD_NOT_SUPPORTED;
              break;
          }
        } break;

        case SWITCH_ACL_TYPE_MAC: {
          switch (acl_table_field) {
            case SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_MAC_FIELD_SOURCE_MAC;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_DST_MAC:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_MAC_FIELD_DEST_MAC;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI:  // Unsupported
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_MAC_FIELD_VLAN_PRI;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI:  // Unsupported
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_MAC_FIELD_VLAN_CFI;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_MAC_FIELD_ETH_TYPE;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
              switch_acl[acl_type][acl_attr_index] = -2;
              break;

            default:
              switch_acl[acl_type][acl_attr_index] =
                  SAI_ACL_FIELD_NOT_SUPPORTED;
              break;
          }
        } break;

        case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
          switch (acl_table_field) {
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:  // Unsupported
              switch_acl[acl_type][acl_attr_index] =
                  SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
              switch_acl[acl_type][acl_attr_index] = -2;
              break;

            default:
              switch_acl[acl_type][acl_attr_index] =
                  SAI_ACL_FIELD_NOT_SUPPORTED;
              break;
          }
        } break;

        case SWITCH_ACL_TYPE_ECN: {
          switch (acl_table_field) {
            case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
              switch_acl[acl_type][acl_attr_index] = SWITCH_ACL_ECN_FIELD_DSCP;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_ECN:
              switch_acl[acl_type][acl_attr_index] = SWITCH_ACL_ECN_FIELD_ECN;
              break;

            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
            case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
              switch_acl[acl_type][acl_attr_index] = -2;
              break;

            default:
              switch_acl[acl_type][acl_attr_index] =
                  SAI_ACL_FIELD_NOT_SUPPORTED;
              break;
          }
        } break;

        default:
          switch_acl[acl_type][acl_attr_index] = SAI_ACL_FIELD_NOT_SUPPORTED;
          break;
      }
    }
  }
}

static int *sai_acl_p4_match_table_get(switch_acl_type_t table_type) {
  switch (table_type) {
    case SWITCH_ACL_TYPE_IP:
    case SWITCH_ACL_TYPE_IPV6:
    case SWITCH_ACL_TYPE_MAC:
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
    case SWITCH_ACL_TYPE_ECN:
      return switch_acl[table_type];
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      return switch_acl[SWITCH_ACL_TYPE_IP];
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      return switch_acl[SWITCH_ACL_TYPE_IPV6];
    default:
      return NULL;
  }
}

/*
    Ensure that all the fields in the attribute list can be handled by the ACL
*/
static sai_status_t sai_acl_match_table_type_get(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ switch_acl_type_t *acl_type) {
  uint32_t index1 = 0, index2 = 0;
  int *table;
  uint32_t i = 0;
  bool table_matched = TRUE;
  bool mirror_acl = false;

  for (index1 = 0; index1 < SWITCH_ACL_TYPE_MAX; index1++) {
    table = sai_acl_p4_match_table_get(index1);
    if (!table) {
      continue;
    }
    table_matched = TRUE;
    SAI_LOG_INFO("ACL attrib count %d", attr_count);
    for (index2 = 0; index2 < attr_count; index2++) {
      SAI_LOG_INFO("ACL attrib %d\n", attr_list[index2].id);
      // skip ports and VLAN attributes on check
      switch (attr_list[index2].id) {
        case SAI_ACL_TABLE_ATTR_ACL_STAGE:
        case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
        case SAI_ACL_TABLE_ATTR_SIZE:
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:   // Unsupported
        case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:  // Unsupported
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:    // Unsupported
          break;
        case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
          for (i = 0; i < attr_list[index2].value.s32list.count; i++) {
            switch (attr_list[index2].value.s32list.list[i]) {
              case SAI_ACL_ACTION_TYPE_MIRROR_INGRESS:
                mirror_acl = true;
                break;
            }
          }
          break;
        // ignore above for matching fields
        default:
          if (attr_list[index2].id >= SAI_ACL_TABLE_ATTR_FIELD_START &&
              attr_list[index2].id <= SAI_ACL_TABLE_ATTR_FIELD_END) {
            if (table[attr_list[index2].id - SAI_ACL_TABLE_ATTR_FIELD_START] ==
                -1) {
              table_matched = FALSE;
            }
          }
          break;
      }
    }
    if (table_matched && index2 == attr_count) {
      *acl_type = index1;
      if (mirror_acl) {
        if (*acl_type == SWITCH_ACL_TYPE_IP) {
          *acl_type = SWITCH_ACL_TYPE_IP_MIRROR_ACL;
        } else if (*acl_type == SWITCH_ACL_TYPE_IPV6) {
          *acl_type = SWITCH_ACL_TYPE_IPV6_MIRROR_ACL;
        }
      }
      return SAI_STATUS_SUCCESS;
    }
  }
  return SAI_STATUS_FAILURE;
}

static sai_status_t sai_acl_match_table_field(
    _In_ int table_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _Out_ int *match_fields,
    _Out_ int *actions) {
  uint32_t index = 0;
  int id = 0;
  int *table;

  table = sai_acl_p4_match_table_get(table_id);
  if (!table) {
    return SAI_STATUS_FAILURE;
  }

  for (index = 0; index < attr_count; index++) {
    id = attr_list[index].id;
    if ((id >= SAI_ACL_TABLE_ATTR_FIELD_START) &&
        (id <= SAI_ACL_TABLE_ATTR_FIELD_END)) {
      id -= SAI_ACL_TABLE_ATTR_FIELD_START;
      if (table[id] != -1) {
        match_fields[index] = table[id];
      } else {
        return SAI_STATUS_FAILURE;
      }
    }
  }
  return SAI_STATUS_SUCCESS;
}

static int switch_acl_ip_mirror_field_type(switch_acl_type_t acl_type,
                                           int field) {
  if ((acl_type == SWITCH_ACL_TYPE_IP) ||
      (acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)) {
    switch (field) {
      case SWITCH_ACL_IP_FIELD_IPV4_SRC:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_SRC
                    : field);
      case SWITCH_ACL_IP_FIELD_IPV4_DEST:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_DEST
                    : field);
      case SWITCH_ACL_IP_FIELD_IP_PROTO:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_PROTO
                    : field);
      case SWITCH_ACL_IP_FIELD_ICMP_TYPE:
      case SWITCH_ACL_IP_FIELD_ICMP_CODE:
      case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_TCP_FLAGS
                    : field);
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE
                    : field);
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE
                    : field);
      case SWITCH_ACL_IP_FIELD_IP_DSCP:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_DSCP
                    : field);
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT
                    : field);
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT
                    : field);
      case SWITCH_ACL_IP_FIELD_ETH_TYPE:
        return ((acl_type == SWITCH_ACL_TYPE_IP_MIRROR_ACL)
                    ? SWITCH_ACL_IP_MIRROR_ACL_FIELD_ETH_TYPE
                    : field);
      default:
        return field;
    }
  } else {
    switch (field) {
      case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_SRC
                    : field);
      case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_DEST
                    : field);
      case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IP_PROTO
                    : field);
      case SWITCH_ACL_IPV6_FIELD_ICMP_TYPE:
      case SWITCH_ACL_IPV6_FIELD_ICMP_CODE:
      case SWITCH_ACL_IPV6_FIELD_TCP_FLAGS:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_TCP_FLAGS
                    : field);
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE
                    : field);
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE
                    : field);
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT
                    : field);
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT
                    : field);
      case SWITCH_ACL_IPV6_FIELD_ETH_TYPE:
        return ((acl_type == SWITCH_ACL_TYPE_IPV6_MIRROR_ACL)
                    ? SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_ETH_TYPE
                    : field);
      default:
        return field;
    }
  }
}

static sai_status_t sai_acl_xform_field_value(
    _In_ switch_acl_type_t acl_type,
    _In_ switch_direction_t direction,
    _In_ int field,
    _In_ void *dest,
    _In_ const sai_acl_field_data_t *source) {
  sai_object_id_t *objlist = NULL;
  uint32_t index = 0;
  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP:
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL: {
      switch_acl_ip_key_value_pair_t *kvp =
          (switch_acl_ip_key_value_pair_t *)dest;
      kvp->field = switch_acl_ip_mirror_field_type(acl_type, field);
      switch (field) {
        case SWITCH_ACL_IP_FIELD_IPV4_SRC:
          kvp->value.ipv4_source = ntohl(source->data.ip4);
          kvp->mask.u.mask = ntohl(source->mask.ip4);
          break;
        case SWITCH_ACL_IP_FIELD_IPV4_DEST:
          kvp->value.ipv4_dest = ntohl(source->data.ip4);
          kvp->mask.u.mask = ntohl(source->mask.ip4);
          break;
        case SWITCH_ACL_IP_FIELD_IP_PROTO:
          kvp->value.ip_proto = source->data.u16;
          kvp->mask.u.mask = source->mask.u16;
          break;
        case SWITCH_ACL_IP_FIELD_ICMP_TYPE:
        case SWITCH_ACL_IP_FIELD_ICMP_CODE:
        case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
          kvp->value.tcp_flags = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        case SWITCH_ACL_IP_FIELD_TTL:
          kvp->value.ttl = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        case SWITCH_ACL_IP_FIELD_IP_FLAGS:
          kvp->value.ip_flags = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        case SWITCH_ACL_IP_FIELD_IP_FRAGMENT:
          kvp->value.ip_frag = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;

        case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
        case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
          objlist = source->data.objlist.list;
          for (index = 0; index < source->data.objlist.count; index++) {
            switch_handle_t range_handle = 0;
            range_handle = (switch_handle_t)objlist[index];
            switch_range_type_t range_type = SWITCH_RANGE_TYPE_NONE;
            switch_api_acl_range_type_get(device, range_handle, &range_type);
            if (range_type == SWITCH_RANGE_TYPE_SRC_PORT) {
              kvp->value.sport_range_handle = range_handle;
              if (acl_type == SWITCH_ACL_TYPE_IP) {
                kvp->field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
              } else {
                kvp->field =
                    SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE;
              }
            }
            if (range_type == SWITCH_RANGE_TYPE_DST_PORT) {
              kvp->value.dport_range_handle = range_handle;
              if (acl_type == SWITCH_ACL_TYPE_IP) {
                kvp->field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
              } else {
                kvp->field = SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE;
              }
            }
          }
          break;
        case SWITCH_ACL_IP_FIELD_IP_DSCP:
          kvp->value.dscp = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        case SWITCH_ACL_IP_FIELD_ETH_TYPE:
          kvp->value.eth_type = source->data.u16;
          kvp->mask.u.mask = source->mask.u16;
          break;
        case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        case SWITCH_ACL_IP_FIELD_L4_DEST_PORT: {
          switch_handle_t range_handle = 0;
          switch_range_t switch_range;
          switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
          sai_status_t status = SAI_STATUS_SUCCESS;
          switch_range_type_t range_type =
              (field == SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT)
                  ? SWITCH_RANGE_TYPE_SRC_PORT
                  : SWITCH_RANGE_TYPE_DST_PORT;
          switch_range.start_value = (source->data.u16 & source->mask.u16);
          switch_range.end_value = (source->data.u16 & source->mask.u16);
          switch_status = switch_api_acl_range_create(device,
                                                      SWITCH_API_DIRECTION_BOTH,
                                                      range_type,
                                                      &switch_range,
                                                      &range_handle);
          status = sai_switch_status_to_sai_status(switch_status);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("Failed to create acl range for %d: %s",
                          source->data.u16,
                          sai_status_to_string(status));
            return status;
          }
          if (range_type == SWITCH_RANGE_TYPE_SRC_PORT) {
            kvp->value.sport_range_handle = range_handle;
            if (acl_type == SWITCH_ACL_TYPE_IP) {
              kvp->field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT;
            } else {
              kvp->field = SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT;
            }
          }
          if (range_type == SWITCH_RANGE_TYPE_DST_PORT) {
            kvp->value.dport_range_handle = range_handle;
            if (acl_type == SWITCH_ACL_TYPE_IP) {
              kvp->field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT;
            } else {
              kvp->field = SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT;
            }
          }
        } break;
        default:
          break;
      }
    } break;
    case SWITCH_ACL_TYPE_IPV6:
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL: {
      switch_acl_ipv6_key_value_pair_t *kvp =
          (switch_acl_ipv6_key_value_pair_t *)dest;
      kvp->field = switch_acl_ip_mirror_field_type(acl_type, field);
      switch (field) {
        case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
          memcpy(kvp->value.ipv6_source.u.addr8, source->data.ip6, 16);
          memcpy(kvp->mask.u.mask.u.addr8, source->mask.ip6, 16);
          break;
        case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
          memcpy(kvp->value.ipv6_dest.u.addr8, source->data.ip6, 16);
          memcpy(kvp->mask.u.mask.u.addr8, source->mask.ip6, 16);
          break;
        case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
          kvp->value.ip_proto = source->data.u16;
          kvp->mask.u.mask.u.addr8[0] = source->mask.u16;
          break;
        case SWITCH_ACL_IPV6_FIELD_ICMP_TYPE:
        case SWITCH_ACL_IPV6_FIELD_ICMP_CODE:
        case SWITCH_ACL_IPV6_FIELD_TCP_FLAGS:
          kvp->value.tcp_flags = source->data.u8;
          kvp->mask.u.mask.u.addr8[0] = source->mask.u8;
          break;
        case SWITCH_ACL_IPV6_FIELD_TTL:
          kvp->value.ttl = source->data.u8;
          kvp->mask.u.mask.u.addr8[0] = source->mask.u8;
          break;
        case SWITCH_ACL_IPV6_FIELD_FLOW_LABEL:
          break;
        case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
        case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
          objlist = source->data.objlist.list;
          for (index = 0; index < source->data.objlist.count; index++) {
            switch_handle_t range_handle = 0;
            range_handle = (switch_handle_t)objlist[index];
            switch_range_type_t range_type = SWITCH_RANGE_TYPE_NONE;
            switch_api_acl_range_type_get(device, range_handle, &range_type);
            if (range_type == SWITCH_RANGE_TYPE_SRC_PORT) {
              kvp->value.sport_range_handle = range_handle;
              if (acl_type == SWITCH_ACL_TYPE_IPV6) {
                kvp->field = SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE;
              } else {
                kvp->field =
                    SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE;
              }
            }
            if (range_type == SWITCH_RANGE_TYPE_DST_PORT) {
              kvp->value.dport_range_handle = range_handle;
              if (acl_type == SWITCH_ACL_TYPE_IPV6) {
                kvp->field = SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE;
              } else {
                kvp->field =
                    SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE;
              }
            }
          }
          break;
        case SWITCH_ACL_IP_FIELD_ETH_TYPE:
          kvp->value.eth_type = source->data.u16;
          kvp->mask.u.mask16 = source->mask.u16;
          break;
        case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        case SWITCH_ACL_IP_FIELD_L4_DEST_PORT: {
          switch_handle_t range_handle = 0;
          switch_range_t switch_range;
          switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
          sai_status_t status = SAI_STATUS_SUCCESS;
          switch_range_type_t range_type =
              (field == SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT)
                  ? SWITCH_RANGE_TYPE_SRC_PORT
                  : SWITCH_RANGE_TYPE_DST_PORT;
          switch_range.start_value = (source->data.u16 & source->mask.u16);
          switch_range.end_value = (source->data.u16 & source->mask.u16);
          switch_status = switch_api_acl_range_create(device,
                                                      SWITCH_API_DIRECTION_BOTH,
                                                      range_type,
                                                      &switch_range,
                                                      &range_handle);
          status = sai_switch_status_to_sai_status(switch_status);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("Failed to create acl range for %d: %s",
                          source->data.u16,
                          sai_status_to_string(status));
            return status;
          }
          if (range_type == SWITCH_RANGE_TYPE_SRC_PORT) {
            kvp->value.sport_range_handle = range_handle;
            if (acl_type == SWITCH_ACL_TYPE_IPV6) {
              kvp->field = SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT;
            } else {
              kvp->field = SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT;
            }
          }
          if (range_type == SWITCH_RANGE_TYPE_DST_PORT) {
            kvp->value.dport_range_handle = range_handle;
            if (acl_type == SWITCH_ACL_TYPE_IPV6) {
              kvp->field = SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT;
            } else {
              kvp->field = SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT;
            }
          }
        } break;
        default:
          break;
      }
    } break;
    case SWITCH_ACL_TYPE_MAC: {
      switch_acl_mac_key_value_pair_t *kvp =
          (switch_acl_mac_key_value_pair_t *)dest;
      kvp->field = field;
      switch (field) {
        case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
          memcpy(kvp->value.source_mac.mac_addr, source->data.mac, 6);
          memcpy(&kvp->mask.u.mask, source->mask.mac, 6);
          break;
        case SWITCH_ACL_MAC_FIELD_DEST_MAC:
          memcpy(kvp->value.dest_mac.mac_addr, source->data.mac, 6);
          memcpy(&kvp->mask.u.mask, source->mask.mac, 6);
          break;
        case SWITCH_ACL_MAC_FIELD_VLAN_PRI:
          kvp->value.vlan_pri = source->data.u8;
          kvp->mask.u.mask16 = source->mask.u8;
          break;
        case SWITCH_ACL_MAC_FIELD_VLAN_CFI:
          kvp->value.vlan_cfi = source->data.u8;
          kvp->mask.u.mask16 = source->mask.u8;
          break;
        case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
          kvp->value.eth_type = source->data.u16;
          kvp->mask.u.mask16 = source->mask.u16;
          break;
        default:
          break;
      }
    } break;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM: {
      switch_acl_egress_system_key_value_pair_t *kvp =
          (switch_acl_egress_system_key_value_pair_t *)dest;
      kvp->field = field;
      switch (field) {
        case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT:
          kvp->value.egr_port = source->data.oid;
          kvp->mask.u.mask = 0xFFFF;
          break;
        default:
          break;
      }
    } break;

    case SWITCH_ACL_TYPE_ECN: {
      switch_acl_ecn_key_value_pair_t *kvp =
          (switch_acl_ecn_key_value_pair_t *)dest;
      kvp->field = field;
      switch (field) {
        case SWITCH_ACL_ECN_FIELD_DSCP:
          kvp->value.dscp = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        case SWITCH_ACL_ECN_FIELD_ECN:
          kvp->value.ecn = source->data.u8;
          kvp->mask.u.mask = source->mask.u8;
          break;
        default:
          break;
      }
    } break;
    default:
      break;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_direction_get(uint32_t attr_count,
                                   const sai_attribute_t *attr_list,
                                   switch_direction_t *direction) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t index = 0;
  *direction = SWITCH_API_DIRECTION_INGRESS;

  for (index = 0; index < attr_count; index++) {
    // skip ports and VLAN attributes on check
    switch (attr_list[index].id) {
      case SAI_ACL_TABLE_ATTR_ACL_STAGE: {
        sai_acl_stage_t acl_stage = attr_list[index].value.s32;
        switch (acl_stage) {
          case SAI_ACL_STAGE_EGRESS:
            *direction = SWITCH_API_DIRECTION_EGRESS;
            break;
          case SAI_ACL_STAGE_INGRESS:
          default:
            *direction = SWITCH_API_DIRECTION_INGRESS;
            break;
        }
      }
    }
  }

  return status;
}

/*
 * Expects one binding point, fails otherwise
 */
sai_status_t sai_acl_binding_point_get(uint32_t attr_count,
                                       const sai_attribute_t *attr_list,
                                       switch_handle_type_t *bp_type) {
  uint32_t index = 0;
  *bp_type = SWITCH_HANDLE_TYPE_NONE;

  for (index = 0; index < attr_count; index++) {
    if (attr_list[index].id == SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST) {
      if (attr_list[index].value.s32list.count != 1) {
        return SAI_STATUS_NOT_SUPPORTED;
      }

      switch (attr_list[index].value.s32list.list[0]) {
        case SAI_ACL_BIND_POINT_TYPE_PORT:
          *bp_type = SWITCH_HANDLE_TYPE_PORT;
          break;
        case SAI_ACL_BIND_POINT_TYPE_LAG:
          *bp_type = SWITCH_HANDLE_TYPE_LAG;
          break;
        case SAI_ACL_BIND_POINT_TYPE_VLAN:
          *bp_type = SWITCH_HANDLE_TYPE_BD;
          break;
        case SAI_ACL_BIND_POINT_TYPE_ROUTER_INTF:
          *bp_type = SWITCH_HANDLE_TYPE_RIF;
          break;
        default:
          return SAI_STATUS_NOT_SUPPORTED;
          break;
      }
    }
  }

  if (*bp_type == SWITCH_HANDLE_TYPE_NONE) {
    // not found..
    return SAI_STATUS_FAILURE;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_group_type_get(uint32_t attr_count,
                                    const sai_attribute_t *attr_list) {
  uint32_t index = 0;

  for (index = 0; index < attr_count; index++) {
    if (attr_list[index].id == SAI_ACL_TABLE_GROUP_ATTR_TYPE) {
      if (attr_list[index].value.u32 == SAI_ACL_TABLE_GROUP_TYPE_SEQUENTIAL) {
        return SAI_STATUS_NOT_SUPPORTED;
      }
    }
  }

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Create an ACL Group
*
* Arguments:
*  [out] acl_group_id - the the acl table id
 * [in] switch_id  Switch Object id
*  [in] attr_count - number of attributes
*  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_acl_group(_Out_ sai_object_id_t *acl_group_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_direction_t direction = SWITCH_API_DIRECTION_INGRESS;
  switch_handle_type_t bp_type = SWITCH_HANDLE_TYPE_NONE;
  switch_handle_t acl_group_handle = SWITCH_API_INVALID_HANDLE;

  *acl_group_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  status = sai_acl_direction_get(attr_count, attr_list, &direction);
  if (status != SAI_STATUS_SUCCESS) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("failed to get acl direction: %s",
                  sai_status_to_string(status));
    return status;
  }

  status = sai_acl_binding_point_get(attr_count, attr_list, &bp_type);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl binding point: %s",
                  sai_status_to_string(status));
    return status;
  }

  status = sai_acl_group_type_get(attr_count, attr_list);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("unexpected acl group type!: %s",
                  sai_status_to_string(status));
    return status;
  }
  status = switch_api_acl_list_group_create(
      device, direction, bp_type, &acl_group_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl group: %s",
                  sai_status_to_string(status));
  }

  *acl_group_id = acl_group_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*   Delete an ACL group
*
* Arguments:
*  [out] acl_group_id - the the acl table id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_acl_group(_In_ sai_object_id_t acl_group_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_group_id) ==
             SAI_OBJECT_TYPE_ACL_TABLE_GROUP);

  switch_status =
      switch_api_acl_list_group_delete(device, (switch_handle_t)acl_group_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl table 0x%lx : %s",
                  acl_group_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*   Create an ACL table
*
* Arguments:
*  [out] acl_table_id - the the acl table id
 * [in] switch_id  Switch Object id
*  [in] attr_count - number of attributes
*  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_acl_table(_Out_ sai_object_id_t *acl_table_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_acl_type_t acl_type = 0;
  switch_direction_t direction = SWITCH_API_DIRECTION_INGRESS;
  switch_handle_type_t bp_type = SWITCH_HANDLE_TYPE_NONE;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  *acl_table_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  if (is_dtel_acl(attr_count, attr_list)) {
    return sai_create_dtel_watchlist_table(
        acl_table_id, switch_id, attr_count, attr_list);
  }

  status = sai_acl_match_table_type_get(attr_count, attr_list, &acl_type);
  if (status != SAI_STATUS_SUCCESS) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("failed to find match table: %s",
                  sai_status_to_string(status));
    return status;
  }

  status = sai_acl_direction_get(attr_count, attr_list, &direction);
  if (status != SAI_STATUS_SUCCESS) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("failed to get acl direction: %s",
                  sai_status_to_string(status));
    return status;
  }

  status = sai_acl_binding_point_get(attr_count, attr_list, &bp_type);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl binding point: %s",
                  sai_status_to_string(status));
    return status;
  }

  status = switch_api_acl_list_create(
      device, direction, acl_type, bp_type, &acl_handle);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl table: %s",
                  sai_status_to_string(status));
  }
  *acl_table_id = acl_handle;

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*   Delete an ACL table
*
* Arguments:
*   [in] acl_table_id - the acl table id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_acl_table(_In_ sai_object_id_t acl_table_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_table_id) == SAI_OBJECT_TYPE_ACL_TABLE);

  if ((acl_table_id & (1 << (SWITCH_HANDLE_TYPE_SHIFT - 1))) != 0) {
    return sai_remove_dtel_watchlist_table(acl_table_id);
  }

  switch_status =
      switch_api_acl_list_delete(device, (switch_handle_t)acl_table_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl table 0x%lx : %s",
                  acl_table_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

sai_status_t sai_create_acl_group_member(
    _Out_ sai_object_id_t *acl_table_group_member_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t acl_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t acl_group_member_handle = SWITCH_API_INVALID_HANDLE;
  const sai_attribute_t *attribute = NULL;

  *acl_table_group_member_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (uint16_t index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
        acl_group_handle = attribute->value.oid;
        break;
      case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
        acl_handle = attribute->value.oid;
        break;
      case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
        break;
      default:
        SAI_LOG_ERROR(
            "failed to create acl group member: "
            "invalid attribute: (%s)",
            sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
    }
  }
  status = switch_api_acl_group_member_create(
      device, acl_group_handle, acl_handle, &acl_group_member_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl group member: %s",
                  sai_status_to_string(status));
  }

  *acl_table_group_member_id = acl_group_member_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

sai_status_t sai_remove_acl_group_member(_In_ sai_object_id_t
                                             acl_group_member_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_group_member_id) ==
             SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER);

  switch_status = switch_api_acl_group_member_delete(
      device, (switch_handle_t)acl_group_member_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl group member 0x%lx : %s",
                  acl_group_member_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

switch_color_t sai_color_to_switch_color(sai_packet_color_t sai_color) {
  switch (sai_color) {
    case SAI_PACKET_COLOR_GREEN:
      return SWITCH_COLOR_GREEN;

    case SAI_PACKET_COLOR_YELLOW:
      return SWITCH_COLOR_YELLOW;

    case SAI_PACKET_COLOR_RED:
      return SWITCH_COLOR_RED;

    default:
      SAI_LOG_ERROR("Invalid sai color");
      SAI_ASSERT(0);
      return SWITCH_COLOR_GREEN;
  }
}

switch_color_t sai_switch_color_to_sai_color(switch_color_t switch_color) {
  switch (switch_color) {
    case SWITCH_COLOR_GREEN:
      return SAI_PACKET_COLOR_GREEN;

    case SWITCH_COLOR_YELLOW:
      return SAI_PACKET_COLOR_YELLOW;

    case SWITCH_COLOR_RED:
      return SAI_PACKET_COLOR_RED;

    default:
      SAI_LOG_ERROR("Invalid sai color");
      SAI_ASSERT(0);
      return SAI_PACKET_COLOR_GREEN;
  }
}

sai_status_t sai_acl_type_update(_In_ uint32_t attr_count,
                                 _In_ const sai_attribute_t *attr_list) {
  sai_object_id_t acl_table_id = 0ULL;
  uint32_t index = 0;
  bool ingress_mirror_acl = false;
  bool update_acl_type = false;
  switch_acl_type_t acl_type = 0;
  switch_acl_type_t new_acl_type = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_ENTRY_ATTR_TABLE_ID:
        acl_table_id = attr_list[index].value.oid;
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
        ingress_mirror_acl = true;
        update_acl_type = true;

      default:
        break;
    }
  }

  if (acl_table_id == SAI_NULL_OBJECT_ID) {
    SAI_LOG_ERROR("failed to update acl type. acl table id is null\n");
    return SAI_STATUS_INVALID_PARAMETER;
  }

  status = switch_api_acl_type_get(device, acl_table_id, &acl_type);
  if (ingress_mirror_acl) {
    if (acl_type == SWITCH_ACL_TYPE_IP) {
      new_acl_type = SWITCH_ACL_TYPE_IP_MIRROR_ACL;
    } else if (acl_type == SWITCH_ACL_TYPE_IPV6) {
      new_acl_type = SWITCH_ACL_TYPE_IPV6_MIRROR_ACL;
    } else {
      return SAI_STATUS_SUCCESS;
    }
  }

  if (update_acl_type) {
    switch_status = switch_api_acl_type_set(device, acl_table_id, new_acl_type);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to update acl type.\n");
      return status;
    }
  }

  return status;
}

sai_status_t sai_acl_type_kvp_field_size(switch_acl_type_t acl_type,
                                         switch_uint32_t *field_size) {
  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP:
      *field_size = sizeof(switch_acl_ip_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_SYSTEM:
      *field_size = sizeof(switch_acl_system_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IPV6:
      *field_size = sizeof(switch_acl_ipv6_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_MAC:
      *field_size = sizeof(switch_acl_mac_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_EGRESS_SYSTEM:
      *field_size = sizeof(switch_acl_egress_system_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IP_RACL:
      *field_size = sizeof(switch_acl_ip_racl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IPV6_RACL:
      *field_size = sizeof(switch_acl_ipv6_racl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IP_MIRROR_ACL:
      *field_size = sizeof(switch_acl_ip_mirror_acl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IPV6_MIRROR_ACL:
      *field_size = sizeof(switch_acl_ipv6_mirror_acl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_MAC_QOS:
      *field_size = sizeof(switch_acl_mac_qos_acl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IP_QOS:
      *field_size = sizeof(switch_acl_ip_qos_acl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_IPV6_QOS:
      *field_size = sizeof(switch_acl_ipv6_qos_acl_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    case SWITCH_ACL_TYPE_ECN:
      *field_size = sizeof(switch_acl_ecn_key_value_pair_t);
      return SAI_STATUS_SUCCESS;
    default:
      *field_size = 0;
      return SWITCH_STATUS_INVALID_PARAMETER;
  }
}

/*
* Routine Description:
*   Create an ACL entry
*
* Arguments:
*   [out] acl_entry_id - the acl entry id
*   [in] switch_id The Switch Object id
*   [in] attr_count - number of attributes
*   [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_acl_entry(_Out_ sai_object_id_t *acl_entry_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_object_id_t acl_table_id = 0ULL;
  int *match_fields = NULL;
  void *kvp = NULL;
  unsigned int field_size;
  sai_packet_action_t packet_action = 0;
  switch_acl_action_t acl_action = 0;
  switch_acl_action_params_t action_params;
  switch_acl_opt_action_params_t opt_action_params;
  uint32_t priority = 0;
  switch_acl_type_t acl_type;
  uint32_t index1 = 0, index2 = 0;
  int *actions = NULL;
  switch_handle_t acl_entry_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t kvp_size = 0;
  *acl_entry_id = SAI_NULL_OBJECT_ID;
  switch_uint32_t switch_acl_priority = 0;
  switch_direction_t direction = SWITCH_API_DIRECTION_INGRESS;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  if (is_dtel_acl(attr_count, attr_list)) {
    return sai_create_dtel_watchlist_entry(
        acl_entry_id, switch_id, attr_count, attr_list);
  }

  status = sai_acl_type_update(attr_count, attr_list);
  SAI_ASSERT(status == SAI_STATUS_SUCCESS);

  memset(&action_params, 0, sizeof(switch_acl_action_params_t));
  memset(&opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
  // get the table id
  for (index1 = 0; index1 < attr_count; index1++) {
    switch (attr_list[index1].id) {
      case SAI_ACL_ENTRY_ATTR_TABLE_ID:
        // ACL table identifier
        acl_table_id = attr_list[index1].value.oid;
        SAI_ASSERT(sai_object_type_query(acl_table_id) ==
                   SAI_OBJECT_TYPE_ACL_TABLE);
        break;
      case SAI_ACL_ENTRY_ATTR_PRIORITY:
        // ACL entry priority
        priority = attr_list[index1].value.u32;
        break;

      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:  // Unsupported
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT: {
        // ACTION handling
        switch_handle_t handle =
            (switch_handle_t)attr_list[index1].value.aclaction.parameter.oid;
        /*
        if (SAI_CPU_PORT(port_handle)) {
            acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
        } else  {
        */
        acl_action = SWITCH_ACL_ACTION_REDIRECT;
        // set the action params
        action_params.redirect.handle = handle;
      } break;
      case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
        acl_action = 0;
        packet_action = attr_list[index1].value.aclaction.parameter.u32;
        if (packet_action == SAI_PACKET_ACTION_DROP) {
          acl_action = SWITCH_ACL_ACTION_DROP;
        } else if (packet_action == SAI_PACKET_ACTION_FORWARD) {
          acl_action = SWITCH_ACL_ACTION_PERMIT;
        }
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_FLOOD:  // Unsupported
        acl_action = SWITCH_ACL_ACTION_FLOOD_TO_VLAN;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS: {
        switch_handle_t handle =
            (switch_handle_t)attr_list[index1].value.aclaction.parameter.oid;
        acl_action = SWITCH_ACL_ACTION_SET_MIRROR;
        opt_action_params.mirror_handle = handle;
      } break;
      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS: {
        switch_handle_t handle =
            (switch_handle_t)attr_list[index1].value.aclaction.parameter.oid;
        acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_SET_MIRROR;
        opt_action_params.mirror_handle = handle;
      } break;
      case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER: {
        switch_handle_t handle =
            (switch_handle_t)attr_list[index1].value.aclaction.parameter.oid;
        opt_action_params.meter_handle = handle;
      } break;
      case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER: {
        switch_handle_t handle =
            (switch_handle_t)attr_list[index1].value.aclaction.parameter.oid;
        opt_action_params.counter_handle = handle;
      } break;
      case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
        opt_action_params.color = sai_color_to_switch_color(
            attr_list[index1].value.aclaction.parameter.s32);
        acl_action = SWITCH_ACL_ACTION_TC_AND_COLOR;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
        opt_action_params.tc = attr_list[index1].value.aclaction.parameter.u8;
        acl_action = SWITCH_ACL_ACTION_TC_AND_COLOR;
        break;
    }
  }
  switch_acl_priority = sai_acl_priority_to_switch_priority(priority);

  status =
      switch_api_acl_type_get(device, (switch_handle_t)acl_table_id, &acl_type);

  // switch on type to get more values!
  field_size = SWITCH_ACL_IP_FIELD_MAX;
  match_fields = SAI_MALLOC(sizeof(int) * field_size);
  if (!match_fields) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to create acl entry: %s",
                  sai_status_to_string(status));
    return status;
  }

  // init the array to unknown
  for (index1 = 0; index1 < field_size; index1++) {
    match_fields[index1] = -1;
  }
  actions = SAI_MALLOC(sizeof(int) * SWITCH_ACL_ACTION_MAX);
  if (!actions) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to create acl entry: %s",
                  sai_status_to_string(status));
    return status;
  }
  // get the match fields
  status = sai_acl_match_table_field(
      acl_type, attr_count, attr_list, match_fields, actions);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl entry: %s",
                  sai_status_to_string(status));
    return status;
  }
  sai_acl_type_kvp_field_size(acl_type, &kvp_size);
  // allocate to store key-value pairs
  kvp = SAI_MALLOC(kvp_size * field_size);
  SAI_MEMSET(kvp, 0, kvp_size);
  if (!kvp) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to create acl entry: %s",
                  sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_acl_direction_get(device, acl_table_id, &direction);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get ACL direction for ACL 0x%lx: %s",
                  acl_table_id,
                  sai_status_to_string(status));
    return status;
  }
  // Translate the ATTR to field values
  index2 = 0;
  for (index1 = 0; index1 < field_size; index1++) {
    if (match_fields[index1] != -1) {
      if (match_fields[index1] >= 0) {
        sai_acl_xform_field_value(acl_type,
                                  direction,
                                  match_fields[index1],
                                  (((char *)kvp) + (index2 * kvp_size)),
                                  &(attr_list[index1].value.aclfield));
        index2++;
      }
    }
  }

  // create the rule
  switch_status = switch_api_acl_rule_create(device,
                                             acl_table_id,
                                             switch_acl_priority,
                                             index2,
                                             kvp,
                                             acl_action,
                                             &action_params,
                                             &opt_action_params,
                                             &acl_entry_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl entry: %s",
                  sai_status_to_string(status));
    SAI_FREE(kvp);
    SAI_FREE(actions);
    SAI_FREE(match_fields);
    return status;
  }

  *acl_entry_id = acl_entry_handle;
  SAI_FREE(kvp);
  SAI_FREE(actions);
  SAI_FREE(match_fields);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*   Delete an ACL entry
*
* Arguments:
*  [in] acl_entry_id - the acl entry id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_acl_entry(_In_ sai_object_id_t acl_entry_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_entry_id) == SAI_OBJECT_TYPE_ACL_ENTRY);

  if ((acl_entry_id & (1 << (SWITCH_HANDLE_TYPE_SHIFT - 1))) != 0) {
    return sai_remove_dtel_watchlist_entry(acl_entry_id);
  }

  switch_status = switch_api_acl_rule_delete(
      device, (switch_handle_t)0, (switch_handle_t)acl_entry_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl entry 0x%lx : %s",
                  acl_entry_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

sai_status_t sai_set_acl_entry(_In_ sai_object_id_t acl_entry_id,
                               _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t switch_acl_priority = 0;
  switch_acl_action_t acl_action = 0;
  switch_acl_action_params_t action_params;
  switch_acl_opt_action_params_t opt_action_params;
  sai_packet_action_t packet_action = 0;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null acl attribute: %s", sai_status_to_string(status));
    return status;
  }

  if ((acl_entry_id & (1 << (SWITCH_HANDLE_TYPE_SHIFT - 1))) != 0) {
    return sai_set_dtel_watchlist_entry(acl_entry_id, attr);
  }

  if (attr->id >= SAI_ACL_ENTRY_ATTR_FIELD_START &&
      attr->id <= SAI_ACL_ENTRY_ATTR_FIELD_END) {
    status = SAI_STATUS_NOT_SUPPORTED;
    SAI_LOG_ERROR("Modifying acl_entry field attributes are not supported %s",
                  sai_status_to_string(status));
    return status;
  }

  memset(&action_params, 0, sizeof(switch_acl_action_params_t));
  memset(&opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
  switch (attr->id) {
    case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT: {
      // ACTION handling
      switch_handle_t handle = (switch_handle_t)attr->value.aclfield.data.oid;
      /*
      if (SAI_CPU_PORT(port_handle)) {
          acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
      } else  {
      */
      acl_action = SWITCH_ACL_ACTION_REDIRECT;
      // set the action params
      action_params.redirect.handle = handle;
    } break;
    case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
      acl_action = 0;
      packet_action = attr->value.aclfield.data.s32;
      if (packet_action == SAI_PACKET_ACTION_DROP) {
        acl_action = SWITCH_ACL_ACTION_DROP;
      } else if (packet_action == SAI_PACKET_ACTION_FORWARD) {
        acl_action = SWITCH_ACL_ACTION_PERMIT;
      }
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_FLOOD:  // Unsupported
      acl_action = SWITCH_ACL_ACTION_FLOOD_TO_VLAN;
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS: {
      switch_handle_t handle =
          (switch_handle_t)attr->value.aclaction.parameter.oid;
      acl_action = SWITCH_ACL_ACTION_SET_MIRROR;
      opt_action_params.mirror_handle = handle;
    } break;
    case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS: {
      switch_handle_t handle =
          (switch_handle_t)attr->value.aclaction.parameter.oid;
      acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_SET_MIRROR;
      opt_action_params.mirror_handle = handle;
    } break;
    case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER: {
      switch_handle_t handle =
          (switch_handle_t)attr->value.aclaction.parameter.oid;
      opt_action_params.meter_handle = handle;
    } break;
    case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER: {
      switch_handle_t handle =
          (switch_handle_t)attr->value.aclaction.parameter.oid;
      opt_action_params.counter_handle = handle;
    } break;

    case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
      opt_action_params.color =
          sai_color_to_switch_color(attr->value.aclaction.parameter.s32);
      break;
    case SAI_ACL_ENTRY_ATTR_PRIORITY:
      SAI_LOG_ERROR("Modifying ACL priority is not supported");
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  switch_status = switch_api_acl_entry_action_set(device,
                                                  acl_entry_id,
                                                  switch_acl_priority,
                                                  acl_action,
                                                  &action_params,
                                                  &opt_action_params);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to update acl entry: %s",
                  sai_status_to_string(status));
    return status;
  }
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_acl_initialize_kvp(void **kvp,
                                    switch_acl_type_t acl_type,
                                    switch_uint16_t rules_count) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP:
      *kvp = (switch_acl_ip_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_ip_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_MAC:
      *kvp = (switch_acl_mac_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_mac_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_IPV6:
      *kvp = (switch_acl_ipv6_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_ipv6_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_SYSTEM:
      *kvp = (switch_acl_system_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_system_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_IP_RACL:
      *kvp = (switch_acl_ip_racl_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_ip_racl_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_IPV6_RACL:
      *kvp = (switch_acl_ipv6_racl_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_ipv6_racl_key_value_pair_t)));
      break;

    case SWITCH_ACL_TYPE_ECN:
      *kvp = (switch_acl_ecn_key_value_pair_t *)(malloc(
          rules_count * sizeof(switch_acl_ecn_key_value_pair_t)));
      break;

    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }
  if (!(*kvp)) {
    status = SAI_STATUS_NO_MEMORY;
  }
  return status;
}

sai_status_t sai_acl_entry_fields_get(switch_handle_t acl_entry_handle,
                                      switch_uint16_t rules_count,
                                      switch_acl_type_t acl_type,
                                      sai_attribute_t *attr) {
  void *kvp = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  status = sai_acl_initialize_kvp(&kvp, acl_type, rules_count);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("could not initialize KVP: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_acl_entry_rules_get(device, acl_entry_handle, kvp);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl entry rules: %s",
                  sai_status_to_string(status));
    free(kvp);
    return status;
  }

  switch (acl_type) {
    case SWITCH_ACL_TYPE_IP: {
      switch_acl_ip_key_value_pair_t *ip_kvp =
          (switch_acl_ip_key_value_pair_t *)kvp;
      switch (attr->id) {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
          attr->value.aclfield.data.ip4 = htonl(ip_kvp->value.ipv4_source);
          attr->value.aclfield.mask.ip4 = htonl(ip_kvp->mask.u.mask);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
          attr->value.aclfield.data.ip4 = htonl(ip_kvp->value.ipv4_dest);
          attr->value.aclfield.mask.ip4 = htonl(ip_kvp->mask.u.mask);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
          attr->value.aclfield.data.u8 = ip_kvp->value.ttl;
          attr->value.aclfield.mask.u8 = ip_kvp->mask.u.mask;
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS:
          attr->value.aclfield.data.u8 = ip_kvp->value.ip_flags;
          attr->value.aclfield.mask.u8 = ip_kvp->mask.u.mask;
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
          attr->value.aclfield.data.u16 = ip_kvp->value.ip_proto;
          attr->value.aclfield.mask.u16 = ip_kvp->mask.u.mask;
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
          attr->value.aclfield.data.u8 = ip_kvp->value.tcp_flags;
          attr->value.aclfield.mask.u8 = ip_kvp->mask.u.mask;
          break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
          attr->value.aclfield.data.u16 = ip_kvp->value.eth_type;
          attr->value.aclfield.mask.u16 = ip_kvp->mask.u.mask;
          break;

        default:
          status = SAI_STATUS_NOT_SUPPORTED;
          break;
      }
    } break;

    case SWITCH_ACL_TYPE_IPV6: {
      switch_acl_ipv6_key_value_pair_t *ipv6_kvp =
          (switch_acl_ipv6_key_value_pair_t *)kvp;
      switch (attr->id) {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
          memcpy(attr->value.aclfield.data.ip6,
                 ipv6_kvp->value.ipv6_source.u.addr8,
                 16);
          memcpy(attr->value.aclfield.mask.ip6,
                 &ipv6_kvp->mask.u.mask.u.addr8,
                 16);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
          memcpy(attr->value.aclfield.data.ip6,
                 ipv6_kvp->value.ipv6_dest.u.addr8,
                 16);
          memcpy(attr->value.aclfield.mask.ip6,
                 &ipv6_kvp->mask.u.mask.u.addr8,
                 16);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
          attr->value.aclfield.data.u16 = ipv6_kvp->value.ip_proto;
          attr->value.aclfield.mask.u16 = ipv6_kvp->mask.u.mask.u.addr8[0];
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
          attr->value.aclfield.data.u8 = ipv6_kvp->value.tcp_flags;
          attr->value.aclfield.mask.u8 = ipv6_kvp->mask.u.mask.u.addr8[0];
          break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
          attr->value.aclfield.data.u8 = ipv6_kvp->value.ttl;
          attr->value.aclfield.mask.u8 = ipv6_kvp->mask.u.mask.u.addr8[0];
          break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
          attr->value.aclfield.data.u16 = ipv6_kvp->value.eth_type;
          attr->value.aclfield.mask.u16 = ipv6_kvp->mask.u.mask16;
          break;

        default:
          status = SAI_STATUS_NOT_SUPPORTED;
          break;
      }
    } break;

    case SWITCH_ACL_TYPE_MAC: {
      switch_acl_mac_key_value_pair_t *mac_kvp =
          (switch_acl_mac_key_value_pair_t *)kvp;
      switch (attr->id) {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
          memcpy(attr->value.aclfield.data.mac,
                 mac_kvp->value.source_mac.mac_addr,
                 6);
          memcpy(attr->value.aclfield.mask.mac, &mac_kvp->mask.u.mask, 6);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
          memcpy(attr->value.aclfield.data.mac,
                 mac_kvp->value.dest_mac.mac_addr,
                 6);
          memcpy(attr->value.aclfield.mask.mac, &mac_kvp->mask.u.mask, 6);
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
          attr->value.aclfield.data.u16 = mac_kvp->value.eth_type;
          attr->value.aclfield.mask.u16 = mac_kvp->mask.u.mask16;
          break;

        default:
          status = SAI_STATUS_NOT_SUPPORTED;
      }
    }

    case SWITCH_ACL_TYPE_ECN: {
      switch_acl_ecn_key_value_pair_t *ecn_kvp =
          (switch_acl_ecn_key_value_pair_t *)kvp;
      switch (attr->id) {
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
          attr->value.aclfield.data.u8 = ecn_kvp->value.dscp;
          attr->value.aclfield.mask.u8 = ecn_kvp->mask.u.mask;
          break;

        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
          attr->value.aclfield.data.u8 = ecn_kvp->value.ecn;
          attr->value.aclfield.mask.u8 = ecn_kvp->mask.u.mask;
          break;
      }
    } break;

    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }
  free(kvp);
  return status;
}

sai_status_t sai_get_acl_entry(_In_ sai_object_id_t acl_entry_handle,
                               _In_ uint32_t attr_count,
                               _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  uint32_t index;
  sai_attribute_t *attr = NULL;
  switch_acl_action_params_t action_params;
  switch_acl_opt_action_params_t opt_action_params;
  switch_acl_action_t packet_action;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_uint16_t rules_count = 0;
  switch_acl_type_t acl_type;
  switch_handle_t acl_table_id;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  memset(&action_params, 0, sizeof(switch_acl_action_params_t));
  memset(&opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));

  if ((acl_entry_handle & (1 << (SWITCH_HANDLE_TYPE_SHIFT - 1))) != 0) {
    return sai_get_dtel_watchlist_entry(
        acl_entry_handle, attr_count, attr_list);
  }

  switch_status = switch_api_acl_entry_acl_table_get(
      device, acl_entry_handle, &acl_table_id);

  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl table handle: %s",
                  sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_acl_type_get(device, (switch_handle_t)acl_table_id, &acl_type);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl type: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_acl_entry_action_get(device,
                                                  acl_entry_handle,
                                                  &packet_action,
                                                  &action_params,
                                                  &opt_action_params);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl entry action: %s",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_acl_entry_rules_count_get(
      device, acl_entry_handle, &rules_count);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get acl entry rules count: %s",
                  sai_status_to_string(status));
    return status;
  }

  for (index = 0, attr = attr_list; index < attr_count; index++, attr++) {
    switch (attr->id) {
      case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT:
        attr->value.aclfield.data.oid =
            (packet_action == SWITCH_ACL_ACTION_REDIRECT)
                ? action_params.redirect.handle
                : SAI_NULL_OBJECT_ID;
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
        if (packet_action == SWITCH_ACL_ACTION_DROP) {
          attr->value.aclfield.data.s32 = SAI_PACKET_ACTION_DROP;
        } else if (packet_action == SWITCH_ACL_ACTION_PERMIT) {
          attr->value.aclfield.data.s32 = SAI_PACKET_ACTION_FORWARD;
        }
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
        attr->value.aclfield.data.oid =
            (packet_action == SWITCH_ACL_ACTION_SET_MIRROR)
                ? opt_action_params.mirror_handle
                : SAI_NULL_OBJECT_ID;
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
        attr->value.aclfield.data.oid = opt_action_params.mirror_handle;
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER:
        attr->value.aclfield.data.oid = opt_action_params.meter_handle;
        break;

      case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER:
        attr->value.aclfield.data.oid = opt_action_params.counter_handle;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
        attr->value.aclaction.parameter.s32 =
            sai_switch_color_to_sai_color(opt_action_params.color);
        break;

      case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
      case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
      case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
      case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:

      case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
      case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
      case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
      case SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS:
      case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
      case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:

      case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
      case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
      case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
        status = sai_acl_entry_fields_get(
            acl_entry_handle, rules_count, acl_type, attr);
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }
  SAI_LOG_EXIT();
  return status;
}

/**
 * Routine Description:
 *   @brief Create an ACL counter
 *
 * Arguments:
 *   @param[out] acl_counter_id - the acl counter id
 *   @param[in] switch_id The switch Object id
 *   @param[in] attr_count - number of attributes
 *   @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_create_acl_counter(_Out_ sai_object_id_t *acl_counter_id,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t acl_counter_handle = SWITCH_API_INVALID_HANDLE;
  *acl_counter_id = SAI_NULL_OBJECT_ID;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_acl_counter_create(device, &acl_counter_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl counter: %s",
                  sai_status_to_string(status));
    return status;
  }
  *acl_counter_id = acl_counter_handle;

  SAI_LOG_EXIT();

  return status;
}

/**
 * Routine Description:
 *   @brief Delete an ACL counter
 *
 * Arguments:
 *  @param[in] acl_counter_id - the acl counter id
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_remove_acl_counter(_In_ sai_object_id_t acl_counter_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_counter_id) ==
             SAI_OBJECT_TYPE_ACL_COUNTER);

  switch_status =
      switch_api_acl_counter_delete(device, (switch_handle_t)acl_counter_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl counter 0x%lx : %s",
                  acl_counter_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *   @brief Set ACL counter attribute
 *
 * Arguments:
 *    @param[in] acl_counter_id - the acl counter id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t sai_set_acl_counter_attribute(_In_ sai_object_id_t acl_counter_id,
                                           _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_counter_id) ==
             SAI_OBJECT_TYPE_ACL_COUNTER);

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *   @brief Get ACL counter attribute
 *
 * Arguments:
 *    @param[in] acl_counter_id - acl counter id
 *    @param[in] attr_count - number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t sai_get_acl_counter_attribute(_In_ sai_object_id_t acl_counter_id,
                                           _In_ uint32_t attr_count,
                                           _Out_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_counter_t counter;
  sai_attribute_t *attribute = NULL;
  uint32_t index = 0;

  SAI_ASSERT(sai_object_type_query(acl_counter_id) ==
             SAI_OBJECT_TYPE_ACL_COUNTER);

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  memset(&counter, 0, sizeof(switch_counter_t));
  switch_status = switch_api_acl_counter_get(device, acl_counter_id, &counter);
  status = sai_switch_status_to_sai_status(switch_status);
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
      case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
        break;
      case SAI_ACL_COUNTER_ATTR_PACKETS:
        attribute->value.u64 = counter.num_packets;
        break;
      case SAI_ACL_COUNTER_ATTR_BYTES:
        attribute->value.u64 = counter.num_bytes;
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

sai_status_t sai_acl_range_attribute_to_switch_acl_range_attrbute(
    sai_acl_range_type_t range_type, switch_range_type_t *switch_range_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (range_type) {
    case SAI_ACL_RANGE_TYPE_L4_SRC_PORT_RANGE:
      *switch_range_type = SWITCH_RANGE_TYPE_SRC_PORT;
      break;

    case SAI_ACL_RANGE_TYPE_L4_DST_PORT_RANGE:
      *switch_range_type = SWITCH_RANGE_TYPE_DST_PORT;
      break;

    case SAI_ACL_RANGE_TYPE_OUTER_VLAN:
    case SAI_ACL_RANGE_TYPE_INNER_VLAN:
      *switch_range_type = SWITCH_RANGE_TYPE_VLAN;
      break;

    case SAI_ACL_RANGE_TYPE_PACKET_LENGTH:
      *switch_range_type = SWITCH_RANGE_TYPE_PACKET_LENGTH;
      break;

    default:
      *switch_range_type = SWITCH_RANGE_TYPE_NONE;
  }

  return status;
}

/**
 *   Routine Description:
 *     @brief Create an ACL Range
 *
 *  Arguments:
 *  @param[out] acl_range_id - the acl range id
 *  @param[in] switch_id The switch Object id
 *  @param[in] attr_count - number of attributes
 *  @param[in] attr_list - array of attributes
 *
 *  Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_create_acl_range(_Out_ sai_object_id_t *acl_range_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  const sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_range_type_t range_type = SWITCH_RANGE_TYPE_NONE;
  switch_range_t switch_range;
  switch_direction_t direction = SWITCH_API_DIRECTION_BOTH;
  switch_handle_t range_handle = 0;
  uint32_t i = 0;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_ACL_RANGE_ATTR_TYPE:
        sai_acl_range_attribute_to_switch_acl_range_attrbute(
            attribute->value.s32, &range_type);
        break;

      case SAI_ACL_RANGE_ATTR_LIMIT:
        switch_range.start_value = attribute->value.s32range.min;
        switch_range.end_value = attribute->value.s32range.max;
        break;
    }
  }
  *acl_range_id = SAI_NULL_OBJECT_ID;

  switch_status = switch_api_acl_range_create(
      device, direction, range_type, &switch_range, &range_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create acl range : %s",
                  sai_status_to_string(status));
  }

  *acl_range_id = (sai_object_id_t)range_handle;

  SAI_LOG_INFO(
      "create ACL range object 0x%lx, type %d, limit 0x%04x - 0x%04x\n",
      *acl_range_id,
      range_type,
      switch_range.start_value,
      switch_range.end_value);

  SAI_LOG_EXIT();

  return status;
}

/**
 *  Routine Description:
 *    @brief Remove an ACL Range
 *
 *  Arguments:
 *    @param[in] acl_range_id - the acl range id
 *
 *  Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_remove_acl_range(_In_ sai_object_id_t acl_range_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_range_id) == SAI_OBJECT_TYPE_ACL_RANGE);

  switch_status =
      switch_api_acl_range_delete(device, (switch_handle_t)acl_range_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete acl range 0x%lx : %s",
                  acl_range_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *   @brief Set ACL range attribute
 *
 * Arguments:
 *    @param[in] acl_range_id - the acl range id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_set_acl_range_attribute(_In_ sai_object_id_t acl_range_id,
                                         _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_range_id) == SAI_OBJECT_TYPE_ACL_RANGE);

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *   @brief Get ACL range attribute
 *
 * Arguments:
 *    @param[in] acl_range_id - acl range id
 *    @param[in] attr_count - number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t sai_get_acl_range_attribute(_In_ sai_object_id_t acl_range_id,
                                         _In_ uint32_t attr_count,
                                         _Out_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(acl_range_id) == SAI_OBJECT_TYPE_ACL_RANGE);

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
 * Some stubs
 */
sai_status_t sai_set_acl_table_group_attribute(
    _In_ sai_object_id_t acl_id, _In_ const sai_attribute_t *attr) {
  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_get_acl_table_group_attribute(
    _In_ sai_object_id_t acl_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list) {
  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_set_acl_table_group_member_attribute(
    _In_ sai_object_id_t acl_id, _In_ const sai_attribute_t *attr) {
  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_get_acl_table_group_member_attribute(
    _In_ sai_object_id_t acl_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list) {
  return SAI_STATUS_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// DTel watchlist
//------------------------------------------------------------------------------

typedef enum sai_dtel_watchlist_type_ {
  SAI_DTEL_WATCHLIST_TYPE_FLOW,
  SAI_DTEL_WATCHLIST_TYPE_DROP,
} sai_dtel_watchlist_type_t;

typedef struct sai_dtel_watchlist_entry_match_ {
  sai_acl_field_data_t ether_type;
  sai_acl_field_data_t ipv4_src;
  sai_acl_field_data_t ipv4_dst;
  sai_acl_field_data_t ip_proto;
  sai_acl_field_data_t dscp;
  sai_uint16_t l4_port_src_start;
  sai_uint16_t l4_port_src_end;
  sai_uint16_t l4_port_dst_start;
  sai_uint16_t l4_port_dst_end;
  sai_acl_field_data_t tunnel_vni;
  sai_acl_field_data_t inner_ether_type;
  sai_acl_field_data_t inner_ipv4_src;
  sai_acl_field_data_t inner_ipv4_dst;
  sai_acl_field_data_t inner_ip_proto;
  sai_uint16_t inner_l4_port_src_start;
  sai_uint16_t inner_l4_port_src_end;
  sai_uint16_t inner_l4_port_dst_start;
  sai_uint16_t inner_l4_port_dst_end;
} sai_dtel_watchlist_entry_match_t;

typedef struct sai_dtel_watchlist_entry_action_ {
  bool watch;
  bool report_all;
  sai_uint16_t int_session;
  sai_uint8_t percent;
} sai_dtel_watchlist_entry_action_t;

typedef struct sai_dtel_watchlist_entry_ {
  switch_handle_t handle;
  switch_dtel_type_t type;
  sai_dtel_watchlist_entry_match_t match;
  sai_dtel_watchlist_entry_action_t action;
  sai_uint16_t priority;
  bool used;
} sai_dtel_watchlist_entry_t;

#define DTEL_WATCHLIST_TABLE_MAX_SIZE 2048
typedef struct sai_dtel_watchlist_info_ {
  sai_dtel_watchlist_type_t type;
  switch_handle_t handle;
  sai_dtel_watchlist_entry_t entries[DTEL_WATCHLIST_TABLE_MAX_SIZE];
  sai_uint32_t id_stack[DTEL_WATCHLIST_TABLE_MAX_SIZE];
  uint32_t table_size;
  int top;
  bool created;
} sai_dtel_watchlist_info_t;

static sai_dtel_watchlist_info_t flow_watchlist;
static sai_dtel_watchlist_info_t drop_watchlist;

switch_handle_t watchlist_table_handle(sai_uint32_t type) {
  switch_handle_type_t handle = sai_id_to_oid(SWITCH_HANDLE_TYPE_ACL, type);
  handle |= 1 << (SWITCH_HANDLE_TYPE_SHIFT - 1);
  return handle;
}

sai_uint32_t watchlist_table_type(switch_handle_t handle) {
  return handle & ((1 << (SWITCH_HANDLE_TYPE_SHIFT - 1)) - 1);
}

switch_handle_t watchlist_entry_handle(sai_uint32_t type, sai_uint32_t index) {
  switch_handle_type_t handle = sai_id_to_oid(SWITCH_HANDLE_TYPE_ACE, index);
  handle |= 1 << (SWITCH_HANDLE_TYPE_SHIFT - 1);
  handle |= type << (SWITCH_HANDLE_TYPE_SHIFT - 4);
  return handle;
}

sai_uint32_t watchlist_entry_id(switch_handle_t handle) {
  return handle & ((1 << (SWITCH_HANDLE_TYPE_SHIFT - 4)) - 1);
}

sai_uint32_t watchlist_entry_type(switch_handle_t handle) {
  sai_uint32_t type = handle & ((1 << (SWITCH_HANDLE_TYPE_SHIFT - 1)) - 1);
  type = type >> (SWITCH_HANDLE_TYPE_SHIFT - 4);
  return type;
}

void sai_watchlist_init(sai_dtel_watchlist_info_t *watchlist,
                        sai_dtel_watchlist_type_t type) {
  SAI_MEMSET(watchlist, 0, sizeof(sai_dtel_watchlist_info_t));
  watchlist->type = type;
  watchlist->handle = watchlist_table_handle(type);
  if (type == SAI_DTEL_WATCHLIST_TYPE_FLOW) {
    watchlist->table_size = DTEL_FLOW_WATCHLIST_TABLE_SIZE;
  } else if (type == SAI_DTEL_WATCHLIST_TYPE_DROP) {
    watchlist->table_size = DTEL_FLOW_WATCHLIST_TABLE_SIZE;
  }
  for (uint32_t i = 0; i < watchlist->table_size; i++) {
    watchlist->type = type;
    watchlist->entries[i].handle = watchlist_entry_handle(type, i);
    watchlist->entries[i].used = false;
    watchlist->id_stack[i] = i;
  }
  watchlist->top = watchlist->table_size - 1;
  watchlist->created = false;
}

void sai_acl_dtel_watchlist_init() {
  sai_watchlist_init(&flow_watchlist, SAI_DTEL_WATCHLIST_TYPE_FLOW);
  sai_watchlist_init(&drop_watchlist, SAI_DTEL_WATCHLIST_TYPE_DROP);
}

bool is_dtel_acl(uint32_t attr_count, const sai_attribute_t *attr_list) {
  sai_uint32_t index = 0;
  uint32_t i;
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST: {
        for (i = 0; i < attr_list[index].value.s32list.count; i++) {
          switch (attr_list[index].value.s32list.list[i]) {
            case SAI_ACL_ACTION_TYPE_ACL_DTEL_FLOW_OP:
            case SAI_ACL_ACTION_TYPE_DTEL_DROP_REPORT_ENABLE:
              return true;
            default:
              break;
          }
        }
      } break;
      case SAI_ACL_ENTRY_ATTR_TABLE_ID:
        if (attr_list[index].value.oid == flow_watchlist.handle ||
            attr_list[index].value.oid == drop_watchlist.handle) {
          return true;
        }
    }
  }
  return false;
}

sai_status_t sai_create_dtel_watchlist_table(
    _Out_ sai_object_id_t *acl_table_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t index = 0;
  uint32_t i;
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST: {
        for (i = 0; i < attr_list[index].value.s32list.count; i++) {
          switch (attr_list[index].value.s32list.list[i]) {
            case SAI_ACL_ACTION_TYPE_ACL_DTEL_FLOW_OP:
              if (flow_watchlist.created) {
                *acl_table_id = flow_watchlist.handle;
                status = SAI_STATUS_ITEM_ALREADY_EXISTS;
              } else {
                flow_watchlist.created = true;
                *acl_table_id = flow_watchlist.handle;
                SAI_LOG_INFO("DTel flow watchlist created");
                return SAI_STATUS_SUCCESS;
              }
              break;
            case SAI_ACL_ACTION_TYPE_DTEL_DROP_REPORT_ENABLE:
              if (drop_watchlist.created) {
                *acl_table_id = drop_watchlist.handle;
                status = SAI_STATUS_ITEM_ALREADY_EXISTS;
              } else {
                *acl_table_id = drop_watchlist.handle;
                drop_watchlist.created = true;
                SAI_LOG_INFO("DTel drop watchlist created");
                return SAI_STATUS_SUCCESS;
              }
              break;
            default:
              status = SAI_STATUS_INVALID_PARAMETER;
              break;
          }
        }
      }
    }
  }

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel watchlist acl create failed(%s)\n",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_remove_dtel_watchlist_table(_In_ sai_object_id_t
                                                 acl_table_id) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  switch (watchlist_table_type(acl_table_id)) {
    case SAI_DTEL_WATCHLIST_TYPE_FLOW:
      flow_watchlist.created = false;
      SAI_LOG_INFO("DTel flow watchlist removed");
      break;
    case SAI_DTEL_WATCHLIST_TYPE_DROP:
      drop_watchlist.created = false;
      SAI_LOG_INFO("DTel drop watchlist removed");
      break;
    default:
      status = SAI_STATUS_INVALID_PARAMETER;
      break;
  }
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel watchlist acl remove failed(%s)\n",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();
  return status;
}

sai_status_t _sai_create_dtel_watchlist_entry(
    sai_object_id_t *acl_entry_id,
    uint32_t attr_count,
    const sai_attribute_t *attr_list,
    sai_dtel_watchlist_info_t *watchlist) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;
  if (watchlist->top < 1) {
    status = SAI_STATUS_TABLE_FULL;
    SAI_LOG_ERROR("DTel watchlist acl entry create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_dtel_watchlist_entry_t entry;
  SAI_MEMSET(&entry, 0, sizeof(sai_dtel_watchlist_entry_t));
  entry.match.l4_port_src_end = 0xFFFF;
  entry.match.l4_port_dst_end = 0xFFFF;
  entry.match.inner_l4_port_src_end = 0xFFFF;
  entry.match.inner_l4_port_dst_end = 0xFFFF;
  entry.action.percent = 100;

  switch_twl_match_info_t match_info;
  match_info.field_count = 0;
  match_info.fields =
      SAI_MALLOC(sizeof(switch_twl_key_value_pair_t) * SWITCH_TWL_FIELD_MAX);

  int count = 0;
  bool flow_op = false;
  bool drop_report = false;
  switch_twl_key_value_pair_t *kvp = NULL;
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
        memcpy(&entry.match.ether_type,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_ETHER_TYPE;
        kvp->value.ether_type = attr_list[index].value.aclfield.data.u16;
        kvp->mask = attr_list[index].value.aclfield.mask.u16;
        count += 1;
        SAI_LOG_INFO("DTel -- field ETHER_TYPE, value 0x%04x, mask 0x%04x",
                     kvp->value.ether_type,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
        memcpy(&entry.match.ipv4_src,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_IPV4_SRC;
        kvp->value.ipv4 = ntohl(attr_list[index].value.aclfield.data.ip4);
        kvp->mask = ntohl(attr_list[index].value.aclfield.mask.ip4);
        count += 1;
        SAI_LOG_INFO("DTel -- field SRC_IP, value 0x%08x, mask 0x%08x",
                     kvp->value.ipv4,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
        memcpy(&entry.match.ipv4_dst,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_IPV4_DST;
        kvp->value.ipv4 = ntohl(attr_list[index].value.aclfield.data.ip4);
        kvp->mask = ntohl(attr_list[index].value.aclfield.mask.ip4);
        count += 1;
        SAI_LOG_INFO("DTel -- field DST_IP, value 0x%08x, mask 0x%08x",
                     kvp->value.ipv4,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
        memcpy(&entry.match.ip_proto,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_IP_PROTO;
        kvp->value.ip_proto = attr_list[index].value.aclfield.data.u8;
        kvp->mask = attr_list[index].value.aclfield.mask.u8;
        count += 1;
        SAI_LOG_INFO("DTel -- field IP_PROTOCOL, value 0x%02x, mask 0x%02x",
                     kvp->value.ip_proto,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        memcpy(&entry.match.dscp,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_DSCP;
        kvp->value.dscp = attr_list[index].value.aclfield.data.u8;
        kvp->mask = attr_list[index].value.aclfield.mask.u8;
        count += 1;
        SAI_LOG_INFO("DTel -- field DSCP, value 0x%02x, mask 0x%02x",
                     kvp->value.dscp,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_L4_PORT_SRC_START;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.l4_port_src_start = kvp->value.l4_port;
        count += 1;
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_L4_PORT_SRC_END;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.l4_port_src_end = kvp->value.l4_port;
        count += 1;
        SAI_LOG_INFO("DTel -- field L4_SRC_PORT, value 0x%04x",
                     entry.match.l4_port_src_start);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_L4_PORT_DST_START;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.l4_port_dst_start = kvp->value.l4_port;
        count += 1;
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_L4_PORT_DST_END;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.l4_port_dst_end = kvp->value.l4_port;
        count += 1;
        SAI_LOG_INFO("DTel -- field L4_DST_PORT, value 0x%04x",
                     entry.match.l4_port_dst_start);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
        for (uint32_t index2 = 0;
             index2 < attr_list[index].value.aclfield.data.objlist.count;
             index2++) {
          switch_handle_t range_handle =
              (switch_handle_t)
                  attr_list[index].value.aclfield.data.objlist.list[index2];
          switch_range_t range;
          switch_status =
              switch_api_acl_range_get(device, range_handle, &range);
          switch_range_type_t range_type = SWITCH_RANGE_TYPE_NONE;
          switch_api_acl_range_type_get(device, range_handle, &range_type);
          if (range_type == SWITCH_RANGE_TYPE_SRC_PORT) {
            kvp = &match_info.fields[count];
            kvp->field = SWITCH_TWL_FIELD_L4_PORT_SRC_START;
            kvp->value.l4_port = range.start_value;
            kvp->mask = 0;
            entry.match.l4_port_src_start = kvp->value.l4_port;
            count += 1;
            kvp = &match_info.fields[count];
            kvp->field = SWITCH_TWL_FIELD_L4_PORT_SRC_END;
            kvp->value.l4_port = range.end_value;
            kvp->mask = 0;
            entry.match.l4_port_src_end = kvp->value.l4_port;
            count += 1;
            SAI_LOG_INFO("DTel -- field L4_SRC_PORT_RANGE, 0x%04x - 0x%04x",
                         range.start_value,
                         range.end_value);
          } else if (range_type == SWITCH_RANGE_TYPE_DST_PORT) {
            kvp = &match_info.fields[count];
            kvp->field = SWITCH_TWL_FIELD_L4_PORT_DST_START;
            kvp->value.l4_port = range.start_value;
            kvp->mask = 0;
            entry.match.l4_port_dst_start = kvp->value.l4_port;
            count += 1;
            kvp = &match_info.fields[count];
            kvp->field = SWITCH_TWL_FIELD_L4_PORT_DST_END;
            kvp->value.l4_port = range.end_value;
            kvp->mask = 0;
            entry.match.l4_port_dst_end = kvp->value.l4_port;
            count += 1;
            SAI_LOG_INFO("DTel -- field L4_DST_PORT_RANGE, 0x%04x - 0x%04x",
                         range.start_value,
                         range.end_value);
          }
        }
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_TUNNEL_VNI:
        memcpy(&entry.match.tunnel_vni,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_TUNNEL_VNI;
        kvp->value.tunnel_vni = attr_list[index].value.aclfield.data.u32;
        kvp->mask = attr_list[index].value.aclfield.mask.u32;
        count += 1;
        SAI_LOG_INFO("DTel -- field TUNNEL_VNI, value 0x%lx, mask 0x%lx",
                     kvp->value.tunnel_vni,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE:
        memcpy(&entry.match.inner_ether_type,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_ETHER_TYPE;
        kvp->value.ether_type = attr_list[index].value.aclfield.data.u16;
        kvp->mask = attr_list[index].value.aclfield.mask.u16;
        count += 1;
        SAI_LOG_INFO(
            "DTel -- field INNER_ETHER_TYPE, value 0x%04x, mask 0x%04x",
            kvp->value.ether_type,
            kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP:
        memcpy(&entry.match.inner_ipv4_src,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_IPV4_SRC;
        kvp->value.ipv4 = ntohl(attr_list[index].value.aclfield.data.ip4);
        kvp->mask = ntohl(attr_list[index].value.aclfield.mask.ip4);
        count += 1;
        SAI_LOG_INFO("DTel -- field INNER_SRC_IP, value 0x%08x, mask 0x%08x",
                     kvp->value.ipv4,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP:
        memcpy(&entry.match.inner_ipv4_dst,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_IPV4_DST;
        kvp->value.ipv4 = ntohl(attr_list[index].value.aclfield.data.ip4);
        kvp->mask = ntohl(attr_list[index].value.aclfield.mask.ip4);
        count += 1;
        SAI_LOG_INFO("DTel -- field INNER_DST_IP, value 0x%08x, mask 0x%08x",
                     kvp->value.ipv4,
                     kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL:
        memcpy(&entry.match.inner_ip_proto,
               &attr_list[index].value,
               sizeof(sai_acl_field_data_t));
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_IP_PROTO;
        kvp->value.ip_proto = attr_list[index].value.aclfield.data.u8;
        kvp->mask = attr_list[index].value.aclfield.mask.u8;
        count += 1;
        SAI_LOG_INFO(
            "DTel -- field INNER_IP_PROTOCOL, value 0x%02x, mask 0x%02x",
            kvp->value.ip_proto,
            kvp->mask);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_SRC_PORT:
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_START;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.inner_l4_port_src_start = kvp->value.l4_port;
        count += 1;
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_END;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.inner_l4_port_src_end = kvp->value.l4_port;
        count += 1;
        SAI_LOG_INFO("DTel -- field INNER_L4_SRC_PORT, value 0x%04x",
                     entry.match.inner_l4_port_src_start);
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_DST_PORT:
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_L4_PORT_DST_START;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.inner_l4_port_dst_start = kvp->value.l4_port;
        count += 1;
        kvp = &match_info.fields[count];
        kvp->field = SWITCH_TWL_FIELD_INNER_L4_PORT_DST_END;
        kvp->value.l4_port = attr_list[index].value.aclfield.data.u16;
        kvp->mask = 0;
        entry.match.inner_l4_port_dst_end = kvp->value.l4_port;
        count += 1;
        SAI_LOG_INFO("DTel -- field INNER_L4_DST_PORT, value 0x%04x",
                     entry.match.inner_l4_port_dst_start);
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_ACL_DTEL_FLOW_OP:
        flow_op = true;
        switch (attr_list[index].value.aclaction.parameter.s32) {
          case SAI_ACL_DTEL_FLOW_OP_NOP:
#ifdef P4_INT_EP_ENABLE
            entry.type = SWITCH_DTEL_TYPE_INT;
#elif defined(P4_POSTCARD_ENABLE)
            entry.type = SWITCH_DTEL_TYPE_POSTCARD;
#endif
            entry.action.watch = false;
            SAI_LOG_INFO("DTel -- action: nop");
            break;
          case SAI_ACL_DTEL_FLOW_OP_INT:
            entry.type = SWITCH_DTEL_TYPE_INT;
            entry.action.watch = true;
            SAI_LOG_INFO("DTel -- action: enable INT");
            break;
          case SAI_ACL_DTEL_FLOW_OP_POSTCARD:
            entry.type = SWITCH_DTEL_TYPE_POSTCARD;
            entry.action.watch = true;
            SAI_LOG_INFO("DTel -- action: enable postcard");
            break;
        }
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_DROP_REPORT_ENABLE:
        entry.type = SWITCH_DTEL_TYPE_MOD;
        entry.action.watch = attr_list[index].value.aclaction.enable;
        drop_report = true;
        SAI_LOG_INFO("DTel -- action: enable drop report");
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS:
        entry.action.report_all = attr_list[index].value.aclaction.enable;
        if (entry.action.report_all)
          SAI_LOG_INFO("DTel -- action: report all packets");
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_INT_SESSION:
        entry.action.int_session =
            sai_oid_to_id(attr_list[index].value.aclaction.parameter.oid);
        SAI_LOG_INFO("DTel -- action: INT session 0x%lx",
                     attr_list[index].value.aclaction.parameter.oid);
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_FLOW_SAMPLE_PERCENT:
        entry.action.percent = attr_list[index].value.aclaction.parameter.u8;
        SAI_LOG_INFO("DTel -- action: flow sample percent %d",
                     entry.action.percent);
        break;
      case SAI_ACL_ENTRY_ATTR_PRIORITY:
        entry.priority = attr_list[index].value.u32;
        break;
    }
  }
  if (flow_op && drop_report) {
    SAI_LOG_ERROR("DTel Flow Op and Drop report enabled for the same entry\n");
    status = SAI_STATUS_INVALID_PARAMETER;
    return status;
  }
  match_info.field_count = count;

  switch_twl_action_params_t action_params;
  SAI_MEMSET(&action_params, 0, sizeof(action_params));
  if (entry.type == SWITCH_DTEL_TYPE_INT) {
    action_params._int.session_id = entry.action.int_session;
    action_params._int.report_all_packets = entry.action.report_all;
    action_params._int.flow_sample_percent = entry.action.percent;
  } else if (entry.type == SWITCH_DTEL_TYPE_POSTCARD) {
    action_params._postcard.report_all_packets = entry.action.report_all;
    action_params._postcard.flow_sample_percent = entry.action.percent;
  } else if (entry.type == SWITCH_DTEL_TYPE_MOD) {
    action_params._mod.report_queue_tail_drops = true;
  }

  switch_status = switch_api_dtel_watchlist_entry_create(device,
                                                         entry.type,
                                                         &match_info,
                                                         entry.priority,
                                                         entry.action.watch,
                                                         &action_params);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel watchlist entry create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint32_t id = watchlist->id_stack[watchlist->top];
  watchlist->entries[id].type = entry.type;
  memcpy(&(watchlist->entries[id].match),
         &entry.match,
         sizeof(sai_dtel_watchlist_entry_match_t));
  memcpy(&(watchlist->entries[id].action),
         &entry.action,
         sizeof(sai_dtel_watchlist_entry_action_t));
  watchlist->entries[id].priority = entry.priority;
  watchlist->entries[id].used = true;
  watchlist->top -= 1;

  *acl_entry_id = watchlist->entries[id].handle;

  SAI_LOG_INFO("DTel watchlist entry 0x%lx created\n", *acl_entry_id);

  SAI_FREE(match_info.fields);
  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_create_dtel_watchlist_entry(
    _Out_ sai_object_id_t *acl_entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_ENTRY_ATTR_TABLE_ID:
        if (attr_list[index].value.oid == flow_watchlist.handle) {
          SAI_LOG_INFO("DTel create flow watchlist entry...\n");
          return _sai_create_dtel_watchlist_entry(
              acl_entry_id, attr_count, attr_list, &flow_watchlist);
        } else if (attr_list[index].value.oid == drop_watchlist.handle) {
          SAI_LOG_INFO("DTel create drop watchlist entry...\n");
          return _sai_create_dtel_watchlist_entry(
              acl_entry_id, attr_count, attr_list, &drop_watchlist);
        }
        break;
    }
  }
  status = SAI_STATUS_INVALID_PARAMETER;
  SAI_LOG_ERROR("DTel watchlist acl entry create failed(%s)\n",
                sai_status_to_string(status));

  SAI_LOG_EXIT();
  return status;
}

void sai_dtel_watchlist_entry_match_kvps(
    sai_dtel_watchlist_entry_match_t *match,
    switch_twl_match_info_t *match_info) {
  match_info->field_count = 18;
  match_info->fields =
      SAI_MALLOC(sizeof(switch_twl_key_value_pair_t) * match_info->field_count);

  match_info->fields[0].field = SWITCH_TWL_FIELD_ETHER_TYPE;
  match_info->fields[0].value.ether_type = match->ether_type.data.u16;
  match_info->fields[0].mask = match->ether_type.mask.u16;
  match_info->fields[1].field = SWITCH_TWL_FIELD_IPV4_SRC;
  match_info->fields[1].value.ipv4 = ntohl(match->ipv4_src.data.ip4);
  match_info->fields[1].mask = ntohl(match->ipv4_src.mask.ip4);
  match_info->fields[2].field = SWITCH_TWL_FIELD_IPV4_DST;
  match_info->fields[2].value.ipv4 = ntohl(match->ipv4_dst.data.ip4);
  match_info->fields[2].mask = ntohl(match->ipv4_dst.mask.ip4);
  match_info->fields[3].field = SWITCH_TWL_FIELD_IP_PROTO;
  match_info->fields[3].value.ip_proto = match->ip_proto.data.u8;
  match_info->fields[3].mask = match->ip_proto.mask.u8;
  match_info->fields[4].field = SWITCH_TWL_FIELD_DSCP;
  match_info->fields[4].value.dscp = match->dscp.data.u8;
  match_info->fields[4].mask = match->dscp.mask.u8;
  match_info->fields[5].field = SWITCH_TWL_FIELD_L4_PORT_SRC_START;
  match_info->fields[5].value.l4_port = match->l4_port_src_start;
  match_info->fields[6].field = SWITCH_TWL_FIELD_L4_PORT_SRC_END;
  match_info->fields[6].value.l4_port = match->l4_port_src_end;
  match_info->fields[7].field = SWITCH_TWL_FIELD_L4_PORT_DST_START;
  match_info->fields[7].value.l4_port = match->l4_port_dst_start;
  match_info->fields[8].field = SWITCH_TWL_FIELD_L4_PORT_DST_END;
  match_info->fields[8].value.l4_port = match->l4_port_dst_end;
  match_info->fields[9].field = SWITCH_TWL_FIELD_TUNNEL_VNI;
  match_info->fields[9].value.tunnel_vni = match->tunnel_vni.data.u32;
  match_info->fields[9].mask = match->tunnel_vni.mask.u32;
  match_info->fields[10].field = SWITCH_TWL_FIELD_INNER_ETHER_TYPE;
  match_info->fields[10].value.ether_type = match->inner_ether_type.data.u16;
  match_info->fields[10].mask = match->inner_ether_type.mask.u16;
  match_info->fields[11].field = SWITCH_TWL_FIELD_INNER_IPV4_SRC;
  match_info->fields[11].value.ipv4 = ntohl(match->inner_ipv4_src.data.ip4);
  match_info->fields[11].mask = ntohl(match->inner_ipv4_src.mask.ip4);
  match_info->fields[12].field = SWITCH_TWL_FIELD_INNER_IPV4_DST;
  match_info->fields[12].value.ipv4 = ntohl(match->inner_ipv4_dst.data.ip4);
  match_info->fields[12].mask = ntohl(match->inner_ipv4_dst.mask.ip4);
  match_info->fields[13].field = SWITCH_TWL_FIELD_INNER_IP_PROTO;
  match_info->fields[13].value.ip_proto = match->inner_ip_proto.data.u8;
  match_info->fields[13].mask = match->inner_ip_proto.mask.u8;
  match_info->fields[14].field = SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_START;
  match_info->fields[14].value.l4_port = match->inner_l4_port_src_start;
  match_info->fields[15].field = SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_END;
  match_info->fields[15].value.l4_port = match->inner_l4_port_src_end;
  match_info->fields[16].field = SWITCH_TWL_FIELD_INNER_L4_PORT_DST_START;
  match_info->fields[16].value.l4_port = match->inner_l4_port_dst_start;
  match_info->fields[17].field = SWITCH_TWL_FIELD_INNER_L4_PORT_DST_END;
  match_info->fields[17].value.l4_port = match->inner_l4_port_dst_end;
  /*
  SAI_LOG_DEBUG("DTel watchlist entry kvps :");
  SAI_LOG_DEBUG("DTel -- kvp ETHER_TYPE, value 0x%04x, mask 0x%04x",
                match_info->fields[0].value.ether_type,
                match_info->fields[0].mask);
  SAI_LOG_DEBUG("DTel -- kvp SRC_IP, value 0x%08x, mask 0x%08x",
                match_info->fields[1].value.ipv4,
                match_info->fields[1].mask);
  SAI_LOG_DEBUG("DTel -- kvp DST_IP, value 0x%08x, mask 0x%08x",
                match_info->fields[2].value.ipv4,
                match_info->fields[2].mask);
  SAI_LOG_DEBUG("DTel -- kvp IP_PROTO, value 0x%02x, mask 0x%02x",
                match_info->fields[3].value.ip_proto,
                match_info->fields[3].mask);
  SAI_LOG_DEBUG("DTel -- kvp DSCP, value 0x%02x, mask 0x%02x",
                match_info->fields[4].value.dscp,
                match_info->fields[4].mask);
  SAI_LOG_DEBUG("DTel -- kvp L4_SRC_PORT_RANGE, 0x%04x - 0x%04x",
                match_info->fields[5].value.l4_port,
                match_info->fields[6].value.l4_port);
  SAI_LOG_DEBUG("DTel -- kvp L4_DST_PORT_RANGE, 0x%04x - 0x%04x",
                match_info->fields[7].value.l4_port,
                match_info->fields[8].value.l4_port);
  SAI_LOG_DEBUG("DTel -- kvp TUNNEL_VNI, value 0x%lx, mask 0x%lx",
                match_info->fields[9].value.tunnel_vni,
                match_info->fields[9].mask);
  SAI_LOG_DEBUG("DTel -- kvp INNER_ETHER_TYPE, value 0x%04x, mask 0x%04x",
                match_info->fields[10].value.ether_type,
                match_info->fields[10].mask);
  SAI_LOG_DEBUG("DTel -- kvp INNER_SRC_IP, value 0x%08x, mask 0x%08x",
                match_info->fields[11].value.ipv4,
                match_info->fields[11].mask);
  SAI_LOG_DEBUG("DTel -- kvp INNER_DST_IP, value 0x%08x, mask 0x%08x",
                match_info->fields[12].value.ipv4,
                match_info->fields[12].mask);
  SAI_LOG_DEBUG("DTel -- kvp INNER_IP_PROTO, value 0x%02x, mask 0x%02x",
                match_info->fields[13].value.ip_proto,
                match_info->fields[13].mask);
  SAI_LOG_DEBUG("DTel -- kvp INNER_L4_SRC_PORT_RANGE, 0x%04x - 0x%04x",
                match_info->fields[14].value.l4_port,
                match_info->fields[15].value.l4_port);
  SAI_LOG_DEBUG("DTel -- kvp INNER_L4_DST_PORT_RANGE, 0x%04x - 0x%04x\n",
                match_info->fields[16].value.l4_port,
                match_info->fields[17].value.l4_port);
  */
}

sai_status_t _sai_remove_dtel_watchlist_entry(
    sai_object_id_t acl_entry_id, sai_dtel_watchlist_info_t *watchlist) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  sai_uint32_t id = watchlist_entry_id(acl_entry_id);

  if (id >= watchlist->table_size) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel watchlist entry 0x%lx remove failed (%s)\n",
                  acl_entry_id,
                  sai_status_to_string(status));
    return status;
  }

  if (watchlist->entries[id].used == false) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel watchlist entry remove failed (%s)\n",
                  acl_entry_id,
                  sai_status_to_string(status));
    return status;
  }

  switch_twl_match_info_t match_info;
  sai_dtel_watchlist_entry_match_kvps(&(watchlist->entries[id].match),
                                      &match_info);

  switch_status = switch_api_dtel_watchlist_entry_delete(
      device, watchlist->entries[id].type, &match_info);

  SAI_FREE(match_info.fields);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel watchlist entry remove failed (%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  watchlist->entries[id].used = false;
  watchlist->top += 1;
  watchlist->id_stack[watchlist->top] = id;

  SAI_LOG_INFO("DTel watchlist entry 0x%lx removed\n", acl_entry_id);

  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_remove_dtel_watchlist_entry(_In_ sai_object_id_t
                                                 acl_entry_id) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t type = watchlist_entry_type(acl_entry_id);

  if (type == SAI_DTEL_WATCHLIST_TYPE_FLOW) {
    return _sai_remove_dtel_watchlist_entry(acl_entry_id, &flow_watchlist);
  } else if (type == SAI_DTEL_WATCHLIST_TYPE_DROP) {
    return _sai_remove_dtel_watchlist_entry(acl_entry_id, &drop_watchlist);
  }

  SAI_LOG_EXIT();
  return status;
}

sai_status_t _sai_set_dtel_watchlist_entry(
    sai_object_id_t acl_entry_id,
    const sai_attribute_t *attr,
    sai_dtel_watchlist_info_t *watchlist) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  sai_uint32_t id = watchlist_entry_id(acl_entry_id);
  if (id >= watchlist->table_size) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel watchlist entry set failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (watchlist->entries[id].used == false) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel watchlist entry set failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint16_t priority = watchlist->entries[id].priority;
  bool watch = watchlist->entries[id].action.watch;
  bool report_all = watchlist->entries[id].action.report_all;
  sai_uint16_t int_session = watchlist->entries[id].action.int_session;
  sai_uint8_t percent = watchlist->entries[id].action.percent;

  switch (attr->id) {
    case SAI_ACL_ENTRY_ATTR_PRIORITY:
      priority = attr->value.u32;
      if (priority == watchlist->entries[id].priority) {
        return SAI_STATUS_SUCCESS;
      }
      break;

    case SAI_ACL_ENTRY_ATTR_ACTION_ACL_DTEL_FLOW_OP:
      switch (attr->value.aclaction.parameter.s32) {
        case SAI_ACL_DTEL_FLOW_OP_NOP:
          watch = false;
          if (watch == watchlist->entries[id].action.watch) {
            return SAI_STATUS_SUCCESS;
          }
          break;
        case SAI_ACL_DTEL_FLOW_OP_INT:
          watch = true;
          if (watch == watchlist->entries[id].action.watch) {
            return SAI_STATUS_SUCCESS;
          }
          break;
        case SAI_ACL_DTEL_FLOW_OP_POSTCARD:
          watch = true;
          if (watch == watchlist->entries[id].action.watch) {
            return SAI_STATUS_SUCCESS;
          }
          break;
      }
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_DROP_REPORT_ENABLE:
      watch = attr->value.aclaction.enable;
      if (watch == watchlist->entries[id].action.watch) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS:
      report_all = attr->value.aclaction.enable;
      if (report_all == watchlist->entries[id].action.report_all) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_INT_SESSION:
      int_session = sai_oid_to_id(attr->value.aclaction.parameter.oid);
      if (int_session == watchlist->entries[id].action.int_session) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_FLOW_SAMPLE_PERCENT:
      percent = attr->value.aclaction.parameter.u8;
      if (percent == watchlist->entries[id].action.percent) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      SAI_LOG_ERROR("DTel watchlist entry set failed(%s)\n",
                    sai_status_to_string(status));
      return status;
  }

  switch_twl_action_params_t action_params;
  SAI_MEMSET(&action_params, 0, sizeof(action_params));
  if (watchlist->entries[id].type == SWITCH_DTEL_TYPE_INT) {
    action_params._int.session_id = int_session;
    action_params._int.report_all_packets = report_all;
    action_params._int.flow_sample_percent = percent;
  } else if (watchlist->entries[id].type == SWITCH_DTEL_TYPE_POSTCARD) {
    action_params._postcard.report_all_packets = report_all;
    action_params._postcard.flow_sample_percent = percent;
  } else if (watchlist->entries[id].type == SWITCH_DTEL_TYPE_MOD) {
    action_params._mod.report_queue_tail_drops = true;
  }

  switch_twl_match_info_t match_info;
  sai_dtel_watchlist_entry_match_kvps(&(watchlist->entries[id].match),
                                      &match_info);

  switch_status =
      switch_api_dtel_watchlist_entry_update(device,
                                             watchlist->entries[id].type,
                                             &match_info,
                                             priority,
                                             watch,
                                             &action_params);
  SAI_FREE(match_info.fields);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel watchlist entry set failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  watchlist->entries[id].priority = priority;
  watchlist->entries[id].action.watch = watch;
  watchlist->entries[id].action.report_all = report_all;
  watchlist->entries[id].action.int_session = int_session;
  watchlist->entries[id].action.percent = percent;

  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_set_dtel_watchlist_entry(_In_ sai_object_id_t acl_entry_id,
                                          _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t type = watchlist_entry_type(acl_entry_id);

  if (type == SAI_DTEL_WATCHLIST_TYPE_FLOW) {
    return _sai_set_dtel_watchlist_entry(acl_entry_id, attr, &flow_watchlist);
  } else if (type == SAI_DTEL_WATCHLIST_TYPE_DROP) {
    return _sai_set_dtel_watchlist_entry(acl_entry_id, attr, &drop_watchlist);
  }

  SAI_LOG_EXIT();
  return status;
}

sai_status_t _sai_get_dtel_watchlist_entry(
    sai_object_id_t acl_entry_id,
    uint32_t attr_count,
    sai_attribute_t *attr_list,
    sai_dtel_watchlist_info_t *watchlist) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t id = watchlist_entry_id(acl_entry_id);

  if (id >= watchlist->table_size) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel watchlist entry get failed (%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (watchlist->entries[id].used == false) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel watchlist entry get failed (%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_dtel_watchlist_entry_t *entry = NULL;
  entry = &(watchlist->entries[id]);

  uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
        memcpy(&attr_list[index].value,
               &(entry->match.ether_type),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
        memcpy(&attr_list[index].value,
               &(entry->match.ipv4_src),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
        memcpy(&attr_list[index].value,
               &(entry->match.ipv4_dst),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
        memcpy(&attr_list[index].value,
               &(entry->match.ip_proto),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        memcpy(&attr_list[index].value,
               &(entry->match.dscp),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_TUNNEL_VNI:
        memcpy(&attr_list[index].value,
               &(entry->match.tunnel_vni),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE:
        memcpy(&attr_list[index].value,
               &(entry->match.inner_ether_type),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP:
        memcpy(&attr_list[index].value,
               &(entry->match.inner_ipv4_src),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP:
        memcpy(&attr_list[index].value,
               &(entry->match.inner_ipv4_dst),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL:
        memcpy(&attr_list[index].value,
               &(entry->match.inner_ip_proto),
               sizeof(sai_acl_field_data_t));
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_DROP_REPORT_ENABLE:
        attr_list[index].value.booldata = entry->action.watch;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS:
        attr_list[index].value.booldata = entry->action.report_all;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_INT_SESSION:
        attr_list[index].value.u16 = entry->action.int_session;
        break;
      case SAI_ACL_ENTRY_ATTR_ACTION_DTEL_FLOW_SAMPLE_PERCENT:
        attr_list[index].value.u8 = entry->action.percent;
        break;
      case SAI_ACL_ENTRY_ATTR_PRIORITY:
        attr_list[index].value.u32 = entry->priority;
        break;
    }
  }

  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_get_dtel_watchlist_entry(_In_ sai_object_id_t acl_entry_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_uint32_t type = watchlist_entry_type(acl_entry_id);

  if (type == SAI_DTEL_WATCHLIST_TYPE_FLOW) {
    return _sai_get_dtel_watchlist_entry(
        acl_entry_id, attr_count, attr_list, &flow_watchlist);
  } else if (type == SAI_DTEL_WATCHLIST_TYPE_DROP) {
    return _sai_get_dtel_watchlist_entry(
        acl_entry_id, attr_count, attr_list, &drop_watchlist);
  }

  SAI_LOG_EXIT();
  return status;
}

/*
*  ACL methods table retrieved with sai_api_query()
*/
sai_acl_api_t acl_api = {
    .create_acl_table = sai_create_acl_table,
    .remove_acl_table = sai_remove_acl_table,
    .create_acl_entry = sai_create_acl_entry,
    .remove_acl_entry = sai_remove_acl_entry,
    .set_acl_entry_attribute = sai_set_acl_entry,
    .get_acl_entry_attribute = sai_get_acl_entry,
    .create_acl_counter = sai_create_acl_counter,
    .remove_acl_counter = sai_remove_acl_counter,
    .set_acl_counter_attribute = sai_set_acl_counter_attribute,
    .get_acl_counter_attribute = sai_get_acl_counter_attribute,
    .create_acl_range = sai_create_acl_range,
    .remove_acl_range = sai_remove_acl_range,
    .set_acl_range_attribute = sai_set_acl_range_attribute,
    .get_acl_range_attribute = sai_get_acl_range_attribute,
    .create_acl_table_group = sai_create_acl_group,
    .remove_acl_table_group = sai_remove_acl_group,
    .set_acl_table_group_attribute = sai_set_acl_table_group_attribute,
    .get_acl_table_group_attribute = sai_get_acl_table_group_attribute,
    .create_acl_table_group_member = sai_create_acl_group_member,
    .remove_acl_table_group_member = sai_remove_acl_group_member,
    .set_acl_table_group_member_attribute =
        sai_set_acl_table_group_member_attribute,
    .get_acl_table_group_member_attribute =
        sai_get_acl_table_group_member_attribute};
sai_status_t sai_acl_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing acl");
  sai_api_service->acl_api = acl_api;
  sai_acl_qualifiers_load();
  sai_acl_dtel_watchlist_init();
  return SAI_STATUS_SUCCESS;
}
