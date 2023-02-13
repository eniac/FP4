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

#ifndef __SWITCH_NHOP_H__
#define __SWITCH_NHOP_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** nexthop id type */
typedef enum switch_nhop_id_type_s {
  SWITCH_NHOP_ID_TYPE_NONE = 0x0,
  SWITCH_NHOP_ID_TYPE_ONE_PATH = 0x1,
  SWITCH_NHOP_ID_TYPE_ECMP = 0x2,
  SWITCH_NHOP_ID_TYPE_WCMP = 0x3
} switch_nhop_id_type_t;

typedef enum switch_nhop_rewrite_type_s {
  SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2 = 0x0,
  SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3 = 0x1,
  SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR = 0x2,
  SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3_VNI = 0x3
} switch_nhop_tunnel_rewrite_type_t;

/** nexthop type */
typedef enum switch_nhop_type_s {
  SWITCH_NHOP_TYPE_NONE = 0x0,
  SWITCH_NHOP_TYPE_IP = 0x1,
  SWITCH_NHOP_TYPE_TUNNEL = 0x2,
  SWITCH_NHOP_TYPE_MPLS = 0x3,
  SWITCH_NHOP_TYPE_GLEAN = 0x4,
  SWITCH_NHOP_TYPE_DROP = 0x5
} switch_nhop_type_t;

typedef enum switch_nhop_tunnel_type_s {
  SWITCH_NHOP_TUNNEL_TYPE_NONE = 0x0,
  SWITCH_NHOP_TUNNEL_TYPE_VLAN = 0x1,
  SWITCH_NHOP_TUNNEL_TYPE_LN = 0x2,
  SWITCH_NHOP_TUNNEL_TYPE_VRF = 0x3
} switch_nhop_tunnel_type_t;

typedef struct PACKED switch_nhop_key_s {
  /** handle */
  switch_handle_t handle;

  /** ip address */
  switch_ip_addr_t ip_addr;

} switch_nhop_key_t;

/** Nexthop Key */
typedef struct switch_api_nhop_info_s {
  /** nhop type */
  switch_nhop_type_t nhop_type;

  /** nhop tunnel rewrite type */
  switch_nhop_tunnel_rewrite_type_t rewrite_type;

  /** nhop tunnel type */
  switch_nhop_tunnel_type_t nhop_tunnel_type;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** network handle - vlan/ln */
  switch_handle_t network_handle;

  /** rif handle */
  switch_handle_t rif_handle;

  /** tunnel handle */
  switch_handle_t tunnel_handle;

  /** mpls handle */
  switch_handle_t mpls_handle;

  /** interface handle */
  switch_handle_t intf_handle;

  /** mpls label stack handle */
  switch_handle_t label_stack_handle;

  /** nhop ip address */
  switch_ip_addr_t ip_addr;

  /** tunnel vni */
  switch_vni_t tunnel_vni;

  /** tunnel dmac address */
  switch_mac_addr_t mac_addr;

} switch_api_nhop_info_t;

/**
 Create a Nexthop
 @param device - device to program the nexthop
 @param nhop_key- Interface to be associated with the nexthop and nexthop ip
*/
switch_status_t switch_api_nhop_create(
    const switch_device_t device,
    const switch_api_nhop_info_t *api_nhop_info,
    switch_handle_t *nhop_handle);

/**
 Update a Nexthop
 @param device - device to program the nexthop
 @param handle - handle of the next hop to update
 @param nhop_key - nhop key with new info
*/
switch_status_t switch_api_nhop_set(switch_device_t device,
                                    switch_handle_t nhop_handle,
                                    switch_api_nhop_info_t *api_nhop_info);

/**
 Get attributes of a Nexthop
 @param device - device to program the nexthop
 @param handle - handle of the next hop to get
 @param [out]nhop_key - pointer to the attribute obj
*/
switch_status_t switch_api_nhop_get(switch_device_t device,
                                    switch_handle_t nhop_handle,
                                    switch_api_nhop_info_t *api_nhop_info);

/**
 Delete a Nexthop
 @param device device on which to create nhop group
 @param nhop_handle - Handle that identifies nexthop uniquely
*/
switch_status_t switch_api_nhop_delete(const switch_device_t device,
                                       const switch_handle_t nhop_handle);

/**
 Create a ECMP Group
 @param device - device to create the ecmp group
*/
switch_status_t switch_api_ecmp_create(const switch_device_t device,
                                       switch_handle_t *ecmp_handle);

/**
 Delete a ECMP Group
 @param ecmp_handle - Handle that identifies ECMP group uniquely
*/
switch_status_t switch_api_ecmp_delete(const switch_device_t device,
                                       const switch_handle_t ecmp_handle);

/**
 Add nexthop member to ecmp group
 @param device - device to program the nexthop
 @param ecmp_handle - handle that identifies ECMP group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be added to the ECMP Group
*/
switch_status_t switch_api_ecmp_member_add(const switch_device_t device,
                                           const switch_handle_t ecmp_handle,
                                           const switch_uint32_t num_nhops,
                                           const switch_handle_t *nhop_handles,
                                           switch_handle_t *member_handle);

/**
 Delete nexthop member from ecmp group
 @param device - device to program the nexthop
 @param ecmp_handle - handle that identifies ECMP group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be added to the ECMP Group
*/
switch_status_t switch_api_ecmp_member_delete(
    const switch_device_t device,
    const switch_handle_t ecmp_handle,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles);

/*
 Create ECMP Group along with the members.
 @param member_count - Number of nexthops
 @param nhop_handle - List of nexthops to be added to ECMP group
*/
switch_status_t switch_api_ecmp_create_with_members(
    const switch_device_t device,
    const switch_uint32_t num_nhops,
    const switch_handle_t *nhop_handles,
    switch_handle_t *ecmp_handle,
    switch_handle_t *member_handle);

/**
 Reactivate nexthop member from ecmp group
 @param device - device to program the nexthop
 @param ecmp_handle - handle that identifies ECMP group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be activated
*/
switch_status_t switch_api_ecmp_member_activate(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list);

/**
 Deactivate nexthop member from ecmp group
 @param device - device to program the nexthop
 @param ecmp_handle - handle that identifies ECMP group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be deactivated
*/
switch_status_t switch_api_ecmp_member_deactivate(
    switch_device_t device,
    switch_handle_t ecmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list);

/*
Create a WCMP group
@param device - device to create the WCMP group
*/
switch_status_t switch_api_wcmp_create(switch_device_t device,
                                       switch_handle_t *wcmp_handle);

/*
Delete a WCMP group
@param device - device with the WCMP group
@param wcmp_handle - handle that identifies WCMP group
*/
switch_status_t switch_api_wcmp_delete(switch_device_t device,
                                       switch_handle_t wcmp_handle);

/*
Add nexthop members to WCMP group
@param device - device with the WCMP group
@param wcmp_handle - handle that uniquely identifies WCMP group
@param nhop_count - number of nexthops
@param nhop_handle_list - list of nexthops to be added to the WCMP group
@param nhop_weight_list - list of nexthop weights
*/
switch_status_t switch_api_wcmp_member_add(switch_device_t device,
                                           switch_handle_t wcmp_handle,
                                           uint16_t nhop_count,
                                           switch_handle_t *nhop_handle_list,
                                           uint16_t *nhop_weight_list);

/*
Update the weight of nexthop members of WCMP group
@param device - device with the WCMP group
@param wcmp_handle - handle that uniquely identifies WCMP group
@param nhop_count - number of nexthops
@param nhop_handle_list - list of nexthops to be modified
@param nhop_weight_list - list of nexthop weights
*/
switch_status_t switch_api_wcmp_member_modify(switch_device_t device,
                                              switch_handle_t wcmp_handle,
                                              uint16_t nhop_count,
                                              switch_handle_t *nhop_handle_list,
                                              uint16_t *nhop_weight_list);
/*
Delete nexthop members from WCMP group
@param device - device to create the WCMP group
@param wcmp_handle - handle that uniquely identifies WCMP group
@param nhop_count - number of nexthops
@param nhop_handle_list - list of nexthops to be removed from the WCMP group
*/
switch_status_t switch_api_wcmp_member_delete(
    switch_device_t device,
    switch_handle_t wcmp_handle,
    uint16_t nhop_count,
    switch_handle_t *nhop_handle_list);

/*
 Return nexthop handle from (intf_handle, ip address)
 @param nhop_key- Interface to be associated with the nexthop and nexthop ip
 */
switch_status_t switch_api_nhop_handle_get(const switch_device_t device,
                                           const switch_nhop_key_t *nhop_key,
                                           switch_handle_t *nhop_handle);

/*
 Get neighbor handle from nexthop handle
 @param nhop_handle nexthop handle
 */
switch_status_t switch_api_neighbor_handle_get(
    const switch_device_t device,
    const switch_handle_t nhop_handle,
    switch_handle_t *neighbor_handle);

/*
 Get to know whether nhop is single path or ecmp
 @param nhop_handle nexthop handle
*/
switch_status_t switch_api_nhop_id_type_get(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            switch_nhop_id_type_t *nhop_type);

switch_status_t switch_api_ecmp_members_delete(switch_device_t device,
                                               switch_handle_t ecmp_handle);

switch_status_t switch_api_ecmp_members_get(const switch_device_t device,
                                            const switch_handle_t ecmp_handle,
                                            switch_uint16_t *num_nhops,
                                            switch_handle_t **nhop_handles);

switch_status_t switch_api_nhop_handle_dump(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            const void *cli_ctx);
switch_status_t switch_api_nhop_table_size_get(switch_device_t device,
                                               switch_size_t *tbl_size);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_NHOP_H__ */
