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

#include "switchapi/switch_queue.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_queue_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_queue_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_queue_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_QUEUE, SWITCH_QUEUE_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue init failed for device %d: "
        "queue handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_max_queues_get_internal(
    switch_device_t device, switch_uint32_t *max_queues) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *max_queues = SWITCH_MAX_PORT_QUEUE;

  return status;
}

switch_status_t switch_api_max_cpu_queues_get_internal(
    switch_device_t device, switch_uint32_t *max_queues) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *max_queues = SWITCH_MAX_CPU_QUEUE;

  return status;
}

switch_status_t switch_api_max_traffic_class_get_internal(
    switch_device_t device, switch_uint32_t *max_tc) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *max_tc = SWITCH_MAX_TRAFFIC_CLASSES;

  return status;
}

switch_status_t switch_queue_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_QUEUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue free failed for device: %d "
        "queue handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_queue_create_internal(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_queue_info_t *api_queue_info,
    switch_handle_t *queue_handle) {
  switch_port_info_t *port_info = NULL;
  switch_queue_info_t *queue_info = NULL;
  switch_queue_info_t *tmp_queue_info = NULL;
  switch_handle_t tmp_queue_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_DEBUG("queue create on device %d port handle 0x%lx qid %d\n",
                   device,
                   api_queue_info->port_handle,
                   api_queue_info->queue_id);

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(api_queue_info->port_handle));
  if (!SWITCH_PORT_HANDLE(api_queue_info->port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue create failed on device %d port handle 0x%lx qid %d: "
        "port handle invalid:(%s)\n",
        device,
        api_queue_info->port_handle,
        api_queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, api_queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue create failed on device %d port handle 0x%lx qid %d: "
        "port get failed:(%s)\n",
        device,
        api_queue_info->port_handle,
        api_queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(api_queue_info->queue_id < port_info->max_queues);
  if (api_queue_info->queue_id >= port_info->max_queues) {
    SWITCH_LOG_ERROR(
        "queue create failed on device %d port handle 0x%lx qid %d: "
        "queue id invalid:(%s)\n",
        device,
        api_queue_info->port_handle,
        api_queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  tmp_queue_handle = port_info->queue_handles[api_queue_info->queue_id];
  if (SWITCH_QUEUE_HANDLE(tmp_queue_handle)) {
    status = switch_queue_get(device, tmp_queue_handle, &tmp_queue_info);
    if (status == SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      SWITCH_LOG_ERROR(
          "queue create failed on device %d port handle 0x%lx qid %d: "
          "queue id exists:(%s)\n",
          device,
          api_queue_info->port_handle,
          api_queue_info->queue_id,
          switch_error_to_string(status));
      return status;
    }
  }

  handle = switch_queue_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "queue create failed on device %d port handle 0x%lx qid %d: "
        "queue handle create failed:(%s)\n",
        device,
        api_queue_info->port_handle,
        api_queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue create failed on device %d port handle 0x%lx qid %d: "
        "queue get failed:(%s)\n",
        device,
        api_queue_info->port_handle,
        api_queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  queue_info->port_handle = api_queue_info->port_handle;
  queue_info->queue_id = api_queue_info->queue_id;
  port_info->queue_handles[api_queue_info->queue_id] = handle;
  port_info->num_queues++;

  SWITCH_LOG_DEBUG(
      "queue created on device %d port handle 0x%lx "
      "queue handle 0x%lx qid %d\n",
      device,
      api_queue_info->port_handle,
      handle,
      api_queue_info->queue_id);

  *queue_handle = handle;

  return status;
}

switch_status_t switch_api_queue_delete_internal(
    const switch_device_t device, const switch_handle_t queue_handle) {
  switch_port_info_t *port_info = NULL;
  switch_queue_info_t *queue_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_DEBUG(
      "queue delete on device %d queue handle 0x%lx\n", device, queue_handle);

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue delete failed on device %d queue handle 0x%lx: "
        "queue handle invalid:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue delete failed on device %d queue handle 0x%lx: "
        "queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue delete failed on device %d queue handle 0x%lx: "
        "port handle 0x%lx qid %d: port get failed:(%s)\n",
        device,
        queue_handle,
        queue_info->port_handle,
        queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  port_info->queue_handles[queue_info->queue_id] = SWITCH_API_INVALID_HANDLE;
  port_info->num_queues--;

  status = switch_queue_handle_delete(device, queue_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "queue deleted on device %d queue handle 0x%lx\n", device, queue_handle);

  return status;
}

switch_status_t switch_api_queues_get_internal(switch_device_t device,
                                               switch_handle_t port_handle,
                                               uint32_t *num_queues,
                                               switch_handle_t *queue_handles) {
  switch_port_info_t *port_info = NULL;
  switch_uint32_t index = 0;
  switch_uint32_t queue_index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!num_queues || !queue_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue handles get failed on device %d port handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue handles get failed on device %d port handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue handles get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  queue_index = 0;
  for (index = 0; index < port_info->num_queues; index++) {
    if (SWITCH_QUEUE_HANDLE(port_info->queue_handles[index])) {
      queue_handles[queue_index++] = port_info->queue_handles[index];
    }
  }

  *num_queues = queue_index;
  SWITCH_LOG_DEBUG(
      "queue get successful on device %d port handle 0x%lx "
      "num queues %d\n",
      device,
      port_handle,
      *num_queues);

  return status;
}

switch_status_t switch_api_queue_color_drop_enable_internal(
    switch_device_t device, switch_handle_t queue_handle, bool enable) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue color drop enable failed on device %d "
        "queue handle 0x%lx enable %d: queue handle invalid:(%s)\n",
        device,
        queue_handle,
        enable,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue color drop enable failed on device %d "
        "queue handle 0x%lx enable %d: queue get failed:(%s)\n",
        device,
        queue_handle,
        enable,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue color drop enable failed on device %d "
        "queue handle 0x%lx enable %d: port get failed:(%s)\n",
        device,
        queue_handle,
        enable,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_color_drop_enable(
      device, port_info->dev_port, queue_info->queue_id, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue color drop enable failed on device %d "
        "queue handle 0x%lx enable %d qid %d: "
        "port get failed:(%s)\n",
        device,
        queue_handle,
        enable,
        queue_info->queue_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "queue color drop enabled on device %d "
      "queue handle 0x%lx qid %d enable %d\n",
      device,
      queue_handle,
      queue_info->queue_id,
      enable);

  queue_info->color_drop_enable = enable;

  return status;
}

switch_status_t switch_api_queue_color_limit_set_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_color_t color,
    switch_uint32_t limit) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  SWITCH_ASSERT(color < SWITCH_COLOR_MAX);
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue limit set failed on device %d queue handle 0x%lx "
        "color %s limit %d: "
        "queue handle invalid:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue limit set failed on device %d queue handle 0x%lx "
        "color %s limit %d: "
        "queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue limit set failed on device %d queue handle 0x%lx "
        "color %s limit %d: "
        "port get failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_color_limit_set(
      device, port_info->dev_port, queue_info->queue_id, color, limit);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue limit set failed on device %d queue handle 0x%lx "
        "color %s limit %d: "
        "queue pd color limit set failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "queue color limit on device %d queue handle 0x%lx"
      "color %s limit %d\n",
      device,
      queue_handle,
      switch_color_to_string(color),
      limit);

  queue_info->color_limit[color] = limit;

  return status;
}

switch_status_t switch_api_queue_color_hysteresis_set_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_color_t color,
    switch_uint32_t limit) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  SWITCH_ASSERT(color < SWITCH_COLOR_MAX);
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue hystersis set failed on device %d "
        "queue handle 0x%lx color %s limit %d: "
        "queue handle invalid:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue hystersis set failed on device %d "
        "queue handle 0x%lx color %s limit %d: "
        "queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue hystersis set failed on device %d "
        "queue handle 0x%lx color %s limit %d: "
        "port get failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_color_hysteresis_set(
      device, port_info->dev_port, queue_info->queue_id, color, limit);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue hystersis set failed on device %d "
        "queue handle 0x%lx color %s limit %d: "
        "pd hystersis set failed:(%s)\n",
        device,
        queue_handle,
        switch_color_to_string(color),
        limit,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "queue hysteresis limit on device %d queue handle 0x%lx"
      "color %s limit %d\n",
      device,
      queue_handle,
      switch_color_to_string(color),
      limit);

  queue_info->hysteresis_limit[color] = limit;

  return status;
}

switch_status_t switch_api_queue_pfc_cos_mapping_internal(
    switch_device_t device, switch_handle_t queue_handle, switch_uint8_t cos) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue pfc cos mapping set failed on device %d "
        "queue handle 0x%lx cos %d: queue handle invalid:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue pfc cos mapping set failed on device %d "
        "queue handle 0x%lx cos %d: queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue pfc cos mapping set failed on device %d "
        "queue handle 0x%lx cos %d: port get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_pfc_cos_mapping(
      device, port_info->dev_port, queue_info->queue_id, cos);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue pfc cos mapping set failed on device %d "
        "queue handle 0x%lx cos %d: queue pfc cos mapping failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "queue pfc cos mapping on device %d queue handle 0x%lx cos %d\n",
      device,
      queue_handle,
      cos);

  return status;
}

switch_status_t switch_api_dtel_tail_drop_deflection_queue_set_internal(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_handle_t queue_handle) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue mirror on drop set failed on device %d "
        "queue handle 0x%lx queue handle invalid:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue mirror on drop set failed on device %d "
        "queue handle 0x%lx queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue pfc cos mapping set failed on device %d "
        "queue handle 0x%lx port get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_tail_drop_deflection_queue_set(
      device, pipe_id, port_info->dev_port, queue_info->queue_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue mirror on drop set failed on device %d "
        "queue handle 0x%lx mod set failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "queue mod set on device %d queue handle 0x%lx\n", device, queue_handle);

  return status;
}

switch_status_t switch_api_queue_shaping_set_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    bool pps,
    switch_uint32_t burst_size,
    switch_uint64_t rate) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set queue shaping on device %d: queue info get failed for "
        "handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Failed to set queue shaping on device %d: port get failed for "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        queue_info->port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_shaping_set(
      device, port_info->dev_port, queue_info->queue_id, pps, burst_size, rate);
  return status;
}

switch_status_t switch_api_queue_dwrr_weight_set_internal(
    switch_device_t device, switch_handle_t queue_handle, uint16_t weight) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set DWRR on device %d: queue info get failed for handle "
        "0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Failed to set DWRR on device %d: port get failed for "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        queue_info->port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(weight <= 100);
  status = switch_pd_queue_scheduling_dwrr_weight_set(
      device, port_info->dev_port, queue_info->queue_id, weight);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "FAiled to set dwrr on device %d: weight set in hw failed for handle "
        "0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_queue_strict_priority_set_internal(
    switch_device_t device, switch_handle_t queue_handle, bool priority) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set queue priority on device: %d queue info get failed for "
        "handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Failed to set queue priority on device %d: port get failed for "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        queue_info->port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_scheduling_strict_priority_set(
      device, port_info->dev_port, queue_info->queue_id, priority);
  return status;
}

switch_status_t switch_api_queue_guaranteed_rate_set_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    bool pps,
    uint32_t burst_size,
    switch_uint64_t rate) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set min guarateed rate on device %d: queue info get failed "
        "for handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Failed to set guaranteed min rate on device %d: port get failed for "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        queue_info->port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_guaranteed_rate_set(
      device, port_info->dev_port, queue_info->queue_id, pps, burst_size, rate);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "FAiled to set min guaranteed rate on device %d: min rate set failed "
        "for handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_queue_index_get_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_uint8_t *queue_index) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue index get failed on device: %d: for queue handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }
  *queue_index = queue_info->queue_id;
  return status;
}

switch_status_t switch_api_queue_port_get_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *port_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue port get failed on device: %d: for queue handle 0x%lx: %s",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }
  *port_handle = queue_info->port_handle;
  return status;
}

switch_status_t switch_api_queue_drop_get_internal(switch_device_t device,
                                                   switch_handle_t queue_handle,
                                                   uint64_t *num_packets) {
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue drop count get failed on device %d queue handle 0x%lx "
        "queue get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue drop count get failed on device %d queue handle 0x%lx "
        "port get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_queue_drop_count_get(
      device, port_info->dev_port, queue_info->queue_id, num_packets);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue drop count get failed on device %d queue handle 0x%lx "
        "queue pd drop count get failed:(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}
#ifdef __cplusplus
}
#endif

switch_status_t switch_api_queue_color_limit_set(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 switch_color_t color,
                                                 switch_uint32_t limit) {
  SWITCH_MT_WRAP(switch_api_queue_color_limit_set_internal(
      device, queue_handle, color, limit))
}

switch_status_t switch_api_queue_color_hysteresis_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_color_t color,
    switch_uint32_t limit) {
  SWITCH_MT_WRAP(switch_api_queue_color_hysteresis_set_internal(
      device, queue_handle, color, limit))
}

switch_status_t switch_api_queues_get(switch_device_t device,
                                      switch_handle_t port_handle,
                                      switch_uint32_t *num_queues,
                                      switch_handle_t *queue_handles) {
  SWITCH_MT_WRAP(switch_api_queues_get_internal(
      device, port_handle, num_queues, queue_handles))
}

switch_status_t switch_api_queue_pfc_cos_mapping(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 switch_uint8_t cos) {
  SWITCH_MT_WRAP(
      switch_api_queue_pfc_cos_mapping_internal(device, queue_handle, cos))
}

switch_status_t switch_api_queue_color_drop_enable(switch_device_t device,
                                                   switch_handle_t queue_handle,
                                                   bool enable) {
  SWITCH_MT_WRAP(
      switch_api_queue_color_drop_enable_internal(device, queue_handle, enable))
}

switch_status_t switch_api_max_cpu_queues_get(switch_device_t device,
                                              switch_uint32_t *max_queues) {
  SWITCH_MT_WRAP(switch_api_max_cpu_queues_get_internal(device, max_queues))
}

switch_status_t switch_api_max_queues_get(switch_device_t device,
                                          switch_uint32_t *max_queues) {
  SWITCH_MT_WRAP(switch_api_max_queues_get_internal(device, max_queues))
}

switch_status_t switch_api_max_traffic_class_get(switch_device_t device,
                                                 switch_uint32_t *max_tc) {
  SWITCH_MT_WRAP(switch_api_max_traffic_class_get_internal(device, max_tc))
}

switch_status_t switch_api_dtel_tail_drop_deflection_queue_set(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_handle_t queue_handle) {
  SWITCH_MT_WRAP(switch_api_dtel_tail_drop_deflection_queue_set_internal(
      device, pipe_id, queue_handle))
}

switch_status_t switch_api_queue_delete(const switch_device_t device,
                                        const switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_queue_delete_internal(device, port_handle))
}

switch_status_t switch_api_queue_create(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_queue_info_t *api_queue_info,
    switch_handle_t *queue_handle) {
  SWITCH_MT_WRAP(switch_api_queue_create_internal(
      device, flags, api_queue_info, queue_handle));
}

switch_status_t switch_api_queue_shaping_set(switch_device_t device,
                                             switch_handle_t queue_handle,
                                             bool pps,
                                             switch_uint32_t burst_bytes,
                                             switch_uint64_t rate) {
  SWITCH_MT_WRAP(switch_api_queue_shaping_set_internal(
      device, queue_handle, pps, burst_bytes, rate))
}

switch_status_t switch_api_queue_dwrr_weight_set(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 uint16_t weight) {
  SWITCH_MT_WRAP(
      switch_api_queue_dwrr_weight_set_internal(device, queue_handle, weight))
}

switch_status_t switch_api_queue_strict_priority_set(
    switch_device_t device, switch_handle_t queue_handle, bool priority) {
  SWITCH_MT_WRAP(switch_api_queue_strict_priority_set_internal(
      device, queue_handle, priority))
}

switch_status_t switch_api_queue_guaranteed_rate_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    bool pps,
    uint32_t burst_bytes,
    uint32_t rate) {
  SWITCH_MT_WRAP(switch_api_queue_guaranteed_rate_set_internal(
      device, queue_handle, pps, burst_bytes, rate))
}

switch_status_t switch_api_queue_index_get(switch_device_t device,
                                           switch_handle_t queue_handle,
                                           switch_uint8_t *index) {
  SWITCH_MT_WRAP(
      switch_api_queue_index_get_internal(device, queue_handle, index))
}

switch_status_t switch_api_queue_port_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(
      switch_api_queue_port_get_internal(device, queue_handle, port_handle))
}

switch_status_t switch_api_queue_drop_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          uint64_t *num_packets) {
  SWITCH_MT_WRAP(
      switch_api_queue_drop_get_internal(device, queue_handle, num_packets));
}
