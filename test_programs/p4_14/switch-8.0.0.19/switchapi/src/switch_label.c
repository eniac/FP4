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

#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_label_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_LABEL, SWITCH_MAX_LABELS);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_label_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LABEL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label free failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_api_label_create_internal(
    switch_device_t device,
    switch_label_type_t label_type,
    switch_handle_t *label_handle) {
  switch_label_info_t *label_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(label_type == SWITCH_LABEL_TYPE_VLAN ||
                label_type == SWITCH_LABEL_TYPE_INTERFACE);
  if (label_type != SWITCH_LABEL_TYPE_VLAN &&
      label_type != SWITCH_LABEL_TYPE_INTERFACE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("label create failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *label_handle = switch_label_handle_create(device);
  if (*label_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("label create failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_label_get(device, *label_handle, &label_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label create failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  label_info->label_type = label_type;

  return status;
}

switch_status_t switch_api_label_delete_internal(switch_device_t device,
                                                 switch_handle_t label_handle) {
  switch_label_info_t *label_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LABEL_HANDLE(label_handle));
  if (!SWITCH_LABEL_HANDLE(label_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("label delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_label_get(device, label_handle, &label_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_label_handle_delete(device, label_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_label_member_add_internal(
    switch_device_t device,
    switch_handle_t label_handle,
    switch_handle_t handle) {
  switch_label_info_t *label_info = NULL;
  switch_label_entry_t *label_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LABEL_HANDLE(label_handle));
  if (!SWITCH_LABEL_HANDLE(label_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("label member add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_label_get(device, label_handle, &label_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label member add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  label_entry = SWITCH_MALLOC(device, sizeof(switch_label_entry_t), 0x1);
  if (!label_entry) {
    SWITCH_LOG_ERROR("label member add failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(label_entry, 0x0, sizeof(switch_label_entry_t));
  label_entry->handle = handle;

  status = SWITCH_LIST_INSERT(
      &(label_info->handle_list), &(label_entry->node), label_entry);

  if (label_info->label_type == SWITCH_LABEL_TYPE_VLAN) {
    SWITCH_ASSERT(SWITCH_VLAN_HANDLE(handle));
  } else {
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(handle));
  }

  return status;
}

switch_status_t switch_api_label_member_delete_internal(
    switch_device_t device,
    switch_handle_t label_handle,
    switch_handle_t handle) {
  switch_label_info_t *label_info = NULL;
  switch_label_entry_t *label_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LABEL_HANDLE(label_handle));
  if (!SWITCH_LABEL_HANDLE(label_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("label member delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_label_get(device, label_handle, &label_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label member delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (label_info->label_type == SWITCH_LABEL_TYPE_VLAN) {
    SWITCH_ASSERT(SWITCH_VLAN_HANDLE(handle));
  } else {
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(handle));
  }

  FOR_EACH_IN_LIST(label_info->handle_list, node) {
    label_entry = (switch_label_entry_t *)node->data;
    if (label_entry->handle == handle) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!node) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("label member delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&label_info->handle_list, &label_entry->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("label member delete failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_label_create(switch_device_t device,
                                        switch_label_type_t label_type,
                                        switch_handle_t *label_handle) {
  SWITCH_MT_WRAP(
      switch_api_label_create_internal(device, label_type, label_handle))
}

switch_status_t switch_api_label_member_delete(switch_device_t device,
                                               switch_handle_t label_handle,
                                               switch_handle_t handle) {
  SWITCH_MT_WRAP(
      switch_api_label_member_delete_internal(device, label_handle, handle))
}

switch_status_t switch_api_label_member_add(switch_device_t device,
                                            switch_handle_t label_handle,
                                            switch_handle_t handle) {
  SWITCH_MT_WRAP(
      switch_api_label_member_add_internal(device, label_handle, handle))
}

switch_status_t switch_api_label_delete(switch_device_t device,
                                        switch_handle_t label_handle) {
  SWITCH_MT_WRAP(switch_api_label_delete_internal(device, label_handle))
}
