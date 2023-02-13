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

#ifndef __SWITCH_MIRROR_H__
#define __SWITCH_MIRROR_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
*   @defgroup Mirror Mirroring API
*  API functions to manage mirror sessions
*  @{
*/
// begin of MIRROR API

/** Mirror ID */
typedef unsigned int switch_mirror_id_t;

/** mirror Session type */
typedef enum switch_mirror_session_type_s {
  SWITCH_MIRROR_SESSION_TYPE_SIMPLE = 0x0,
  SWITCH_MIRROR_SESSION_TYPE_TRUNCATE = 0x1,
  SWITCH_MIRROR_SESSION_TYPE_COALESCE = 0x2
} switch_mirror_session_type_t;

/** mirror Type */
typedef enum switch_mirror_type_s {
  SWITCH_MIRROR_TYPE_NONE = 0,
  SWITCH_MIRROR_TYPE_LOCAL = 1,
  SWITCH_MIRROR_TYPE_REMOTE = 2,
  SWITCH_MIRROR_TYPE_ENHANCED_REMOTE = 3,
  SWITCH_MIRROR_TYPE_DTEL_REPORT = 4
} switch_mirror_type_t;

/** mirror remote span type */
typedef enum switch_mirror_rspan_type_s {
  SWITCH_MIRROR_RSPAN_TYPE_NONE = 0x0,
  SWITCH_MIRROR_RSPAN_TYPE_VLAN_ID = 0x1,
  SWITCH_MIRROR_RSPAN_TYPE_VLAN_HANDLE = 0x2
} switch_mirror_rspan_type_t;

typedef enum switch_mirror_span_mode_s {
  SWITCH_MIRROR_SPAN_REWRITE_TYPE_NONE = 0x0,

  /**
   * tunnel nexthop is required in this mode.
   * uses nexthop tracking
   */
  SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP = 0x1,

  /**
   * tunnel parameters like sip and dip is required.
   * tunnel will be created implicitly.
   * uses nexthop tracking
   */
  SWITCH_MIRROR_SPAN_MODE_TUNNEL_PARAMS = 0x2,

  /**
   * tunnel is resolved and the final adjacencies
   * are provided.
   * nexthop tracking is not used
   */
  SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE = 0x3,

} switch_mirror_span_mode_t;

typedef enum switch_mirror_attribute_s {
  SWITCH_MIRROR_ATTRIBUTE_SRC_IP = (1 << 0),
  SWITCH_MIRROR_ATTRIBUTE_DST_IP = (1 << 1),
  SWITCH_MIRROR_ATTRIBUTE_SRC_MAC = (1 << 2),
  SWITCH_MIRROR_ATTRIBUTE_DST_MAC = (1 << 3),
  SWITCH_MIRROR_ATTRIBUTE_TTL = (1 << 4),
  SWITCH_MIRROR_ATTRIBUTE_TOS = (1 << 5),
  SWITCH_MIRROR_ATTRIBUTE_VLAN_ID = (1 << 6),
  SWITCH_MIRROR_ATTRIBUTE_VLAN_TPID = (1 << 7),
  SWITCH_MIRROR_ATTRIBUTE_VLAN_COS = (1 << 8),
} switch_mirror_attribute_t;

/** mirror Session Info */
typedef struct switch_api_mirror_info_s {
  /** mirror type - local/remote/erspan */
  switch_mirror_type_t mirror_type;

  /** mirror session id */
  switch_mirror_id_t session_id;

  /** mirror session type */
  switch_mirror_session_type_t session_type;

  /** egress port handle */
  switch_handle_t egress_port_handle;

  /** direction - ingress/egress */
  switch_direction_t direction;

  /** nexthop handle */
  switch_handle_t nhop_handle;

  /** remote span type */
  switch_mirror_rspan_type_t rspan_type;

  /** vlan id */
  switch_vlan_t vlan_id;

  /** ethertype */
  switch_uint16_t vlan_tpid;

  /** class of service */
  switch_uint8_t cos;

  /** vlan handle */
  switch_handle_t vlan_handle;

  /** vlan tag valid */
  bool vlan_tag_valid;

  /** span mode - valid only for ER and Telemetry */
  switch_mirror_span_mode_t span_mode;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** erspan source mac */
  switch_mac_addr_t src_mac;

  /** destinationmac */
  switch_mac_addr_t dst_mac;

  /** source ip address */
  switch_ip_addr_t src_ip;

  /** destination ip address */
  switch_ip_addr_t dst_ip;

  /** type of service */
  switch_uint8_t tos;

  /** ttl */
  switch_uint8_t ttl;

  /** maximum packet length */
  switch_uint32_t max_pkt_len;

  /** packet extract length */
  switch_uint32_t extract_len;

  /** timeout in micro seconds */
  switch_uint32_t timeout_usec;

} switch_api_mirror_info_t;

/**
 * MAX mirroring sessions supported
 */
#define SWITCH_MAX_MIRROR_SESSIONS 1024
/**
* ID for cpu mirror session
*/
#define SWITCH_CPU_MIRROR_SESSION_ID 250

/**
* ID for mirror on drop session
*/
#define SWITCH_MIRROR_ON_DROP_SESSION_ID 1015

/**
 * Platform Id for Traffic Manager
 */
#define SWITCH_PLATFORM_ID_TM 60

/**
 * Platform Id for Egress pipeline
 */
#define SWITCH_PLATFORM_ID_EGRESS 62

/*
 * Base ID for coalesced mirror session. A total of 8 ids from the base are
 * reserved for coalescing mirroring. Only these IDs can be used for coalesing.
 */

#define SWITCH_COALESCED_MIRROR_BASE_SESSION_ID 1016  // BF_MIRROR_COAL_BASE_SID
#define SWITCH_MAX_COALESCED_MIRROR_SESSIONS 8  // BF_MIRROR_COAL_SESSION_MAX
#define SWITCH_COALESCED_MIRROR_MAX_SESSION_ID \
  (SWITCH_COALESCED_MIRROR_BASE_SESSION_ID +   \
   SWITCH_MAX_COALESCED_MIRROR_SESSIONS - 1)

/**
 Create a mirror sesion
 @param device device on which to create mirror session
 @param api_mirror_info parameters of mirror session
*/

switch_status_t switch_api_mirror_session_create(
    switch_device_t device,
    switch_api_mirror_info_t *api_mirror_info,
    switch_handle_t *mirror_handle);

/**
 Update a mirror sesion
 @param device device on which to create mirror session
 @param mirror_handle mirror handle
 @param api_mirror_info parameters of mirror session
*/
switch_status_t switch_api_mirror_session_update(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const switch_uint64_t flags,
    const switch_api_mirror_info_t *api_mirror_info);

/**
 delete the mirror session
 @param device device
 @param mirror_handle mirror handle
*/
switch_status_t switch_api_mirror_session_delete(
    const switch_device_t device, const switch_handle_t mirror_handle);

switch_status_t switch_mirror_nhop_create(const switch_device_t device,
                                          const switch_handle_t mirror_handle,
                                          const switch_handle_t nhop_handle);

switch_status_t switch_mirror_nhop_delete(const switch_device_t device,
                                          const switch_handle_t mirror_handle);

switch_status_t switch_api_mirror_session_type_get(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_type_t *type);

switch_status_t switch_api_mirror_session_monitor_port_set(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *mirror_info);

switch_status_t switch_api_mirror_session_monitor_vlan_set(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *mirror_info);
switch_status_t switch_api_mirror_session_info_get(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_api_mirror_info_t *info);

switch_status_t switch_api_mirror_handle_dump(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const void *cli_ctx);

/** @} */  // end of Mirror API

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_MIRROR_H__ */
