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

#include <saistp.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_stp.h>
#include <switchapi/switch_vlan.h>

static sai_api_t api_id = SAI_API_STP;

#define SAI_STP_GROUP_PORT_OBJECT(_stp_group_id, _bridge_port_id)  \
  ((_bridge_port_id & 0xFFFF) | ((_stp_group_id & 0xFFFF) << 16) | \
   (SWITCH_HANDLE_TYPE_STP_PORT << SWITCH_HANDLE_TYPE_SHIFT)) &    \
      0xFFFFFFFF

#define SAI_STP_GROUP_OBJECT(_stp_group_port_id)           \
  ((((_stp_group_port_id & 0xFFFFFFFF) >> 16) & 0x3FF) |   \
   (SWITCH_HANDLE_TYPE_STP << SWITCH_HANDLE_TYPE_SHIFT)) & \
      0xFFFFFFFF

#define SAI_STP_PORT_OBJECT(_stp_group_port_id)                  \
  (((_stp_group_port_id & 0xFFFFFFFF) & 0xFFFF) |                \
   (SWITCH_HANDLE_TYPE_INTERFACE << SWITCH_HANDLE_TYPE_SHIFT)) & \
      0xFFFFFFFF

/**
 * @brief Create stp instance with default port state as forwarding.
 *
 * @param[out] stp_id stp instance id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_create_stp_entry(_Out_ sai_object_id_t *stp_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t stp_handle = SWITCH_API_INVALID_HANDLE;
  *stp_id = SAI_NULL_OBJECT_ID;

  switch_status =
      (sai_object_id_t)switch_api_stp_group_create(device, 0, &stp_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create stp entry: %s",
                  sai_status_to_string(status));
    return status;
  }
  *stp_id = stp_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Remove stp instance.
 *
 * @param[in] stp_id stp instance id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_remove_stp_entry(_In_ sai_object_id_t stp_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(stp_id) == SAI_OBJECT_TYPE_STP);

  switch_status = switch_api_stp_group_delete(device, (switch_handle_t)stp_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove stp entry %lx: %s",
                  stp_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Update stp state of a port in specified stp instance.
 *
 * @param[in] stp_id stp instance id
 * @param[in] port_id port id
 * @param[in] stp_port_state stp state of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_set_stp_entry_attribute(_In_ sai_object_id_t stp_id,
                                         _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(stp_id) == SAI_OBJECT_TYPE_STP);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Retrieve stp state of a port in specified stp instance.
 *
 * @param[in] stp_id stp instance id
 * @param[in] port_id port id
 * @param[out] stp_port_state stp state of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_get_stp_entry_attribute(_In_ sai_object_id_t stp_id,
                                         _In_ uint32_t attr_count,
                                         _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  unsigned int i = 0;
  sai_attribute_t *attr;
  sai_object_list_t *obj_list = NULL;
  switch_handle_t *vlan_list = NULL;
  switch_handle_t *port_list = NULL;
  switch_uint16_t num_ports = 0;
  switch_uint16_t num_vlans = 0;
  unsigned int count = 0;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(stp_id) == SAI_OBJECT_TYPE_STP);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_STP_ATTR_VLAN_LIST:
        status = switch_api_stp_group_members_get(
            device, stp_id, &num_vlans, &vlan_list);
        if (num_vlans) {
          obj_list = &attr_list->value.objlist;
          for (count = 0; count < num_vlans; count++) {
            obj_list->list[count] = vlan_list[count];
          }
          obj_list->count = num_vlans;
        } else {
          obj_list->count = 0;
        }
        break;

      case SAI_STP_ATTR_PORT_LIST:
        status = switch_api_stp_interfaces_get(
            device, stp_id, &num_ports, &port_list);
        if (num_ports) {
          obj_list = &attr_list->value.objlist;
          for (count = 0; count < num_ports; count++) {
            obj_list->list[count] = port_list[count];
          }
          obj_list->count = num_ports;
        } else {
          obj_list->count = 0;
        }
        break;

      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

sai_stp_port_state_t sai_switch_stp_state_to_sai_stp_state(
    switch_stp_state_t switch_stp_state) {
  sai_stp_port_state_t sai_stp_port_state = SAI_STP_PORT_STATE_FORWARDING;
  switch (switch_stp_state) {
    case SWITCH_PORT_STP_STATE_LEARNING:
      sai_stp_port_state = SAI_STP_PORT_STATE_LEARNING;
      break;
    case SWITCH_PORT_STP_STATE_FORWARDING:
      sai_stp_port_state = SAI_STP_PORT_STATE_FORWARDING;
      break;
    case SWITCH_PORT_STP_STATE_BLOCKING:
      sai_stp_port_state = SAI_STP_PORT_STATE_BLOCKING;
      break;
    default:
      sai_stp_port_state = 0;
  }

  return sai_stp_port_state;
}

switch_stp_state_t sai_stp_state_to_switch_stp_state(
    sai_stp_port_state_t stp_state) {
  switch_stp_state_t switch_stp_state = SWITCH_PORT_STP_STATE_NONE;
  switch (stp_state) {
    case SAI_STP_PORT_STATE_LEARNING:
      switch_stp_state = SWITCH_PORT_STP_STATE_LEARNING;
      break;
    case SAI_STP_PORT_STATE_FORWARDING:
      switch_stp_state = SWITCH_PORT_STP_STATE_FORWARDING;
      break;
    case SAI_STP_PORT_STATE_BLOCKING:
      switch_stp_state = SWITCH_PORT_STP_STATE_BLOCKING;
      break;
    default:
      switch_stp_state = SWITCH_PORT_STP_STATE_NONE;
      break;
  }

  return switch_stp_state;
}

/**
 * @brief Create stp port object
 *
 * @param[out] stp_port_id stp port id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_create_stp_port(_Out_ sai_object_id_t *stp_port_id,
                                 _In_ sai_object_id_t switch_id,
                                 _In_ uint32_t attr_count,
                                 _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_stp_state_t switch_stp_state = SWITCH_PORT_STP_STATE_NONE;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_object_id_t stp_group_id = 0;
  sai_object_id_t bridge_port_id = 0;
  sai_attribute_t attribute;
  uint32_t index = 0;

  *stp_port_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = attr_list[index];
    switch (attribute.id) {
      case SAI_STP_PORT_ATTR_STP:
        stp_group_id = attribute.value.oid;
        SAI_ASSERT(sai_object_type_query(stp_group_id) == SAI_OBJECT_TYPE_STP);
        break;

      case SAI_STP_PORT_ATTR_BRIDGE_PORT:
        bridge_port_id = attribute.value.oid;
        SAI_ASSERT(sai_object_type_query(bridge_port_id) ==
                   SAI_OBJECT_TYPE_BRIDGE_PORT);
        break;

      case SAI_STP_PORT_ATTR_STATE:
        switch_stp_state =
            sai_stp_state_to_switch_stp_state(attribute.value.u32);
        SAI_ASSERT(switch_stp_state != SWITCH_PORT_STP_STATE_NONE);
        break;
      default:
        break;
    }
  }

  switch_status = switch_api_stp_interface_state_set(
      device, stp_group_id, bridge_port_id, switch_stp_state);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create stp port entry %lx: %s",
                  stp_group_id,
                  sai_status_to_string(status));
  }

  *stp_port_id = SAI_STP_GROUP_PORT_OBJECT(stp_group_id, bridge_port_id);

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove stp port object.
 *
 * @param[in] stp_port_id stp object id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_remove_stp_port(_In_ sai_object_id_t stp_port_id) {
  SAI_LOG_ENTER();

  sai_object_id_t stp_group_id = 0;
  sai_object_id_t bridge_port_id = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(stp_port_id) == SAI_OBJECT_TYPE_STP_PORT);

  stp_group_id = SAI_STP_GROUP_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(stp_group_id) == SAI_OBJECT_TYPE_STP);

  bridge_port_id = SAI_STP_PORT_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(bridge_port_id) ==
             SAI_OBJECT_TYPE_BRIDGE_PORT);

  switch_status = switch_api_stp_interface_state_set(
      device, stp_group_id, bridge_port_id, SWITCH_PORT_STP_STATE_NONE);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove stp port entry %lx: %s",
                  stp_port_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Set the attribute of STP port.
 *
 * @param[in] stp_port_id stp port id
 * @param[in] attr attribute value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_set_stp_port_attribute(_In_ sai_object_id_t stp_port_id,
                                        _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_stp_port_state_t stp_port_state = SAI_STP_PORT_STATE_FORWARDING;
  switch_stp_state_t switch_stp_state = SWITCH_PORT_STP_STATE_NONE;
  sai_object_id_t bridge_port_id = 0;
  sai_object_id_t stp_group_id = 0;

  SAI_ASSERT(sai_object_type_query(stp_port_id) == SAI_OBJECT_TYPE_STP_PORT);

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  stp_group_id = SAI_STP_GROUP_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(stp_group_id) == SAI_OBJECT_TYPE_STP);

  bridge_port_id = SAI_STP_PORT_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(bridge_port_id) ==
             SAI_OBJECT_TYPE_BRIDGE_PORT);

  switch (attr->id) {
    case SAI_STP_PORT_ATTR_STATE:
      stp_port_state = attr->value.u32;
      switch_stp_state = sai_stp_state_to_switch_stp_state(stp_port_state);

      switch_status = switch_api_stp_interface_state_set(
          device, stp_group_id, bridge_port_id, switch_stp_state);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set stp port state %lx: %s",
                      stp_port_id,
                      sai_status_to_string(status));
      }
      break;

    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Get the attribute of STP port.
 *
 * @param[in] stp_port_id stp port id
 * @param[in] attr_count number of the attribute
 * @param[in] attr_list attribute value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_get_stp_port_attribute(_In_ sai_object_id_t stp_port_id,
                                        _In_ uint32_t attr_count,
                                        _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_stp_port_state_t sai_stp_port_state = SAI_STP_PORT_STATE_FORWARDING;
  switch_stp_state_t switch_stp_state = SWITCH_PORT_STP_STATE_NONE;
  sai_attribute_t *attr = attr_list;

  sai_object_id_t bridge_port_id = 0;
  sai_object_id_t stp_group_id = 0;
  unsigned int i = 0;

  SAI_ASSERT(sai_object_type_query(stp_port_id) == SAI_OBJECT_TYPE_STP_PORT);

  stp_group_id = SAI_STP_GROUP_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(stp_group_id) == SAI_OBJECT_TYPE_STP);

  bridge_port_id = SAI_STP_PORT_OBJECT(stp_port_id);
  SAI_ASSERT(sai_object_type_query(bridge_port_id) ==
             SAI_OBJECT_TYPE_BRIDGE_PORT);

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_STP_PORT_ATTR_STATE:
        switch_status = switch_api_stp_interface_state_get(
            device, stp_group_id, bridge_port_id, &switch_stp_state);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get stp port state %lx: %s",
                        stp_port_id,
                        sai_status_to_string(status));
          return status;
        }
        sai_stp_port_state =
            sai_switch_stp_state_to_sai_stp_state(switch_stp_state);
        attr->value.oid = sai_stp_port_state;
        break;

      case SAI_STP_PORT_ATTR_BRIDGE_PORT:
        attr->value.oid = bridge_port_id;
        break;

      case SAI_STP_PORT_ATTR_STP:
        attr->value.oid = stp_group_id;
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief STP method table retrieved with sai_api_query()
 */
sai_stp_api_t stp_api = {.create_stp = sai_create_stp_entry,
                         .remove_stp = sai_remove_stp_entry,
                         .set_stp_attribute = sai_set_stp_entry_attribute,
                         .get_stp_attribute = sai_get_stp_entry_attribute,
                         .create_stp_port = sai_create_stp_port,
                         .remove_stp_port = sai_remove_stp_port,
                         .set_stp_port_attribute = sai_set_stp_port_attribute,
                         .get_stp_port_attribute = sai_get_stp_port_attribute};

sai_status_t sai_stp_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing spanning tree");
  sai_api_service->stp_api = stp_api;
  return SAI_STATUS_SUCCESS;
}
