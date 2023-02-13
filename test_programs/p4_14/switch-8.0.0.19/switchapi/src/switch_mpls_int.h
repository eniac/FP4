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

#ifndef __SWITCH_MPLS_INT_H__
#define __SWITCH_MPLS_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** mpls handle wrappers */
#define switch_mpls_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_MPLS, sizeof(switch_mpls_info_t))

#define switch_mpls_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MPLS, _handle)

#define switch_mpls_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_MPLS, _handle, (void **)_info)

/** label stack wrappers */
#define switch_mpls_label_stack_handle_create(_device)      \
  switch_handle_create(_device,                             \
                       SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK, \
                       sizeof(switch_mpls_label_stack_info_t))

#define switch_mpls_label_stack_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK, _handle)

#define switch_mpls_label_stack_get(_device, _handle, _info) \
  switch_handle_get(                                         \
      _device, SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK, _handle, (void **)_info)

#define SWITCH_MPLS_HANDLE_SIZE 8192

#define SWITCH_LABEL_STACK_HANDLE_SIZE 8192

/** hardware ingress mpls tunnel type used for decap */
typedef enum switch_mpls_tunnel_subtype_ingress_s {
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_NONE = 0,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_1 = 6,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_2 = 7,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_3 = 8,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_1 = 9,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_2 = 10,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_3 = 11
} switch_mpls_tunnel_subtype_ingress_t;

typedef enum switch_mpls_tunnel_type_ingress_s {
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS = 6,
  SWITCH_TUNNEL_TYPE_INGRESS_MPLS_UDP = 13,
} switch_mpls_tunnel_type_ingress_t;

typedef enum switch_mpls_tunnel_type_egress_s {
  SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN = 13,
  SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN = 14,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN = 20,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN = 21,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN = 22,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN = 23,
} switch_mpls_tunnel_type_egress_t;

typedef struct switch_mpls_info_s {
  /** mpls info */
  switch_api_mpls_info_t api_mpls_info;

  /** bd handle */
  switch_handle_t bd_handle;

  /** mpls ingress tunnel type */
  switch_mpls_tunnel_type_ingress_t ingress_tunnel_type;

  /** mpls egress tunnel type */
  switch_mpls_tunnel_type_egress_t egress_tunnel_type;

  /** mpls ingress tunnel subtype */
  switch_mpls_tunnel_subtype_ingress_t mpls_tunnel_type;

  /** tunnel dmac index */
  switch_id_t tunnel_dmac_index;

  /** hardware handles for tunnel table */
  switch_pd_hdl_t pd_hdl[2];

} switch_mpls_info_t;

/** label stack */
typedef struct switch_mpls_label_stack_info_s {
  /** list of mpls labels */
  switch_mpls_label_stack_t label_stack;

  /** tunnel rewrite pd handle */
  switch_pd_hdl_t rw_pd_hdl;

  /** tunnel handle */
  switch_handle_t tunnel_handle;

} switch_mpls_label_stack_info_t;

/** mpls device context */
typedef struct switch_mpls_context_s {
  /** mpls label array indexed by label */
  switch_array_t *mpls_array;

} switch_mpls_context_t;

switch_status_t switch_mpls_init(switch_device_t device);

switch_status_t switch_mpls_free(switch_device_t device);

switch_status_t switch_mpls_default_entries_add(switch_device_t device);

switch_status_t switch_mpls_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_MPLS_INT_H__ */
