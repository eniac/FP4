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

#ifndef _switch_mcast_h_
#define _switch_mcast_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_vlan.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup MULTICAST Multicast API
 *  API functions listed to create/delete multicast trees and to add/delete multicast routes MFIB
 *  @{
 The multicast API's are controlled based on the multicast protocols that propagates the route entries
 to MFIB. A multicast tree handle is programmed against each route in mfib. The tree is derived in the queuing block
 and the packet is replicated based on the members.
 */  // begin of MCAST
                                       // MCAST

/** Multicast mode */
typedef enum switch_mcast_mode_ {
  SWITCH_API_MCAST_IPMC_NONE,
  SWITCH_API_MCAST_IPMC_PIM_SM,
  SWITCH_API_MCAST_IPMC_PIM_BIDIR
} switch_mcast_mode_t;

typedef struct switch_mcast_member_s {
  switch_handle_t network_handle; /** Vlan or LN handle */
  switch_handle_t handle;         /** Port, Lag or interface handle */
} switch_mcast_member_t;

/* MCAST API's */
/**
  Create a Multicast Tree
  @param device - device that programs the tree
*/
switch_status_t switch_api_multicast_index_create(const switch_device_t device,
                                                  switch_handle_t *mgid_handle);

/**
  Delete a multicast tree
  @param device - device that programs the tree
  @param mgid_handle - Handle that uniquely identifies multicast tree
*/
switch_status_t switch_api_multicast_index_delete(
    const switch_device_t device, const switch_handle_t mgid_handle);

/**
 Add a list of members to multicast tree
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param mbr_count - Count of members
 @param mbrs - List of interfaces to be added to multicast tree
*/
switch_status_t switch_api_multicast_member_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs);

/**
 Delete a list of members to multicast tree
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param mbr_count - Count of members
 @param mbrs - List of interfaces to be deleted from multicast tree
*/
switch_status_t switch_api_multicast_member_delete(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_uint16_t num_mbrs,
    const switch_mcast_member_t *mbrs);

/**
 Add an ecmp group to multicast tree
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param ecmp_nhop_handle - Handle the uniquely identifies the ecmp nh group
*/
switch_status_t switch_api_multicast_ecmp_member_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle);

/**
 Remove an ecmp group from multicast tree
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param ecmp_nhop_handle - Handle the uniquely identifies the ecmp nh group
*/
switch_status_t switch_api_multicast_ecmp_member_delete(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t ecmp_nhop_handle);

/**
 Get the list of members of a multicast tree
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param mbr_count - Count of members
 @param mbrs - List of interfaces part of the multicast tree
*/
switch_status_t switch_api_multicast_member_get(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    switch_uint16_t *num_mbrs,
    switch_mcast_member_t **mbrs);

/**
 Add a (S,G) or (*, G) entry to MFIB.
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param rpf_handle - Handle that uniquely identifies RPF group
 @param vlan_vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 @param mc_mode - Multicast mode to indicate PIM SM/PIM BIDIR
*/
switch_status_t switch_api_multicast_mroute_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode);

/**
 Delete a (S,G) or (*, G) entry from MFIB.
 @param device - device that programs the tree
 @param vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
*/
switch_status_t switch_api_multicast_mroute_delete(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip);

/**
 Get stats per (S,G) or (*, G) entry from MFIB.
 @param device - device that programs the tree
 @param vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 &param counter - Counters returned
*/
switch_status_t switch_api_multicast_mroute_stats_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_counter_t *counter);

/**
 Configure MGID handle on RPF fail or mroute miss
 @param device - device that programs the tree
 @param mgid_handle - MGID handle
 @param vlan_handle - vlan handle
*/
switch_status_t switch_api_multicast_mroute_miss_mgid_set(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vlan_handle);

/**
 Update mgid of (S,G) or (*, G) entry to MFIB.
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param vlan_vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 @param mc_mode - Multicast mode to indicate PIM SM/PIM BIDIR
*/
switch_status_t switch_api_multicast_mroute_mgid_set(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode);

/**
 Update rpf of (S,G) or (*, G) entry to MFIB.
 @param device - device that programs the tree
 @param rpf_handle - Handle that uniquely identifies an RPF group
 @param vlan_vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 @param mc_mode - Multicast mode to indicate PIM SM/PIM BIDIR
*/
switch_status_t switch_api_multicast_mroute_rpf_set(
    const switch_device_t device,
    const switch_handle_t rpf_handle,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    const switch_mcast_mode_t mc_mode);

/**
 For a (S,G) or (*, G) get the multicast tree
 @param device - device that programs the tree
 @param vrf_handle - VRF handle
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 @param mgid_handle - MGID handle of multicast tree
 @param rpf_handle - RPF handle of multicast tree
*/
switch_status_t switch_api_multicast_mroute_tree_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle,
    switch_handle_t *rpf_handle);

/**
 Add an L2 (S,G) or (*, G) route entry to MFIB.
 @param device - device that programs the tree
 @param mgid_handle - Handle that uniquely identifies multicast tree
 @param vlan_handle - Handle of vlan to add L2 route
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
*/
switch_status_t switch_api_multicast_l2route_add(
    const switch_device_t device,
    const switch_handle_t mgid_handle,
    const switch_handle_t bd_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip);

/**
 Delete an L2 (S,G) or (*, G) route entry to MFIB.
 @param device - device that programs the tree
 @param vlan_handle - Handle of vlan to delete L2 route
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
*/
switch_status_t switch_api_multicast_l2route_delete(
    const switch_device_t device,
    const switch_handle_t bd_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip);

/**
 For an L2 (S,G) or (*, G) get the multicast tree
 @param device - device that programs the tree
 @param vlan_handle - Handle of vlan to delete L2 route
 @param src_ip - Source IP address
 @param grp_ip - Group IP address
 @param mgid_handle - Handle of multicast tree
*/
switch_status_t switch_api_multicast_l2route_tree_get(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const switch_ip_addr_t *src_ip,
    const switch_ip_addr_t *grp_ip,
    switch_handle_t *mgid_handle);

/** @} */  // end of mcast API

#ifdef __cplusplus
}
#endif

#endif
