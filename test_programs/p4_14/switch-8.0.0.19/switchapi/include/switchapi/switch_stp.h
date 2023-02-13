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

#ifndef _switch_stp_h_
#define _switch_stp_h_

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup stp Spanning Tree Group API
 *  API functions listed to configure stp
 *  @{
 */
// begin of stp
//
// Spanning Tree Group API

/** Maximum spanning tree instances */
#define SWITCH_MAX_STP_INSTANCES 1024

/** Maximum vlan in stp instance */
#define SWITCH_MAX_VLAN_PER_STP_INSTANCE 16

/** Spanning Tree Group State */
typedef enum switch_stp_state_ {
  SWITCH_PORT_STP_STATE_NONE = 0,
  SWITCH_PORT_STP_STATE_DISABLED = 1,
  SWITCH_PORT_STP_STATE_LEARNING = 2,
  SWITCH_PORT_STP_STATE_FORWARDING = 3,
  SWITCH_PORT_STP_STATE_BLOCKING = 4
} switch_stp_state_t;

/** Spanning tree mode */
typedef enum switch_stp_mode_ {
  SWITCH_PORT_STP_MODE_DISABLED = 0,
  SWITCH_PORT_STP_MODE_STP = 1,
  SWITCH_PORT_STP_MODE_RSTP = 2,
  SWITCH_PORT_STP_MODE_MSTP = 3
} switch_stp_mode_t;

/**
 Create a spanning Tree group
 @param device device
 @param stp_mode spanning tree mode
*/
switch_status_t switch_api_stp_group_create(const switch_device_t device,
                                            const switch_stp_mode_t stp_mode,
                                            switch_handle_t *stp_handle);

/**
 Delete a spanning tree group
 @param device device
 @param stg_handle handle of the spanning tree group
*/
switch_status_t switch_api_stp_group_delete(const switch_device_t device,
                                            const switch_handle_t stp_handle);

/**
 Add VLAN to the stp
 @param device device
 @param stg_handle spanning tree group handle
 @param vlan_count count of vlans
 @param vlan_handle list of vlan handles
*/
switch_status_t switch_api_stp_group_member_add(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle);

/**
 Remove VLAN from the stp
 @param device device
 @param stg_handle spanning tree group handle
 @param vlan_count count of vlans
 @param vlan_handle list of vlan handles
*/
switch_status_t switch_api_stp_group_member_remove(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle);

/**
 Set the port belonging to a stp in one of discard, learn or forward
 @param device device
 @param stg_handle handle of the Spanning tree group
 @param intf_handle - spanning tree interface
 @param state stp state
*/
switch_status_t switch_api_stp_interface_state_set(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t handle,
    const switch_stp_state_t state);

/**
 Get the state of the port belonging to a stp
 @param device device
 @param stg_handle handle of the Spanning tree group
 @param intf_handle - spanning tree interface
 @param state stp state
*/
switch_status_t switch_api_stp_interface_state_get(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t intf_handle,
    switch_stp_state_t *state);

switch_status_t switch_api_stp_group_members_get(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    switch_uint16_t *num_entries,
    switch_handle_t **network_handles);

switch_status_t switch_api_stp_interfaces_get(const switch_device_t device,
                                              const switch_handle_t stp_handle,
                                              switch_uint16_t *num_entries,
                                              switch_handle_t **port_handles);

/** @} */
// end of stp
#ifdef __cplusplus
}
#endif

#endif
