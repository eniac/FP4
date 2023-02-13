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

#ifndef __SWITCH_VLAN_H__
#define __SWITCH_VLAN_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_status.h"
#include "switch_bd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_vlan_attribute_s {
  SWITCH_VLAN_ATTR_LEARNING_ENABLED = (1 << 0),
  SWITCH_VLAN_ATTR_IGMP_SNOOPING_ENABLED = (1 << 1),
  SWITCH_VLAN_ATTR_MLD_SNOOPING_ENABLED = (1 << 2),
  SWITCH_VLAN_ATTR_AGING_INTERVAL = (1 << 3),
  SWITCH_VLAN_ATTR_STP_HANDLE = (1 << 4),
  SWITCH_VLAN_ATTR_MRPF_GROUP = (1 << 5)
} switch_vlan_attribute_t;

typedef struct switch_api_vlan_info_s {
  bool learning_enabled;
  bool igmp_snooping_enabled;
  bool mld_snooping_enabled;
  switch_int32_t aging_interval;
  switch_handle_t stp_handle;
  switch_mrpf_group_t mrpf_group;
} switch_api_vlan_info_t;

/** Vlan Interface info */
typedef struct switch_vlan_interface_ {
  switch_handle_t vlan_handle;   /**< vlan handle */
  switch_handle_t intf_handle;   /**< interface handle */
  switch_handle_t member_handle; /**< member handle */
} switch_vlan_interface_t;

/** @defgroup VLAN VLAN configuration API
 *  API functions listed to configure VLAN
    The basic L2 domain for isolating traffic is configured using
    configuration of VLANs.  The maximum number of VLANs supported
    on the device is limited to 4k (4096). The operations on VLAN
    correspond to setting up broadcast domain and optionally ingress
    and egress VLAN translate tables.
 *  @{
 */  // begin of VLAN

// VLAN

/**
 VLAN create
 @param device device
 @param vlan_id Id of the VLAN
*/
switch_status_t switch_api_vlan_create(const switch_device_t device,
                                       const switch_vlan_t vlan_id,
                                       switch_handle_t *vlan_handle);

/**
 Delete VLAN
 @param device device
 @param vlan_handle handle of VLAN returned by create
*/

switch_status_t switch_api_vlan_delete(const switch_device_t device,
                                       const switch_handle_t vlan_handle);

switch_status_t switch_api_vlan_id_delete(const switch_device_t device,
                                          const switch_vlan_t vlan_id);

switch_status_t switch_api_vlan_attribute_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    const switch_api_vlan_info_t *api_vlan_info);

switch_status_t switch_api_vlan_attribute_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint64_t flags,
    switch_api_vlan_info_t *api_vlan_info);

/**
  Set vlan learning attribute
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - enable - Boolean value
*/
switch_status_t switch_api_vlan_learning_set(const switch_device_t device,
                                             const switch_handle_t vlan_handle,
                                             const bool enable);
/**
  Get vlan learning attribute
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - enable - Boolean value
*/
switch_status_t switch_api_vlan_learning_get(const switch_device_t device,
                                             const switch_handle_t vlan_handle,
                                             bool *enable);
/**
  Set mac age interval on vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of mac learning enabled
*/
switch_status_t switch_api_vlan_aging_interval_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_int32_t age_interval);

/**
  Get mac age interval on vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of mac age interval
*/
switch_status_t switch_api_vlan_aging_interval_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_int32_t *age_interval);

/**
  Set igmp snooping enable flag for vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of igmp snooping enabled flag
*/
switch_status_t switch_api_vlan_igmp_snooping_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable);

/**
  Get igmp snooping enable flag for vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of igmp snooping enabled flag
*/
switch_status_t switch_api_vlan_igmp_snooping_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable);

/**
  Set mld snooping enable flag for vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of mld snooping enabled flag
*/
switch_status_t switch_api_vlan_mld_snooping_set(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const bool enable);

/**
  Get mld snooping enable flag for vlan
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param value - Value of mld snooping enabled flag
*/
switch_status_t switch_api_vlan_mld_snooping_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    bool *enable);

/**
  Add ports to vlan. By default, ports will be added to the flood list
  based on the flood type.
  @param device device
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param port_count - Number of ports to be added to vlan
  @param vlan_port - List of interfaces/ports/lags
*/
switch_status_t switch_api_vlan_member_add(const switch_device_t device,
                                           const switch_handle_t vlan_handle,
                                           const switch_handle_t intf_handle,
                                           switch_handle_t *member_handles);

/**
  Remove ports from vlan. By default, ports will be removed from flood list
  based on the flood type.
  @param device device
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param port_count - Number of ports to be removed from vlan
  @param vlan_port- List of interfaces/ports/lags
*/
switch_status_t switch_api_vlan_member_remove(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_handle_t intf_handle);

switch_status_t switch_api_vlan_member_remove_by_member_handle(
    const switch_device_t device, const switch_handle_t member_handles);

/**
  Get the list of interfaces that belong to a vlan.
  @param device device
  @param vlan_handle - Vlan handle that identifies vlan uniquely
  @param mbr_count - Number of interfaces
  @param mbrs - List of interfaces
*/
switch_status_t switch_api_vlan_interfaces_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_uint16_t *num_entries,
    switch_vlan_interface_t **mbrs);

/**
 Get vlan id to vlan handle mapping
 @param device device
 @param vlan_id vlan id
 @param vlan_handle vlan handle
*/
switch_status_t switch_api_vlan_id_to_handle_get(const switch_device_t device,
                                                 const switch_vlan_t vlan_id,
                                                 switch_handle_t *vlan_handle);

/**
 Get vlan handle to vlan id mapping
 @param vlan_handle vlan handle
 @param vlan_id vlan id
*/
switch_status_t switch_api_vlan_handle_to_id_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    switch_vlan_t *vlan_id);

/**
 Enable vlan statistics
 @param device device to be programmed
 @param vlan_handle Vlan handle that identifies vlan uniquely
 */
switch_status_t switch_api_vlan_stats_enable(const switch_device_t device,
                                             const switch_handle_t vlan_handle);

/**
 Enable vlan statistics
 @param device device to be programmed
 @param vlan_handle Vlan handle that identifies vlan uniquely
 */
switch_status_t switch_api_vlan_stats_disable(
    const switch_device_t device, const switch_handle_t vlan_handle);

/**
 Get vlan statistics
 @param device device to be programmed
 @param vlan_handle Vlan handle that identifies vlan uniquely
 @param count number of counter ids
 @param counter_ids list of counter ids
 @param counters counter values to be returned
 */
switch_status_t switch_api_vlan_stats_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_uint8_t num_entries,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters);

switch_status_t switch_api_vlan_handle_dump(const switch_device_t device,
                                            const switch_handle_t vlan_handle,
                                            const void *cli_ctx);

switch_status_t switch_api_vlan_id_dump(const switch_device_t device,
                                        const switch_vlan_t vlan_id,
                                        const void *cli_ctx);

/**
  Set spanning tree handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - stp_handle - Spanning tree handle
*/
switch_status_t switch_api_vlan_stp_handle_set(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_handle_t stp_handle);

/**
  Get spanning tree handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - stp_handle - Spanning tree handle
*/
switch_status_t switch_api_vlan_stp_handle_get(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_handle_t *stp_handle);

/**
  Set multicast RPF group
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - mrpf_group - Multicast RPF group
*/
switch_status_t switch_api_vlan_mrpf_group_set(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_mrpf_group_t mrpf_group);

/**
  Get multicast RPF group
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - mrpf_group - Multicast RPF group
*/
switch_status_t switch_api_vlan_mrpf_group_get(switch_device_t device,
                                               switch_handle_t vlan_handle,
                                               switch_mrpf_group_t *mrpf_group);

/**
  Set bd label via ACL group handle
  The bd label will be derived from ACL group handle
  @param device – device
  @param vlan_handle – VLAN handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_vlan_ingress_acl_group_set(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t acl_group);

/**
  Get ACL group handle
  The bd label is derived from ACL group handle
  @param device – device
  @param vlan_handle – VLAN handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_vlan_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t *acl_group);

/**
  Set custom bd label
  This API has be used to set bd label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param vlan_handle – VLAN handle
  @param label – bd label
*/
switch_status_t switch_api_vlan_ingress_acl_label_set(
    switch_device_t device, switch_handle_t vlan_handle, switch_uint16_t label);

/**
  Get custom bd label
  This API has be used to get bd label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param vlan_handle – VLAN handle
  @param label – bd label
*/
switch_status_t switch_api_vlan_ingress_acl_label_get(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_uint16_t *label);

/**
  Set bd label via ACL group handle
  The bd label will be derived from ACL group handle
  @param device – device
  @param vlan_handle – VLAN handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_vlan_egress_acl_group_set(switch_device_t device,
                                                     switch_handle_t bd_handle,
                                                     switch_handle_t acl_group);

/**
  Get ACL group handle
  The bd label is derived from ACL group handle
  @param device – device
  @param vlan_handle – VLAN handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_vlan_egress_acl_group_get(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t *acl_group);

/**
  Set custom bd label
  This API has be used to set bd label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param vlan_handle – VLAN handle
  @param label – bd label
*/
switch_status_t switch_api_vlan_egress_acl_label_set(
    switch_device_t device, switch_handle_t vlan_handle, switch_uint16_t label);

/**
  Get custom bd label
  This API has be used to get bd label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param vlan_handle – VLAN handle
  @param label – bd label
*/
switch_status_t switch_api_vlan_egress_acl_label_get(
    switch_device_t device,
    switch_handle_t vlan_handle,
    switch_uint16_t *label);
/**
  Get VLAN Id of a vlan member handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - vlan_id - Return vlan id
*/
switch_status_t switch_api_vlan_member_vlan_id_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_vlan_t *vlan_id);

/**
  Get vlan tagging mode of a vlan member handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - tagging_mode - Return boolean tagging mode
*/
switch_status_t switch_api_vlan_member_vlan_tagging_mode_get(
    switch_device_t device, switch_handle_t vlan_member_handle, bool *tag_mode);

/**
  Get interface handle of a vlan member handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - intf_handle - Return interface handle
*/
switch_status_t switch_api_vlan_member_intf_handle_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_handle_t *intf_handle);

/**
  Get bd value of a vlan handle
  @param - device - device
  @param - vlan_handle - Vlan handle that identifies vlan uniquely
  @param - bd value - Return bd value which is derived via the bd handle
*/
switch_status_t switch_api_vlan_bd_get(switch_device_t device,
                                       switch_handle_t vlan_handle,
                                       switch_uint32_t *bd);

/** @} */  // end of VLAN

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_VLAN_H__ */
