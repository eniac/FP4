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

#include "switch_pd_types.h"

#ifndef _SWITCH_MIRROR_INT_H_
#define _SWITCH_MIRROR_INT_H_

#define SWITCH_MIRROR_SESSIONS_MAX 1024

#define switch_mirror_handle_create(_device) \
  switch_handle_create(                      \
      _device, SWITCH_HANDLE_TYPE_MIRROR, sizeof(switch_mirror_info_t))

#define switch_mirror_handle_set(_device, _id) \
  switch_handle_create_and_set(                \
      _device, SWITCH_HANDLE_TYPE_MIRROR, _id, sizeof(switch_mirror_info_t))

#define switch_mirror_handle_get(_id) \
  id_to_handle(SWITCH_HANDLE_TYPE_MIRROR, _id)

#define switch_mirror_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MIRROR, _handle)

#define switch_mirror_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_MIRROR, _handle, (void **)_info)

#define SWITCH_MIRROR_SESSION_ID_COALESCING(_session_id)       \
  ((_session_id >= SWITCH_COALESCED_MIRROR_BASE_SESSION_ID) && \
   (_session_id <= SWITCH_COALESCED_MIRROR_MAX_SESSION_ID))    \
      ? TRUE                                                   \
      : FALSE

#define SWITCH_MIRROR_TYPE(_mirror_info) \
  _mirror_info->api_mirror_info.mirror_type

static inline char *switch_mirror_session_type_to_string(
    switch_mirror_session_type_t session_type) {
  switch (session_type) {
    case SWITCH_MIRROR_SESSION_TYPE_SIMPLE:
      return "simple";
    case SWITCH_MIRROR_SESSION_TYPE_TRUNCATE:
      return "truncate";
    case SWITCH_MIRROR_SESSION_TYPE_COALESCE:
      return "coalesce";
    default:
      return "unknown";
  }
}

static inline char *switch_mirror_type_to_string(
    switch_mirror_type_t mirror_type) {
  switch (mirror_type) {
    case SWITCH_MIRROR_TYPE_NONE:
      return "none";
    case SWITCH_MIRROR_TYPE_LOCAL:
      return "local";
    case SWITCH_MIRROR_TYPE_REMOTE:
      return "remote";
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE:
      return "enhanced remote";
    case SWITCH_MIRROR_TYPE_DTEL_REPORT:
      return "telemetry report";
    default:
      return "unknown";
  }
}

#pragma pack(1)
/* This is an intenal header used when coal mirror frame is processed
 * This header is defined in P4 program and is extracted by egress
 * parser
 * This header is programmed in the hw as a set of 4 32 bit registers,
 * where first two bytes are fixed as - num_samples, compiler_defined_indication
 * the rest of the bytes are P4 program defined as -
 * +-------------+-------------+-------------+-------------+
 * | num_samples | compiler_def|       mirror_id           | Word 0
 * +-------------+-------------+-------------+-------------+
 * +-------------+-------------+-------------+-------------+ Word 1-3 (unused)
 * +-------------+-------------+-------------+-------------+
 * +-------------+-------------+-------------+-------------+
 */
typedef struct switch_coal_pkt_hdr_ {
  uint32_t reg_hdr0;
} switch_coal_pkt_hdr_t;

#pragma pack()

/** stores mirror info and associated hardware handles */
typedef struct switch_mirror_info_s {
  /** application mirror info */
  switch_api_mirror_info_t api_mirror_info;

  switch_uint32_t max_pkt_len;

  /** interface handle created for remote mirror */
  switch_handle_t intf_handle;

  /** vlan handle created for remote mirror */
  switch_handle_t vlan_handle;

  /** tunnel handle created for erspan type params */
  switch_handle_t tunnel_handle;

  /** overlay rif handle created for erspan type params */
  switch_handle_t orif_handle;

  /** underlay rif handle created for erspan type params */
  switch_handle_t urif_handle;

  switch_pd_hdl_t pd_hdl;
  switch_coal_pkt_hdr_t int_coal_pkt_hdr;
  switch_uint8_t int_hdr_len;

  /** multicast tree handle */
  switch_handle_t mgid_handle;

  /** enable mirror session */
  bool enable;

} switch_mirror_info_t;

/** mirror device context */
typedef struct switch_mirror_context_s {
  /** mirror session id allocator */
  switch_id_allocator_t *session_id_allocator;

  /** mirror drop handle */
  switch_handle_t mirror_drop_handle;

  /** negative mirroring pd handle */
  switch_pd_hdl_t neg_mirror_pd_hdl;

} switch_mirror_context_t;

switch_status_t switch_mirror_init(switch_device_t device);

switch_status_t switch_mirror_free(switch_device_t device);

switch_status_t switch_mirror_default_entries_add(switch_device_t device);

switch_status_t switch_mirror_default_entries_delete(switch_device_t device);

switch_status_t switch_api_mirror_session_update_mgid(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const switch_handle_t mgid_handle);

#endif /* _SWITCH_MIRROR_INT_H_ */
