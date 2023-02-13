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

#ifndef __SWITCH_RIF_H__
#define __SWITCH_RIF_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_tunnel.h"
#include "switch_nat.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_rif_type_s {
  SWITCH_RIF_TYPE_NONE = 0,
  SWITCH_RIF_TYPE_VLAN = 1,
  SWITCH_RIF_TYPE_LN = 2,
  SWITCH_RIF_TYPE_INTF = 3,
  SWITCH_RIF_TYPE_LOOPBACK = 4,
  SWITCH_RIF_TYPE_MAX
} switch_rif_type_t;

typedef enum switch_urpf_mode_s {
  SWITCH_URPF_MODE_NONE = 0,
  SWITCH_URPF_MODE_LOOSE = 1,
  SWITCH_URPF_MODE_STRICT = 2
} switch_urpf_mode_t;

/** Interface attributes */
typedef enum switch_rif_attr_s {
  SWITCH_RIF_ATTR_VRF_HANDLE = 1 << 1,
  SWITCH_RIF_ATTR_RMAC_HANDLE = 1 << 4,
  SWITCH_RIF_ATTR_IPV4_UNICAST = 1 << 5,
  SWITCH_RIF_ATTR_IPV6_UNICAST = 1 << 6,
  SWITCH_RIF_ATTR_IPV4_MULTICAST = 1 << 5,
  SWITCH_RIF_ATTR_IPV6_MULTICAST = 1 << 6,
  SWITCH_RIF_ATTR_IPV4_URPF_MODE = 1 << 7,
  SWITCH_RIF_ATTR_IPV6_URPF_MODE = 1 << 8,
  SWITCH_RIF_ATTR_MTU_HANDLE = 1 << 9,

} switch_rif_attr_t;

/** rif information */
typedef struct switch_api_rif_info_s {
  switch_rif_type_t rif_type;

  switch_handle_t intf_handle;
  switch_vlan_t vlan;
  switch_handle_t ln_handle;

  switch_handle_t vrf_handle;  /**< vrf handle */
  switch_handle_t rmac_handle; /**< rmac group id */

  bool ipv4_unicast;                 /**< IPv4 unicast enabled */
  bool ipv6_unicast;                 /**< IPv6 unicast enabled */
  bool ipv4_multicast;               /**< IPv4 multicast enabled */
  bool ipv6_multicast;               /**< IPV6 multicast menabled */
  switch_urpf_mode_t ipv4_urpf_mode; /**< IPv4 urpf mode */
  switch_urpf_mode_t ipv6_urpf_mode; /**< IPv6 urpf mode */
  switch_handle_t mtu_handle;

  // TODO: unused? remove?
  switch_nat_mode_t nat_mode;
} switch_api_rif_info_t;

switch_status_t switch_rif_init(switch_device_t device);
switch_status_t switch_rif_free(switch_device_t device);

switch_status_t switch_api_rif_create(switch_device_t device,
                                      switch_api_rif_info_t *api_rif_info,
                                      switch_handle_t *rif_handle);
switch_status_t switch_api_rif_delete(switch_device_t device,
                                      switch_handle_t rif_handle);

switch_status_t switch_api_rif_vrf_handle_set(switch_device_t device,
                                              switch_handle_t rif_handle,
                                              switch_handle_t vrf_handle);

switch_status_t switch_api_rif_vrf_handle_get(switch_device_t device,
                                              switch_handle_t rif_handle,
                                              switch_handle_t *vrf_handle);

switch_status_t switch_api_rif_ipv4_unicast_set(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool set);

switch_status_t switch_api_rif_ipv6_unicast_set(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool set);

switch_status_t switch_api_rif_ipv4_multicast_set(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool set);

switch_status_t switch_api_rif_ipv6_multicast_set(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool set);

switch_status_t switch_api_rif_intf_handle_get(switch_device_t device,
                                               switch_handle_t rif_handle,
                                               switch_handle_t *intf_handle);

switch_status_t switch_api_rif_ipv4_unicast_get(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool *ipv4_unicast);

switch_status_t switch_api_rif_ipv6_unicast_get(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool *ipv6_unicast);
switch_status_t switch_api_rif_ipv4_multicast_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool *ipv4_multicast);

switch_status_t switch_api_rif_ipv6_multicast_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool *ipv6_multicast);

switch_status_t switch_api_rif_mtu_set(switch_device_t device,
                                       switch_handle_t rif_handle,
                                       switch_handle_t mtu_handle);

switch_status_t switch_api_rif_mtu_get(switch_device_t device,
                                       switch_handle_t rif_handle,
                                       switch_handle_t *mtu_handle);

switch_status_t switch_api_rif_type_get(switch_device_t device,
                                        switch_handle_t rif_handle,
                                        switch_rif_type_t *type);

switch_status_t switch_api_rif_attach_intf(switch_device_t device,
                                           switch_handle_t rif_handle,
                                           switch_handle_t intf_handle);

switch_status_t switch_api_rif_dettach_intf(switch_device_t device,
                                            switch_handle_t rif_handle);

switch_status_t switch_api_rif_attach_ln(switch_device_t device,
                                         switch_handle_t rif_handle,
                                         switch_handle_t ln_handle);

switch_status_t switch_api_rif_dettach_ln(switch_device_t device,
                                          switch_handle_t rif_handle);

switch_status_t switch_api_rif_attribute_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    switch_api_rif_info_t *api_rif_info);

switch_status_t switch_api_rif_attribute_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    const switch_api_rif_info_t *api_rif_info);

switch_status_t switch_api_rif_attribute_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    switch_api_rif_info_t *api_rif_info);

switch_status_t switch_api_rif_rmac_handle_get(const switch_device_t device,
                                               const switch_handle_t rif_handle,
                                               switch_handle_t *rmac_handle);

switch_status_t switch_api_rif_ingress_acl_group_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_handle_t acl_group_handle);

switch_status_t switch_api_rif_ingress_acl_group_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group_handle);

/**
  Set RIF vlan label
  This API will be used to set label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param rif_handle – RIF handle
  @param label – rif label
*/
switch_status_t switch_api_rif_ingress_acl_label_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_bd_label_t label);

/**
  Get RIF vlan label
  This API will be used to get label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param rif_handle – RIF handle
  @param label – rif label
*/
switch_status_t switch_api_rif_ingress_acl_label_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label);

switch_status_t switch_api_rif_egress_acl_group_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_handle_t acl_group_handle);

switch_status_t switch_api_rif_egress_acl_group_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group_handle);

/**
  Set RIF vlan label
  This API will be used to set label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param rif_handle – RIF handle
  @param label – rif label
*/
switch_status_t switch_api_rif_egress_acl_label_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_bd_label_t label);

/**
  Get RIF vlan label
  This API will be used to get label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param rif_handle – RIF handle
  @param label – rif label
*/
switch_status_t switch_api_rif_egress_acl_label_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label);

/**
  Get bd value of a rif handle
  @param - device - device
  @param - rif_handle - Rif handle that identifies rif uniquely
  @param - bd value - Return bd value which is derived via the bd handle
*/
switch_status_t switch_api_rif_bd_get(switch_device_t device,
                                      switch_handle_t rif_handle,
                                      switch_uint32_t *bd);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RIF_H__ */
