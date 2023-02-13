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

#ifndef __SWITCH_LABEL_INT_H__
#define __SWITCH_LABEL_INT_H__

#define SWITCH_MAX_LABELS 1024

/** label handle wrappers */
#define switch_label_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_LABEL, sizeof(switch_label_info_t))

#define switch_label_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_LABEL, _handle)

#define switch_label_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_LABEL, _handle, (void **)_info)

/** label entry */
typedef struct switch_label_entry_s {
  /** vlan or interface handle */
  switch_handle_t handle;

  /** list node */
  switch_node_t node;

} switch_label_entry_t;

/** label info identified by label handle */
typedef struct switch_label_info_ {
  /** label type - vlan/interface */
  switch_label_type_t label_type;

  /** list of handles */
  switch_list_t handle_list;

} switch_label_info_t;

switch_status_t switch_label_init(switch_device_t device);

switch_status_t switch_label_free(switch_device_t device);

#endif /* __SWITCH_LABEL_INT_H__ */
