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

#ifndef __SWITCH_L3_H__
#define __SWITCH_L3_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file switch_l2.h
 * brief This file contains API to program ipv4/ipv6 fib and lpm tables. The
 *       basic routing APIs are controlled by the manipulation of these routing
 *       tables – fib and lpm.
 */

/** @defgroup L3 L3 Routing API
 *  API functions to configure Routing tables
 *  @{
 */

/** route events */
typedef enum switch_route_event_s {
  SWITCH_ROUTE_EVENT_CREATE = (1 << 0),
  SWITCH_ROUTE_EVENT_UPDATE = (1 << 1),
  SWITCH_ROUTE_EVENT_DELETE = (1 << 2)
} switch_route_event_t;

/** route flush events */
typedef enum switch_route_flush_type_s {
  SWITCH_ROUTE_FLUSH_TYPE_IPV4 = (1 << 0),
  SWITCH_ROUTE_FLUSH_TYPE_IPV6 = (1 << 1),
  SWITCH_ROUTE_FLUSH_TYPE_VRF = (1 << 2),
} switch_route_flush_type_t;

/** mtu type */
typedef enum switch_mtu_type_s {
  SWITCH_MTU_TYPE_IPV4 = (1 << 0),
  SWITCH_MTU_TYPE_IPV6 = (1 << 1)
} switch_mtu_type_t;

/** route type */
typedef enum switch_route_type_s {
  SWITCH_ROUTE_TYPE_HOST = 0,
  SWITCH_ROUTE_TYPE_MYIP = 1
} switch_route_type_t;

/** route entry */
typedef struct switch_api_route_entry_s {
  /** router interface handle */
  switch_handle_t rif_handle;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip_address;

  /** nexthop handle */
  switch_handle_t nhop_handle;

  /** route type */
  switch_route_type_t route_type;

  /** route installed by neighbor */
  bool neighbor_installed;
} switch_api_route_entry_t;

/**
 * @brief address add - add an entry to host or lpm table to match based on
 *        the prefix length on a router interface
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_interface_address_add(
    switch_device_t device, switch_api_route_entry_t *api_route_entry);

/**
 * @brief address delete - delete an entry to host or lpm table to match based
 *on
 *        the prefix length on a router interface
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_interface_address_delete(
    switch_device_t device, switch_api_route_entry_t *api_route_entry);

/**
 Get the handle of the interface that route lookup returns for a host addr
 @param device device
 @param vrf virtual domain identifier
 @param ip_addr IP address (host or prefix)
 @param intf_handle pointer to return Interface handle
*/
switch_status_t switch_api_l3_route_nhop_get(switch_device_t device,
                                             switch_handle_t vrf_handle,
                                             switch_ip_addr_t *ip_addr,
                                             switch_handle_t *intf_handle);

/**
 * @brief route add - add an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_add(
    switch_device_t device, switch_api_route_entry_t *api_route_entry);

/**
 * @brief route update - update an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_update(
    switch_device_t device, switch_api_route_entry_t *api_route_entry);

/**
 * @brief route delete - delete an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_delete(
    switch_device_t device, switch_api_route_entry_t *route_entry);

/**
 Lookup FIB table (host or LPM) for a given host address
 Return nexthop handle (single path or ECMP group)
 Return INVALID_HANDLE if lookup fails
 @param device device
 @param vrf virtual domain identifier
 @param ip_addr IP address
 @param nhop_handle pointer to return Nexthop  Handle
*/
switch_status_t switch_api_l3_route_lookup(
    switch_device_t device,
    switch_api_route_entry_t *api_route_entry,
    switch_handle_t *nhop_handle);

switch_status_t switch_api_rif_ipv4_unicast_enabled_set(
    switch_device_t device, switch_handle_t intf_handle, bool set);

switch_status_t switch_api_rif_ipv6_unicast_enabled_set(
    switch_device_t device, switch_handle_t intf_handle, bool set);

switch_status_t switch_api_rif_ipv4_urpf_mode_set(switch_device_t device,
                                                  switch_handle_t intf_handle,
                                                  switch_urpf_mode_t urpf_mode);

switch_status_t switch_api_rif_ipv6_urpf_mode_set(switch_device_t device,
                                                  switch_handle_t intf_handle,
                                                  switch_urpf_mode_t urpf_mode);
/**
 create mtu entry
 @param device device
 @param mtu_index mtu index
 @param mtu mtu value
 */
switch_status_t switch_api_l3_mtu_create(switch_device_t device,
                                         switch_uint64_t flags,
                                         switch_mtu_t mtu,
                                         switch_handle_t *mtu_handle);

switch_status_t switch_api_l3_mtu_update(switch_device_t device,
                                         switch_handle_t mtu_handle,
                                         switch_mtu_t mtu);

switch_status_t switch_api_l3_mtu_delete(switch_device_t device,
                                         switch_handle_t mtu_handle);

switch_status_t switch_api_l3_mtu_get(switch_device_t device,
                                      switch_handle_t mtu_handle,
                                      switch_mtu_t *mtu);

switch_status_t switch_api_l3_route_handle_lookup(
    const switch_device_t device,
    const switch_api_route_entry_t *api_route_entry,
    switch_handle_t *route_handle);

switch_status_t switch_api_l3_route_handle_dump(
    const switch_device_t device,
    const switch_handle_t route_handle,
    void *cli_ctx);

switch_status_t switch_api_l3_route_dump(
    const switch_device_t device,
    const switch_api_route_entry_t *route_entry,
    void *cli_ctx);

switch_status_t switch_api_route_table_size_get(switch_device_t device,
                                                switch_size_t *tbl_size);

switch_status_t switch_api_l3_mtu_size_create(switch_device_t device,
                                              switch_mtu_t mtu,
                                              switch_handle_t *mtu_handle);

switch_status_t switch_api_l3_mtu_size_delete(switch_device_t device,
                                              switch_mtu_t mtu);

/**
 * @brief routing table iterator function pointer
 *
 * @param[out] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
typedef switch_status_t (*switch_l3_table_iterator_fn)(
    const switch_api_route_entry_t *api_route_entry);

/**
 * @brief routing table iterator
 *
 * @param[in] iterator_fn iterator function will be called for every
 *            route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_entries_get(
    const switch_device_t device, switch_l3_table_iterator_fn iterator_fn);

/**
 * @brief routing table iterator
 *
 * @param[in] device device id
 * @param[in] vrf_handle vrf handle
 * @param[in] iterator_fn iterator function will be called for route entry
 *            matching the vrf handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn);

/**
 * @brief v4 routing table iterator
 *
 * @param[in] device device id
 * @param[in] vrf_handle vrf handle
 * @param[in] iterator_fn iterator function will be called for route entry
 *            matching the vrf handle and ipv4 routes
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_v4_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn);

/**
 * @brief v6 routing table iterator
 *
 * @param[in] device device id
 * @param[in] vrf_handle vrf handle
 * @param[in] iterator_fn iterator function will be called for route entry
 *            matching the vrf handle and ipv6 routes
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_v6_route_entries_get_by_vrf(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_l3_table_iterator_fn iterator_fn);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_L3_H__ */
