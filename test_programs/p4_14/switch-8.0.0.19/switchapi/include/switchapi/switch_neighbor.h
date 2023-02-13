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

#ifndef __SWITCH_NEIGHBOR_H__
#define __SWITCH_NEIGHBOR_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup ARP ARP/Neighbor API
 *  API functions to add IP-Mac associations
 *  @{
 */  // begin of ARP API

// ARP
/** ARP information */

/** Neighbor type */
typedef enum switch_neighbor_tunnel_type_s {
  SWITCH_NEIGHBOR_TUNNEL_TYPE_NONE = 0,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_L3VPN = 1,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_PUSH_L3VPN = 2,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN = 3,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN = 4,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_IPV4_TUNNEL = 5,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_IPV6_TUNNEL = 6,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L2VPN = 7,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L3VPN = 8,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_PUSH_L3VPN = 9,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_L3VPN = 10,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L2VPN = 11,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L3VPN = 12,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_PUSH_L3VPN = 13,
  SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_L3VPN = 14,
} switch_neighbor_tunnel_type_t;

/** Neighbor rewrite type */
typedef enum switch_neighbor_rw_type_s {
  SWITCH_API_NEIGHBOR_RW_TYPE_L2 = 0,
  SWITCH_API_NEIGHBOR_RW_TYPE_L3 = 1,
  SWITCH_API_NEIGHBOR_RW_TYPE_L2_MIRROR = 2,
  SWITCH_API_NEIGHBOR_RW_TYPE_L3_VNI = 3
} switch_neighbor_rw_type_t;

typedef enum switch_neighbor_type_s {
  SWITCH_NEIGHBOR_TYPE_IP = 0x0,
  SWITCH_NEIGHBOR_TYPE_NHOP = 0x1
} switch_neighbor_type_t;

/** Neighbor identifier */
typedef struct switch_api_neighbor_info_s {
  /** neighbor type */
  switch_neighbor_type_t neighbor_type;

  /** neighbor tunnel type */
  switch_neighbor_tunnel_type_t neighbor_tunnel_type;

  /** rewrite type */
  switch_neighbor_rw_type_t rw_type;

  /** nhop handle */
  switch_handle_t nhop_handle;

  /** rif handle */
  switch_handle_t rif_handle;

  /** ip address */
  switch_ip_addr_t ip_addr;

  /** destination mac address */
  switch_mac_addr_t mac_addr;

  /** set host route */
  bool set_host_route;

} switch_api_neighbor_info_t;

/**
ARP entry add
@param device device
@param neighbor - ARP information used to set egress table
*/
switch_status_t switch_api_neighbor_create(
    switch_device_t device,
    switch_api_neighbor_info_t *api_neighbor,
    switch_handle_t *neighbor_handle);

/**
ARP entry delete
@param device device
@param neighbor_handle - handle of the arp entry
*/
switch_status_t switch_api_neighbor_delete(switch_device_t device,
                                           switch_handle_t neighbor_handle);

switch_status_t switch_api_neighbor_handle_dump(
    const switch_device_t device,
    const switch_handle_t nieighbor_handle,
    const void *cli_ctx);
/** @} */  // end of ARP API

switch_status_t switch_api_neighbor_entry_rewrite_mac_get(
    switch_device_t device,
    switch_handle_t neighbor_handle,
    switch_mac_addr_t *mac);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_NEIGHBOR_H__ */
