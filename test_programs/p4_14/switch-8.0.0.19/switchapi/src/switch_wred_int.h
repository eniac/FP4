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

#ifndef _SWITCH_WRED_INT_H_
#define _SWITCH_WRED_INT_H_

/* Number of wred entry handles per wred profile */
#define SWITCH_PD_WRED_ENT_HDLS 6

typedef struct switch_wred_info_ {
  switch_api_wred_info_t api_wred_info;
  /* list of hardware entry handles */
  switch_pd_hdl_t ent_hdls[SWITCH_PD_WRED_ENT_HDLS];

  /* list of attached queues */
  switch_list_t queue_ent_list;
} switch_wred_info_t;

/* queue entry */
typedef struct switch_wred_queue_entry_s {
  /* list node */
  switch_node_t node;

  /* queue handle */
  switch_handle_t handle;

  /* queue id */
  switch_qid_t id;

  /* egress port */
  switch_port_t port;

  switch_meter_counter_t packet_color;

  /* hardware entry handle */
  switch_pd_hdl_t ent_hdl;
} switch_wred_queue_entry_t;

typedef struct switch_wred_profile_info_ {
  switch_handle_t wred_handles[SWITCH_COLOR_MAX];
  switch_api_wred_profile_info_t api_info;
} switch_wred_profile_info_t;

#define switch_wred_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_WRED, sizeof(switch_wred_info_t))

#define switch_wred_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_WRED, _handle)

#define switch_wred_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_WRED, _handle, (void **)_info)

#define switch_wred_counter_handle_create(_device) \
  switch_handle_create(                            \
      _device, SWITCH_HANDLE_TYPE_WRED, sizeof(switch_wred_info_t))

#define switch_wred_counter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_WRED, _handle)

#define switch_wred_profile_handle_create(_device)      \
  switch_handle_create(_device,                         \
                       SWITCH_HANDLE_TYPE_WRED_PROFILE, \
                       sizeof(switch_wred_profile_info_t))

#define switch_wred_profile_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_WRED_PROFILE, _handle)

#define switch_wred_profile_get(_device, _handle, _info) \
  switch_handle_get(                                     \
      _device, SWITCH_HANDLE_TYPE_WRED_PROFILE, _handle, (void **)_info)

switch_status_t switch_wred_init(switch_device_t device);

switch_status_t switch_wred_free(switch_device_t device);

switch_status_t switch_wred_default_entries_add(switch_device_t device);

switch_status_t switch_wred_default_entries_delete(switch_device_t device);

#endif /* _SWITCH_WRED_INT_H_ */
