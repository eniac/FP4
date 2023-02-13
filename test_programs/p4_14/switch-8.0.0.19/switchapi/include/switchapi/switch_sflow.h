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

#ifndef _switch_sflow_h_
#define _switch_sflow_h_

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Maximum sflow session */
#define SWITCH_MAX_SFLOW_SESSIONS \
  16  // MAX_SFLOW_SESSIONS from p4_table_sizes.h

/** Maximum sflow access control entries */
#define SWITCH_MAX_SFLOW_ACES 512  // MAX_SFLOW_SESSIONS from p4_table_sizes.h

/** sflow match fields */
typedef enum switch_sflow_match_field_ {
  SWITCH_SFLOW_MATCH_PORT = 0,
  SWITCH_SFLOW_MATCH_VLAN = 1,
  SWITCH_SFLOW_MATCH_SIP = 2,
  SWITCH_SFLOW_MATCH_DIP = 3,
  SWITCH_SFLOW_MATCH_FIELD_MAX = 4,
} switch_sflow_match_field_t;

/** sflow match values */
typedef union switch_sflow_match_value_ {
  switch_handle_t port;
  switch_vlan_t vlan;
  switch_uint32_t sip;
  switch_uint32_t dip;
} switch_sflow_match_value_t;

/** sflow match mask - same as masks used for acl */
typedef union switch_sflow_match_mask_ {
  unsigned type : 1; /**< mask type */
  union {
    switch_uint64_t mask;      /**< mask value */
    switch_int32_t start, end; /**< mask range */
  } u;                         /**< ip mask union */
} switch_sflow_match_mask_t;

/** Egress acl key value pair */
typedef struct switch_sflow_match_key_value_pair_ {
  switch_sflow_match_field_t field; /**< sflow match fields */
  switch_sflow_match_value_t value; /**< sflow match values */
  switch_sflow_match_mask_t mask;   /**< sflow match masks */
} switch_sflow_match_key_value_pair_t;

/** Sflow collector type */
typedef enum {
  SFLOW_COLLECTOR_TYPE_CPU = 0,
  SFLOW_COLLECTOR_TYPE_REMOTE = 1
} switch_sflow_collector_type_e;

/** Sflow sampling mode */
typedef enum {
  SWITCH_SFLOW_SAMPLE_PKT = 0,
  SWITCH_SFLOW_SAMPLE_COALESCED,
} switch_sflow_sample_mode_e;

/** sflow session struct */
typedef struct switch_api_sflow_session_info_ {
  switch_uint32_t session_id;
  switch_uint32_t timeout_usec;  // 0 => 100us (default)
  switch_uint32_t sample_rate;   // 0 => every 10k pkts (default)
  switch_uint32_t extract_len;   // 0 => 80 (default)
  switch_handle_t egress_port_handle;
  switch_sflow_collector_type_e collector_type;
  switch_sflow_sample_mode_e sample_mode;
} switch_api_sflow_session_info_t;

switch_status_t switch_api_sflow_session_create(
    const switch_device_t device,
    const switch_api_sflow_session_info_t *api_sflow_info,
    switch_handle_t *sflow_handle);

switch_status_t switch_api_sflow_session_delete(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const bool cleanup);

switch_status_t switch_api_sflow_session_attach(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_direction_t direction,
    const switch_uint16_t priority,
    const switch_uint32_t sample_rate,
    const switch_uint16_t kvp_count,
    const switch_sflow_match_key_value_pair_t *kvp,
    switch_handle_t *entry_handle);

switch_status_t switch_api_sflow_session_detach(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t entry_handle);

switch_status_t switch_api_sflow_session_port_set(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t port,
    const switch_direction_t dir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _switch_sflow_h_ */
