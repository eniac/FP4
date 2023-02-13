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

#include <saibridge.h>
#include <saivlan.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_l2.h>
#include <switchapi/switch_vlan.h>
#include <switchapi/switch_interface.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_ln.h>
#include <switchapi/switch_tunnel.h>

static sai_api_t api_id = SAI_API_BRIDGE;

/* This handle is only used to refer to 1Q bridge.  */
static switch_handle_t DEFAULT_BRIDGE_1Q = SWITCH_API_INVALID_HANDLE;
tommy_list dot1q_bridgeport;
struct bridge_port {
  tommy_node node;
  switch_handle_t handle;
};

switch_handle_t sai_bridge_get_default1q_bridge() { return DEFAULT_BRIDGE_1Q; }

#define SAI_VLAN_ID_MAX 4095
/**
 * @brief Create bridge port
 *
 * @param[out] bridge_port_id Bridge port ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_create_bridge_port(_Out_ sai_object_id_t *bridge_port_id,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  sai_bridge_port_type_t sai_bridge_port_type = 0;
  switch_handle_t port_lag_handle = SWITCH_API_INVALID_HANDLE;
  sai_uint16_t vlan_id = 0;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bridge_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t interface_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t intf_api_info = {0};
  const sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  *bridge_port_id = SAI_NULL_OBJECT_ID;
  bridge_handle = sai_bridge_get_default1q_bridge();
  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (unsigned index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_BRIDGE_PORT_ATTR_TYPE:
        sai_bridge_port_type = attribute->value.s32;
        break;
      case SAI_BRIDGE_PORT_ATTR_PORT_ID:
        port_lag_handle = attr_list[index].value.oid;
        break;
      case SAI_BRIDGE_PORT_ATTR_VLAN_ID:
        vlan_id = attr_list[index].value.u16;
        break;
      case SAI_BRIDGE_PORT_ATTR_RIF_ID:
        rif_handle = attr_list[index].value.oid;
        break;
      case SAI_BRIDGE_PORT_ATTR_TUNNEL_ID:
        tunnel_handle = attr_list[index].value.oid;
        break;
      case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
        bridge_handle = attr_list[index].value.oid;
        break;
      case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
        break;
      case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:      // Unsupported
      case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:  // Unsupported
      case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:  // Unsupported
      default:
        //        return SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }
  if (bridge_handle == SWITCH_API_INVALID_HANDLE) {
    SAI_LOG_ERROR("unexpected attribute : %s", sai_status_to_string(status));
    return SAI_STATUS_INVALID_PARAMETER;
  }

  switch (sai_bridge_port_type) {
    case SAI_BRIDGE_PORT_TYPE_PORT:
      /* must be 1Q bridge */

      if (port_lag_handle == SWITCH_API_INVALID_HANDLE) {
        SAI_LOG_ERROR("unexpected attribute : %s",
                      sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }

      intf_api_info.handle = port_lag_handle;
      intf_api_info.type = (bridge_handle == DEFAULT_BRIDGE_1Q)
                               ? SWITCH_INTERFACE_TYPE_TRUNK
                               : SWITCH_INTERFACE_TYPE_ACCESS;

      break;

    case SAI_BRIDGE_PORT_TYPE_SUB_PORT:
      /* Must be 1D bridge */

      if (bridge_handle == DEFAULT_BRIDGE_1Q) {
        SAI_LOG_ERROR("unexpected attribute : %s",
                      sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }
      if (port_lag_handle == SWITCH_API_INVALID_HANDLE) {
        SAI_LOG_ERROR("unexpected attribute : %s",
                      sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }
      if (vlan_id >= SAI_VLAN_ID_MAX) {
        SAI_LOG_ERROR("unexpected attribute : %s",
                      sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }
      intf_api_info.handle = port_lag_handle;
      intf_api_info.vlan = vlan_id;
      intf_api_info.type = SWITCH_INTERFACE_TYPE_PORT_VLAN;
      break;

    case SAI_BRIDGE_PORT_TYPE_1Q_ROUTER:  // Unsupported
      // TODO:
      return SAI_STATUS_NOT_SUPPORTED;
      break;

    case SAI_BRIDGE_PORT_TYPE_1D_ROUTER:
      /*
       *  SAI view:
       *    Bridge ~~~ Bridge port ~~~ RIF ~~~ VRF
       *
       *  API view:
       *    Bridge ~~~ RIF ~~~ VRF
       *
       *    We create a intf of type None to satisfy SAI view.
       *    This bridge port is not useful in any other way.
       *
       */

      if (rif_handle == SWITCH_API_INVALID_HANDLE) {
        SAI_LOG_ERROR("null attribute : %s", sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }
      intf_api_info.type = SWITCH_INTERFACE_TYPE_NONE;
      intf_api_info.rif_handle = rif_handle;
      switch_status =
          switch_api_rif_attach_ln(device, rif_handle, bridge_handle);
      if ((status = sai_switch_status_to_sai_status(switch_status)) !=
          SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to create  interface: %s",
                      sai_status_to_string(status));
      }

      break;
    case SAI_BRIDGE_PORT_TYPE_TUNNEL:
      switch_status = switch_api_tunnel_interface_get(
          device, tunnel_handle, &interface_handle);
      if ((status = sai_switch_status_to_sai_status(switch_status)) !=
          SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to create  interface: %s",
                      sai_status_to_string(status));
        return SAI_STATUS_INVALID_PARAMETER;
      }
      *bridge_port_id = interface_handle;
      goto end;
    default:
      return SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  switch_status =
      switch_api_interface_create(device, &intf_api_info, &interface_handle);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create  interface: %s",
                  sai_status_to_string(status));
    if (switch_status == SWITCH_STATUS_ITEM_ALREADY_EXISTS) {
      switch_api_interface_by_type_get(
          device, intf_api_info.handle, intf_api_info.type, &interface_handle);
      status = SAI_STATUS_SUCCESS;
    }
  }

  if ((bridge_handle != DEFAULT_BRIDGE_1Q) &&
      ((sai_bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT) ||
       (sai_bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT))) {
    switch_status = switch_api_logical_network_member_add(
        device, bridge_handle, interface_handle);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to create  interface: %s",
                    sai_status_to_string(status));
    }
  }
  *bridge_port_id = interface_handle;

  if (bridge_handle == DEFAULT_BRIDGE_1Q) {
    struct bridge_port *bp = SAI_MALLOC(sizeof(struct bridge_port));
    bp->handle = *bridge_port_id;
    tommy_list_insert_tail(&dot1q_bridgeport, &bp->node, bp);
  }

end:
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Remove bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_remove_bridge_port(_In_ sai_object_id_t bridge_port_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_LOG_ENTER();
  switch_handle_t bridge_id = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t api_intf_info = {0};
  switch_handle_t interface_handle = bridge_port_id;

  switch_status = switch_api_interface_attribute_get(
      device, bridge_port_id, (switch_uint64_t)UINT64_MAX, &api_intf_info);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    return status;
  }

  if (api_intf_info.type == SWITCH_INTERFACE_TYPE_TUNNEL) {
    /* Nothing to do here. Tunnel delete will remove the interface */
    return SAI_STATUS_SUCCESS;
  }

  switch_status =
      switch_api_interface_ln_handle_get(device, interface_handle, &bridge_id);

  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed remove bridge port: %s",
                  sai_status_to_string(status));
    return status;
  }

  if (api_intf_info.type == SWITCH_INTERFACE_TYPE_NONE) {
    /* 1D bp-rif */
    /* An interface of type SWITCH_INTERFACE_TYPE_NONE is used to satisfy
     * SAI. refer to description in  sai_create_bridge_port*/
    switch_status = switch_api_rif_dettach_ln(device, api_intf_info.rif_handle);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed remove bridge port: %s",
                    sai_status_to_string(status));
      return status;
    }
  } else if (bridge_id != DEFAULT_BRIDGE_1Q &&
             bridge_id != SWITCH_API_INVALID_HANDLE) {
    /* 1D bridge port, non rif, remove from LN */
    switch_status = switch_api_logical_network_member_remove(
        device, bridge_id, interface_handle);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to create  interface: %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  // delete from default dot1qbridge list
  if (bridge_id == DEFAULT_BRIDGE_1Q) {
    struct bridge_port *bp = NULL;
    tommy_node *node;
    node = tommy_list_head(&dot1q_bridgeport);
    while (node) {
      bp = (struct bridge_port *)node->data;
      if (bp->handle == bridge_port_id) {
        break;
      }
      node = node->next;
    }
    if (node) {
      // remove node
      struct bridge_port *fbp =
          tommy_list_remove_existing(&dot1q_bridgeport, node);
      SAI_FREE(fbp);
    }
  }
  switch_status = switch_api_interface_delete(device, interface_handle);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete interface: %s",
                  sai_status_to_string(status));
    return status;
  }
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Set attribute for bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_set_bridge_port_attribute(_In_ sai_object_id_t bridge_port_id,
                                           _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  if (!attr) {
    SAI_LOG_ERROR("null attribute : %s", sai_status_to_string(status));
    return SAI_STATUS_INVALID_PARAMETER;
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get attributes of bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_get_bridge_port_attribute(_In_ sai_object_id_t bridge_port_id,
                                           _In_ uint32_t attr_count,
                                           _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }
  for (unsigned int i = 0; i < attr_count; i++) {
    switch_handle_t port_id = 0;
    switch (attr_list[i].id) {
      case SAI_BRIDGE_PORT_ATTR_TYPE:
        attr_list[i].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;
        break;
      case SAI_BRIDGE_PORT_ATTR_PORT_ID:
        switch_api_interface_handle_get(0, bridge_port_id, &port_id);
        attr_list[i].value.oid = port_id;
        break;
    }
  }

  SAI_LOG_EXIT();

  return status;
}
/**
 * @brief Create bridge
 *
 * @param[out] bridge_id Bridge ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_create_bridge(_Out_ sai_object_id_t *bridge_id,
                               _In_ sai_object_id_t switch_id,
                               _In_ uint32_t attr_count,
                               _In_ const sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  const sai_attribute_t *attribute;
  sai_bridge_type_t sai_bridge_type;
  sai_uint32_t max_learned_addresses = 0;
  bool learning_disable = FALSE;
  switch_handle_t ln_handle = SWITCH_API_INVALID_HANDLE;
  *bridge_id = SAI_NULL_OBJECT_ID;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  attribute = get_attr_from_list(SAI_BRIDGE_ATTR_TYPE, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
    return status;
  }
  sai_bridge_type = attribute->value.s32;

  for (unsigned index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_BRIDGE_ATTR_TYPE:
        break;
      case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
        max_learned_addresses = attr_list[index].value.u32;
        (void)max_learned_addresses;
        // ToDo: add support
        //        return SAI_STATUS_NOT_SUPPORTED;
        break;
      case SAI_BRIDGE_ATTR_LEARN_DISABLE:
        if (sai_bridge_type == SAI_BRIDGE_TYPE_1Q) {
          //          return SAI_STATUS_NOT_SUPPORTED;
        }
        learning_disable = attr_list[index].value.booldata;
        break;
      default:
        //        return SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  if (sai_bridge_type == SAI_BRIDGE_TYPE_1Q) {
    *bridge_id = DEFAULT_BRIDGE_1Q;
  } else {
    switch_status = switch_api_logical_network_create(switch_id, &ln_handle);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to create  bridge: %s",
                    sai_status_to_string(status));
      return status;
    }
    switch_status = switch_api_logical_network_learning_set(
        switch_id, ln_handle, !learning_disable);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to create  bridge: %s",
                    sai_status_to_string(status));
      return status;
    }
    *bridge_id = ln_handle;
  }
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Remove bridge
 *
 * @param[in] bridge_id Bridge ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_remove_bridge(_In_ sai_object_id_t bridge_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_LOG_ENTER();
  switch_device_t device = 0;
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete  bridge: %s", sai_status_to_string(status));
    return status;
  }

  if (bridge_id == DEFAULT_BRIDGE_1Q) {
    return SAI_STATUS_NOT_SUPPORTED;
  } else {
    switch_status = switch_api_logical_network_delete(device, bridge_id);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to delete  bridge: %s",
                    sai_status_to_string(status));
      return status;
    }
  }
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Set attribute for bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_set_bridge_attribute(_In_ sai_object_id_t bridge_id,
                                      _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute : %s", sai_status_to_string(status));
    return status;
  }

  status = SAI_STATUS_NOT_SUPPORTED;
  SAI_LOG_EXIT();

  return status;
}

static sai_status_t switch_bridge_counters_to_sai_bridge_counters(
    _In_ uint32_t number_of_counters,
    _In_ const sai_bridge_stat_t *counter_ids,
    _In_ switch_counter_t *switch_counters,
    _Out_ uint64_t *counters) {
  uint32_t index = 0;
  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_BRIDGE_STAT_IN_OCTETS:
        counters[index] = switch_counters[SWITCH_BD_STATS_IN_UCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_IN_MCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_IN_BCAST].num_bytes;
        break;
      case SAI_BRIDGE_STAT_IN_PACKETS:
        counters[index] =
            switch_counters[SWITCH_BD_STATS_IN_UCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_IN_MCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_IN_BCAST].num_packets;
        break;
      case SAI_BRIDGE_STAT_OUT_OCTETS:
        counters[index] = switch_counters[SWITCH_BD_STATS_OUT_UCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_OUT_MCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_OUT_BCAST].num_bytes;
        break;
      case SAI_BRIDGE_STAT_OUT_PACKETS:
        counters[index] =
            switch_counters[SWITCH_BD_STATS_OUT_UCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_OUT_MCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_OUT_BCAST].num_packets;
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

/**
 * @brief Get bridge statistics counters.
 *
 * @param[in] bridge_id Bridge id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_stats(_In_ sai_object_id_t bridge_id,
                                  _In_ uint32_t number_of_counters,
                                  _In_ const sai_bridge_stat_t *counter_ids,
                                  _Out_ uint64_t *counters) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_counter_t *switch_counters = NULL;
  switch_bd_counter_id_t *stat_ids = NULL;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  switch_counters = SAI_MALLOC(sizeof(switch_counter_t) * SWITCH_BD_STATS_MAX);
  if (!switch_counters) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get bridge stats 0x%lx %s",
                  bridge_id,
                  sai_status_to_string(status));
    return status;
  }

  stat_ids = SAI_MALLOC(sizeof(switch_bd_counter_id_t) * SWITCH_BD_STATS_MAX);
  if (!stat_ids) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get bridge stats 0x%lx %s",
                  bridge_id,
                  sai_status_to_string(status));
    SAI_FREE(switch_counters);
    return status;
  }

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    stat_ids[index] = index;
  }

  switch_status = switch_api_logical_network_stats_get(
      device, bridge_id, SWITCH_BD_STATS_MAX, stat_ids, switch_counters);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get bridge stats 0x%lx %s",
                  bridge_id,
                  sai_status_to_string(status));
    SAI_FREE(stat_ids);
    SAI_FREE(switch_counters);
    return status;
  }

  switch_bridge_counters_to_sai_bridge_counters(
      number_of_counters, counter_ids, switch_counters, counters);

  SAI_FREE(stat_ids);
  SAI_FREE(switch_counters);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Clear bridge statistics counters.
 *
 * @param[in] bridge_id Bridge id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_bridge_stats(_In_ sai_object_id_t bridge_id,
                                    _In_ uint32_t number_of_counters,
                                    _In_ const sai_bridge_stat_t *counter_ids) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

switch_status_t switch_api_logical_network_members_get(
    switch_device_t device,
    switch_handle_t ln_handle,
    switch_uint16_t *mbr_count,
    switch_handle_t **mbrs);

/**
 * @brief Get attributes of bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_get_bridge_attribute(_In_ sai_object_id_t bridge_id,
                                      _In_ uint32_t attr_count,
                                      _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_ENTER();

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }
  for (unsigned int i = 0; i < attr_count; i++) {
    switch (attr_list[i].id) {
      case SAI_BRIDGE_ATTR_TYPE:
        break;
      case SAI_BRIDGE_ATTR_PORT_LIST:
        if (bridge_id == sai_bridge_get_default1q_bridge()) {
          tommy_node *n = tommy_list_head(&dot1q_bridgeport);
          unsigned int j = 0;
          attr_list[i].value.objlist.count = 0;
          while (n) {
            struct bridge_port *obj = n->data;  // gets the object pointer
            attr_list[i].value.objlist.list[j] = obj->handle;
            n = n->next;  // go to the next element
            attr_list[i].value.objlist.count++;
            j++;
          }
        }
        break;
      case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
      case SAI_BRIDGE_ATTR_LEARN_DISABLE:
      default:
        SAI_LOG_ERROR("Unsupported get bridge attribute %d", attr_list[i].id);
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  SAI_LOG_EXIT();
  return status;
}

/**
 * @brief Get bridge port statistics counters.
 *
 * @param[in] bridge_port_id Bridge port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_port_stats(
    _In_ sai_object_id_t bridge_port_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_bridge_port_stat_t *counter_ids,
    _Out_ uint64_t *counters) {
  switch_interface_counter_id_t
      switch_counter_ids[SWITCH_INTERFACE_COUNTER_MAX];
  uint32_t index = 0;
  switch_counter_t switch_counters[SWITCH_INTERFACE_COUNTER_MAX];
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  memset(switch_counters, 0x0, sizeof(switch_counters));
  memset(switch_counter_ids, 0x0, sizeof(switch_counter_ids));

  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_BRIDGE_PORT_STAT_IN_OCTETS:
        switch_counter_ids[index] = SWITCH_INTERFACE_COUNTER_IN_BYTES;
        break;
      case SAI_BRIDGE_PORT_STAT_IN_PACKETS:
        switch_counter_ids[index] = SWITCH_INTERFACE_COUNTER_IN_PACKETS;
        break;
      case SAI_BRIDGE_PORT_STAT_OUT_OCTETS:
        switch_counter_ids[index] = SWITCH_INTERFACE_COUNTER_OUT_BYTES;
        break;
      case SAI_BRIDGE_PORT_STAT_OUT_PACKETS:
        switch_counter_ids[index] = SWITCH_INTERFACE_COUNTER_OUT_PACKETS;
        break;
      default:
        break;
    }
  }

  switch_status = switch_api_interface_stats_get(device,
                                                 bridge_port_id,
                                                 number_of_counters,
                                                 switch_counter_ids,
                                                 switch_counters);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get bridge port stats: %s",
                  sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_BRIDGE_PORT_STAT_IN_OCTETS:
      case SAI_BRIDGE_PORT_STAT_OUT_OCTETS:
        counters[index] = switch_counters[index].num_bytes;
        break;
      case SAI_BRIDGE_PORT_STAT_IN_PACKETS:
      case SAI_BRIDGE_PORT_STAT_OUT_PACKETS:
        counters[index] = switch_counters[index].num_packets;
        break;
      default:
        break;
    }
  }

  return status;
}

/**
 * @brief Clear bridge port statistics counters.
 *
 * @param[in] bridge_port_id Bridge port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_bridge_port_stats(
    _In_ sai_object_id_t bridge_port_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_bridge_port_stat_t *counter_ids) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_bridge_api_t bridge_api = {
    .create_bridge = sai_create_bridge,
    .remove_bridge = sai_remove_bridge,
    .set_bridge_attribute = sai_set_bridge_attribute,
    .get_bridge_attribute = sai_get_bridge_attribute,
    .create_bridge_port = sai_create_bridge_port,
    .remove_bridge_port = sai_remove_bridge_port,
    .set_bridge_port_attribute = sai_set_bridge_port_attribute,
    .get_bridge_port_attribute = sai_get_bridge_port_attribute,
    .get_bridge_stats = sai_get_bridge_stats,
    .clear_bridge_stats = sai_clear_bridge_stats,
    .get_bridge_port_stats = sai_get_bridge_port_stats,
    .clear_bridge_port_stats = sai_clear_bridge_port_stats};

extern sai_status_t sai_create_vlan_member(
    _Out_ sai_object_id_t *vlan_member_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list);

sai_status_t sai_bridge_initialize(sai_api_service_t *sai_api_service) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t vlan_handle;

  SAI_LOG_DEBUG("initializing bridge");
  sai_api_service->bridge_api = bridge_api;

  switch_status = switch_api_logical_network_create(0, &DEFAULT_BRIDGE_1Q);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create  bridge: %s", sai_status_to_string(status));
    return status;
  }
  switch_api_device_info_t api_device_info;
  switch_uint64_t flags = 0;
  memset(&api_device_info, 0x0, sizeof(api_device_info));
  flags |= SWITCH_DEVICE_ATTR_PORT_LIST;
  flags |= SWITCH_DEVICE_ATTR_MAX_PORTS;
  flags |= SWITCH_DEVICE_ATTR_DEFAULT_VLAN;
  switch_api_device_attribute_get(device, flags, &api_device_info);
  sai_object_id_t vlan_member_id;
  sai_attribute_t vlan_attr_list[3];
  sai_attribute_t attr_list[3];

  switch_status = switch_api_vlan_id_to_handle_get(
      device, api_device_info.default_vlan, &vlan_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    switch_api_vlan_create(device, api_device_info.default_vlan, &vlan_handle);
  }
  SAI_ASSERT(vlan_handle != SWITCH_API_INVALID_HANDLE);

  attr_list[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
  attr_list[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;
  attr_list[1].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
  attr_list[1].value.oid = DEFAULT_BRIDGE_1Q;
  vlan_attr_list[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
  vlan_attr_list[0].value.oid = vlan_handle;
  vlan_attr_list[1].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
  vlan_attr_list[1].value.s32 = SAI_VLAN_TAGGING_MODE_UNTAGGED;
  tommy_list_init(&dot1q_bridgeport);

  for (unsigned int i = 0; i < (api_device_info.max_ports / 4) + 1; i++) {
    sai_object_id_t bridge_port_id;
    attr_list[2].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr_list[2].value.oid = api_device_info.port_list.handles[i];
    sai_create_bridge_port(&bridge_port_id, 1, 3, attr_list);

    vlan_attr_list[2].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    vlan_attr_list[2].value.oid = bridge_port_id;
    sai_create_vlan_member(&vlan_member_id, 1, 3, vlan_attr_list);
  }

  return SAI_STATUS_SUCCESS;
}
