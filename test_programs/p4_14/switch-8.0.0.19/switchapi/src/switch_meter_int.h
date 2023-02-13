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

#ifndef __SWITCH_METER_INT_H__
#define __SWITCH_METER_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** meter handle wrappers */
#define switch_meter_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_METER, sizeof(switch_meter_info_t))

#define switch_meter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_METER, _handle)

#define switch_meter_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_METER, _handle, (void **)_info)

typedef enum switch_meter_target_type_ {
  SWITCH_METER_TYPE_NORMAL,
  SWITCH_METER_TYPE_COPP,
  SWITCH_METER_TYPE_STORM_CONTROL,
} switch_meter_target_type_t;

static inline char *switch_meter_mode_to_string(
    switch_meter_mode_t meter_mode) {
  switch (meter_mode) {
    case SWITCH_METER_MODE_TWO_RATE_THREE_COLOR:
      return "Tr3C";
    case SWITCH_METER_MODE_STORM_CONTROL:
      return "storm control";
    default:
      return "none";
  }
}

static inline char *switch_meter_type_to_string(
    switch_meter_type_t meter_type) {
  switch (meter_type) {
    case SWITCH_METER_TYPE_PACKETS:
      return "packets";
    case SWITCH_METER_TYPE_BYTES:
      return "bytes";
    default:
      return "none";
  }
}

static inline char *switch_meter_target_type_to_string(
    switch_meter_target_type_t type) {
  switch (type) {
    case SWITCH_METER_TYPE_NORMAL:
      return "normal";
    case SWITCH_METER_TYPE_COPP:
      return "copp";
    case SWITCH_METER_TYPE_STORM_CONTROL:
      return "storm";
    default:
      return "none";
  }
}

static inline char *switch_meter_color_source_to_string(
    switch_meter_color_source_t color_source) {
  switch (color_source) {
    case SWITCH_METER_COLOR_SOURCE_BLIND:
      return "blind";
    case SWITCH_METER_COLOR_SOURCE_AWARE:
      return "aware";
    default:
      return "none";
  }
}

static inline char *switch_packet_action_to_string(
    switch_acl_action_t acl_action) {
  switch (acl_action) {
    case SWITCH_ACL_ACTION_PERMIT:
      return "permit";
    case SWITCH_ACL_ACTION_DROP:
      return "drop";
    default:
      return "unknown";
  }
}

static inline char *switch_meter_counter_id_to_string(
    switch_meter_counter_t counter_id) {
  switch (counter_id) {
    case SWITCH_METER_COUNTER_GREEN:
      return "green";
    case SWITCH_METER_COUNTER_YELLOW:
      return "yellow";
    case SWITCH_METER_COUNTER_RED:
      return "red";
    default:
      return "none";
  }
}

/** meter info identified by meter handle */
typedef struct switch_meter_info_s {
  /** application meter info */
  switch_api_meter_t api_meter_info;

  /** meter counter stats */
  switch_counter_t counters[SWITCH_METER_COUNTER_MAX];

  /** meter index table hardware handle */
  switch_pd_hdl_t meter_idx_pd_hdl;

  /** meter action table hardware handle */
  switch_pd_hdl_t action_pd_hdl[SWITCH_COLOR_MAX];

  /** policer type */
  switch_meter_target_type_t meter_type;
} switch_meter_info_t;

switch_status_t switch_meter_init(switch_device_t device);

switch_status_t switch_meter_free(switch_device_t device);

switch_status_t switch_meter_default_entries_add(switch_device_t device);

switch_status_t switch_meter_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_METER_INT_H__ */
