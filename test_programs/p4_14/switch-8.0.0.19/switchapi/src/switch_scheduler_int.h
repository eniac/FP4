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

#ifndef __SWITCH_SCHEDULER_INT_H__
#define __SWITCH_SCHEDULER_INT_H__

#include "switchapi/switch_scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** scheduler handle wrappers */
#define switch_scheduler_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_SCHEDULER, sizeof(switch_scheduler_info_t))

#define switch_scheduler_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_SCHEDULER, _handle)

#define switch_scheduler_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_SCHEDULER, _handle, (void **)_info)

#define switch_scheduler_group_handle_create(_device)      \
  switch_handle_create(_device,                            \
                       SWITCH_HANDLE_TYPE_SCHEDULER_GROUP, \
                       sizeof(switch_scheduler_info_t))

#define switch_scheduler_group_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_SCHEDULER_GROUP, _handle)

#define switch_scheduler_group_get(_device, _handle, _info) \
  switch_handle_get(                                        \
      _device, SWITCH_HANDLE_TYPE_SCHEDULER_GROUP, _handle, (void **)_info)

#define SWITCH_SCHEDULER_HANDLE_SIZE 2048
#define SWITCH_SCHEDULER_GROUP_HANDLE_SIZE 4096
#define SWITCH_SCHEDULER_GROUP_CHILD_COUNT 1

switch_status_t switch_scheduler_init(switch_device_t device);

switch_status_t switch_scheduler_free(switch_device_t device);

typedef struct switch_scheduler_group_entry_s {
  switch_node_t node;

  switch_handle_t queue_handle;
  switch_handle_t port_handle;
  switch_handle_t scheduler_group_handle;
} switch_scheduler_group_entry_t;

typedef struct switch_scheduler_info_ {
  switch_scheduler_api_info_t api_info;
  switch_list_t scheduler_group_list;
} switch_scheduler_info_t;

typedef struct switch_scheduler_group_info_ {
  switch_scheduler_group_api_info_t api_info;
} switch_scheduler_group_info_t;

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_SCHEDULER_INT_H__ */
