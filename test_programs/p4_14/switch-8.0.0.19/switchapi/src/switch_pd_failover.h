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

#ifndef _SWITCH_PD_FAILOVER_H_
#define _SWITCH_PD_FAILOVER_H_

#ifdef SWITCH_PD

// Selector pools with more than 120 entries are composed of multiple words of
// selector memory, each containing up to 120 entries.
#define MAX_PORT_INSTANCES (MAX_LAG_GROUP_SIZE + 119) / 120

#define MAX_NHOP_INSTANCES (MAX_ECMP_GROUP_SIZE + 119) / 120

#define ECMP_FAILOVER_RECIRC_PATTERN_VALUE 0xe2000000
#define ECMP_FAILOVER_RECIRC_PATTERN_MASK 0xffff0000

switch_status_t switch_pd_failover_pktgen_enable(switch_device_t device);

switch_status_t switch_pd_failover_pktgen_disable(switch_device_t device);

switch_status_t switch_pd_lag_failover_pktgen_init(switch_device_t device);

switch_status_t switch_pd_lag_failover_recirc_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_lag_failover_lookup_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_lag_failover_lookup_entry_add(
    switch_pd_target_t p4_pd_device,
    unsigned int port,
    unsigned int instance_id,
    int index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_lag_failover_lookup_modify_entry(
    switch_pd_target_t p4_pd_device, int index, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_lag_group_register_callback(switch_device_t device,
                                                      void *cookie);

switch_status_t switch_pd_lag_action_profile_set_fallback_member(
    switch_device_t device);

switch_status_t switch_pd_ecmp_failover_pktgen_init(switch_device_t device);

switch_status_t switch_pd_ecmp_failover_recirc_entry_add(
    switch_device_t device);

switch_status_t switch_pd_prepare_for_recirc_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_ecmp_failover_lookup_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_ecmp_failover_lookup_entry_add(
    switch_pd_target_t p4_pd_device,
    uint16_t nhop_index,
    unsigned int instance_id,
    int index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ecmp_failover_lookup_modify_entry(
    switch_pd_target_t p4_pd_device, int index, p4_pd_entry_hdl_t entry_hdl);

switch_status_t switch_pd_ecmp_group_register_callback(switch_device_t device,
                                                       void *cookie);

typedef struct switch_pd_failover_member_s {
  switch_node_t node;
  switch_uint32_t instance_id;
  switch_int32_t index;
  switch_pd_hdl_t entry_hdl;
  union {
    switch_port_t port;
    uint16_t nhop_index;
  } u;
} switch_pd_failover_member_t;

#endif /* SWITCH_PD */

#endif /* _SWITCH_PD_FAILOVER_H_ */
