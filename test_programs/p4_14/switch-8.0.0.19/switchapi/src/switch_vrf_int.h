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

#ifndef __SWITCH_VRF_INT_H__
#define __SWITCH_VRF_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_MAX_VRF 1024

/* vrf handle wrappers */
#define switch_vrf_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_VRF, sizeof(switch_vrf_info_t))

#define switch_vrf_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_VRF, _handle)

#define switch_vrf_get(_device, _handle, _info, _status)                       \
  do {                                                                         \
    switch_vrf_context_t *_vrf_ctx = NULL;                                     \
    switch_handle_t _vrf_handle = SWITCH_API_INVALID_HANDLE;                   \
    _status = switch_device_api_context_get(                                   \
        _device, SWITCH_API_TYPE_VRF, (void **)&_vrf_ctx);                     \
    SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                           \
    _status =                                                                  \
        SWITCH_ARRAY_GET(&_vrf_ctx->vrf_array, _handle, (void *)&_vrf_handle); \
    SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                           \
    switch_handle_get(                                                         \
        _device, SWITCH_HANDLE_TYPE_VRF, _vrf_handle, (void **)_info);         \
  } while (0);

#define switch_vrf_get_internal(_device, _handle, _info, _status) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_VRF, _handle, (void **)_info);

#define SWITCH_VRF_CORE(_info) FALSE

/** stores vrf info and its associated hardware handles */
typedef struct switch_vrf_info_s {
  /**
   * rmac handle used by vrf.
   * used for l3 interfaces with no rmac handles
   */
  switch_handle_t rmac_handle;

  /** vrf id */
  switch_vrf_t vrf_id;

  /** vrf handle - self pointer */
  switch_handle_t vrf_handle;

  /** bd vrf handle - application vrf handle */
  switch_handle_t bd_vrf_handle;

  /** ipv4 unicast enabled */
  bool ipv4_unicast_enabled;

  /** ipv6 unicast enabled */
  bool ipv6_unicast_enabled;

  /** bd handle */
  switch_handle_t bd_handle;

  /** ipv4 vrf routes */
  switch_list_t ipv4_routes;

  /** ipv6 routes */
  switch_list_t ipv6_routes;

  /** ipv4 vrf lpm routes */
  switch_lpm_trie_t *ipv4_lpm_trie;

  /** ipv6 vrf lpm routes */
  switch_lpm_trie_t *ipv6_lpm_trie;

} switch_vrf_info_t;

#define SWITCH_BD_HANDLE_TO_VRF_HANDLE(_bd_handle) \
  ((_bd_handle & 0xFFFF) | (SWITCH_HANDLE_TYPE_VRF << SWITCH_HANDLE_TYPE_SHIFT))

/** vrf device context */
typedef struct switch_vrf_context_s {
  /** vrf id to handle array */
  switch_array_t vrf_id_array;

  /** bd vrf handle to vrf handle */
  switch_array_t vrf_array;

} switch_vrf_context_t;

switch_status_t switch_vrf_init(switch_device_t device);

switch_status_t switch_vrf_free(switch_device_t device);

switch_status_t switch_vrf_default_entries_add(switch_device_t device);

switch_status_t switch_vrf_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_VRF_INT_H__ */
