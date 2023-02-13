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

#include "switchapi/switch_dtel.h"
#include "switch_internal.h"
#include "switch_pd_dtel.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//------------------------------------------------------------------------------
// DTel watchlist add/update/delete/clear
//------------------------------------------------------------------------------

switch_status_t switch_twl_key_init(void *args,
                                    switch_uint8_t *key,
                                    switch_uint32_t *len) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("DTel watchlist key init invalid parameter: %s\n",
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(key, args, sizeof(switch_twl_match_spec_t));
  *len = sizeof(switch_twl_match_spec_t);
  return status;
}

switch_status_t switch_twl_key_compare(const void *key1, const void *key2) {
  return SWITCH_MEMCMP(key1, key2, sizeof(switch_twl_match_spec_t));
}

switch_status_t switch_twl_convert_match_spec(
    switch_uint32_t field_count,
    switch_twl_key_value_pair_t *fields,
    switch_twl_match_spec_t *match_spec) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t i = 0;
  match_spec->l4_port_src_start = 0;
  match_spec->l4_port_src_end = 0xFFFF;
  match_spec->l4_port_dst_start = 0;
  match_spec->l4_port_dst_end = 0xFFFF;
  match_spec->inner_l4_port_src_start = 0;
  match_spec->inner_l4_port_src_end = 0xFFFF;
  match_spec->inner_l4_port_dst_start = 0;
  match_spec->inner_l4_port_dst_end = 0xFFFF;
  for (i = 0; i < field_count; i++) {
    switch (fields[i].field) {
      case SWITCH_TWL_FIELD_ETHER_TYPE:
        match_spec->ether_type = fields[i].value.ether_type;
        match_spec->ether_type_mask = fields[i].mask & 0xFFFF;
        break;
      case SWITCH_TWL_FIELD_IPV4_SRC:
        match_spec->ipv4_src = fields[i].value.ipv4;
        match_spec->ipv4_src_mask = fields[i].mask & 0xFFFFFFFF;
        break;
      case SWITCH_TWL_FIELD_IPV4_DST:
        match_spec->ipv4_dst = fields[i].value.ipv4;
        match_spec->ipv4_dst_mask = fields[i].mask & 0xFFFFFFFF;
        break;
      case SWITCH_TWL_FIELD_IP_PROTO:
        match_spec->ip_proto = fields[i].value.ip_proto;
        match_spec->ip_proto_mask = fields[i].mask & 0xFF;
        break;
      case SWITCH_TWL_FIELD_DSCP:
        // p4 header field is the entire 8b diffserv
        // shift input 6b DSCP to left
        match_spec->dscp = fields[i].value.dscp << 2;
        match_spec->dscp_mask = fields[i].mask << 2;
        break;
      case SWITCH_TWL_FIELD_L4_PORT_SRC: {
        if (fields[i].mask == 0xFFFF) {
          match_spec->l4_port_src_start = fields[i].value.l4_port;
          match_spec->l4_port_src_end = fields[i].value.l4_port;
        }
        break;
      }
      case SWITCH_TWL_FIELD_L4_PORT_SRC_START:
        match_spec->l4_port_src_start = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_L4_PORT_SRC_END:
        match_spec->l4_port_src_end = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_L4_PORT_DST: {
        if (fields[i].mask == 0xFFFF) {
          match_spec->l4_port_dst_start = fields[i].value.l4_port;
          match_spec->l4_port_dst_end = fields[i].value.l4_port;
        }
        break;
      }
      case SWITCH_TWL_FIELD_L4_PORT_DST_START:
        match_spec->l4_port_dst_start = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_L4_PORT_DST_END:
        match_spec->l4_port_dst_end = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_TUNNEL_VNI:
        match_spec->tunnel_vni = fields[i].value.tunnel_vni;
        match_spec->tunnel_vni_mask = fields[i].mask & 0xFFFFFFFF;
        break;
      case SWITCH_TWL_FIELD_INNER_ETHER_TYPE:
        match_spec->inner_ether_type = fields[i].value.ether_type;
        match_spec->inner_ether_type_mask = fields[i].mask & 0xFFFF;
        break;
      case SWITCH_TWL_FIELD_INNER_IPV4_SRC:
        match_spec->inner_ipv4_src = fields[i].value.ipv4;
        match_spec->inner_ipv4_src_mask = fields[i].mask & 0xFFFFFFFF;
        break;
      case SWITCH_TWL_FIELD_INNER_IPV4_DST:
        match_spec->inner_ipv4_dst = fields[i].value.ipv4;
        match_spec->inner_ipv4_dst_mask = fields[i].mask & 0xFFFFFFFF;
        break;
      case SWITCH_TWL_FIELD_INNER_IP_PROTO:
        match_spec->inner_ip_proto = fields[i].value.ip_proto;
        match_spec->inner_ip_proto_mask = fields[i].mask & 0xFF;
        break;
      case SWITCH_TWL_FIELD_INNER_L4_PORT_SRC: {
        if (fields[i].mask == 0xFFFF) {
          match_spec->inner_l4_port_src_start = fields[i].value.l4_port;
          match_spec->inner_l4_port_src_end = fields[i].value.l4_port;
        }
        break;
      }
      case SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_START:
        match_spec->inner_l4_port_src_start = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_END:
        match_spec->inner_l4_port_src_end = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_INNER_L4_PORT_DST: {
        if (fields[i].mask == 0xFFFF) {
          match_spec->inner_l4_port_dst_start = fields[i].value.l4_port;
          match_spec->inner_l4_port_dst_end = fields[i].value.l4_port;
        }
        break;
      }
      case SWITCH_TWL_FIELD_INNER_L4_PORT_DST_START:
        match_spec->inner_l4_port_dst_start = fields[i].value.l4_port;
        break;
      case SWITCH_TWL_FIELD_INNER_L4_PORT_DST_END:
        match_spec->inner_l4_port_dst_end = fields[i].value.l4_port;
        break;
      default:
        break;
    }
  }
  return status;
}

switch_status_t switch_twl_match_spec_print(
    switch_twl_match_spec_t *match_spec) {
  if (match_spec == NULL) {
    return SWITCH_STATUS_INVALID_PARAMETER;
  }
  SWITCH_PD_LOG_DEBUG(
      "ether_type: 0x%04x : 0x%04x "
      "ipv4_src: 0x%08x : 0x%08x "
      "ipv4_dst: 0x%08x : 0x%08x "
      "ipv4_proto: 0x%02x : 0x%02x "
      "l4_sport: 0x%04x - 0x%04x "
      "l4_dport: 0x%04x - 0x%04x "
      "tunnel_vni: 0x%08x : 0x%08x",
      match_spec->ether_type,
      match_spec->ether_type_mask,
      match_spec->ipv4_src,
      match_spec->ipv4_src_mask,
      match_spec->ipv4_dst,
      match_spec->ipv4_dst_mask,
      match_spec->ip_proto,
      match_spec->ip_proto_mask,
      match_spec->l4_port_src_start,
      match_spec->l4_port_src_end,
      match_spec->l4_port_dst_start,
      match_spec->l4_port_dst_end,
      match_spec->tunnel_vni,
      match_spec->tunnel_vni_mask);
  SWITCH_PD_LOG_DEBUG(
      "inner_ether_type: 0x%04x : 0x%04x "
      "inner_ipv4_src: 0x%08x : 0x%08x "
      "inner_ipv4_dst: 0x%08x : 0x%08x "
      "inner_ipv4_proto: 0x%02x : 0x%02x "
      "inner_l4_sport: 0x%04x - 0x%04x "
      "inner_l4_dport: 0x%04x - 0x%04x ",
      match_spec->inner_ether_type,
      match_spec->inner_ether_type_mask,
      match_spec->inner_ipv4_src,
      match_spec->inner_ipv4_src_mask,
      match_spec->inner_ipv4_dst,
      match_spec->inner_ipv4_dst_mask,
      match_spec->inner_ip_proto,
      match_spec->inner_ip_proto_mask,
      match_spec->inner_l4_port_src_start,
      match_spec->inner_l4_port_src_end,
      match_spec->inner_l4_port_dst_start,
      match_spec->inner_l4_port_dst_end);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_dtel_watchlist_entry_create_internal(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!match_info || !action_params) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("Watchlist add invalid parameter for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // priority 0 (highest priority) is reserved
  // priority is uint, so negative no negative priority
  if (priority == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "Watchlist add failed for device %d, "
        "priority 0 reserved for system internal use\n",
        device);
    return status;
  }

  switch (type) {
    case SWITCH_DTEL_TYPE_INT:
      status = switch_dtel_int_watchlist_entry_create(
          device, match_info, priority, watch, action_params);
      break;
    case SWITCH_DTEL_TYPE_POSTCARD:
      status = switch_dtel_postcard_watchlist_entry_create(
          device, match_info, priority, watch, action_params);
      break;
    case SWITCH_DTEL_TYPE_MOD:
      status = switch_dtel_drop_watchlist_entry_create(
          device, match_info, priority, watch, action_params);
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Watchlist type %d add failed for device %d: %s\n",
                     type,
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_dtel_watchlist_entry_update_internal(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!match_info || !action_params) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("Watchlist update invalid parameter for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (priority == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("Watchlist update failed for device %d, ",
                     "priority 0 reserved for system internal use\n",
                     device);
    return status;
  }

  switch (type) {
    case SWITCH_DTEL_TYPE_INT:
      status = switch_dtel_int_watchlist_entry_update(
          device, match_info, priority, watch, action_params);
      break;
    case SWITCH_DTEL_TYPE_POSTCARD:
      status = switch_dtel_postcard_watchlist_entry_update(
          device, match_info, priority, watch, action_params);
      break;
    case SWITCH_DTEL_TYPE_MOD:
      status = switch_dtel_drop_watchlist_entry_update(
          device, match_info, priority, watch, action_params);
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Watchlist type %d update failed for device %d: %s\n",
                     type,
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_dtel_watchlist_entry_delete_internal(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!match_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("Watchlist delete invalid parameter for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch (type) {
    case SWITCH_DTEL_TYPE_INT:
      status = switch_dtel_int_watchlist_entry_delete(device, match_info);
      break;
    case SWITCH_DTEL_TYPE_POSTCARD:
      status = switch_dtel_postcard_watchlist_entry_delete(device, match_info);
      break;
    case SWITCH_DTEL_TYPE_MOD:
      status = switch_dtel_drop_watchlist_entry_delete(device, match_info);
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Watchlist type %d delete failed for device %d: %s\n",
                     type,
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_dtel_watchlist_clear_internal(
    switch_device_t device, switch_dtel_type_t type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch (type) {
    case SWITCH_DTEL_TYPE_INT:
      status = switch_dtel_int_watchlist_clear(device);
      break;
    case SWITCH_DTEL_TYPE_POSTCARD:
      status = switch_dtel_postcard_watchlist_clear(device);
      break;
    case SWITCH_DTEL_TYPE_MOD:
      status = switch_dtel_drop_watchlist_clear(device);
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Watchlist type %d clear failed for device %d: %s\n",
                     type,
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

//------------------------------------------------------------------------------
// DTel mirror session
//------------------------------------------------------------------------------

switch_status_t switch_dtel_mirror_sessions_key_init(void *args,
                                                     switch_uint8_t *key,
                                                     switch_uint32_t *len) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  switch_dtel_mirror_session_entry_t *ms_entry = NULL;
  ms_entry = (switch_dtel_mirror_session_entry_t *)args;
  SWITCH_MEMCPY(key, &ms_entry->mirror_id, sizeof(switch_mirror_id_t));
  *len = sizeof(switch_mirror_id_t);
  SWITCH_ASSERT(*len == sizeof(switch_mirror_id_t));

  return status;
}

switch_int32_t switch_dtel_mirror_key_compare(const void *key1,
                                              const void *key2) {
  return SWITCH_MEMCMP(key1, key2, sizeof(switch_mirror_id_t));
}

#ifdef P4_DTEL_REPORT_LB_ENABLE
typedef struct dtel_mirror_session_foreach_arg_ {
  switch_device_t *device;
  switch_mirror_id_t mirror_id;
  switch_list_t *list;
} dtel_mirror_session_foreach_arg_t;

typedef struct dtel_mirror_session_list_ {
  switch_node_t node;
  switch_dtel_mirror_session_entry_t *hashmap_entry;
} dtel_mirror_session_list_t;

static switch_status_t dtel_compare_dtel_mirror_sessions(switch_device_t device,
                                                         switch_mirror_id_t id1,
                                                         switch_mirror_id_t id2,
                                                         switch_int8_t *res) {
  *res = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  // mirror_id -> mirror_handle -> mirror_session_info -> dst_ip
  switch_handle_t myarg_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ms_entry_handle = SWITCH_API_INVALID_HANDLE;
  ms_entry_handle = switch_mirror_handle_get(id1);
  myarg_handle = switch_mirror_handle_get(id2);
  if (myarg_handle == SWITCH_API_INVALID_HANDLE ||
      ms_entry_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d %s, "
        " cannot get mirror session handle",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_mirror_info_t *myarg_mirror_info = NULL;
  switch_ip_addr_t *myarg_dst_ip;
  status = switch_mirror_get(device, myarg_handle, &myarg_mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d:%s"
        " cannot get mirror info",
        device,
        switch_error_to_string(status));
    return status;
  }
  myarg_dst_ip = &myarg_mirror_info->api_mirror_info.dst_ip;

  switch_mirror_info_t *ms_entry_mirror_info = NULL;
  switch_ip_addr_t *ms_entry_dst_ip;
  status = switch_mirror_get(device, ms_entry_handle, &ms_entry_mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s"
        " cannot get mirror info",
        device,
        switch_error_to_string(status));
    return status;
  }
  ms_entry_dst_ip = &ms_entry_mirror_info->api_mirror_info.dst_ip;

  // give priority to IPV4 if dest IP type is different
  if (ms_entry_dst_ip->type != myarg_dst_ip->type) {
    if (ms_entry_dst_ip->type == SWITCH_API_IP_ADDR_V4 &&
        myarg_dst_ip->type == SWITCH_API_IP_ADDR_V6) {
      *res = -1;
      return status;
    } else {
      *res = 1;
      return status;
    }
  } else if ((ms_entry_dst_ip->type == SWITCH_API_IP_ADDR_V4 &&
              ms_entry_dst_ip->ip.v4addr < myarg_dst_ip->ip.v4addr) ||
             (ms_entry_dst_ip->type == SWITCH_API_IP_ADDR_V6 &&
              ((ms_entry_dst_ip->ip.v6addr.u.addr32[0] <
                myarg_dst_ip->ip.v6addr.u.addr32[0]) ||
               (ms_entry_dst_ip->ip.v6addr.u.addr32[0] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[0] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[1] <
                    myarg_dst_ip->ip.v6addr.u.addr32[1]) ||
               (ms_entry_dst_ip->ip.v6addr.u.addr32[0] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[0] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[1] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[1] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[2] <
                    myarg_dst_ip->ip.v6addr.u.addr32[2]) ||
               (ms_entry_dst_ip->ip.v6addr.u.addr32[0] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[0] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[1] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[1] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[2] ==
                    myarg_dst_ip->ip.v6addr.u.addr32[2] &&
                ms_entry_dst_ip->ip.v6addr.u.addr32[3] <
                    myarg_dst_ip->ip.v6addr.u.addr32[3])))) {
    *res = -1;
    return status;
  } else if ((ms_entry_dst_ip->type == SWITCH_API_IP_ADDR_V4 &&
              ms_entry_dst_ip->ip.v4addr == myarg_dst_ip->ip.v4addr) ||
             (ms_entry_dst_ip->type == SWITCH_API_IP_ADDR_V6 &&
              (ms_entry_dst_ip->ip.v6addr.u.addr32[0] ==
                   myarg_dst_ip->ip.v6addr.u.addr32[0] &&
               ms_entry_dst_ip->ip.v6addr.u.addr32[1] ==
                   myarg_dst_ip->ip.v6addr.u.addr32[1] &&
               ms_entry_dst_ip->ip.v6addr.u.addr32[2] ==
                   myarg_dst_ip->ip.v6addr.u.addr32[2] &&
               ms_entry_dst_ip->ip.v6addr.u.addr32[3] ==
                   myarg_dst_ip->ip.v6addr.u.addr32[3]))) {
    *res = 0;
    return status;
  }
  *res = 1;
  return status;
}

static void dtel_mirror_session_onlypd_delete_foreach(void *arg, void *data) {
  switch_status_t status;
  dtel_mirror_session_foreach_arg_t *myarg =
      (dtel_mirror_session_foreach_arg_t *)arg;
  switch_device_t device = *(myarg->device);
  switch_dtel_mirror_session_entry_t *ms_entry =
      (switch_dtel_mirror_session_entry_t *)data;

  // skip entries with smaller Dst IP address
  switch_int8_t compare_res;
  status = dtel_compare_dtel_mirror_sessions(
      device, ms_entry->mirror_id, myarg->mirror_id, &compare_res);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group:"
        " cannot compare the sessions\n",
        device,
        switch_error_to_string(status));
    return;
  }
  if (compare_res <= 0) {
    return;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group:"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return;
  }

  dtel_mirror_session_list_t *entry =
      SWITCH_MALLOC(device, sizeof(dtel_mirror_session_list_t), 1);
  if (!entry) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group:"
        " cannot malloc to keep track of deleted memeber\n",
        device,
        switch_error_to_string(status));
    return;
  }

  switch_size_t count = SWITCH_HASHTABLE_COUNT(&(dtel_ctx->_mirror.sessions));
  if (count == 1) {
    status = switch_pd_dtel_mirror_session_delete(
        device, dtel_ctx->_mirror.default_session_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel delete mirror session failed for device %d: %s,"
          " cannot delete member from group:"
          " cannot remove mirror session default handle",
          device,
          switch_error_to_string(status));
      return;
    }
    dtel_ctx->_mirror.default_session_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_pd_dtel_mirror_session_delete_member(
      device, ms_entry->pd_mbr_hdl, dtel_ctx->_mirror.default_session_grp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group\n",
        device,
        switch_error_to_string(status));
    return;
  }

  status = SWITCH_HASHTABLE_DELETE_NODE(&(dtel_ctx->_mirror.sessions),
                                        &ms_entry->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member from group:"
        " cannot remove hashtable entry",
        device,
        switch_error_to_string(status));
    return;
  }

  entry->hashmap_entry = ms_entry;
  status = SWITCH_LIST_INSERT(myarg->list, &(entry->node), entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group:"
        " cannot add to list to keep track of deleted memeber\n",
        device,
        switch_error_to_string(status));
    return;
  }
}

static void dtel_mirror_session_delete_foreach(void *arg, void *data) {
  switch_dtel_context_t *dtel_ctx = NULL;
  switch_status_t status;

  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_mirror_session_entry_t *ms_entry =
      (switch_dtel_mirror_session_entry_t *)data;

  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member from group:"
        " cannot get context\n",
        *device,
        switch_error_to_string(status));
    return;
  }

  switch_size_t count = SWITCH_HASHTABLE_COUNT(&(dtel_ctx->_mirror.sessions));
  if (count == 1) {
    status = switch_pd_dtel_mirror_session_delete(
        *device, dtel_ctx->_mirror.default_session_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel delete mirror session failed for device %d: %s,"
          " cannot delete member from group:"
          " cannot remove mirror session default handle",
          *device,
          switch_error_to_string(status));
      return;
    }
    dtel_ctx->_mirror.default_session_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_pd_dtel_mirror_session_delete_member(
      *device, ms_entry->pd_mbr_hdl, dtel_ctx->_mirror.default_session_grp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member from group\n",
        *device,
        switch_error_to_string(status));
    return;
  }

  status = SWITCH_HASHTABLE_DELETE_NODE(&(dtel_ctx->_mirror.sessions),
                                        &ms_entry->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member from group:"
        " cannot remove hashtable entry",
        *device,
        switch_error_to_string(status));
    return;
  }

  SWITCH_FREE(*device, ms_entry);
}

switch_int32_t switch_dtel_mirror_id_compare(const void *key1,
                                             const void *key2) {
  dtel_mirror_session_list_t *k1 = (dtel_mirror_session_list_t *)key1;
  dtel_mirror_session_list_t *k2 = (dtel_mirror_session_list_t *)key2;

  switch_int8_t compare_res;
  if (k1->hashmap_entry->device != k2->hashmap_entry->device) {
    SWITCH_LOG_ERROR("DTel unequal device IDs (%d vs %d)",
                     k1->hashmap_entry->device,
                     k2->hashmap_entry->device);
    return 0;
  }
  switch_status_t status =
      dtel_compare_dtel_mirror_sessions(k1->hashmap_entry->device,
                                        k1->hashmap_entry->mirror_id,
                                        k2->hashmap_entry->mirror_id,
                                        &compare_res);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot delete member of group:"
        " cannot compare the sessions\n",
        k1->hashmap_entry->device,
        switch_error_to_string(status));
    return 0;
  }
  return compare_res;
}
#endif  // P4_DTEL_REPORT_LB_ENABLE

switch_status_t switch_dtel_mirror_init(
    switch_dtel_mirror_info_t *mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_REPORT_LB_ENABLE
  mirror_info->sessions.size = DTEL_MAX_MIRROR_SESSION_PER_GROUP * 2;
  mirror_info->sessions.compare_func = switch_dtel_mirror_key_compare;
  mirror_info->sessions.key_func = switch_dtel_mirror_sessions_key_init;
  mirror_info->sessions.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&mirror_info->sessions);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Cannot init DTel mirror sessions: %s\n",
                     switch_error_to_string(status));
    return status;
  }
  mirror_info->default_session_hdl = SWITCH_PD_INVALID_HANDLE;
  mirror_info->default_session_grp = SWITCH_PD_INVALID_HANDLE;
#endif  // P4_DTEL_REPORT_LB_ENABLE

  return status;
}

switch_status_t switch_api_dtel_report_session_add_internal(
    switch_device_t device, switch_mirror_id_t mirror_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(mirror_id);

#ifdef P4_DTEL_REPORT_LB_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add mirror session failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  // the group handle should have been initialized at init
  SWITCH_ASSERT(SWITCH_PD_HANDLE_VALID(dtel_ctx->_mirror.default_session_grp));

  // check capacity
  switch_size_t count = SWITCH_HASHTABLE_COUNT(&(dtel_ctx->_mirror.sessions));
  if (count > DTEL_MAX_MIRROR_SESSION_PER_GROUP - 1) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR("DTel add mirror session failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  // allocate memory
  switch_dtel_mirror_session_entry_t *ms_entry = NULL;
  ms_entry =
      SWITCH_MALLOC(device, sizeof(switch_dtel_mirror_session_entry_t), 1);
  if (!ms_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("DTel add mirror session failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(ms_entry, 0x0, sizeof(switch_dtel_mirror_session_entry_t));
  SWITCH_MEMCPY(&(ms_entry->mirror_id), &mirror_id, sizeof(switch_mirror_id_t));
  ms_entry->pd_mbr_hdl = 0;
  ms_entry->device = device;

  // search for duplicate
  switch_dtel_mirror_session_entry_t *ms_entry2 = NULL;
  status = SWITCH_HASHTABLE_SEARCH(
      &(dtel_ctx->_mirror.sessions), (void *)(ms_entry), (void **)(&ms_entry2));
  if (status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "DTel add mirror session failed for device %d: %s,"
        " duplicate mirror session entry for %d\n",
        device,
        switch_error_to_string(status),
        mirror_id);
    SWITCH_FREE(device, ms_entry);
    goto cleanup;
  }

  switch_list_t removed_entries;
  status = SWITCH_LIST_INIT(&removed_entries);
  // remove all members that have larger destination IP
  dtel_mirror_session_foreach_arg_t arg;
  arg.device = &device;
  arg.mirror_id = mirror_id;
  arg.list = &removed_entries;
  status =
      SWITCH_HASHTABLE_FOREACH_ARG(&(dtel_ctx->_mirror.sessions),
                                   &dtel_mirror_session_onlypd_delete_foreach,
                                   &arg);

  // add group member
  status = switch_pd_dtel_mirror_session_add_member(
      device,
      mirror_id,
      &ms_entry->pd_mbr_hdl,
      dtel_ctx->_mirror.default_session_grp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add mirror session failed for device %d: %s,"
        " cannot add member to group\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  count = SWITCH_HASHTABLE_COUNT(&(dtel_ctx->_mirror.sessions));
  if (count == 0) {
    // Add selector after we add the first member
    // still didn't update the hash table so count==0
    status = switch_pd_dtel_mirror_session_add_group_selector(
        device,
        dtel_ctx->_mirror.default_session_grp,
        &(dtel_ctx->_mirror.default_session_hdl));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel add mirror session failed for device %d: %s,"
          " cannot add group selector\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  SWITCH_LIST_SORT(&removed_entries, switch_dtel_mirror_id_compare);
  // add all members that have larger mirror id than new one

  switch_node_t *node = NULL;
  dtel_mirror_session_list_t *entry;
  FOR_EACH_IN_LIST(removed_entries, node) {
    entry = node->data;
    status = switch_pd_dtel_mirror_session_add_member(
        device,
        entry->hashmap_entry->mirror_id,
        &entry->hashmap_entry->pd_mbr_hdl,
        dtel_ctx->_mirror.default_session_grp);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel add mirror session failed for device %d: %s,"
          " cannot add member to group\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }

    // add hash table entry
    status = SWITCH_HASHTABLE_INSERT(&(dtel_ctx->_mirror.sessions),
                                     &entry->hashmap_entry->node,
                                     (void *)(entry->hashmap_entry),
                                     (void *)(entry->hashmap_entry));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel add mirror session failed for device %d: %s"
          " at hashtable add. Now cleaning up the group\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }
  FOR_EACH_IN_LIST_END();

  FOR_EACH_IN_LIST(removed_entries, node) {
    entry = node->data;
    SWITCH_FREE(device, entry);
  }
  FOR_EACH_IN_LIST_END();

  // add hash table entry
  status = SWITCH_HASHTABLE_INSERT(&(dtel_ctx->_mirror.sessions),
                                   &ms_entry->node,
                                   (void *)(ms_entry),
                                   (void *)(ms_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add mirror session failed for device %d: %s"
        " at hashtable add. Now cleaning up the group\n",
        device,
        switch_error_to_string(status));
    status = switch_pd_dtel_mirror_session_delete_member(
        device, ms_entry->pd_mbr_hdl, dtel_ctx->_mirror.default_session_grp);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel add mirror session (cleanup) failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
    }
    SWITCH_FREE(device, ms_entry);
    goto cleanup;
  }

cleanup:
#endif  // P4_DTEL_REPORT_LB_ENABLE

  return status;
}

switch_status_t switch_api_dtel_report_session_delete_internal(
    switch_device_t device, switch_mirror_id_t mirror_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(mirror_id);

#ifdef P4_DTEL_REPORT_LB_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_PD_HANDLE_VALID(dtel_ctx->_mirror.default_session_grp));

  // make the entry to search
  switch_dtel_mirror_session_entry_t ms_entry;
  SWITCH_MEMSET(&ms_entry, 0x0, sizeof(switch_dtel_mirror_session_entry_t));
  SWITCH_MEMCPY(&(ms_entry.mirror_id), &mirror_id, sizeof(switch_mirror_id_t));
  ms_entry.pd_mbr_hdl = 0;

  switch_dtel_mirror_session_entry_t *ms_entry2 = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&(dtel_ctx->_mirror.sessions),
                                   (void *)(&ms_entry),
                                   (void **)(&ms_entry2));
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "DTel delete mirror session failed for device %d: %s,"
        " could not find mirror session entry for %d\n",
        device,
        switch_error_to_string(status),
        ms_entry.mirror_id);

    goto cleanup;
  }

  switch_size_t count = SWITCH_HASHTABLE_COUNT(&(dtel_ctx->_mirror.sessions));
  if (count == 1) {
    status = switch_pd_dtel_mirror_session_delete(
        device, dtel_ctx->_mirror.default_session_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("DTel delete mirror session failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
    dtel_ctx->_mirror.default_session_hdl = SWITCH_PD_INVALID_HANDLE;
  }
  status = switch_pd_dtel_mirror_session_delete_member(
      device, ms_entry2->pd_mbr_hdl, dtel_ctx->_mirror.default_session_grp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel delete mirror session failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_HASHTABLE_DELETE(&(dtel_ctx->_mirror.sessions),
                                   (void *)(ms_entry2),
                                   (void **)(&ms_entry2));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel delete mirror session failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_FREE(device, ms_entry2);

cleanup:
#endif  // P4_DTEL_REPORT_LB_ENABLE

  return status;
}

switch_status_t switch_dtel_mirror_sessions_clear(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

#ifdef P4_DTEL_REPORT_LB_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel clear mirror sessions failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  // remove all mirror sessions
  status = SWITCH_HASHTABLE_FOREACH_ARG(&(dtel_ctx->_mirror.sessions),
                                        &dtel_mirror_session_delete_foreach,
                                        &device);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel clear mirror sessions failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
#endif  // P4_DTEL_REPORT_LB_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// DTel Queue Alert
//------------------------------------------------------------------------------

switch_status_t switch_queue_alert_key_init(void *args,
                                            switch_uint8_t *key,
                                            switch_uint32_t *len) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }
  switch_queue_alert_index_entry_t *queue_entry = NULL;
  queue_entry = (switch_queue_alert_index_entry_t *)args;
  SWITCH_MEMCPY(key, &queue_entry->port, sizeof(switch_dev_port_t));
  SWITCH_MEMCPY(key + sizeof(switch_dev_port_t),
                &queue_entry->queue,
                sizeof(switch_qid_t));
  *len = sizeof(switch_dev_port_t) + sizeof(switch_qid_t);
  return status;
}

switch_int32_t switch_queue_alert_key_compare(const void *key1,
                                              const void *key2) {
  return SWITCH_MEMCMP(
      key1, key2, sizeof(switch_dev_port_t) + sizeof(switch_qid_t));
}

switch_status_t switch_dtel_queue_alert_init(
    switch_queue_alert_info_t *queue_alert) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  queue_alert->index_map.size = DTEL_QUEUE_TABLE_SIZE * 2;
  queue_alert->index_map.compare_func = switch_queue_alert_key_compare;
  queue_alert->index_map.key_func = switch_queue_alert_key_init;
  queue_alert->index_map.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&queue_alert->index_map);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Cannot init queue alter index map: %s \n",
                     switch_error_to_string(status));
    return status;
  }
  for (int i = 0; i < DTEL_QUEUE_TABLE_SIZE; i++) {
    queue_alert->index_stack[i] = i;
  }
  queue_alert->top = DTEL_QUEUE_TABLE_SIZE - 1;

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
  return status;
}

switch_status_t switch_api_dtel_queue_report_create_internal(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth,
    switch_uint32_t latency,
    switch_uint16_t quota,
    bool dod) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);
  UNUSED(queue);
  UNUSED(depth);
  UNUSED(latency);
  UNUSED(quota);
  UNUSED(dod);

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  if (quota < 1) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s,"
        " quota must be > 0\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_queue_alert.top <= 0) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s, table full\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch_uint16_t index =
      dtel_ctx->_queue_alert.index_stack[dtel_ctx->_queue_alert.top];

  status =
      switch_pd_dtel_set_queue_alert_threshold(device, index, depth, latency);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set queue alert threshold failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_queue_alert_index_entry_t *queue_entry = NULL;
  queue_entry =
      SWITCH_MALLOC(device, sizeof(switch_queue_alert_index_entry_t), 1);
  if (!queue_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "Queue alert map entry memory allocation failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMSET(queue_entry, 0, sizeof(switch_queue_alert_index_entry_t));
  queue_entry->port = port;
  queue_entry->queue = queue;
  queue_entry->index = index;

  switch_handle_t port_handle;
  status = switch_api_port_id_to_handle_get(device, port, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue alert failed for device %d: %s\n",
                     " cannot get port handle",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_port_info_t *port_info = NULL;
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue alert failed for device %d: %s\n",
                     " cannot get port info",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_queue_alert_index_set(device,
                                                port_info->dev_port,
                                                queue,
                                                index,
                                                dtel_ctx->quantization_shift,
                                                &queue_entry->qalert_pd_hdl,
                                                true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue alert index failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_INSERT(&dtel_ctx->_queue_alert.index_map,
                                   &queue_entry->node,
                                   (void *)(queue_entry),
                                   (void *)(queue_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Queue alert hash table insert failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_queue_change_reset(device, index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue change reset failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (dod) {
    status = switch_pd_dtel_deflect_on_drop_queue_config_add(
        device, port_info->dev_port, queue, &queue_entry->qdod_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("DTel set queue dod failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    status = switch_pd_dtel_queue_report_dod_quota_add(
        device,
        port_info->dev_port,
        queue,
        index,
        &queue_entry->qdod_quota_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel set queue quota for dod failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  } else {
    queue_entry->qdod_pd_hdl = SWITCH_PD_INVALID_HANDLE;
    queue_entry->qdod_quota_pd_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_pd_dtel_queue_report_quota_set(device, index, quota);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue report quota failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_queue_alert.top -= 1;

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
  return status;
}

switch_status_t switch_api_dtel_queue_report_update_internal(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth,
    switch_uint32_t latency,
    switch_uint16_t quota,
    bool dod) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);
  UNUSED(queue);
  UNUSED(depth);
  UNUSED(latency);
  UNUSED(quota);
  UNUSED(dod);
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // ignore latency threshold
  latency = 0xFFFFFFFF;

  if (quota < 1) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel queue alert update failed for device %d: %s,"
        " quota must be > 0\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_queue_alert_index_entry_t queue_entry_key;
  SWITCH_MEMSET(&queue_entry_key, 0, sizeof(switch_queue_alert_index_entry_t));
  queue_entry_key.port = port;
  queue_entry_key.queue = queue;
  switch_queue_alert_index_entry_t *queue_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_queue_alert.index_map,
                                   (void *)(&queue_entry_key),
                                   (void **)&queue_entry);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "Queue alert update hashtable lookup failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_set_queue_alert_threshold(
      device, queue_entry->index, depth, latency);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set queue alert threshold failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dod) {
    switch_handle_t port_handle;
    status = switch_api_port_id_to_handle_get(device, port, &port_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("DTel set queue alert failed for device %d: %s\n",
                       " cannot get port handle",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    switch_port_info_t *port_info = NULL;
    status = switch_port_get(device, port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("DTel set queue alert failed for device %d: %s\n",
                       " cannot get port info",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    if (queue_entry->qdod_pd_hdl == SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_deflect_on_drop_queue_config_add(
          device, port_info->dev_port, queue, &queue_entry->qdod_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("DTel set queue dod failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }
    if (queue_entry->qdod_quota_pd_hdl == SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_queue_report_dod_quota_add(
          device,
          port_info->dev_port,
          queue,
          queue_entry->index,
          &queue_entry->qdod_quota_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "DTel set queue quota for dod failed for device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  } else {
    if (queue_entry->qdod_pd_hdl != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_deflect_on_drop_queue_config_delete(
          device, queue_entry->qdod_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("DTel delete queue dod failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      queue_entry->qdod_pd_hdl = SWITCH_PD_INVALID_HANDLE;
    }
    if (queue_entry->qdod_quota_pd_hdl != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_queue_report_dod_quota_delete(
          device, queue_entry->qdod_quota_pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "DTel delete queue quota for dod failed for device %d: %s\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      queue_entry->qdod_quota_pd_hdl = SWITCH_PD_INVALID_HANDLE;
    }
  }

  status =
      switch_pd_dtel_queue_report_quota_set(device, queue_entry->index, quota);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue report quota failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE
  return status;
}

switch_status_t switch_api_dtel_queue_report_delete_internal(
    switch_device_t device, switch_uint16_t port, switch_uint16_t queue) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);
  UNUSED(queue);

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_queue_alert_index_entry_t queue_entry_key;
  SWITCH_MEMSET(&queue_entry_key, 0, sizeof(switch_queue_alert_index_entry_t));
  queue_entry_key.port = port;
  queue_entry_key.queue = queue;
  switch_queue_alert_index_entry_t *queue_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_queue_alert.index_map,
                                   (void *)(&queue_entry_key),
                                   (void **)&queue_entry);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "Queue alert delete hashtable lookup failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  // set queue threshold to max value
  status = switch_pd_dtel_set_queue_alert_threshold(
      device, queue_entry->index, 0xFFFF, 0xFFFF);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set queue alert threshold failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // revert to default to don't think 0 quota and queue_alert = 0 is
  // a transition from alert to no alert
  status = switch_pd_dtel_queue_report_quota_set(
      device, queue_entry->index, DTEL_QUEUE_REPORT_DEFAULT_QUOTA);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set queue report quota failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (queue_entry->qdod_pd_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_deflect_on_drop_queue_config_delete(
        device, queue_entry->qdod_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("DTel delete queue dod failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
    queue_entry->qdod_pd_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  if (queue_entry->qdod_quota_pd_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_queue_report_dod_quota_delete(
        device, queue_entry->qdod_quota_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel delete queue quota for dod failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    queue_entry->qdod_quota_pd_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_pd_dtel_queue_alert_index_delete(device,
                                                   queue_entry->qalert_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel delete queue alert index failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
  queue_entry->qalert_pd_hdl = SWITCH_PD_INVALID_HANDLE;

  status = SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_queue_alert.index_map,
                                        &queue_entry->node);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "DTel queue alert hashtable delete failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_queue_alert.top >= DTEL_QUEUE_TABLE_SIZE - 1) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR(
        "DTel queue alert delete failed for device %d: %s, stack full\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
  dtel_ctx->_queue_alert.top += 1;
  dtel_ctx->_queue_alert.index_stack[dtel_ctx->_queue_alert.top] =
      queue_entry->index;

cleanup:
  SWITCH_FREE(device, queue_entry);
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

  return status;
}

switch_status_t
switch_api_dtel_queue_remaining_report_quota_during_breach_get_internal(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint16_t *quota) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);
  UNUSED(queue);
  UNUSED(quota);

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel queue alert add failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_queue_alert_index_entry_t queue_entry_key;
  SWITCH_MEMSET(&queue_entry_key, 0, sizeof(switch_queue_alert_index_entry_t));
  queue_entry_key.port = port;
  queue_entry_key.queue = queue;
  switch_queue_alert_index_entry_t *queue_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_queue_alert.index_map,
                                   (void *)(&queue_entry_key),
                                   (void **)&queue_entry);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "Queue alert quota hashtable lookup failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_queue_remaining_report_quota_during_breach_get(
      device, queue_entry->index, quota);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel queue alert get quota failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

  return status;
}

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
void switch_dtel_flow_state_reset_timer_cb(bf_sys_timer_t *timer, void *data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (data == NULL || timer == NULL) {
    return;
  }
  switch_device_t device = *(switch_device_t *)data;
  status = switch_pd_dtel_bloom_filters_reset(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel flow state reset failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    if (timer != NULL) {
      // only stop but don't delete to not corrupt the reference in dtel_ctx
      status = SWITCH_TIMER_STOP(timer);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "DTel flow state reset failed for device %d: %s, cannot stop "
            "timer\n",
            device,
            switch_error_to_string(status));
      }
    }
    return;
  }
}

#endif /* P4_DTEL_FLOW_STATE_TRACK_ENABLE */

switch_status_t switch_api_dtel_flow_state_clear_cycle_internal(
    switch_device_t device, switch_uint16_t cycle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(cycle);

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set flow state clear cycle failed for device %d: %s"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_TIMER_STOP(&dtel_ctx->flowstate_reset_timer);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set flow state clear cycle failed for device %d: %s, cannot stop "
        "timer\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_TIMER_DELETE(&dtel_ctx->flowstate_reset_timer);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set flow state clear cycle failed for device %d: %s, cannot "
        "delete timer\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  dtel_ctx->flowstate_reset_cycle = cycle;

  switch_device_context_t *device_ctx = NULL;
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set flow state clear cycle failed for device %d: %s "
        "cannot get device context",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = bf_sys_timer_create(
      &dtel_ctx->flowstate_reset_timer,
      0,
      (switch_uint32_t)dtel_ctx->flowstate_reset_cycle * 1000,
      switch_dtel_flow_state_reset_timer_cb,
      (void *)&device_ctx->device_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set flow state clear cycle failed for device %d: %s "
        "cannot create timer",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (dtel_ctx->flowstate_reset_cycle != DTEL_FLOW_STATE_TRACK_NO_RESET_CYCLE) {
    status = SWITCH_TIMER_START(&dtel_ctx->flowstate_reset_timer);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel set flow state clear cycle failed for device %d: %s,"
          " cannot start timer\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

cleanup:
#endif /* P4_DTEL_FLOW_STATE_TRACK_ENABLE */

  return status;
}

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
static void dtel_queue_alert_update_foreach(void *arg, void *data) {
  switch_dtel_context_t *dtel_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_device_t device = *((switch_device_t *)arg);
  switch_queue_alert_index_entry_t *queue_entry =
      (switch_queue_alert_index_entry_t *)data;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set quantization shift failed for device %d: %s\n"
        " cannot update qalert table entries:"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return;
  }

  switch_handle_t port_handle;
  status =
      switch_api_port_id_to_handle_get(device, queue_entry->port, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set quantization shift failed for device %d: %s\n"
        " cannot get port handle",
        device,
        switch_error_to_string(status));
    return;
  }

  switch_port_info_t *port_info = NULL;
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set quantization shift failed for device %d: %s\n"
        " cannot get port info",
        device,
        switch_error_to_string(status));
    return;
  }

  status = switch_pd_dtel_queue_alert_index_set(device,
                                                port_info->dev_port,
                                                queue_entry->queue,
                                                queue_entry->index,
                                                dtel_ctx->quantization_shift,
                                                &queue_entry->qalert_pd_hdl,
                                                false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set quantization shift failed for device %d: %s\n"
        " cannot update qalert table",
        device,
        switch_error_to_string(status));
    return;
  }
}

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

switch_status_t switch_api_dtel_latency_quantization_shift_internal(
    switch_device_t device, switch_uint8_t quant_shift) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(quant_shift);

#if defined(P4_DTEL_FLOW_STATE_TRACK_ENABLE) || \
    defined(P4_INT_DIGEST_ENABLE) || defined(P4_DTEL_QUEUE_REPORT_ENABLE)
  switch_dtel_context_t *dtel_ctx = NULL;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set quantization shift failed for device %d: %s\n"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (quant_shift > 32) {
    quant_shift = 32;
  }
  dtel_ctx->quantization_shift = quant_shift;
  switch_pd_hdl_t entry_hdl;

  status = switch_pd_dtel_quantize_latency_set(
      device, dtel_ctx->quantization_shift, &entry_hdl);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set quantization shift failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  // update current entries in dtel_queue_alert table

  status = SWITCH_HASHTABLE_FOREACH_ARG(&(dtel_ctx->_queue_alert.index_map),
                                        &dtel_queue_alert_update_foreach,
                                        &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set quantization shift failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE

cleanup:
#endif  // P4_DTEL_FLOW_STATE_TRACK_ENABLE || P4_INT_DIGEST_ENABLE ||
        // P4_DTEL_QUEUE_REPORT_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// DTel Common APIs
//------------------------------------------------------------------------------

switch_status_t switch_api_dtel_switch_id_set_internal(
    switch_device_t device, switch_uint32_t switch_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel switch ID set failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  dtel_ctx->switch_id = switch_id;

  status = switch_dtel_int_switch_id(device, dtel_ctx->switch_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT switch ID set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_dtel_postcard_switch_id(device, dtel_ctx->switch_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Postcard switch ID set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mirror_on_drop_encap_update(device,
                                                 dtel_ctx->switch_id,
                                                 dtel_ctx->dest_udp_port,
                                                 dtel_ctx->event_infos,
                                                 false,
                                                 dtel_ctx->_mod.me_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Mirror on Drop switch ID set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_dtel_report_udp_dstport_set_internal(
    switch_device_t device, switch_uint16_t dest_udp_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel dest udp_port set failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  dtel_ctx->dest_udp_port = dest_udp_port;

  status = switch_dtel_int_dest_udp_port(device, dest_udp_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT dest udp port set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_dtel_postcard_dest_udp_port(device, dest_udp_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Postcard dest udp port set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_mirror_on_drop_encap_update(device,
                                                 dtel_ctx->switch_id,
                                                 dtel_ctx->dest_udp_port,
                                                 dtel_ctx->event_infos,
                                                 false,
                                                 dtel_ctx->_mod.me_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Mirror on Drop dest udp port set failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_int32_t switch_packet_dtel_event_dscp_compare(const void *key1,
                                                     const void *key2) {
  if (!key1 || !key2) {
    SWITCH_LOG_ERROR("DTel event type dscp compare failed:(%s)\n",
                     switch_error_to_string(SWITCH_STATUS_INVALID_PARAMETER));
    return -1;
  }
  dtel_event_info_t *info1 = (dtel_event_info_t *)key1;
  dtel_event_info_t *info2 = (dtel_event_info_t *)key2;

  return (switch_int32_t)info2->dscp - (switch_int32_t)info1->dscp;
}

switch_status_t switch_dtel_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;
  dtel_ctx = SWITCH_MALLOC(device, sizeof(switch_dtel_context_t), 1);
  if (!dtel_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("DTel initizlization failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(dtel_ctx, 0x0, sizeof(switch_dtel_context_t));
  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_DTEL, (void *)dtel_ctx);
  if (!dtel_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "DTel initizlization failed for device %d: %s, "
        " cannot set context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_LIST_INIT(&dtel_ctx->event_infos_sorted_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel event type initialization failed for device: %u, error: %s "
        "\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
  for (int i = 0; i < SWITCH_DTEL_EVENT_TYPE_MAX; i++) {
    // default dscp is 0
    dtel_ctx->event_infos[i].type = (switch_dtel_event_type_t)i;
    dtel_ctx->event_infos[i].dscp = 0;
    status = SWITCH_LIST_INSERT(&dtel_ctx->event_infos_sorted_list,
                                &(dtel_ctx->event_infos[i].node),
                                &dtel_ctx->event_infos[i]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "DTel event type initialization failed for device: %u, error: "
          "%s \n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  SWITCH_LIST_SORT(&dtel_ctx->event_infos_sorted_list,
                   switch_packet_dtel_event_dscp_compare);

  dtel_ctx->dest_udp_port = 0;
  dtel_ctx->switch_id = 0;
  dtel_ctx->quantization_shift = DTEL_DEFAULT_LATENCY_QUANTIZATION_SHIFT;
  dtel_ctx->flowstate_reset_cycle = DTEL_FLOW_STATE_TRACK_NO_RESET_CYCLE;

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
  switch_device_context_t *device_ctx = NULL;
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel flow state reset timer initialization failed for device %d: %s "
        "cannot get device context",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = bf_sys_timer_create(
      &dtel_ctx->flowstate_reset_timer,
      0,
      (switch_uint32_t)dtel_ctx->flowstate_reset_cycle * 1000,
      switch_dtel_flow_state_reset_timer_cb,
      (void *)&device_ctx->device_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel flow state reset timer initialization failed for device %d: %s "
        "cannot create timer",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
#endif /* P4_DTEL_FLOW_STATE_TRACK_ENABLE */

  // initiate mirror sessions
  status = switch_dtel_mirror_init(&dtel_ctx->_mirror);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel Mirror session initialization failed for device %d: %s",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_dtel_int_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT initialization failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_dtel_postcard_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Postcard initialization failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_mirror_on_drop_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Mirror on Drop initialization failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_dtel_queue_alert_init(&dtel_ctx->_queue_alert);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel queue alert initialization failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_dtel_latency_quantization_shift(
      device, DTEL_DEFAULT_LATENCY_QUANTIZATION_SHIFT);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel latency quantization shift set failed for device %d: %s",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
cleanup:
  return status;
}

switch_status_t switch_dtel_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);

  // get context
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel free failed for device %d: %s, cannot get context\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  // clear mirror sessions
  status = SWITCH_HASHTABLE_DONE(&(dtel_ctx->_mirror.sessions));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  // done with DTel hash tables
  status = SWITCH_HASHTABLE_DONE(&dtel_ctx->_int.watchlist);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT hashtable clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  status = SWITCH_HASHTABLE_DONE(&dtel_ctx->_int.sessions);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT hashtable clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  status = SWITCH_HASHTABLE_DONE(&dtel_ctx->_postcard.watchlist);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Postcard hashtable clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  status = SWITCH_HASHTABLE_DONE(&dtel_ctx->_mod.watchlist);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD hashtable clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  status = SWITCH_HASHTABLE_DONE(&dtel_ctx->_queue_alert.index_map);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Queue alert hashtable clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  // clear context
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_DTEL, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  SWITCH_FREE(device, dtel_ctx);

cleanup:

  return status;
}

static switch_status_t switch_dtel_set_port(switch_device_t device,
                                            switch_dev_port_t dev_port,
                                            switch_port_t port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  status = switch_pd_dtel_ig_port_convert_set(device, dev_port, port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("set port mapping failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_eg_port_convert_set(device, dev_port, port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("set port mapping failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_ig_port_convert_set(device, dev_port, port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("set port mapping failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_eg_port_convert_set(device, dev_port, port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("set port mapping failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
  return status;
}

switch_status_t switch_dtel_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  // init tables
  status = switch_pd_dtel_tables_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add default entries failed for device %d: %s"
        " dtel_pd_tables_init failure\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add default entries failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  // We only support one mirror session group now (default)
  status = switch_pd_dtel_mirror_session_add_group(
      device, &dtel_ctx->_mirror.default_session_grp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel add default entries failed for device %d: %s,"
        " cannot make default mirror session group\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_dtel_postcard_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Postcard default entries add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_mirror_on_drop_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD default entries add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_dtel_int_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT default entries add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  {
    // set port mapping
    switch_port_t fp_port;

    switch_api_device_info_t device_info;
    SWITCH_MEMSET(&device_info, 0x0, sizeof(switch_api_device_info_t));
    switch_uint64_t flags = SWITCH_DEVICE_ATTR_PORT_LIST;
    status = switch_api_device_attribute_get(device, flags, &device_info);
    for (switch_dev_port_t dev_port = 0; dev_port < SWITCH_MAX_PORTS;
         dev_port++) {
      status = switch_device_front_port_get(device, dev_port, &fp_port);
      if (status != SWITCH_STATUS_SUCCESS) {
        status = SWITCH_STATUS_SUCCESS;
        continue;
      }
      status = switch_dtel_set_port(device, dev_port, fp_port);
      if (status != SWITCH_STATUS_SUCCESS) {
        status = SWITCH_STATUS_SUCCESS;
        continue;
      }
    }
  }

cleanup:
  return status;
}

switch_status_t switch_dtel_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_dtel_int_default_entries_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT default entries delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_api_dtel_report_sequence_number_set_internal(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t value) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_REPORT_ENABLE
  status = switch_pd_dtel_report_sequence_number_set(
      device, mirror_session_id, value);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set sequence number failed for device %d: %s,"
        " mirror session %d\n",
        device,
        switch_error_to_string(status),
        mirror_session_id);
    return status;
  }
#endif  // P4_DTEL_REPORT_ENABLE

  return status;
}

switch_status_t switch_api_dtel_report_sequence_number_get_internal(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t *values,
    switch_uint8_t *max_num) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_REPORT_ENABLE
  status = switch_pd_dtel_report_sequence_number_get(
      device, mirror_session_id, values, max_num);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set sequence number failed for device %d: %s,"
        " mirror session %d\n",
        device,
        switch_error_to_string(status),
        mirror_session_id);
    return status;
  }
#endif  // P4_DTEL_REPORT_ENABLE

  return status;
}

switch_status_t switch_api_dtel_event_get_dscp_internal(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t *dscp) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (event_type < 0 || event_type >= SWITCH_DTEL_EVENT_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel get dscp for DTel event failed "
        " for device %d: %s invalid event type\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel get dscp for telelemety event failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  *dscp = dtel_ctx->event_infos[event_type].dscp >> 2;

  return status;
}

switch_status_t switch_api_dtel_event_set_dscp_internal(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t dscp) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (event_type < 0 || event_type >= SWITCH_DTEL_EVENT_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel set dscp for DTel event failed "
        " for device %d: %s invalid event type\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dscp > 0x3f) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel set dscp for DTel event failed "
        " for device %d: %s invalid dscp\n",
        device,
        switch_error_to_string(status));
  }
  dscp <<= 2;  // add ecn bits

  switch_dtel_context_t *dtel_ctx = NULL;

  // get context
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel set dscp for DTel event failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  dtel_ctx->event_infos[event_type].dscp = dscp;
  SWITCH_LIST_SORT(&dtel_ctx->event_infos_sorted_list,
                   switch_packet_dtel_event_dscp_compare);
  switch (event_type) {
#ifdef P4_INT_EP_ENABLE
    case SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE:
    case SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS:
    case SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG:
    case SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH:
      status = switch_pd_dtel_int_upstream_report_disable(device);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_dtel_int_upstream_report_enable(
          device, &dtel_ctx->event_infos_sorted_list);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_dtel_int_sink_local_report_disable(device);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_dtel_int_sink_local_report_enable(
          device, &dtel_ctx->event_infos_sorted_list);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_mirror_on_drop_encap_update(device,
                                                     dtel_ctx->switch_id,
                                                     dtel_ctx->dest_udp_port,
                                                     dtel_ctx->event_infos,
                                                     false,
                                                     dtel_ctx->_mod.me_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      break;
#endif /* P4_INT_EP_ENABLE */
#ifdef P4_INT_TRANSIT_ENABLE
    case SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH:
      status = switch_pd_dtel_int_transit_qalert_delete(device);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_dtel_int_transit_qalert_add(
          device,
          dtel_ctx
              ->event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH]
              .dscp);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
    case SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS:
      // update mod for all 3 above cases
      status = switch_pd_mirror_on_drop_encap_update(device,
                                                     dtel_ctx->switch_id,
                                                     dtel_ctx->dest_udp_port,
                                                     dtel_ctx->event_infos,
                                                     false,
                                                     dtel_ctx->_mod.me_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      break;
#endif /* P4_INT_TRANSIT_ENABLE */
#ifdef P4_POSTCARD_ENABLE
    case SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE:
    case SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS:
    case SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG:
    case SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH:
      status = switch_pd_dtel_postcard_e2e_enable(
          device, &dtel_ctx->event_infos_sorted_list);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      status = switch_pd_mirror_on_drop_encap_update(device,
                                                     dtel_ctx->switch_id,
                                                     dtel_ctx->dest_udp_port,
                                                     dtel_ctx->event_infos,
                                                     false,
                                                     dtel_ctx->_mod.me_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      break;
#endif
    case SWITCH_DTEL_EVENT_TYPE_DROP_REPORT:
    case SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP:
      status = switch_pd_mirror_on_drop_encap_update(device,
                                                     dtel_ctx->switch_id,
                                                     dtel_ctx->dest_udp_port,
                                                     dtel_ctx->event_infos,
                                                     false,
                                                     dtel_ctx->_mod.me_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
      break;
    default:
      break;
  }
cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel set dscp for DTel event failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_dtel_watchlist_entry_create(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  SWITCH_MT_WRAP(switch_api_dtel_watchlist_entry_create_internal(
      device, type, match_info, priority, watch, action_params))
}

switch_status_t switch_api_dtel_watchlist_entry_update(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  SWITCH_MT_WRAP(switch_api_dtel_watchlist_entry_update_internal(
      device, type, match_info, priority, watch, action_params))
}

switch_status_t switch_api_dtel_watchlist_entry_delete(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info) {
  SWITCH_MT_WRAP(
      switch_api_dtel_watchlist_entry_delete_internal(device, type, match_info))
}

switch_status_t switch_api_dtel_watchlist_clear(switch_device_t device,
                                                switch_dtel_type_t type) {
  SWITCH_MT_WRAP(switch_api_dtel_watchlist_clear_internal(device, type))
}

switch_status_t switch_api_dtel_report_session_add(
    switch_device_t device, switch_mirror_id_t mirror_id) {
  SWITCH_MT_WRAP(switch_api_dtel_report_session_add_internal(device, mirror_id))
}

switch_status_t switch_api_dtel_report_session_delete(
    switch_device_t device, switch_mirror_id_t mirror_id) {
  SWITCH_MT_WRAP(
      switch_api_dtel_report_session_delete_internal(device, mirror_id))
}

switch_status_t switch_api_dtel_queue_report_create(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth_threshold,
    switch_uint32_t latency_threshold,
    switch_uint16_t report_quota_during_breach,
    bool report_tail_drops) {
  SWITCH_MT_WRAP(
      switch_api_dtel_queue_report_create_internal(device,
                                                   port,
                                                   queue,
                                                   depth_threshold,
                                                   latency_threshold,
                                                   report_quota_during_breach,
                                                   report_tail_drops))
}

switch_status_t switch_api_dtel_queue_report_update(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth_threshold,
    switch_uint32_t latency_threshold,
    switch_uint16_t report_quota_during_breach,
    bool report_tail_drops) {
  SWITCH_MT_WRAP(
      switch_api_dtel_queue_report_update_internal(device,
                                                   port,
                                                   queue,
                                                   depth_threshold,
                                                   latency_threshold,
                                                   report_quota_during_breach,
                                                   report_tail_drops))
}

switch_status_t switch_api_dtel_queue_report_delete(switch_device_t device,
                                                    switch_port_t port,
                                                    switch_int16_t queue) {
  SWITCH_MT_WRAP(
      switch_api_dtel_queue_report_delete_internal(device, port, queue))
}

switch_status_t switch_api_dtel_queue_remaining_report_quota_during_breach_get(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint16_t *quota) {
  SWITCH_MT_WRAP(
      switch_api_dtel_queue_remaining_report_quota_during_breach_get_internal(
          device, port, queue, quota))
}

switch_status_t switch_api_dtel_flow_state_clear_cycle(switch_device_t device,
                                                       switch_uint16_t cycle) {
  SWITCH_MT_WRAP(switch_api_dtel_flow_state_clear_cycle_internal(device, cycle))
}

switch_status_t switch_api_dtel_latency_quantization_shift(
    switch_device_t device, switch_uint8_t quant_shift) {
  SWITCH_MT_WRAP(
      switch_api_dtel_latency_quantization_shift_internal(device, quant_shift))
}

switch_status_t switch_api_dtel_switch_id_set(switch_device_t device,
                                              switch_uint32_t switch_id) {
  SWITCH_MT_WRAP(switch_api_dtel_switch_id_set_internal(device, switch_id))
}

switch_status_t switch_api_dtel_report_udp_dstport_set(
    switch_device_t device, switch_uint16_t dest_udp_port) {
  SWITCH_MT_WRAP(
      switch_api_dtel_report_udp_dstport_set_internal(device, dest_udp_port))
}

switch_status_t switch_api_dtel_event_get_dscp(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t *dscp) {
  SWITCH_MT_WRAP(
      switch_api_dtel_event_get_dscp_internal(device, event_type, dscp))
}

switch_status_t switch_api_dtel_event_set_dscp(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t dscp) {
  SWITCH_MT_WRAP(
      switch_api_dtel_event_set_dscp_internal(device, event_type, dscp))
}

switch_status_t switch_api_dtel_report_sequence_number_set(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t value) {
  SWITCH_MT_WRAP(switch_api_dtel_report_sequence_number_set_internal(
      device, mirror_session_id, value));
}

switch_status_t switch_api_dtel_report_sequence_number_get(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t *values,
    switch_uint8_t *max_num) {
  SWITCH_MT_WRAP(switch_api_dtel_report_sequence_number_get_internal(
      device, mirror_session_id, values, max_num));
}
