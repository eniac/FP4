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

#ifndef __SWITCH_MPLS_H__
#define __SWITCH_MPLS_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup Tunnel Tunnel API
 *  API functions create tunnel interfaces
 *  @{
 */  // begin of Tunnel API

/** Maximum mpls labels supported */
#define SWITCH_MPLS_LABEL_MAX 5

/** Mpls ipv4 explicit null label */
#define SWITCH_MPLS_IPV4_EXPLICIT_NULL 0

/** Mpls ipv6 explicit null label */
#define SWITCH_MPLS_IPV6_EXPLICIT_NULL 2

/** Mpls tunnel type */
typedef enum switch_mpls_type_s {
  SWITCH_MPLS_TYPE_NONE = 0x0,
  SWITCH_MPLS_TYPE_EOMPLS = 0x1,
  SWITCH_MPLS_TYPE_IPV4_MPLS = 0x2,
  SWITCH_MPLS_TYPE_IPV6_MPLS = 0x3,
  SWITCH_MPLS_TYPE_VPLS = 0x4,
  SWITCH_MPLS_TYPE_PW = 0x5
} switch_mpls_type_t;

/** Mpls mode */
typedef enum switch_mpls_mode_s {
  SWITCH_MPLS_MODE_NONE = 0x0,
  SWITCH_MPLS_MODE_INITIATE = 0x1,
  SWITCH_MPLS_MODE_TRANSIT = 0x2,
  SWITCH_MPLS_MODE_TERMINATE = 0x3
} switch_mpls_mode_t;

typedef enum switch_mpls_tunnel_type_s {
  SWITCH_MPLS_TUNNEL_TYPE_MPLS = 0x1,
  SWITCH_MPLS_TUNNEL_TYPE_MPLS_UDP = 0x2
} switch_mpls_tunnel_type_t;

/** mpls header */
typedef struct switch_mpls_s {
  /** mpls label */
  switch_mpls_label_t label;

  /** type of service */
  switch_uint8_t exp;

  /** time to live */
  switch_uint8_t ttl;

} switch_mpls_t;

typedef struct switch_mpls_label_stack_s {
  /** list of mpls labels */
  switch_mpls_t label_list[SWITCH_MPLS_LABEL_MAX];

  /** number of labels */
  switch_uint16_t num_labels;

  /** bottom of stack bit */
  bool bos;

} switch_mpls_label_stack_t;

/** mpls information */
typedef struct switch_api_mpls_info_s {
  /** mpls tunnel type */
  switch_mpls_tunnel_type_t tunnel_type;

  /** mpls type */
  switch_mpls_type_t mpls_type;

  /** mpls mode */
  switch_mpls_mode_t mpls_mode;

  /** vrf handle for l3vpn */
  switch_handle_t vrf_handle;

  /** router mac handle */
  switch_handle_t rmac_handle;

  /** vlan/ln handle for l2-l3 vpn */
  switch_handle_t network_handle;

  /** swap label */
  switch_mpls_label_t swap_label;

  /** last pop label */
  switch_mpls_label_t pop_label;

  /** number of mpls pop */
  switch_uint8_t pop_count;

  /** interface handle */
  switch_handle_t intf_handle;

  /** nhop handle */
  switch_handle_t nhop_handle;

  /** outer destination mac address */
  switch_mac_addr_t mac_addr;

} switch_api_mpls_info_t;

switch_status_t switch_api_mpls_tunnel_create(
    switch_device_t device,
    switch_api_mpls_info_t *api_mpls_info,
    switch_handle_t *mpls_handle);

switch_status_t switch_api_mpls_tunnel_delete(switch_device_t device,
                                              switch_handle_t mpls_handle);

switch_status_t switch_api_mpls_label_stack_create(
    switch_device_t device,
    switch_mpls_label_stack_t *label_stack,
    switch_handle_t *label_stack_handle);

switch_status_t switch_api_mpls_label_stack_delete(
    switch_device_t device, switch_handle_t label_stack_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_MPLS_H__ */
