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

#ifndef _switch_lag_h_
#define _switch_lag_h_

#include "switch_id.h"
#include "switch_handle.h"
#include "switch_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup LAG LAG configuration API
 *  API functions listed to configure the Link Aggregation groups.
 *  Create LAG, Add/Del ports to LAG and set admin states
    Link aggregation allows for one or more ports to be treated as a single entity for reception
    and transmission of packets through the system. All behavior of packets on all the ports that
    belong to a group is identical on the device. The ports that belong to a single group can span
    multiple pipelines. Transmission of packets through particular ports is determined by the hashing
    scheme selectable globally. Typically the L2 fields (DMAC, SMAC, EtherType) are used for this.
    The choice of this is configurable by the APIs.
 *  @{
 */  // begin of LAG

/**< LACP Key */
typedef unsigned int lacp_key_t;

/** type of LAG */
typedef enum switch_lag_type_ {
  SWITCH_API_LAG_SIMPLE,   /**< simple hash */
  SWITCH_API_LAG_RESILIENT /**< weighted/resilient hash */
} switch_lag_type_t;

// Simple LAG API
/**
 Link Aggregation Group creation
 @param device device to use
 */
switch_status_t switch_api_lag_create(switch_device_t device,
                                      switch_handle_t *lag_handle);

/**
 Link Aggregation Group deletion
 @param device device to use
 @param lag_handle handle of group returned on creation
 */
switch_status_t switch_api_lag_delete(switch_device_t device,
                                      switch_handle_t lag_handle);

/**
 Link Aggregation Group member port add
 @param device device to use
 @param lag_handle handle of group returned on creation
 @param direction allow rx and rx member add separately
 @param port port in the same device on which lag_handle was created
 */
switch_status_t switch_api_lag_member_add(switch_device_t device,
                                          switch_handle_t lag_handle,
                                          switch_direction_t direction,
                                          switch_handle_t port_handle);

/**
 Link Aggregation Group member port delete
 @param device device to use
 @param lag_handle handle of group returned on creation
 @param direction control rx and tx members independently or both
 @param port port in the same device on which lag_handle was created
 */
switch_status_t switch_api_lag_member_delete(switch_device_t device,
                                             switch_handle_t lag_handle,
                                             switch_direction_t direction,
                                             switch_handle_t port_handle);

/**
 Link Aggregation Group member add by handle
 @param device device to use
 @param lag_handle handle of group returned on creation
 @param direction allow rx and rx member add separately
 @param port port in the same device on which lag_handle was created
 */
switch_status_t switch_api_lag_member_create(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_direction_t direction,
    switch_handle_t port_handle,
    switch_handle_t *lag_member_handle);

/**
 Link Aggregation Group member deletion by handle
 @param device device to use
 @param lag_member_handle handle of member returned on creation
 */
switch_status_t switch_api_lag_member_remove(switch_device_t device,
                                             switch_handle_t lag_member_handle);

/**
 Reactivate Link Aggregation Group member port
 @param device device to use
 @param lag_handle handle of group returned on creation
 @param port port in the same device on which lag_handle was created
 */
switch_status_t switch_api_lag_member_activate(switch_device_t device,
                                               switch_handle_t lag_handle,
                                               switch_handle_t port_handle);

/**
 Deactivate Link Aggregation Group member port
 @param device device to use
 @param lag_handle handle of group returned on creation
 @param port port in the same device on which lag_handle was created
 */
switch_status_t switch_api_lag_member_deactivate(switch_device_t device,
                                                 switch_handle_t lag_handle,
                                                 switch_handle_t port_handle);

/**
 Link Aggregation group member count
 @param lag_handle handle of the link aggregation group
 */
unsigned int switch_lag_get_count(switch_handle_t lag_handle);

/**
 Register a iterator function to walk through all the lag
 @param lag_id Lag index
 @param intf_handle List of member interfaces
 @param member_count Number of lag members
 */
typedef switch_status_t (*switch_lag_iterator_fn)(uint8_t lag_id,
                                                  switch_handle_t *intf_handle,
                                                  uint8_t member_count);

/**
  Set LAG label
  This API will be used to set label when bind type is
  SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param lag_handle – LAG handle
  @param label – port label
*/
switch_status_t switch_api_lag_ingress_acl_label_set(switch_device_t device,
                                                     switch_handle_t lag_handle,
                                                     switch_uint16_t label);
switch_status_t switch_api_lag_ingress_acl_label_get(switch_device_t device,
                                                     switch_handle_t lag_handle,
                                                     switch_uint16_t *label);
switch_status_t switch_api_lag_ingress_acl_group_set(switch_device_t device,
                                                     switch_handle_t lag_handle,
                                                     switch_handle_t acl_group);
switch_status_t switch_api_lag_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *acl_group);

/**
  Set LAG label
  This API will be used to set label when bind type is
  SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param lag_handle – LAG handle
  @param label – port label
*/
switch_status_t switch_api_lag_egress_acl_label_set(switch_device_t device,
                                                    switch_handle_t lag_handle,
                                                    switch_uint16_t label);
switch_status_t switch_api_lag_egress_acl_label_get(switch_device_t device,
                                                    switch_handle_t lag_handle,
                                                    switch_uint16_t *label);
switch_status_t switch_api_lag_egress_acl_group_set(switch_device_t device,
                                                    switch_handle_t lag_handle,
                                                    switch_handle_t acl_group);
switch_status_t switch_api_lag_egress_acl_group_get(switch_device_t device,
                                                    switch_handle_t lag_handle,
                                                    switch_handle_t *acl_group);

switch_status_t switch_api_lag_bind_mode_set(switch_device_t device,
                                             switch_handle_t lag_handle,
                                             switch_port_bind_mode_t bind_mode);

switch_status_t switch_api_lag_bind_mode_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_bind_mode_t *bind_mode);

switch_status_t swich_api_lag_handle_from_lag_member_get(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *lag_handle);
/**
 Calls the iterator function for every lag
 @param iterator_fn - Iterator function
 */
switch_status_t switch_api_lag_get(switch_lag_iterator_fn iterator_fn);

/**
 Dump lag table
 */
switch_status_t switch_api_lag_print_all();

/** @} */  // end of LAG

switch_status_t switch_api_lag_members_get(switch_device_t device,
                                           switch_handle_t lag_handle,
                                           switch_handle_t *members_handle);

switch_status_t switch_api_lag_member_count_get(switch_device_t device,
                                                switch_handle_t lag_handle,
                                                switch_uint32_t *member_count);

switch_status_t switch_api_lag_member_port_handle_get(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *port_handle);

switch_status_t switch_api_lag_handle_dump(const switch_device_t device,
                                           const switch_handle_t lag_handle,
                                           const void *cli_ctx);

switch_status_t switch_api_lag_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t lag_member_handle,
    const void *cli_ctx);

switch_status_t switch_api_interface_lag_stats_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters);

#ifdef __cplusplus
}
#endif

#endif
