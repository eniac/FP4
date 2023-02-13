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

#include "switchapi/switch_buffer.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static switch_buffer_pool_usage_t buffer_pool_usage[SWITCH_MAX_DEVICE];

switch_status_t switch_buffer_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_buffer_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_buffer_init(switch_device_t device) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  SWITCH_LOG_ENTER();

  buffer_ctx = SWITCH_MALLOC(device, sizeof(switch_buffer_context_t), 0x1);
  if (!buffer_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "buffer init failed on device %d: "
        "buffer context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(buffer_ctx, 0x0, sizeof(switch_buffer_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_BUFFER, (void *)buffer_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer init failed on device %d: "
        "buffer context set failed (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(buffer_pool_usage, 0, sizeof(switch_buffer_pool_usage_t));
  switch_pd_ingress_pool_init(device,
                              buffer_pool_usage[device].ingress_pd_pool_use);
  switch_pd_egress_pool_init(device,
                             buffer_pool_usage[device].egress_pd_pool_use);
  status = switch_handle_type_init(
      device,
      SWITCH_HANDLE_TYPE_BUFFER_POOL,
      SWITCH_BUFFER_POOL_INGRESS_MAX + SWITCH_BUFFER_POOL_EGRESS_MAX);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer init failed on device %d: pool handle init failed : %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_BUFFER_PROFILE, SWITCH_BUFFER_PROFILE_MAX);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer init failed on device %d: profile handle init failed : %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_buffer_pd_pool_index_get(switch_device_t device,
                                                switch_direction_t dir,
                                                switch_pd_pool_id_t *pool_id) {
  switch_buffer_pd_pool_use_t *pd_pool_use;
  switch_uint32_t index;
  switch_uint32_t max_pool_count;

  if (dir == SWITCH_API_DIRECTION_INGRESS) {
    pd_pool_use = buffer_pool_usage[device].ingress_pd_pool_use;
    max_pool_count = SWITCH_BUFFER_POOL_INGRESS_MAX;
  } else {
    pd_pool_use = buffer_pool_usage[device].egress_pd_pool_use;
    max_pool_count = SWITCH_BUFFER_POOL_EGRESS_MAX;
  }

  for (index = 0; index < max_pool_count; index++) {
    if (pd_pool_use[index].in_use == 0) {
      *pool_id = pd_pool_use[index].pool_id;
      pd_pool_use[index].in_use = 1;
      if (dir == SWITCH_API_DIRECTION_INGRESS) {
        buffer_pool_usage[device].ingress_use_pool_count++;
      } else {
        buffer_pool_usage[device].egress_use_pool_count++;
      }
      return SWITCH_STATUS_SUCCESS;
    }
  }
  return SWITCH_STATUS_INSUFFICIENT_RESOURCES;
}

switch_status_t switch_buffer_pd_pool_index_free(switch_device_t device,
                                                 switch_direction_t dir,
                                                 switch_pd_pool_id_t pool_id) {
  switch_buffer_pd_pool_use_t *pd_pool_use;
  switch_uint32_t index;
  switch_uint32_t max_pool_count;

  if (dir == SWITCH_API_DIRECTION_INGRESS) {
    pd_pool_use = buffer_pool_usage[device].ingress_pd_pool_use;
    max_pool_count = SWITCH_BUFFER_POOL_INGRESS_MAX;
  } else {
    pd_pool_use = buffer_pool_usage[device].egress_pd_pool_use;
    max_pool_count = SWITCH_BUFFER_POOL_EGRESS_MAX;
  }
  for (index = 0; index < max_pool_count; index++) {
    if (pd_pool_use[index].pool_id == pool_id) {
      SWITCH_ASSERT(pd_pool_use[index].in_use);
      pd_pool_use[index].in_use = 0;
      (dir == SWITCH_API_DIRECTION_INGRESS)
          ? --buffer_pool_usage[device].ingress_use_pool_count
          : --buffer_pool_usage[device].egress_use_pool_count;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_buffer_free(switch_device_t device) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BUFFER, (void **)&buffer_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer free failed on device %d: "
        "buffer context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, buffer_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_BUFFER, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_buffer_pool_create_internal(
    switch_device_t device,
    switch_api_buffer_pool_t api_buffer_pool,
    switch_handle_t *pool_handle) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_pd_pool_id_t pool_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_direction_t direction;
  switch_uint32_t pool_size;
  switch_uint8_t icos;
  switch_uint32_t xoff_size;

  direction = api_buffer_pool.direction;
  pool_size = api_buffer_pool.pool_size;
  xoff_size = api_buffer_pool.xoff_size;

  SWITCH_ASSERT(direction == SWITCH_API_DIRECTION_INGRESS ||
                direction == SWITCH_API_DIRECTION_EGRESS);
  SWITCH_ASSERT(pool_size != 0);

  if (((buffer_pool_usage[device].ingress_use_pool_count ==
        SWITCH_BUFFER_POOL_INGRESS_MAX) &&
       (direction == SWITCH_API_DIRECTION_INGRESS)) ||
      ((buffer_pool_usage[device].egress_use_pool_count ==
        SWITCH_BUFFER_POOL_EGRESS_MAX) &&
       (direction == SWITCH_API_DIRECTION_EGRESS))) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR(
        "buffer pool create failed on device %d: not enough hw pools %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  *pool_handle = switch_buffer_pool_handle_create(device);
  if (*pool_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "buffer pool create failed on device %d: handle create failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_pool_get(device, *pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool create failed on device %d: pool info get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_pd_pool_index_get(device, direction, &pool_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "free buffer pool index get failed on device %d: pool index get failed "
        "%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_buffer_pool_set(device, pool_id, pool_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool create failed on device %d: buffer pool set in hw failed "
        "%s",
        device,
        switch_error_to_string(status));
    switch_buffer_pd_pool_index_free(device, direction, pool_id);
    return status;
  }
  if (xoff_size) {
    for (icos = 0; icos < SWITCH_BUFFER_PFC_ICOS_MAX; icos++) {
      status =
          switch_pd_buffer_pool_pfc_limit(device, pool_id, icos, xoff_size);
    }
  }

  buffer_pool_info->pool_id = pool_id;
  SWITCH_MEMCPY(&buffer_pool_info->api_buffer_pool,
                &api_buffer_pool,
                sizeof(switch_api_buffer_pool_t));
  return status;
}

switch_status_t switch_api_buffer_pool_size_set_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t pool_size) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool size set failed on device %d: pool info get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  status =
      switch_pd_buffer_pool_set(device, buffer_pool_info->pool_id, pool_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool size set failed on device %d: buffer pool set in hw "
        "failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  buffer_pool_info->api_buffer_pool.pool_size = pool_size;
  return status;
}

switch_status_t switch_api_buffer_pool_size_get_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *pool_size) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool size get failed on device %d: pool info get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  *pool_size = buffer_pool_info->api_buffer_pool.pool_size;
  return status;
}

switch_status_t switch_api_buffer_pool_type_get_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_direction_t *dir) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool type get failed on device %d: pool info get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  *dir = buffer_pool_info->api_buffer_pool.direction;
  return status;
}

switch_status_t switch_api_buffer_pool_xoff_size_set_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t xoff_size) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t icos;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool xoff size set failed on device %d: pool info get failed "
        "%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  for (icos = 0; icos < SWITCH_BUFFER_PFC_ICOS_MAX; icos++) {
    status = switch_pd_buffer_pool_pfc_limit(
        device, buffer_pool_info->pool_id, icos, xoff_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "buffer pool xoff size set failed on device %d: buffer pool set in "
          "hw "
          "failed %s",
          device,
          switch_error_to_string(status));
      return status;
    }
  }
  buffer_pool_info->api_buffer_pool.xoff_size = xoff_size;
  return status;
}

switch_status_t switch_api_buffer_pool_xoff_size_get_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *xoff_size) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool xoff size get failed on device %d: pool info get failed "
        "%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  *xoff_size = buffer_pool_info->api_buffer_pool.xoff_size;
  return status;
}

switch_status_t switch_api_buffer_pool_usage_get_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool usage get failed on device %d pool handle 0x%lx: "
        "pool get failed:(%s)\n",
        device,
        pool_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_buffer_pool_usage_get(
      device, buffer_pool_info->pool_id, curr_occupancy_bytes, watermark_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool usage get failed on device %d pool handle 0x%lx: "
        "pool pd usage get failed:(%s)\n",
        device,
        pool_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_buffer_pool_delete_internal(
    switch_device_t device, switch_handle_t pool_handle) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BUFFER_POOL_HANDLE(pool_handle));
  if (!SWITCH_BUFFER_POOL_HANDLE(pool_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer pool delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool delete failed on device %d: pool info get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_buffer_pool_set(device, buffer_pool_info->pool_id, 0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool delete failed on device %d: pool size set in h/w failed "
        "%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_pd_pool_index_free(
      device,
      buffer_pool_info->api_buffer_pool.direction,
      buffer_pool_info->pool_id);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_buffer_pool_handle_delete(device, pool_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_buffer_pool_color_drop_enable_internal(
    switch_device_t device, switch_handle_t pool_handle, bool enable) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BUFFER_POOL_HANDLE(pool_handle));
  if (!SWITCH_BUFFER_POOL_HANDLE(pool_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer pool color drop enable failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool color drop enable failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  buffer_pool_info->color_drop_enable = enable;

  status = switch_pd_buffer_pool_color_drop_enable(
      device, buffer_pool_info->pool_id, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool color drop enable failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_total_buffer_size_get_internal(
    switch_device_t device, switch_uint64_t *size) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_total_buffer_size_get(device, size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Total buffer size get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_api_max_ingress_pool_get_internal(
    switch_device_t device, switch_uint8_t *pool_size) {
  *pool_size = SWITCH_BUFFER_POOL_INGRESS_MAX;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_max_egress_pool_get_internal(
    switch_device_t device, switch_uint8_t *pool_size) {
  *pool_size = SWITCH_BUFFER_POOL_EGRESS_MAX;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_buffer_pool_color_limit_set_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_color_t color,
    switch_uint32_t num_bytes) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BUFFER_POOL_HANDLE(pool_handle));
  SWITCH_ASSERT(color < SWITCH_COLOR_MAX);
  if (!SWITCH_BUFFER_POOL_HANDLE(pool_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer pool color limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool color limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  buffer_pool_info->color_drop_limit[color] = num_bytes;

  status = switch_pd_buffer_pool_color_limit_set(
      device, buffer_pool_info->pool_id, color, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool color limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_buffer_pool_color_hysteresis_set_internal(
    switch_device_t device, switch_color_t color, switch_uint32_t num_bytes) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(color < SWITCH_COLOR_MAX);

  buffer_ctx->color_hysteresis[color] = num_bytes;

  status = switch_pd_buffer_pool_color_hysteresis_set(device, color, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool color hysteresis set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_buffer_profile_create_internal(
    switch_device_t device,
    switch_api_buffer_profile_t *buffer_profile,
    switch_handle_t *buffer_profile_handle) {
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *buffer_profile_handle = switch_buffer_profile_handle_create(device);
  if (*buffer_profile_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("buffer profile create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_profile_get(
      device, *buffer_profile_handle, &buffer_profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&buffer_profile_info->buffer_profile,
                buffer_profile,
                sizeof(switch_api_buffer_profile_t));

  SWITCH_LIST_INIT(&(buffer_profile_info->queue_handle_list));
  SWITCH_LIST_INIT(&(buffer_profile_info->ppg_handle_list));

  return status;
}

switch_status_t switch_api_buffer_profile_delete_internal(
    switch_device_t device, switch_handle_t buffer_profile_handle) {
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle));
  if (!SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("buffer profile delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_profile_get(
      device, buffer_profile_handle, &buffer_profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_profile_handle_delete(device, buffer_profile_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_priority_group_buffer_profile_set_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_handle_t buffer_profile_handle) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_buffer_pool_info_t *pool_info = NULL;
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_buffer_ppg_entry_t *buffer_ppg_entry = NULL;
  bool enable = true;
  bool lossless_ppg = false;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (buffer_profile_handle) {
    SWITCH_ASSERT(SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle));
    if (!SWITCH_PPG_HANDLE(ppg_handle) ||
        !SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("buffer profile ppg set failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    enable = true;

    status = switch_buffer_profile_get(
        device, buffer_profile_handle, &buffer_profile_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("buffer profile ppg set failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_buffer_pool_get(
        device, buffer_profile_info->buffer_profile.pool_handle, &pool_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("buffer profile ppg set failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    buffer_ppg_entry =
        SWITCH_MALLOC(device, sizeof(switch_buffer_ppg_entry_t), 1);
    buffer_ppg_entry->handle = ppg_handle;
    SWITCH_LIST_INSERT(&buffer_profile_info->ppg_handle_list,
                       &buffer_ppg_entry->node,
                       buffer_ppg_entry);
  } else {
    enable = false;
  }

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer profile ppg set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ppg_info->buffer_profile_handle = buffer_profile_handle;

  if (pool_info) {
    status = switch_pd_ppg_pool_usage_set(
        device,
        ppg_info->tm_ppg_handle,
        pool_info->pool_id,
        buffer_profile_info ? &(buffer_profile_info->buffer_profile) : 0,
        enable);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("buffer profile ppg set failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  /*
   * Set the PPG as lossless when XON and XOFF thresholds are configured in
   * buffer profile.
   */
  lossless_ppg = false;
  if (buffer_profile_info &&
      buffer_profile_info->buffer_profile.xoff_threshold &&
      buffer_profile_info->buffer_profile.xon_threshold) {
    lossless_ppg = true;
  }
  SWITCH_LOG_DEBUG("Setting PPG index %d, port 0x%lx as lossless PPG",
                   ppg_info->ppg_index,
                   ppg_info->port_handle);

  status = switch_api_ppg_lossless_enable(device, ppg_handle, lossless_ppg);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Failed to set PPG %d, port 0x%lx as lossless PPG",
                     ppg_info->ppg_index,
                     ppg_info->port_handle);
    return status;
  }

  return status;
}

switch_status_t switch_api_priority_group_buffer_profile_get_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_handle_t *buffer_profile_handle) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "buffer profile ppg buffer profile get failed on device %d: for "
        "ppg_handle 0x%lx:  %s",
        device,
        ppg_handle,
        switch_error_to_string(status));
    return status;
  }
  *buffer_profile_handle = ppg_info->buffer_profile_handle;
  return status;
}

switch_status_t switch_api_priority_group_port_get_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_handle_t *port_handle) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "buffer profile ppg port get failed on device %d: for ppg_handle "
        "0x%lx:  %s",
        device,
        ppg_handle,
        switch_error_to_string(status));
    return status;
  }
  *port_handle = ppg_info->port_handle;
  return status;
}

switch_status_t switch_api_priority_group_index_get_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_uint32_t *ppg_index) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "buffer profile ppg index get failed on device %d: for ppg_handle "
        "0x%lx:  %s",
        device,
        ppg_handle,
        switch_error_to_string(status));
    return status;
  }
  *ppg_index = ppg_info->ppg_index;
  return status;
}

switch_status_t switch_api_buffer_skid_limit_set_internal(
    switch_device_t device, switch_uint32_t num_bytes) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  buffer_ctx->skid_limit = num_bytes;

  status = switch_pd_buffer_skid_limit_set(device, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer skid limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_buffer_skid_hysteresis_set_internal(
    switch_device_t device, switch_uint32_t num_bytes) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  buffer_ctx->skid_hysteresis = num_bytes;

  status = switch_pd_buffer_skid_hysteresis_set(device, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer skid hystersis set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_buffer_pool_pfc_limit_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint8_t icos,
    switch_uint32_t num_bytes) {
  switch_buffer_pool_info_t *buffer_pool_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BUFFER_POOL_HANDLE(pool_handle));
  if (!SWITCH_BUFFER_POOL_HANDLE(pool_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer pool pfc limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool pfc limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_buffer_pool_pfc_limit(
      device, buffer_pool_info->pool_id, icos, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer pool pfc limit set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_queue_buffer_profile_set_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t buffer_profile_handle) {
  switch_queue_info_t *queue_info = NULL;
  switch_buffer_pool_info_t *pool_info = NULL;
  switch_buffer_profile_t *buffer_profile_info = NULL;
  bool enable = true;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_buffer_queue_entry_t *buffer_queue_entry = NULL;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  SWITCH_ASSERT(SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle) ||
      !SWITCH_BUFFER_PROFILE_HANDLE(buffer_profile_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer profile queue set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer profile queue set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_profile_get(
      device, buffer_profile_handle, &buffer_profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile queue set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_pool_get(
      device, buffer_profile_info->buffer_profile.pool_handle, &pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile queue set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  queue_info->buffer_profile_handle = buffer_profile_handle;
  enable = buffer_profile_handle == SWITCH_API_INVALID_HANDLE ? false : true;
  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Buffer profile queue set failed on device %d: port get failed for "
        "handle 0x%lx: %s",
        device,
        queue_info->port_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_queue_pool_usage_set(device,
                                          port_info->dev_port,
                                          queue_info->queue_id,
                                          pool_info->pool_id,
                                          &buffer_profile_info->buffer_profile,
                                          enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile queue set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  buffer_queue_entry =
      SWITCH_MALLOC(device, sizeof(switch_buffer_queue_entry_t), 1);
  SWITCH_MEMSET(buffer_queue_entry, 0, sizeof(switch_buffer_queue_entry_t));
  buffer_queue_entry->handle = queue_handle;
  SWITCH_LIST_INSERT(&buffer_profile_info->queue_handle_list,
                     &buffer_queue_entry->node,
                     buffer_queue_entry);

  return status;
}

switch_status_t switch_api_queue_buffer_profile_get_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *buffer_profile_handle) {
  switch_queue_info_t *queue_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("buffer profile queue get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *buffer_profile_handle = queue_info->buffer_profile_handle;
  return status;
}

switch_status_t switch_api_buffer_pool_threshold_mode_get_internal(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_buffer_threshold_mode_t *threshold_mode) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_buffer_pool_info_t *buffer_pool_info = NULL;

  status = switch_buffer_pool_get(device, pool_handle, &buffer_pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer pool threshold type get failed: pool get failed on device %d: "
        "%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  *threshold_mode = buffer_pool_info->api_buffer_pool.threshold_mode;
  return status;
}

switch_status_t switch_api_buffer_profile_info_set_internal(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *api_profile_info) {
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_buffer_queue_entry_t *buffer_queue_entry = NULL;
  switch_node_t *node = NULL;
  switch_handle_t queue_handle;
  switch_queue_info_t *queue_info = NULL;
  switch_buffer_pool_info_t *pool_info;
  switch_handle_t ppg_handle;
  switch_port_priority_group_t *ppg_info = NULL;
  switch_buffer_ppg_entry_t *buffer_ppg_entry = NULL;
  bool lossless_ppg = false;

  status = switch_buffer_profile_get(
      device, buffer_profile_handle, &buffer_profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("buffer profile create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_buffer_pool_get(
      device, buffer_profile_info->buffer_profile.pool_handle, &pool_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer profile size update failed on device %d: pool info get "
        "failed: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_LIST_COUNT(&(buffer_profile_info->queue_handle_list))) {
    /*
     * When buffer profile parameters are updated, iterate over all the queue
     * handles
     * and update the buffer size for all the queues.
     */
    FOR_EACH_IN_LIST(buffer_profile_info->queue_handle_list, node) {
      buffer_queue_entry = (switch_buffer_queue_entry_t *)node->data;
      queue_handle = buffer_queue_entry->handle;

      status = switch_queue_get(device, queue_handle, &queue_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR(
            "buffer profile size update failed on device %d: queue get "
            "failed: %s",
            device,
            switch_error_to_string(status));
        return status;
      }
      api_profile_info->threshold =
          buffer_profile_info->buffer_profile.threshold;
      status = switch_pd_queue_pool_usage_set(device,
                                              queue_info->port_handle,
                                              queue_info->queue_id,
                                              pool_info->pool_id,
                                              api_profile_info,
                                              true);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "buffer profile size update failed on device %d: buffer size "
            "update in hw failed: %s",
            device,
            switch_error_to_string(status));
        return status;
      }
      buffer_profile_info->buffer_profile.buffer_size =
          api_profile_info->buffer_size;
    }
    FOR_EACH_IN_LIST_END();
  }

  if (SWITCH_LIST_COUNT(&(buffer_profile_info->ppg_handle_list))) {
    FOR_EACH_IN_LIST(buffer_profile_info->ppg_handle_list, node) {
      buffer_ppg_entry = (switch_buffer_ppg_entry_t *)node->data;
      ppg_handle = buffer_ppg_entry->handle;

      status = switch_ppg_get(device, ppg_handle, &ppg_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "buffer profile update failed on device %d: ppg get "
            "failed: %s",
            device,
            switch_error_to_string(status));
        return status;
      }
      status = switch_pd_ppg_pool_usage_set(device,
                                            ppg_info->tm_ppg_handle,
                                            pool_info->pool_id,
                                            api_profile_info,
                                            true);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bufer profile update failed on device %d: ppg pool update for "
            "buffer profile handle 0x%lx"
            "failed: %s",
            device,
            buffer_profile_handle,
            switch_error_to_string(status));
        return status;
      }

      lossless_ppg = false;
      if (buffer_profile_info->buffer_profile.xoff_threshold &&
          buffer_profile_info->buffer_profile.xon_threshold) {
        lossless_ppg = true;
      }
      SWITCH_LOG_DEBUG("Setting PPG index %d, port 0x%lx as lossless PPG",
                       ppg_info->ppg_index,
                       ppg_info->port_handle);

      status = switch_api_ppg_lossless_enable(device, ppg_handle, lossless_ppg);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("Failed to set PPG %d, port 0x%lx as lossless PPG",
                         ppg_info->ppg_index,
                         ppg_info->port_handle);
        return status;
      }

      FOR_EACH_IN_LIST_END();
    }
  }

  SWITCH_MEMCPY(&buffer_profile_info->buffer_profile,
                api_profile_info,
                sizeof(switch_api_buffer_profile_t));
  return status;
}

switch_status_t switch_api_buffer_profile_info_get_internal(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *api_profile_info) {
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_buffer_profile_get(
      device, buffer_profile_handle, &buffer_profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer info profile get failed on device %d: profile 0x%lx failed: %s",
        device,
        buffer_profile_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(api_profile_info,
                &buffer_profile_info->buffer_profile,
                sizeof(switch_api_buffer_profile_t));
  return status;
}
#ifdef __cplusplus
}
#endif

switch_status_t switch_api_buffer_pool_delete(switch_device_t device,
                                              switch_handle_t pool_handle) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_delete_internal(device, pool_handle))
}

switch_status_t switch_api_buffer_profile_delete(
    switch_device_t device, switch_handle_t buffer_profile_handle) {
  SWITCH_MT_WRAP(
      switch_api_buffer_profile_delete_internal(device, buffer_profile_handle))
}

switch_status_t switch_api_buffer_pool_color_limit_set(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_color_t color,
    uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_color_limit_set_internal(
      device, pool_handle, color, num_bytes))
}

switch_status_t switch_api_buffer_pool_create(
    switch_device_t device,
    switch_api_buffer_pool_t buffer_pool,
    switch_handle_t *pool_handle) {
  SWITCH_MT_WRAP(
      switch_api_buffer_pool_create_internal(device, buffer_pool, pool_handle))
}

switch_status_t switch_api_buffer_profile_create(
    switch_device_t device,
    switch_api_buffer_profile_t *buffer_info,
    switch_handle_t *buffer_profile_handle) {
  SWITCH_MT_WRAP(switch_api_buffer_profile_create_internal(
      device, buffer_info, buffer_profile_handle))
}

switch_status_t switch_api_buffer_skid_limit_set(switch_device_t device,
                                                 uint32_t buffer_size) {
  SWITCH_MT_WRAP(switch_api_buffer_skid_limit_set_internal(device, buffer_size))
}

switch_status_t switch_api_queue_buffer_profile_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t buffer_profile_handle) {
  SWITCH_MT_WRAP(switch_api_queue_buffer_profile_set_internal(
      device, queue_handle, buffer_profile_handle))
}

switch_status_t switch_api_queue_buffer_profile_get(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *buffer_profile_handle) {
  SWITCH_MT_WRAP(switch_api_queue_buffer_profile_get_internal(
      device, queue_handle, buffer_profile_handle))
}

switch_status_t switch_api_buffer_pool_color_hysteresis_set(
    switch_device_t device, switch_color_t color, uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_color_hysteresis_set_internal(
      device, color, num_bytes))
}

switch_status_t switch_api_buffer_skid_hysteresis_set(switch_device_t device,
                                                      uint32_t num_bytes) {
  SWITCH_MT_WRAP(
      switch_api_buffer_skid_hysteresis_set_internal(device, num_bytes))
}

switch_status_t switch_api_buffer_pool_pfc_limit(switch_device_t device,
                                                 switch_handle_t pool_handle,
                                                 uint8_t icos,
                                                 uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_pfc_limit_internal(
      device, pool_handle, icos, num_bytes))
}

switch_status_t switch_api_priority_group_buffer_profile_set(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t buffer_profile_handle) {
  SWITCH_MT_WRAP(switch_api_priority_group_buffer_profile_set_internal(
      device, pg_handle, buffer_profile_handle))
}

switch_status_t switch_api_priority_group_buffer_profile_get(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t *buffer_profile_handle) {
  SWITCH_MT_WRAP(switch_api_priority_group_buffer_profile_get_internal(
      device, pg_handle, buffer_profile_handle))
}

switch_status_t switch_api_priority_group_port_get(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(switch_api_priority_group_port_get_internal(
      device, pg_handle, port_handle))
}

switch_status_t switch_api_priority_group_index_get(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_uint32_t *ppg_index) {
  SWITCH_MT_WRAP(switch_api_priority_group_index_get_internal(
      device, pg_handle, ppg_index))
}

switch_status_t switch_api_buffer_pool_color_drop_enable(
    switch_device_t device, switch_handle_t pool_handle, bool enable) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_color_drop_enable_internal(
      device, pool_handle, enable))
}

switch_status_t switch_api_total_buffer_size_get(switch_device_t device,
                                                 switch_uint64_t *size) {
  SWITCH_MT_WRAP(switch_api_total_buffer_size_get_internal(device, size))
}

switch_status_t switch_api_max_ingress_pool_get(switch_device_t device,
                                                switch_uint8_t *pool_size) {
  SWITCH_MT_WRAP(switch_api_max_ingress_pool_get_internal(device, pool_size))
}

switch_status_t switch_api_max_egress_pool_get(switch_device_t device,
                                               switch_uint8_t *pool_size) {
  SWITCH_MT_WRAP(switch_api_max_egress_pool_get_internal(device, pool_size))
}

switch_status_t switch_api_buffer_pool_threshold_mode_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_buffer_threshold_mode_t *threshold_mode) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_threshold_mode_get_internal(
      device, pool_handle, threshold_mode))
}

switch_status_t switch_api_buffer_pool_size_set(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_uint32_t size) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_size_set_internal(
      device, buffer_pool_handle, size))
}

switch_status_t switch_api_buffer_pool_size_get(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_uint32_t *size) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_size_get_internal(
      device, buffer_pool_handle, size))
}

switch_status_t switch_api_buffer_pool_type_get(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_direction_t *dir) {
  SWITCH_MT_WRAP(
      switch_api_buffer_pool_type_get_internal(device, buffer_pool_handle, dir))
}

switch_status_t switch_api_buffer_profile_info_set(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *profile_info) {
  SWITCH_MT_WRAP(switch_api_buffer_profile_info_set_internal(
      device, buffer_profile_handle, profile_info))
}

switch_status_t switch_api_buffer_profile_info_get(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *profile_info) {
  SWITCH_MT_WRAP(switch_api_buffer_profile_info_get_internal(
      device, buffer_profile_handle, profile_info))
}

switch_status_t switch_api_buffer_pool_xoff_size_set(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t xoff_size) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_xoff_size_set_internal(
      device, pool_handle, xoff_size))
}

switch_status_t switch_api_buffer_pool_xoff_size_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *xoff_size) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_xoff_size_get_internal(
      device, pool_handle, xoff_size))
}

switch_status_t switch_api_buffer_pool_usage_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes) {
  SWITCH_MT_WRAP(switch_api_buffer_pool_usage_get_internal(
      device, pool_handle, curr_occupancy_bytes, watermark_bytes))
}
