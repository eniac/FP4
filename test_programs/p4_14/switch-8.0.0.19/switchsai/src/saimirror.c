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

#include <saimirror.h>
#include "saiinternal.h"
#include <switchapi/switch_tunnel.h>
#include <switchapi/switch_mirror.h>

static sai_api_t api_id = SAI_API_MIRROR;

static sai_mirror_session_type_t sai_switch_mirror_session_to_sai(
    switch_mirror_type_t type) {
  switch (type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      return SAI_MIRROR_SESSION_TYPE_LOCAL;
    case SWITCH_MIRROR_TYPE_REMOTE:
      return SAI_MIRROR_SESSION_TYPE_REMOTE;
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE:
      return SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;
    default:
      return SAI_MIRROR_SESSION_TYPE_LOCAL;
  }
}

static switch_mirror_type_t sai_session_to_switch_session(
    _In_ sai_mirror_session_type_t mirror_type) {
  switch (mirror_type) {
    case SAI_MIRROR_SESSION_TYPE_LOCAL:
      return SWITCH_MIRROR_TYPE_LOCAL;
    case SAI_MIRROR_SESSION_TYPE_REMOTE:
      return SWITCH_MIRROR_TYPE_REMOTE;
    case SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE:
      return SWITCH_MIRROR_TYPE_ENHANCED_REMOTE;
    default:
      return SWITCH_MIRROR_TYPE_NONE;
  }
}

static void sai_mirror_session_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _Out_ switch_api_mirror_info_t *api_mirror_info) {
  const sai_attribute_t *attribute = NULL;
  uint32_t index = 0;

  memset(api_mirror_info, 0, sizeof(switch_api_mirror_info_t));

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_MIRROR_SESSION_ATTR_TYPE:
        api_mirror_info->mirror_type =
            sai_session_to_switch_session(attribute->value.s32);
        break;
      case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
        api_mirror_info->egress_port_handle = attribute->value.oid;
        break;
      case SAI_MIRROR_SESSION_ATTR_TC:  // Unsupported
        break;
      case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
        api_mirror_info->vlan_tpid = attribute->value.u16;
        break;
      case SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID:
        api_mirror_info->vlan_tag_valid = attribute->value.booldata;
        break;
      case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
        api_mirror_info->vlan_id = attribute->value.u16;
        break;
      case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
        api_mirror_info->tos = attribute->value.u8;
        break;
      case SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE:
        api_mirror_info->span_mode = SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE;
        break;
      case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:  // Unsupported
        break;
      case SAI_MIRROR_SESSION_ATTR_TOS:
        api_mirror_info->tos = attribute->value.u8;
        break;
      case SAI_MIRROR_SESSION_ATTR_TTL:
        api_mirror_info->ttl = attribute->value.u8;
        break;
      case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &api_mirror_info->src_ip);
        break;
      case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &api_mirror_info->dst_ip);
        break;
      case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
        memcpy(&api_mirror_info->src_mac, &attribute->value.mac, 6);
        break;
      case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
        memcpy(&api_mirror_info->dst_mac, &attribute->value.mac, 6);
        break;
      case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
        break;
      case SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE:
        api_mirror_info->max_pkt_len = attribute->value.u16;
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Create mirror session.
 *
 * @param[out] session_id Port mirror session id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_create_mirror_session(_Out_ sai_object_id_t *session_id,
                                       _In_ sai_object_id_t switch_id,
                                       _In_ uint32_t attr_count,
                                       _In_ const sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_mirror_info_t api_mirror_info;
  switch_handle_t session_handle = SWITCH_API_INVALID_HANDLE;
  *session_id = SAI_NULL_OBJECT_ID;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  sai_mirror_session_attribute_parse(attr_count, attr_list, &api_mirror_info);
  status = (sai_object_id_t)switch_api_mirror_session_create(
      device, &api_mirror_info, &session_handle);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create mirror session: %s",
                  sai_status_to_string(status));
  }
  *session_id = session_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Remove mirror session.
 *
 * @param[in] session_id Port mirror session id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_remove_mirror_session(_In_ sai_object_id_t session_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(session_id) ==
             SAI_OBJECT_TYPE_MIRROR_SESSION);

  switch_status = switch_api_mirror_session_delete(device, session_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove mirror session %lx: %s",
                  session_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Set mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_set_mirror_session_attribute(
    _In_ sai_object_id_t session_id, _In_ const sai_attribute_t *attr) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_mirror_info_t api_mirror_info;
  switch_uint64_t flags = 0;
  bool mirror_session_update = FALSE;

  SAI_LOG_ENTER();

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(session_id) ==
             SAI_OBJECT_TYPE_MIRROR_SESSION);

  sai_mirror_session_attribute_parse(1, attr, &api_mirror_info);
  switch (attr->id) {
    case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
      SAI_ASSERT(sai_object_type_query(api_mirror_info.egress_port_handle) ==
                 SAI_OBJECT_TYPE_PORT);
      switch_status = switch_api_mirror_session_monitor_port_set(
          device, session_id, &api_mirror_info);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Failed to set mirror monitor port: %s",
                      sai_status_to_string(status));
        return status;
      }
      break;

    case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
    case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
      switch_status = switch_api_mirror_session_monitor_vlan_set(
          device, session_id, &api_mirror_info);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Failed to set mirror monitor vlan: %s",
                      sai_status_to_string(status));
        return status;
      }
      break;

    case SAI_MIRROR_SESSION_ATTR_VLAN_CFI:
    case SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE:
    case SAI_MIRROR_SESSION_ATTR_TC:  // Unsupported
    case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
    case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
      // Will need tunnel update API support.
      status = SAI_STATUS_NOT_IMPLEMENTED;
      break;

    case SAI_MIRROR_SESSION_ATTR_TOS:
      flags = SWITCH_MIRROR_ATTRIBUTE_TOS;
      mirror_session_update = TRUE;
      break;
    case SAI_MIRROR_SESSION_ATTR_TTL:
      flags = SWITCH_MIRROR_ATTRIBUTE_TTL;
      mirror_session_update = TRUE;
      break;
    case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
      flags = SWITCH_MIRROR_ATTRIBUTE_SRC_IP;
      mirror_session_update = TRUE;
      break;
    case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
      flags = SWITCH_MIRROR_ATTRIBUTE_DST_IP;
      mirror_session_update = TRUE;
      break;
    case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
      flags = SWITCH_MIRROR_ATTRIBUTE_SRC_MAC;
      mirror_session_update = TRUE;
      break;
    case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
      flags = SWITCH_MIRROR_ATTRIBUTE_DST_MAC;
      mirror_session_update = TRUE;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  if (mirror_session_update) {
    switch_status = switch_api_mirror_session_update(
        device, session_id, flags, &api_mirror_info);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("Failed to set erspan params: %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Get mirror session attributes.
 *
 * @param[in] session_id Port mirror session id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_get_mirror_session_attribute(
    _In_ sai_object_id_t session_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_mirror_type_t mirror_type;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_attribute_t *attr;
  switch_api_mirror_info_t *api_mirror_info = NULL;
  uint32_t i = 0;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(session_id) ==
             SAI_OBJECT_TYPE_MIRROR_SESSION);

  api_mirror_info =
      (switch_api_mirror_info_t *)malloc(sizeof(switch_api_mirror_info_t));
  if (!api_mirror_info) {
    SAI_LOG_ERROR("Invalid mirror info memory: %s",
                  sai_status_to_string(SAI_STATUS_NO_MEMORY));
    return SAI_STATUS_NO_MEMORY;
  }

  memset(api_mirror_info, 0, sizeof(switch_api_mirror_info_t));
  status =
      switch_api_mirror_session_info_get(device, session_id, api_mirror_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get mirror session info: %s",
                  sai_status_to_string(status));
    free(api_mirror_info);
    return status;
  }

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
        attr->value.oid = api_mirror_info->egress_port_handle;
        break;

      case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
        attr->value.u16 = api_mirror_info->vlan_id;
        break;

      case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
        attr->value.u8 = api_mirror_info->tos;
        break;

      case SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID:
        attr->value.booldata = api_mirror_info->vlan_tag_valid;
        break;

      case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
        sai_switch_ip_addr_to_sai_ip_addr(&attr->value.ipaddr,
                                          &api_mirror_info->src_ip);
        break;
      case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
        sai_switch_ip_addr_to_sai_ip_addr(&attr->value.ipaddr,
                                          &api_mirror_info->dst_ip);
        break;
      case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
        memcpy(&attr->value.mac, &api_mirror_info->src_mac, 6);
        break;
      case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
        memcpy(&attr->value.mac, &api_mirror_info->dst_mac, 6);
        break;

      case SAI_MIRROR_SESSION_ATTR_TOS:
        attr->value.u8 = api_mirror_info->tos;
        break;

      case SAI_MIRROR_SESSION_ATTR_TTL:
        attr->value.u8 = api_mirror_info->ttl;
        break;

      case SAI_MIRROR_SESSION_ATTR_TYPE:
        switch_status = switch_api_mirror_session_type_get(
            device, session_id, &mirror_type);
        status = sai_switch_status_to_sai_status(switch_status);

        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get mirror session type : %s",
                        sai_status_to_string(status));
          free(api_mirror_info);
          return status;
        }
        attr->value.s32 = sai_switch_mirror_session_to_sai(mirror_type);
        break;
    }
  }
  SAI_LOG_EXIT();

  free(api_mirror_info);
  return status;
}

/*
*  Mirror API methods table retrieved with sai_api_query()
*/
sai_mirror_api_t mirror_api = {
    .create_mirror_session = sai_create_mirror_session,
    .remove_mirror_session = sai_remove_mirror_session,
    .set_mirror_session_attribute = sai_set_mirror_session_attribute,
    .get_mirror_session_attribute = sai_get_mirror_session_attribute,
};

sai_status_t sai_mirror_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing mirror");
  sai_api_service->mirror_api = mirror_api;
  return SAI_STATUS_SUCCESS;
}
