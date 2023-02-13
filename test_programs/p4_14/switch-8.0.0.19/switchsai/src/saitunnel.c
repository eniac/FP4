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

#include <saitunnel.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_tunnel.h>

static sai_api_t api_id = SAI_API_TUNNEL;

sai_status_t sai_tunnel_to_switch_tunnel_type(
    sai_tunnel_type_t sai_tunnel, switch_tunnel_type_t *switch_tunnel) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  switch (sai_tunnel) {
    case SAI_TUNNEL_TYPE_IPINIP:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_IPIP;
      break;
    case SAI_TUNNEL_TYPE_IPINIP_GRE:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_GRE;
      break;
    case SAI_TUNNEL_TYPE_VXLAN:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_VXLAN;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
  }
  return status;
}

switch_tunnel_map_type_t sai_tunnel_map_type_to_switch_tunnel_map_type(
    sai_tunnel_map_type_t map_type, switch_tunnel_map_type_t *switch_map_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (map_type) {
    case SAI_TUNNEL_MAP_TYPE_OECN_TO_UECN:
    case SAI_TUNNEL_MAP_TYPE_UECN_OECN_TO_OECN:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
    case SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE;
      break;
    case SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI;
      break;
    case SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE;
      break;
    case SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI;
      break;
    case SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE;
      break;
    case SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI:
      *switch_map_type = SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}
/**
 * @brief Create tunnel Map
 *
 * @param[out] tunnel_map_id Tunnel Map Id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_create_tunnel_map(_Out_ sai_object_id_t *tunnel_map_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  const sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_tunnel_map_type_t tunnel_map_type = 0;
  switch_api_tunnel_mapper_t tunnel_mapper = {0};
  switch_handle_t tunnel_mapper_handle = SWITCH_API_INVALID_HANDLE;
  uint32_t index = 0;

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_TUNNEL_MAP_ATTR_TYPE:
        status = sai_tunnel_map_type_to_switch_tunnel_map_type(
            attribute->value.s32, &tunnel_map_type);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "sai create tunnel map failed: "
              "tunnel map type mapping failed:(%s)\n",
              sai_status_to_string(status));
          return status;
        }
        break;
      default:
        break;
    }
  }

  tunnel_mapper.tunnel_map_type = tunnel_map_type;

  switch_status = switch_api_tunnel_mapper_create(
      device, &tunnel_mapper, &tunnel_mapper_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel mapper create failed: "
        "tunnel map type %d"
        "switch api tunnel mapper create failed: \n",
        tunnel_map_type,
        sai_status_to_string(status));
    return status;
  }

  *tunnel_map_id = tunnel_mapper_handle;

  return status;
}

/**
 * @brief Remove tunnel Map
 *
 * @param[in] tunnel_map_id Tunnel Map id to be removed
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_map(_In_ sai_object_id_t tunnel_map_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(tunnel_map_id) ==
             SAI_OBJECT_TYPE_TUNNEL_MAP);

  switch_status = switch_api_tunnel_mapper_delete(device, tunnel_map_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel mapper delete failed: "
        "switch api tunnel mapper delete failed: \n",
        sai_status_to_string(status));
    return status;
  }

  return status;
}

/**
 * @brief Set attributes for tunnel map
 *
 * @param[in] tunnel_map_id Tunnel Map Id
 * @param[in] attr Attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_map_attribute(_In_ sai_object_id_t tunnel_map_id,
                                          _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief Get attributes of tunnel map
 *
 * @param[in] tunnel_map_id Tunnel map id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_map_attribute(_In_ sai_object_id_t tunnel_map_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

sai_status_t sai_tunnel_attribute_parse(uint32_t attr_count,
                                        const sai_attribute_t *attr_list,
                                        switch_api_tunnel_info_t *tunnel_info) {
  const sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_TUNNEL_ATTR_TYPE:
        status = sai_tunnel_to_switch_tunnel_type(attribute->value.s32,
                                                  &tunnel_info->tunnel_type);
        SAI_ASSERT(status == SAI_STATUS_SUCCESS);
        break;
      case SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE:
        tunnel_info->underlay_rif_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_ATTR_OVERLAY_INTERFACE:
        tunnel_info->overlay_rif_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_ATTR_ENCAP_SRC_IP:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &tunnel_info->src_ip);
        break;
      case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY:
        tunnel_info->gre_key = attribute->value.u32;
        break;
      case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID:
        break;
      case SAI_TUNNEL_ATTR_ENCAP_MAPPERS:
        SAI_ASSERT(attribute->value.objlist.count == 1);
        tunnel_info->encap_mapper_handle = attribute->value.objlist.list[0];
        break;
      case SAI_TUNNEL_ATTR_DECAP_MAPPERS:
        SAI_ASSERT(attribute->value.objlist.count == 1);
        tunnel_info->decap_mapper_handle = attribute->value.objlist.list[0];
        break;
      case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
      case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
      case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
      case SAI_TUNNEL_ATTR_DECAP_ECN_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_ECN_MODE:
      default:
        break;
    }
  }
  return status;
}

/**
 * @brief Create tunnel
 *
 * @param[out] tunnel_id Tunnel id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_create_tunnel(_Out_ sai_object_id_t *tunnel_id,
                               _In_ sai_object_id_t switch_id,
                               _In_ uint32_t attr_count,
                               _In_ const sai_attribute_t *attr_list) {
  switch_api_tunnel_info_t tunnel_info = {0};
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_interface_info_t intf_info;
  switch_handle_t handle;

  status = sai_tunnel_attribute_parse(attr_count, attr_list, &tunnel_info);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel create failed: "
        "parsing failed:(%s)\n",
        sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_tunnel_create(device, &tunnel_info, &tunnel_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel create failed: "
        "switch tunnel create failed:(%s)\n",
        sai_status_to_string(status));
    return status;
  }

  SAI_MEMSET(&intf_info, 0, sizeof(switch_api_interface_info_t));
  intf_info.handle = tunnel_handle;
  intf_info.type = SWITCH_INTERFACE_TYPE_TUNNEL;
  switch_status = switch_api_interface_create(device, &intf_info, &handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel create failed: "
        "switch tunnel create failed:(%s)\n",
        sai_status_to_string(status));
    return status;
  }

  *tunnel_id = tunnel_handle;

  SAI_LOG_DEBUG("tunnel created: 0x%lx\n", tunnel_handle);

  return status;
}

/**
 * @brief Remove tunnel
 *
 * @param[in] tunnel_id Tunnel id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel(_In_ sai_object_id_t tunnel_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t intf_handle;
  SAI_ASSERT(sai_object_type_query(tunnel_id) == SAI_OBJECT_TYPE_TUNNEL);

  switch_status =
      switch_api_tunnel_interface_get(device, tunnel_id, &intf_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel delete failed: "
        "switch api tunnel intf get failed: \n",
        sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_interface_delete(device, intf_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel delete failed: "
        "switch api tunnel intf delete failed: \n",
        sai_status_to_string(status));
  }

  switch_status = switch_api_tunnel_delete(device, tunnel_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel delete failed: "
        "switch api tunnel delete failed: \n",
        sai_status_to_string(status));
    return status;
  }

  SAI_LOG_DEBUG("tunnel deleted: 0x%lx\n", tunnel_id);

  return status;
}

/**
 * @brief Set tunnel attribute
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_attribute(_In_ sai_object_id_t tunnel_id,
                                      _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief Get tunnel attributes
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_attribute(_In_ sai_object_id_t tunnel_id,
                                      _In_ uint32_t attr_count,
                                      _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

#if 0
/**
 * @brief Get tunnel statistics counters.
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_stats(_In_ sai_object_id_t tunnel_id,
                                   _In_ uint32_t number_of_counters,
                                   _In_ const sai_tunnel_stat_t *counter_ids,
                                   _Out_ uint64_t *counters) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief Clear tunnel statistics counters.
 *
 * @param[in] tunnel_id Tunnel id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_tunnel_stats(
    _In_ sai_object_id_t tunnel_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_tunnel_stat_t *counter_ids) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}
#endif

sai_status_t switch_tunnel_to_sai_tunnel_type(
    switch_tunnel_type_t switch_tunnel, sai_tunnel_type_t *sai_tunnel) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (switch_tunnel) {
    case SWITCH_TUNNEL_TYPE_IPIP:
      *sai_tunnel = SAI_TUNNEL_TYPE_IPINIP;
      break;
    case SWITCH_TUNNEL_TYPE_GRE:
      *sai_tunnel = SAI_TUNNEL_TYPE_IPINIP_GRE;
      break;
    case SWITCH_TUNNEL_TYPE_VXLAN:
      *sai_tunnel = SAI_TUNNEL_TYPE_VXLAN;
      break;
    default:
      *sai_tunnel = 0;
      break;
  }
  return status;
}

char *sai_tunnel_string(sai_tunnel_type_t sai_tunnel_type) {
  switch (sai_tunnel_type) {
    case SAI_TUNNEL_TYPE_IPINIP:
      return "IPinIP";
    case SAI_TUNNEL_TYPE_IPINIP_GRE:
      return "GRE";
    case SAI_TUNNEL_TYPE_VXLAN:
      return "Vxlan";
    case SAI_TUNNEL_TYPE_MPLS:
      return "Mpls";
    default:
      return "Unsupported";
  }
}

sai_status_t sai_tunnel_term_to_switch_type(
    sai_tunnel_term_table_entry_type_t sai_type,
    switch_tunnel_term_entry_type_t *switch_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (sai_type) {
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P:
      *switch_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P;
      break;
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP:
      *switch_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
  }
  return status;
}

sai_status_t switch_tunnel_term_to_sai_type(
    switch_tunnel_term_entry_type_t switch_type,
    sai_tunnel_term_table_entry_type_t *sai_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (switch_type) {
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P:
      *sai_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P;
      break;
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP:
      *sai_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
  }
  return status;
}

char *switch_tunnel_term_type_to_string(
    switch_tunnel_term_entry_type_t switch_type) {
  switch (switch_type) {
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P:
      return "term_type_p2p";
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP:
      return "term_type_p2mp";
    default:
      return "unsupported";
  }
}

static void sai_tunnel_ip_to_logstring(char *logStr, sai_ip_address_t sai_ip) {
  char str[SAI_MAX_ENTRY_STRING_LEN];
  int count = 0;
  sai_ipaddress_to_string(sai_ip, SAI_MAX_ENTRY_STRING_LEN, str, &count);
  SAI_LOG_DEBUG("%s: %s", logStr, str);
}
/**
 * @brief Create tunnel termination table entry
 *
 * @param[out] tunnel_term_table_entry_id Tunnel termination table entry id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_create_tunnel_term_table_entry(
    _Out_ sai_object_id_t *tunnel_term_table_entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_tunnel_term_info_t api_term_info = {0};
  switch_tunnel_term_entry_type_t switch_tunnel_term_type = 0;
  switch_tunnel_type_t switch_tunnel_type = 0;
  const sai_attribute_t *attr = NULL;
  switch_handle_t tunnel_term_handle = SWITCH_API_INVALID_HANDLE;
  unsigned int index = 0;

  SAI_LOG_ENTER();

  SAI_MEMSET(&api_term_info, 0, sizeof(switch_api_tunnel_term_info_t));
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
        api_term_info.vrf_handle = (switch_handle_t)attr->value.oid;
        SAI_LOG_DEBUG("VRF handle 0x%lx", attr->value.oid);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
        sai_ip_addr_to_switch_ip_addr(&attr->value.ipaddr,
                                      &api_term_info.dst_ip);
        sai_tunnel_ip_to_logstring("Tunnel Dip", attr->value.ipaddr);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
        sai_ip_addr_to_switch_ip_addr(&attr->value.ipaddr,
                                      &api_term_info.src_ip);
        sai_tunnel_ip_to_logstring("Tunnel Sip", attr->value.ipaddr);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
        api_term_info.tunnel_handle = (switch_handle_t)attr->value.oid;
        SAI_LOG_DEBUG("Tunnel obj handle 0x%lx", attr->value.oid);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
        status = sai_tunnel_term_to_switch_type(attr->value.s32,
                                                &switch_tunnel_term_type);
        if (status == SAI_STATUS_SUCCESS) {
          api_term_info.term_entry_type = switch_tunnel_term_type;
        }
        SAI_LOG_DEBUG("Tunnel type %s", sai_tunnel_string(attr->value.s32));
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
        status = sai_tunnel_to_switch_tunnel_type(attr->value.s32,
                                                  &switch_tunnel_type);
        if (status == SAI_STATUS_SUCCESS) {
          api_term_info.tunnel_type = switch_tunnel_type;
        }
        SAI_LOG_DEBUG("Tunnel term type %s",
                      switch_tunnel_term_type_to_string(attr->value.s32));
        break;
      default:
        break;
    }
  }
  switch_status = switch_api_tunnel_term_create(
      device, &api_term_info, &tunnel_term_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to create tunnel term table entry");
    return status;
  }
  SAI_LOG_DEBUG("Tunnel term table entry create success, handle 0x%lx",
                tunnel_term_handle);

  *tunnel_term_table_entry_id = tunnel_term_handle;

  SAI_LOG_EXIT();
  return status;
}

/**
 * @brief Remove tunnel termination table entry
 *
 * @param[in] tunnel_term_table_entry_id Tunnel termination table entry id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_term_table_entry(
    _In_ sai_object_id_t tunnel_term_table_entry_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  switch_status =
      switch_api_tunnel_term_delete(device, tunnel_term_table_entry_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to delete tunnel term table entry");
    return status;
  }

  SAI_LOG_DEBUG("Tunnel term table entry delete success, handle 0x%lx",
                tunnel_term_table_entry_id);

  return status;
}

/**
 * @brief Set tunnel termination table entry attribute
 *
 * @param[in] tunnel_term_table_entry_id Tunnel termination table entry id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_term_table_entry_attribute(
    _In_ sai_object_id_t tunnel_term_table_entry_id,
    _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief Get tunnel termination table entry attributes
 *
 * @param[in] tunnel_term_table_entry_id Tunnel termination table entry id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_term_table_entry_attribute(
    _In_ sai_object_id_t tunnel_term_table_entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_tunnel_term_info_t api_term_info;
  sai_attribute_t *attr;
  unsigned int index = 0;
  sai_tunnel_term_table_entry_type_t sai_term_type;
  sai_tunnel_type_t sai_tunnel_type;

  SAI_LOG_ENTER();

  SAI_MEMSET(&api_term_info, 0, sizeof(switch_api_tunnel_term_info_t));
  switch_status = switch_api_tunnel_term_get(
      device, (switch_handle_t)tunnel_term_table_entry_id, &api_term_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to get tunnel term table attribute for handle 0x%lx: %s",
        tunnel_term_table_entry_id,
        sai_status_to_string(status));
    return status;
  }
  for (index = 0, attr = attr_list; index < attr_count; index++, attr++) {
    switch (attr->id) {
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
        attr->value.oid = (sai_object_id_t)api_term_info.vrf_handle;
        SAI_LOG_DEBUG("VRF handle 0x%lx", attr->value.oid);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
        sai_switch_ip_addr_to_sai_ip_addr(&attr->value.ipaddr,
                                          &api_term_info.dst_ip);
        sai_tunnel_ip_to_logstring("Tunnel Dip", attr->value.ipaddr);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
        sai_switch_ip_addr_to_sai_ip_addr(&attr->value.ipaddr,
                                          &api_term_info.src_ip);
        sai_tunnel_ip_to_logstring("Tunnel Sip", attr->value.ipaddr);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
        attr->value.oid = (sai_object_id_t)api_term_info.tunnel_handle;
        SAI_LOG_DEBUG("Tunnel obj handle 0x%lx", attr->value.oid);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
        status = switch_tunnel_to_sai_tunnel_type(api_term_info.tunnel_type,
                                                  &sai_tunnel_type);
        if (status == SAI_STATUS_SUCCESS) {
          attr->value.s32 = sai_tunnel_type;
        }
        SAI_LOG_DEBUG("Tunnel type %s", sai_tunnel_string(attr->value.s32));
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
        status = switch_tunnel_term_to_sai_type(api_term_info.term_entry_type,
                                                &sai_term_type);
        if (status == SAI_STATUS_SUCCESS) {
          attr->value.s32 = sai_term_type;
        }
        SAI_LOG_DEBUG("Tunnel term type %s",
                      switch_tunnel_term_type_to_string(attr->value.s32));
        break;
      default:
        break;
    }
  }
  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_tunnel_map_entry_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    switch_api_tunnel_mapper_entry_t *mapper_entry) {
  uint32_t index = 0;
  const sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE:
        status = sai_tunnel_map_type_to_switch_tunnel_map_type(
            attribute->value.s32, &mapper_entry->tunnel_map_type);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "sai tunnel map entry parse failed: "
              "tunnel map type mapping failed:(%s)\n",
              sai_status_to_string(status));
          return status;
        }
        break;
      case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP:
        mapper_entry->tunnel_mapper_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_KEY:
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_VALUE:
        switch_api_vlan_id_to_handle_get(
            device, attribute->value.u16, &mapper_entry->vlan_handle);
        break;
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY:
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE:
        mapper_entry->tunnel_vni = attribute->value.u32;
        break;
      case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY:
      case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE:
        mapper_entry->ln_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY:
      case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE:
        mapper_entry->vrf_handle = attribute->value.oid;
        break;
      default:
        break;
    }
  }

  return status;
}

/**
 * @brief Create tunnel map item
 *
 * @param[out] tunnel_map_entry_id Tunnel map item id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_create_tunnel_map_entry(
    _Out_ sai_object_id_t *tunnel_map_entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  switch_api_tunnel_mapper_entry_t mapper_entry = {0};
  switch_handle_t mapper_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_tunnel_map_entry_parse(attr_count, attr_list, &mapper_entry);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel map create entry failed: "
        "parsing failed:(%s)\n",
        sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_tunnel_mapper_entry_create(
      device, &mapper_entry, &mapper_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel map create entry failed: "
        "switch tunnel mapper create failed:(%s)\n",
        sai_status_to_string(status));
    return status;
  }

  *tunnel_map_entry_id = mapper_handle;

  SAI_LOG_DEBUG("tunnel map entry created: 0x%lx\n", mapper_handle);

  return status;
}

/**
 * @brief Remove tunnel map item
 *
 * @param[in] tunnel_map_entry_id Tunnel map item id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_map_entry(_In_ sai_object_id_t
                                             tunnel_map_entry_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  SAI_ASSERT(sai_object_type_query(tunnel_map_entry_id) ==
             SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY);

  switch_status =
      switch_api_tunnel_mapper_entry_delete(device, tunnel_map_entry_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "sai tunnel mapper entry delete failed: "
        "switch api tunnel mapper entry delete failed: \n",
        sai_status_to_string(status));
    return status;
  }

  SAI_LOG_DEBUG("tunnel map entry deleted: 0x%lx\n", tunnel_map_entry_id);

  return status;
}

/**
 * @brief Set tunnel map item attribute
 *
 * @param[in] tunnel_map_entry_id Tunnel map item id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_map_entry_attribute(
    _In_ sai_object_id_t tunnel_map_entry_id,
    _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief Get tunnel map item attributes
 *
 * @param[in] tunnel_map_entry_id Tunnel map item id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_map_entry_attribute(
    _In_ sai_object_id_t tunnel_map_entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/**
 * @brief TUNNEL method table retrieved with sai_api_query()
 */
sai_tunnel_api_t tunnel_api = {
    .create_tunnel_map = sai_create_tunnel_map,
    .remove_tunnel_map = sai_remove_tunnel_map,
    .set_tunnel_map_attribute = sai_set_tunnel_map_attribute,
    .get_tunnel_map_attribute = sai_get_tunnel_map_attribute,
    .create_tunnel = sai_create_tunnel,
    .remove_tunnel = sai_remove_tunnel,
    .set_tunnel_attribute = sai_set_tunnel_attribute,
    .get_tunnel_attribute = sai_get_tunnel_attribute,
    //.get_tunnel_stats = sai_get_tunnel_stats,
    //.clear_tunnel_stats = sai_clear_tunnel_stats,
    .create_tunnel_term_table_entry = sai_create_tunnel_term_table_entry,
    .remove_tunnel_term_table_entry = sai_remove_tunnel_term_table_entry,
    .set_tunnel_term_table_entry_attribute =
        sai_set_tunnel_term_table_entry_attribute,
    .get_tunnel_term_table_entry_attribute =
        sai_get_tunnel_term_table_entry_attribute,
    .create_tunnel_map_entry = sai_create_tunnel_map_entry,
    .remove_tunnel_map_entry = sai_remove_tunnel_map_entry,
    .set_tunnel_map_entry_attribute = sai_set_tunnel_map_entry_attribute,
    .get_tunnel_map_entry_attribute = sai_get_tunnel_map_entry_attribute};

sai_status_t sai_tunnel_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing tunnel");
  sai_api_service->tunnel_api = tunnel_api;
  return SAI_STATUS_SUCCESS;
}
