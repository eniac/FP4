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

#include "switchapi/switch_handle.h"
#include "switchapi/switch_status.h"

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_handle_type_t switch_handle_type_get(switch_handle_t handle) {
  switch_handle_type_t type = SWITCH_HANDLE_TYPE_NONE;
  type = handle >> SWITCH_HANDLE_TYPE_SHIFT;
  return type;
}

switch_status_t switch_handle_type_init(switch_device_t device,
                                        switch_handle_type_t type,
                                        switch_size_t size) {
  return switch_handle_type_allocator_init(
      device, type, size * 4, true /*grow*/, false /*zero_based*/);
}

switch_status_t switch_handle_type_allocator_init(switch_device_t device,
                                                  switch_handle_type_t type,
                                                  switch_uint32_t num_handles,
                                                  bool grow_on_demand,
                                                  bool zero_based) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_info_t *handle_info = NULL;
  switch_id_allocator_t *allocator = NULL;
  switch_size_t size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  handle_info = SWITCH_MALLOC(device, sizeof(switch_handle_info_t), 1);
  if (!handle_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("handle %s init failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(handle_info, 0x0, sizeof(switch_handle_info_t));

  size = (num_handles + 3) / 4;
  status = switch_api_id_allocator_new(device, size, zero_based, &allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_FREE(device, handle_info);
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR("handle %s init failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  handle_info->type = type;
  handle_info->initial_size = size;
  handle_info->allocator = allocator;
  handle_info->num_in_use = 0;
  handle_info->num_handles = num_handles;
  handle_info->grow_on_demand = grow_on_demand;
  handle_info->zero_based = zero_based;

  status = SWITCH_ARRAY_INSERT(
      &device_ctx->handle_info_array, type, (void *)handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s init failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    switch_api_id_allocator_destroy(device, handle_info->allocator);
    SWITCH_FREE(device, handle_info);
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_handle_type_free(switch_device_t device,
                                        switch_handle_type_t type) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_info_t *handle_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle free failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle free failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle free failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_GET(
      &device_ctx->handle_info_array, type, (void *)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s free failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  switch_api_id_allocator_destroy(device, handle_info->allocator);
  status = SWITCH_ARRAY_DELETE(&device_ctx->handle_info_array, type);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s free failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, handle_info);
  return status;
}

switch_handle_t _switch_handle_create(switch_device_t device,
                                      switch_handle_type_t type) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_info_t *handle_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle allocate failed: %s\n",
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status = SWITCH_ARRAY_GET(
      &device_ctx->handle_info_array, type, (void **)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s allocate failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  if ((handle_info->num_in_use < handle_info->num_handles) ||
      handle_info->grow_on_demand) {
    status =
        switch_api_id_allocator_allocate(device, handle_info->allocator, &id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("handle %s allocate failed: %s\n",
                       switch_handle_type_to_string(type),
                       switch_error_to_string(status));
      return SWITCH_API_INVALID_HANDLE;
    }
    handle_info->num_in_use++;
    handle = id_to_handle(type, id);
  }

  return handle;
}

switch_handle_t _switch_handle_set_and_create(switch_device_t device,
                                              switch_handle_t type,
                                              switch_uint32_t id) {
  switch_handle_info_t *handle_info = NULL;
  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle allocate failed: %s\n",
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status = SWITCH_ARRAY_GET(
      &device_ctx->handle_info_array, type, (void *)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s allocate failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  switch_api_id_allocator_set(device, handle_info->allocator, id);
  handle_info->num_in_use++;
  return id_to_handle(type, id);
}

switch_status_t _switch_handle_delete(switch_device_t device,
                                      switch_handle_t handle) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_info_t *handle_info = NULL;
  switch_uint32_t id = 0;
  switch_handle_type_t type = SWITCH_HANDLE_TYPE_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle init failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  type = switch_handle_type_get(handle);
  status = SWITCH_ARRAY_GET(
      &device_ctx->handle_info_array, type, (void *)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s free failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  id = handle_to_id(handle);
  switch_api_id_allocator_release(device, handle_info->allocator, id);
  handle_info->num_in_use--;
  return SWITCH_STATUS_SUCCESS;
}

switch_handle_t switch_handle_create(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_uint32_t size) {
  switch_device_context_t *device_ctx = NULL;
  void *i_info = NULL;
  void *handle_array = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  handle_array = &device_ctx->handle_array[type];
  handle = _switch_handle_create(device, type);

  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR("handle %s create failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  i_info = SWITCH_MALLOC(device, size, 1);
  if (!i_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("handle %s create failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }

  SWITCH_MEMSET(i_info, 0, size);

  status = SWITCH_ARRAY_INSERT(handle_array, handle, (void *)i_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    SWITCH_FREE(device, i_info);
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }
  return handle;
}

switch_handle_t switch_handle_create_and_set(switch_device_t device,
                                             switch_handle_type_t type,
                                             switch_uint32_t id,
                                             switch_uint32_t size) {
  switch_device_context_t *device_ctx = NULL;
  void *i_info = NULL;
  switch_array_t *handle_array = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  handle_array = &device_ctx->handle_array[type];
  handle = _switch_handle_set_and_create(device, type, id);

  SWITCH_ASSERT(handle != SWITCH_API_INVALID_HANDLE);

  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  i_info = SWITCH_MALLOC(device, size, 1);
  if (!i_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }

  SWITCH_MEMSET(i_info, 0, size);

  status = SWITCH_ARRAY_INSERT(handle_array, handle, (void *)i_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    SWITCH_FREE(device, i_info);
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }
  return handle;
}

switch_status_t switch_handle_get(switch_device_t device,
                                  switch_handle_type_t type,
                                  switch_handle_t handle,
                                  void **i_info) {
  switch_device_context_t *device_ctx = NULL;
  void *handle_array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_HANDLE_VALID(handle, type)) {
    SWITCH_LOG_ERROR("handle type not %s: handle: %lx\n",
                     switch_handle_type_to_string(type),
                     handle);
    return SWITCH_STATUS_INVALID_HANDLE;
  }
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }
  handle_array = &device_ctx->handle_array[type];

  status = SWITCH_ARRAY_GET(handle_array, handle, (void **)i_info);

  type = switch_handle_type_get(handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s get failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_handle_delete(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_handle_t handle) {
  switch_device_context_t *device_ctx = NULL;
  void *i_info = NULL;
  void *handle_array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s create and set failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }
  handle_array = &device_ctx->handle_array[type];

  status = SWITCH_ARRAY_GET(handle_array, handle, (void **)&i_info);

  type = switch_handle_type_get(handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s delete failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(handle_array, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s delete failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }

  status = _switch_handle_delete(device, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle %s delete failed: %s\n",
                     switch_handle_type_to_string(type),
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, i_info);
  return status;
}

switch_status_t switch_api_handle_iterate_internal(
    switch_device_t device,
    switch_handle_type_t type,
    switch_handle_t old_handle,
    switch_handle_t *new_handle) {
  switch_device_context_t *device_ctx = NULL;
  void *i_info = NULL;
  switch_array_t *handle_array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(type < SWITCH_HANDLE_TYPE_MAX);
  SWITCH_ASSERT(new_handle != NULL);

  UNUSED(i_info);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle first get failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  handle_array = &device_ctx->handle_array[type];
  *new_handle = SWITCH_API_INVALID_HANDLE;

  if (old_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_ARRAY_FIRST_GET((*handle_array), *new_handle, void, i_info);
  } else {
    SWITCH_ASSERT(switch_handle_type_get(old_handle) == type);
    SWITCH_ARRAY_NEXT_GET(
        (*handle_array), old_handle, *new_handle, void, i_info);
  }

  return status;
}

switch_status_t switch_api_handle_count_get(switch_device_t device,
                                            switch_handle_type_t type,
                                            switch_size_t *num_entries) {
  switch_device_context_t *device_ctx = NULL;
  switch_handle_info_t *handle_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(type < SWITCH_HANDLE_TYPE_MAX);
  SWITCH_ASSERT(num_entries != NULL);
  *num_entries = 0;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle count get failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_GET(
      &device_ctx->handle_info_array, type, (void *)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handle count get failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  *num_entries = handle_info->num_in_use;

  return status;
}

switch_status_t switch_api_handles_get_internal(switch_device_t device,
                                                switch_handle_type_t type,
                                                switch_size_t *num_entries,
                                                switch_handle_t **handles) {
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_size_t count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(type < SWITCH_HANDLE_TYPE_MAX);
  SWITCH_ASSERT(handles != NULL);
  SWITCH_ASSERT(num_entries != NULL);

  status = switch_api_handle_count_get(device, type, num_entries);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("handles get failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  if (*num_entries == 0) {
    SWITCH_LOG_DEBUG("num of handles is %d for handle %s\n",
                     *num_entries,
                     switch_handle_type_to_string(type));
    return status;
  }

  *handles = SWITCH_MALLOC(device, sizeof(switch_handle_t), *num_entries);
  if (!(*handles)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("handles get failed: %s\n",
                     switch_error_to_string(status));
    return status;
  }

  FOR_EACH_HANDLE_BEGIN(device, type, handle) {
    if (handle != SWITCH_API_INVALID_HANDLE) {
      (*handles)[count++] = handle;
    }
  }
  FOR_EACH_HANDLE_END();

  SWITCH_ASSERT(count == *num_entries);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_handles_get(switch_device_t device,
                                       switch_handle_type_t type,
                                       switch_size_t *num_entries,
                                       switch_handle_t **handles) {
  SWITCH_MT_WRAP(
      switch_api_handles_get_internal(device, type, num_entries, handles))
}

switch_status_t switch_api_handle_iterate(switch_device_t device,
                                          switch_handle_type_t type,
                                          switch_handle_t old_handle,
                                          switch_handle_t *new_handle) {
  SWITCH_MT_WRAP(
      switch_api_handle_iterate_internal(device, type, old_handle, new_handle))
}
