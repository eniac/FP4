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

#ifndef __SWITCH_QUEUE_INT_H__
#define __SWITCH_QUEUE_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_QUEUE_HANDLE_SIZE 4096

/** queue handle wrappers */
#define switch_queue_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_QUEUE, sizeof(switch_queue_info_t))

#define switch_queue_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_QUEUE, _handle)

#define switch_queue_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_QUEUE, _handle, (void **)_info)

/** queue struct to queue info and its associated hardware handles */
typedef struct switch_queue_info_s {
  bool color_drop_enable;

  /** port associated with queue */
  switch_handle_t port_handle;

  /** buffer profile handle assocaited with queue */
  switch_handle_t buffer_profile_handle;

  /** queue id */
  switch_qid_t queue_id;

  /** wred profile handle */
  switch_handle_t wred_profile_handle;

  /** wred handles */
  switch_handle_t wred_handles[SWITCH_METER_COUNTER_MAX];
  switch_handle_t wred_stats_handles[SWITCH_METER_COUNTER_MAX];

  /** color limit */
  switch_uint32_t color_limit[SWITCH_COLOR_MAX];

  /** hystersis limit */
  switch_uint32_t hysteresis_limit[SWITCH_COLOR_MAX];

} switch_queue_info_t;

switch_status_t switch_queue_init(switch_device_t device);

switch_status_t switch_queue_free(switch_device_t device);

switch_status_t switch_queue_default_entries_add(switch_device_t device);

switch_status_t switch_queue_default_entries_delete(switch_device_t device);

// Default burst size is 16284 bytes
// Default port shape rate is set to 100G
#define DEFAULT_BURST_SIZE 16384
#define DEFAULT_PORT_SHAPE_RATE 100000000000
#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_QUEUE_INT_H__ */
