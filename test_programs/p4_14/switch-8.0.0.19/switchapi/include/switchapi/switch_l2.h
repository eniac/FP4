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

#ifndef __SWITCH_L2_H__
#define __SWITCH_L2_H__

#include "switch_base_types.h"
#include "switch_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file switch_l2.h
 * brief This file contains API to program smac and dmac tables. The basic
 *       switching APIs are controlled by the manipulation of the MAC tables –
 *       source and destination. Addresses learnt from packets on a port
 *       and/or VLAN are used to switch packets destined that address
 *       within the VLAN.
 */

/**
 * @defgroup L2 L2 Switching API
 *  API functions listed to configure Mac Address tables
 *  @{
 */

/** mac entry type - static or dynamic */
typedef enum switch_mac_entry_type_s {
  SWITCH_MAC_ENTRY_UNSPECIFIED = 0,
  SWITCH_MAC_ENTRY_DYNAMIC = 1,
  SWITCH_MAC_ENTRY_STATIC = 2,
} switch_mac_entry_type_t;

/** mac action - forward or drop */
typedef enum switch_mac_action_s {
  SWITCH_MAC_ACTION_DROP = 0,
  SWITCH_MAC_ACTION_FORWARD = 1
} switch_mac_action_t;

/** mac event */
typedef enum switch_mac_event_s {
  SWITCH_MAC_EVENT_CREATE = (1 << 0),
  SWITCH_MAC_EVENT_MOVE = (1 << 1),
  SWITCH_MAC_EVENT_DELETE = (1 << 2),
  SWITCH_MAC_EVENT_LEARN = (1 << 3),
  SWITCH_MAC_EVENT_AGE = (1 << 4),
} switch_mac_event_t;

/** mac flush type */
typedef enum switch_mac_flush_type_s {
  SWITCH_MAC_FLUSH_TYPE_NETWORK = (1 << 0),
  SWITCH_MAC_FLUSH_TYPE_INTERFACE = (1 << 1),
  SWITCH_MAC_FLUSH_TYPE_MAC_TYPE = (1 << 2),
  SWITCH_MAC_FLUSH_TYPE_ALL = (1 << 3)
} switch_mac_flush_type_t;

/** mac entry */
typedef struct switch_api_mac_entry_s {
  /** network handle - vlan/ln */
  switch_handle_t network_handle;

  /** mac address */
  switch_mac_addr_t mac;

  /** handle - interface/nhop/mgid */
  switch_handle_t handle;

  /** mac entry type - static/dynamic */
  switch_mac_entry_type_t entry_type;

  /** tunnel dst ip */
  switch_ip_addr_t ip_addr;

  /** mac action - permit/deny */
  switch_mac_action_t mac_action;

} switch_api_mac_entry_t;

/**
 * @brief mac notifications
 *
 * @param[out] device device id
 * @param[out] mac_entry mac entry
 * @param[out] mac_event mac event
 * @param[out] app_data application cookie
 */
typedef void (*switch_mac_notification_fn)(
    const switch_device_t device,
    const switch_api_mac_entry_t *mac_entry,
    const switch_mac_event_t mac_event,
    void *app_data);

/**
 * @brief Add a mac address entry to dmac table
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entry_add(
    switch_device_t device, switch_api_mac_entry_t *mac_entry);

/**
 * @brief Add set of mac address entries to dmac table
 *
 * @param[in] device device id
 * @param[in] num_entries number of mac entries
 * @param[in] mac_entries array of mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entries_add(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries);

/**
 * @brief Update a mac address entry in dmac table
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entry_update(
    switch_device_t device, switch_api_mac_entry_t *mac_entry);

/**
 * @brief Update set of mac address entries in dmac table
 *
 * @param[in] device device id
 * @param[in] num_entries number of mac entries
 * @param[in] mac_entries array of mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entries_update(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries);

/**
 * @brief Delete a mac address entry from dmac table
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entry_delete(
    switch_device_t device, switch_api_mac_entry_t *mac_entry);

/**
 * @brief Delete set of mac address entries from dmac table
 *
 * @param[in] device device id
 * @param[in] num_entries number of mac entries
 * @param[in] mac_entries array of mac entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entries_delete(
    switch_device_t device,
    switch_size_t num_entries,
    switch_api_mac_entry_t *mac_entries);

/**
 * @brief Mac notification callback registeration
 *
 * @param[in] device device id
 * @param[in] app_id application id
 * @param[in] mac_event_flags
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_notification_register(
    const switch_device_t device,
    const switch_app_id_t app_id,
    const switch_uint16_t mac_event_flags,
    switch_mac_notification_fn cb_fn);

/**
 * @brief Mac notification callback deregisteration
 *
 * @param[in] device device id
 * @param[in] app_id application id
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_notification_deregister(
    const switch_device_t device, const switch_app_id_t app_id);

/**
 * @brief Set dmac table learn timeout
 *
 * @param[in] device device id
 * @param[in] timeout timeout value
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_set_learning_timeout(
    const switch_device_t device, const switch_uint32_t timeout);

/**
 * @brief Delete dmac table entries by vlan or interface
 *
 * @param[in] device device id
 * @param[in] flush_type flush type - vlan/ln/interface
 * @param[in] network_handle network handle - vlan/ln
 * @param[in] intf_handle interface handle
 * @param[in] mac_entry_type mac type - static/dynamic
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_table_entry_flush(
    const switch_device_t device,
    const switch_uint64_t flush_type,
    const switch_handle_t network_handle,
    const switch_handle_t intf_handle,
    const switch_mac_entry_type_t mac_entry_type);

/**
 * @brief Get mac handle from mac entry
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 * @param[out] mac_handle mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_entry_handle_get(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *mac_handle);

/**
 * @brief Get mac entry type from mac entry
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 * @param[out] entry_type mac entry type - static/dynamic
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_entry_type_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_entry_type_t *entry_type);

/**
 * @brief Get interface handle from mac entry
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 * @param[out] intf_handle interface handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_entry_port_id_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_handle_t *intf_handle);

/**
 * @brief Get mac action from mac entry
 *
 * @param[in] device device id
 * @param[in] mac_entry mac entry
 * @param[out] mac_action mac action - forward/drop
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_mac_entry_packet_action_get(
    switch_device_t device,
    switch_api_mac_entry_t *api_mac_entry,
    switch_mac_action_t *mac_action);

switch_status_t switch_api_mac_entry_dump(
    const switch_device_t device,
    const switch_api_mac_entry_t *api_mac_entry,
    void *cli_ctx);

switch_status_t switch_api_mac_entry_handle_dump(
    const switch_device_t device,
    const switch_handle_t mac_handle,
    void *cli_ctx);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_L2_H__ */
