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

#include <saifdb.h>
#include "saiinternal.h"
#include <switchapi/switch_l2.h>
#include <switchapi/switch_vlan.h>
#include <switchapi/switch_interface.h>
#include <linux/if_ether.h>

static sai_api_t api_id = SAI_API_FDB;

static void sai_fdb_entry_to_string(_In_ const sai_fdb_entry_t *fdb_entry,
                                    _Out_ char *entry_string) {
  snprintf(entry_string,
           SAI_MAX_ENTRY_STRING_LEN,
           "fdb entry mac [%02x:%02x:%02x:%02x:%02x:%02x] vlan %u",
           fdb_entry->mac_address[0],
           fdb_entry->mac_address[1],
           fdb_entry->mac_address[2],
           fdb_entry->mac_address[3],
           fdb_entry->mac_address[4],
           fdb_entry->mac_address[5],
           fdb_entry->vlan_id);
}

static sai_status_t sai_fdb_entry_parse(const sai_fdb_entry_t *fdb_entry,
                                        switch_api_mac_entry_t *mac_entry) {
  if (fdb_entry->bridge_type == SAI_FDB_ENTRY_BRIDGE_TYPE_1Q) {
    switch_api_vlan_id_to_handle_get(
        device, fdb_entry->vlan_id, &mac_entry->network_handle);
  } else if (fdb_entry->bridge_type == SAI_FDB_ENTRY_BRIDGE_TYPE_1D) {
    mac_entry->network_handle = fdb_entry->bridge_id;
  } else {
    return SWITCH_STATUS_NOT_SUPPORTED;
  }

  memcpy(mac_entry->mac.mac_addr, fdb_entry->mac_address, ETH_ALEN);
  return SWITCH_STATUS_SUCCESS;
}

static void sai_fdb_entry_attribute_parse(uint32_t attr_count,
                                          const sai_attribute_t *attr_list,
                                          switch_api_mac_entry_t *mac_entry) {
  const sai_attribute_t *attribute;
  uint32_t i = 0;
  sai_packet_action_t action = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_FDB_ENTRY_ATTR_TYPE:
        switch (attribute->value.s32) {
          case SAI_FDB_ENTRY_TYPE_DYNAMIC:
            mac_entry->entry_type = SWITCH_MAC_ENTRY_DYNAMIC;
            break;

          case SAI_FDB_ENTRY_TYPE_STATIC:
            mac_entry->entry_type = SWITCH_MAC_ENTRY_STATIC;
            break;
        }
        break;

      case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
        mac_entry->handle = attribute->value.oid;
        break;

      case SAI_FDB_ENTRY_ATTR_ENDPOINT_IP:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &mac_entry->ip_addr);
        break;

      case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
        action = (switch_mac_action_t)attribute->value.s32;
        switch (action) {
          case SAI_PACKET_ACTION_DROP:
            mac_entry->mac_action = SWITCH_MAC_ACTION_DROP;
            break;
          case SAI_PACKET_ACTION_FORWARD:
            mac_entry->mac_action = SWITCH_MAC_ACTION_FORWARD;
            break;
          default:
            return;
        }
        break;
    }
  }
}

/*
* Routine Description:
*    Create FDB entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_fdb_entry(_In_ const sai_fdb_entry_t *fdb_entry,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  switch_api_mac_entry_t mac_entry;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  SAI_LOG_ENTER();

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);
  sai_fdb_entry_attribute_parse(attr_count, attr_list, &mac_entry);

  switch_status = switch_api_mac_table_entry_add(device, &mac_entry);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_ALREADY_EXISTS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    SAI_LOG_ERROR("failed to create fdb entry %s : %s",
                  entry_string,
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove FDB entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_fdb_entry(_In_ const sai_fdb_entry_t *fdb_entry) {
  switch_api_mac_entry_t mac_entry;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  SAI_LOG_ENTER();

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);

  switch_status = switch_api_mac_table_entry_delete(device, &mac_entry);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    SAI_LOG_ERROR("failed to remove fdb entry %s : %s",
                  entry_string,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Set fdb entry attribute value
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_fdb_entry_attribute(_In_ const sai_fdb_entry_t *fdb_entry,
                                         _In_ const sai_attribute_t *attr) {
  switch_api_mac_entry_t mac_entry;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  SAI_LOG_ENTER();

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);
  sai_fdb_entry_attribute_parse(1, attr, &mac_entry);

  switch_status = switch_api_mac_table_entry_update(device, &mac_entry);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    SAI_LOG_ERROR("failed to update fdb entry %s : %s",
                  entry_string,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Get fdb entry attribute value
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_fdb_entry_attribute(_In_ const sai_fdb_entry_t *fdb_entry,
                                         _In_ uint32_t attr_count,
                                         _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  unsigned int i = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_mac_entry_type_t entry_type;
  switch_mac_action_t mac_action;
  switch_handle_t intf_handle;
  switch_api_mac_entry_t mac_entry;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  SAI_LOG_ENTER();

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_FDB_ENTRY_ATTR_TYPE:
        switch_status =
            switch_api_mac_entry_type_get(device, &mac_entry, &entry_type);
        status = sai_switch_status_to_sai_status(switch_status);

        if (status != SAI_STATUS_SUCCESS) {
          sai_fdb_entry_to_string(fdb_entry, entry_string);
          SAI_LOG_ERROR("failed to get fdb entry type for %s : %s",
                        entry_string,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.oid = (entry_type == SWITCH_MAC_ENTRY_DYNAMIC)
                              ? SAI_FDB_ENTRY_TYPE_DYNAMIC
                              : SAI_FDB_ENTRY_TYPE_STATIC;
        break;

      case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
        switch_status =
            switch_api_mac_entry_port_id_get(device, &mac_entry, &intf_handle);
        status = sai_switch_status_to_sai_status(switch_status);

        if (status != SAI_STATUS_SUCCESS) {
          sai_fdb_entry_to_string(fdb_entry, entry_string);
          SAI_LOG_ERROR("failed to get port-id for entry %s : %s",
                        entry_string,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.oid =
            (intf_handle == SWITCH_API_INVALID_HANDLE ? SAI_NULL_OBJECT_ID
                                                      : intf_handle);
        break;

      case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
        switch_status = switch_api_mac_entry_packet_action_get(
            device, &mac_entry, &mac_action);
        status = sai_switch_status_to_sai_status(switch_status);

        if (status != SAI_STATUS_SUCCESS) {
          sai_fdb_entry_to_string(fdb_entry, entry_string);
          SAI_LOG_ERROR("failed to get packet action for entry %s : %s",
                        entry_string,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.oid = (mac_action == SWITCH_MAC_ACTION_DROP)
                              ? SAI_PACKET_ACTION_DROP
                              : SAI_PACKET_ACTION_FORWARD;
        break;

      default:
        break;
    }
  }
  SAI_LOG_EXIT();
  return status;
}

/*
* Routine Description:
*    Remove all FDB entries by attribute set in sai_fdb_flush_attr
*
* Arguments:
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_flush_fdb_entries(_In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  const sai_attribute_t *attribute;
  uint32_t index = 0;
  sai_object_id_t intf_handle = 0;
  sai_vlan_id_t vlan_id = 0;
  switch_uint64_t flush_type = 0;
  switch_handle_t vlan_handle = 0;
  sai_fdb_flush_entry_type_t entry_type = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_mac_entry_type_t switch_mac_entry_type = 0;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID:
        intf_handle = attribute->value.oid;
        flush_type |= SWITCH_MAC_FLUSH_TYPE_INTERFACE;
        break;
      case SAI_FDB_FLUSH_ATTR_VLAN_ID:
        vlan_id = attribute->value.u16;
        switch_api_vlan_id_to_handle_get(device, vlan_id, &vlan_handle);
        flush_type |= SWITCH_MAC_FLUSH_TYPE_NETWORK;
        break;
      case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
        entry_type = attribute->value.s32;
        switch (entry_type) {
          case SAI_FDB_FLUSH_ENTRY_TYPE_DYNAMIC:
            switch_mac_entry_type = SWITCH_MAC_ENTRY_DYNAMIC;
            break;
          case SAI_FDB_FLUSH_ENTRY_TYPE_STATIC:
            switch_mac_entry_type = SWITCH_MAC_ENTRY_STATIC;
            break;
          default:
            break;
        }

      default:
        break;
    }
  }

  switch_status = switch_api_mac_table_entry_flush(
      device, flush_type, vlan_handle, intf_handle, switch_mac_entry_type);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to flush fdb entry %s : %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

static sai_fdb_event_t switch_mac_event_to_sai_fdb_event(
    switch_mac_event_t mac_event) {
  switch (mac_event) {
    case SWITCH_MAC_EVENT_MOVE:
      return SAI_FDB_EVENT_MOVE;
    case SWITCH_MAC_EVENT_DELETE:
      return SAI_FDB_EVENT_FLUSHED;
    case SWITCH_MAC_EVENT_LEARN:
      return SAI_FDB_EVENT_LEARNED;
    case SWITCH_MAC_EVENT_AGE:
      return SAI_FDB_EVENT_AGED;
    case SWITCH_MAC_EVENT_CREATE:
    default:
      return 0;
  }
}

static void sai_mac_notify_cb(const switch_device_t device,
                              const switch_api_mac_entry_t *mac_entry,
                              const switch_mac_event_t mac_event,
                              void *app_data) {
  SAI_LOG_ENTER();

  if (!sai_switch_notifications.on_fdb_event) {
    return;
  }

  if (!mac_entry) {
    SAI_LOG_ERROR("invalid argument");
    return;
  }

  switch_vlan_t vlan_id;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  status = switch_api_vlan_handle_to_id_get(
      device, mac_entry->network_handle, &vlan_id);
  (void)status;

  sai_fdb_event_notification_data_t fdb_event;
  memset(&fdb_event, 0, sizeof(fdb_event));
  fdb_event.event_type = switch_mac_event_to_sai_fdb_event(mac_event);
  memcpy(fdb_event.fdb_entry.mac_address, mac_entry->mac.mac_addr, ETH_ALEN);
  fdb_event.fdb_entry.switch_id =
      (((unsigned long)SWITCH_HANDLE_TYPE_DEVICE) << SWITCH_HANDLE_TYPE_SHIFT) |
      0x1;
  fdb_event.fdb_entry.vlan_id = vlan_id;
  sai_attribute_t attr_list[2];
  memset(attr_list, 0, sizeof(attr_list));
  attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
  attr_list[0].value.s32 = SAI_FDB_ENTRY_TYPE_DYNAMIC;
  attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
  attr_list[1].value.oid = mac_entry->handle;
  fdb_event.attr = attr_list;
  fdb_event.attr_count = 2;
  sai_switch_notifications.on_fdb_event(1, &fdb_event);

  SAI_LOG_EXIT();

  (void)status;
  return;
}

/*
*  FDB methods table retrieved with sai_api_query()
*/
sai_fdb_api_t fdb_api = {.create_fdb_entry = sai_create_fdb_entry,
                         .remove_fdb_entry = sai_remove_fdb_entry,
                         .set_fdb_entry_attribute = sai_set_fdb_entry_attribute,
                         .get_fdb_entry_attribute = sai_get_fdb_entry_attribute,
                         .flush_fdb_entries = sai_flush_fdb_entries};

sai_status_t sai_fdb_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("initializing fdb");
  sai_api_service->fdb_api = fdb_api;
  switch_uint16_t mac_event_flags = 0;
  mac_event_flags |= SWITCH_MAC_EVENT_LEARN | SWITCH_MAC_EVENT_AGE |
                     SWITCH_MAC_EVENT_MOVE | SWITCH_MAC_EVENT_DELETE |
                     SWITCH_MAC_EVENT_CREATE;
  switch_api_mac_notification_register(
      device, SWITCH_SAI_APP_ID, mac_event_flags, &sai_mac_notify_cb);
  return SAI_STATUS_SUCCESS;
}
