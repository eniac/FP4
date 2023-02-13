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

#ifndef __SWITCH_BUFFER_INT_H__
#define __SWITCH_BUFFER_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** maximum ingress buffer pools */
#define SWITCH_BUFFER_POOL_INGRESS_MAX 4

/** maximum egress buffer pools */
#define SWITCH_BUFFER_POOL_EGRESS_MAX 4

/** maximum buffer profiles */
#define SWITCH_BUFFER_PROFILE_MAX 64

#define SWITCH_BUFFER_PFC_ICOS_MAX 8

#define SWITCH_BUFFER_MAX_THRESHOLD 255

/** buffer pool handle wrappers */
#define switch_buffer_pool_handle_create(_device)      \
  switch_handle_create(_device,                        \
                       SWITCH_HANDLE_TYPE_BUFFER_POOL, \
                       sizeof(switch_buffer_pool_info_t))

#define switch_buffer_pool_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_BUFFER_POOL, _handle)

#define switch_buffer_pool_get(_device, _handle, _info) \
  switch_handle_get(                                    \
      _device, SWITCH_HANDLE_TYPE_BUFFER_POOL, _handle, (void **)_info)

/** buffer profile handle wrappers */
#define switch_buffer_profile_handle_create(_device)      \
  switch_handle_create(_device,                           \
                       SWITCH_HANDLE_TYPE_BUFFER_PROFILE, \
                       sizeof(switch_buffer_profile_t))

#define switch_buffer_profile_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_BUFFER_PROFILE, _handle)

#define switch_buffer_profile_get(_device, _handle, _info) \
  switch_handle_get(                                       \
      _device, SWITCH_HANDLE_TYPE_BUFFER_PROFILE, _handle, (void **)_info)

/** buffer pool info */
typedef struct switch_buffer_pool_info_s {
  /** application buffer pool info */
  switch_api_buffer_pool_t api_buffer_pool;

  /** pd pool id */
  switch_pd_pool_id_t pool_id;

  /** enable color drop for this buffer pool */
  bool color_drop_enable;

  /** color drop limit per color */
  switch_uint32_t color_drop_limit[SWITCH_COLOR_MAX];

} switch_buffer_pool_info_t;

typedef struct switch_buffer_pd_pool_use_s {
  bool in_use;
  switch_pd_pool_id_t pool_id;
} switch_buffer_pd_pool_use_t;

typedef struct switch_buffer_pool_usage_s {
  switch_buffer_pd_pool_use_t
      ingress_pd_pool_use[SWITCH_BUFFER_POOL_INGRESS_MAX];
  switch_buffer_pd_pool_use_t egress_pd_pool_use[SWITCH_BUFFER_POOL_EGRESS_MAX];
  switch_uint8_t ingress_use_pool_count;
  switch_uint8_t egress_use_pool_count;
} switch_buffer_pool_usage_t;

/** buffer profile info */
typedef struct switch_buffer_profile_s {
  /** application buffer profile info */
  switch_api_buffer_profile_t buffer_profile;

  /** list of queue handles sharing the buffer profile */
  switch_list_t queue_handle_list;

  /** list of ppg handles sharing the buffer profile */
  switch_list_t ppg_handle_list;
} switch_buffer_profile_t;

typedef struct switch_buffer_queue_entry_s {
  switch_node_t node;
  switch_handle_t handle;
} switch_buffer_queue_entry_t;

typedef switch_buffer_queue_entry_t switch_buffer_ppg_entry_t;

/** buffer device context */
typedef struct switch_buffer_context_s {
  /** number of ingress pools */
  switch_uint8_t ingress_pool_count;

  /** number of egress pools */
  switch_uint8_t egress_pool_count;

  /** color limit per color */
  switch_uint32_t color_hysteresis[SWITCH_COLOR_MAX];

  /** skid pool limit */
  switch_uint32_t skid_limit;

  /** skid hysteresis limit */
  switch_uint32_t skid_hysteresis;
} switch_buffer_context_t;

switch_status_t switch_buffer_init(switch_device_t device);

switch_status_t switch_buffer_free(switch_device_t device);

switch_status_t switch_buffer_default_entries_add(switch_device_t device);

switch_status_t switch_buffer_default_entries_delete(switch_device_t device);

switch_status_t switch_api_buffer_context_dump(const switch_device_t device,
                                               const void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_BUFFER_INT_H__ */
