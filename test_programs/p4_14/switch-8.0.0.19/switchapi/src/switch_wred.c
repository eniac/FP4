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

#include "switchapi/switch_wred.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_wred_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  switch_size_t wred_table_size = 0;
  status =
      switch_api_table_size_get(device, SWITCH_TABLE_WRED, &wred_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_WRED, wred_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_WRED_COUNTER, wred_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_WRED_PROFILE, wred_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "wred init failed for device %d: profile handle init failed %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("wred init done successfully for device %d\n", device);

  SWITCH_LOG_EXIT();

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_wred_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_WRED
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_WRED);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_WRED_COUNTER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_wred_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_wred_action_default_entry_add(device);
  return status;
}

switch_status_t switch_wred_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  // TODO
  return status;
}

switch_status_t switch_api_wred_create_internal(
    switch_device_t device,
    switch_api_wred_info_t *api_wred_info,
    switch_handle_t *wred_handle) {
  switch_wred_info_t *wred_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!api_wred_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  SWITCH_ASSERT(wred_handle != NULL);
  *wred_handle = switch_wred_handle_create(device);
  if (*wred_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    goto cleanup;
  }

  status = switch_wred_get(device, *wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_MEMCPY(
      &wred_info->api_wred_info, api_wred_info, sizeof(switch_api_wred_info_t));

  status = switch_pd_wred_early_drop_set(device, *wred_handle, wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_LIST_INIT(&(wred_info->queue_ent_list));

  status =
      switch_pd_wred_action_table_entry_add(device, *wred_handle, wred_info);

cleanup:
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG("wred create success on device %d with handle 0x%lx\n",
                        device,
                        *wred_handle);
  } else {
    SWITCH_PD_LOG_ERROR("wred create failed on device %d : %s\n",
                        device,
                        switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_api_wred_delete_internal(switch_device_t device,
                                                switch_handle_t wred_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_info_t *wred_info = NULL;
  switch_wred_queue_entry_t *queue_entry = NULL;
  switch_node_t *node = NULL;

  SWITCH_ASSERT(SWITCH_WRED_HANDLE(wred_handle));
  if (!SWITCH_WRED_HANDLE(wred_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  status = switch_wred_get(device, wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  status = switch_pd_wred_action_table_entry_delete(device, wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  /* Detach the wred profile from all of the queues */
  if (SWITCH_LIST_COUNT(&(wred_info->queue_ent_list)) != 0) {
    FOR_EACH_IN_LIST(wred_info->queue_ent_list, node) {
      queue_entry = (switch_wred_queue_entry_t *)node->data;
      status = switch_api_wred_detach(
          device, queue_entry->handle, queue_entry->packet_color);
      if (status != SWITCH_STATUS_SUCCESS) {
        goto cleanup;
      }
    }
    FOR_EACH_IN_LIST_END();
  }

  status = switch_wred_handle_delete(device, wred_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_wred_update_internal(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_api_wred_info_t *api_wred_info) {
  switch_wred_info_t *wred_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_wred_info != NULL);
  if (!api_wred_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("wred update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_WRED_HANDLE(wred_handle));
  if (!SWITCH_WRED_HANDLE(wred_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("wred update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wred_get(device, wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      &wred_info->api_wred_info, api_wred_info, sizeof(switch_api_wred_info_t));

  status = switch_pd_wred_early_drop_set(device, wred_handle, wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  status =
      switch_pd_wred_action_table_entry_update(device, wred_handle, wred_info);

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_wred_get_internal(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_api_wred_info_t *api_wred_info) {
  switch_wred_info_t *wred_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WRED_HANDLE(wred_handle));
  if (!SWITCH_WRED_HANDLE(wred_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("wred get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(api_wred_info != NULL);
  if (!api_wred_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("wred get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_wred_get(device, wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      api_wred_info, &wred_info->api_wred_info, sizeof(switch_api_wred_info_t));

  return status;
}

switch_status_t switch_api_wred_attach_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_meter_counter_t packet_color,
    switch_handle_t wred_handle) {
  switch_wred_info_t *wred_info = NULL;
  switch_queue_info_t *queue_info = NULL;
  switch_wred_queue_entry_t *queue_entry = NULL;
  switch_port_info_t *port_info = NULL;
  switch_handle_t *wred_stats_handle = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_WRED_HANDLE(wred_handle));
  if (!SWITCH_WRED_HANDLE(wred_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  status = switch_wred_get(device, wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(queue_info->port_handle));
  if (!SWITCH_PORT_HANDLE(queue_info->port_handle)) {
    goto cleanup;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  // Check if it the queue is already attached to another wred profile
  if (queue_info->wred_handles[packet_color] != SWITCH_API_INVALID_HANDLE) {
    status = switch_api_wred_detach(device, queue_handle, packet_color);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

  // Assign a new wred_stats handle if it's not already allocated.
  wred_stats_handle = &(queue_info->wred_stats_handles[packet_color]);
  if (*wred_stats_handle == SWITCH_API_INVALID_HANDLE) {
    *wred_stats_handle = switch_wred_counter_handle_create(device);
  }

  queue_entry = SWITCH_MALLOC(device, sizeof(switch_wred_queue_entry_t), 1);
  SWITCH_MEMSET(queue_entry, 0, sizeof(switch_wred_queue_entry_t));
  queue_entry->handle = queue_handle;
  queue_entry->id = queue_info->queue_id;
  queue_entry->packet_color = packet_color;
  queue_entry->port = port_info->dev_port;

  status = switch_pd_wred_index_table_entry_add(
      device, queue_entry, wred_handle, *wred_stats_handle);

  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  queue_info->wred_handles[packet_color] = wred_handle;

  status = SWITCH_LIST_INSERT(
      &wred_info->queue_ent_list, &queue_entry->node, queue_entry);

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred attach failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_wred_detach_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_meter_counter_t packet_color) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_wred_info_t *wred_info = NULL;
  switch_wred_queue_entry_t *queue_entry = NULL;
  switch_node_t *node = NULL;
  switch_pd_hdl_t *ent_hdl = NULL;
  switch_handle_t wred_handle;

  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  wred_handle = queue_info->wred_handles[packet_color];
  if (!wred_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue %d is not attached to any wred profiles. "
        "wred detach failed on device %d: %s\n",
        queue_handle,
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_wred_get(device, wred_handle, &wred_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }
  SWITCH_ASSERT(wred_info);

  FOR_EACH_IN_LIST(wred_info->queue_ent_list, node) {
    queue_entry = (switch_wred_queue_entry_t *)node->data;
    if (queue_entry->handle == queue_handle &&
        queue_entry->packet_color == packet_color) {
      ent_hdl = &queue_entry->ent_hdl;
      if (ent_hdl != SWITCH_API_INVALID_HANDLE) {
        status = switch_pd_wred_index_table_entry_delete(device, *ent_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          goto cleanup;
        }
        *ent_hdl = SWITCH_API_INVALID_HANDLE;
      }

      queue_info->wred_handles[packet_color] = SWITCH_API_INVALID_HANDLE;
      SWITCH_LIST_DELETE(&(wred_info->queue_ent_list), node);
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred detach failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_wred_stats_get_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_uint8_t count,
    switch_wred_counter_t *counter_ids,
    switch_counter_t *counters) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_wred_counter_t counter_id;
  switch_meter_counter_t packet_color;
  switch_uint8_t index = 0;

  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }
  SWITCH_ASSERT(queue_info != NULL);

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(queue_info->port_handle));
  if (!SWITCH_PORT_HANDLE(queue_info->port_handle)) {
    goto cleanup;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }
  SWITCH_ASSERT(port_info != NULL);

  for (index = 0; index < count; ++index) {
    counter_id = counter_ids[index];
    switch (counter_id) {
      case SWITCH_WRED_STATS_GREEN_DROPPED:
      case SWITCH_WRED_STATS_GREEN_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_GREEN;
        break;
      case SWITCH_WRED_STATS_YELLOW_DROPPED:
      case SWITCH_WRED_STATS_YELLOW_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_YELLOW;
        break;
      case SWITCH_WRED_STATS_RED_DROPPED:
      case SWITCH_WRED_STATS_RED_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_RED;
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    status =
        switch_pd_wred_stats_get(device,
                                 counter_id,
                                 queue_info->wred_stats_handles[packet_color],
                                 &counters[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred stats failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_wred_stats_clear_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_uint8_t count,
    switch_wred_counter_t *counter_ids) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_wred_counter_t counter_id;
  switch_meter_counter_t packet_color;
  switch_uint8_t index = 0;

  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    goto cleanup;
  }
  SWITCH_ASSERT(queue_info != NULL);

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(queue_info->port_handle));
  if (!SWITCH_PORT_HANDLE(queue_info->port_handle)) {
    goto cleanup;
  }

  status = switch_port_get(device, queue_info->port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }
  SWITCH_ASSERT(port_info != NULL);

  for (index = 0; index < count; ++index) {
    counter_id = counter_ids[index];
    switch (counter_id) {
      case SWITCH_WRED_STATS_GREEN_DROPPED:
      case SWITCH_WRED_STATS_GREEN_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_GREEN;
        break;
      case SWITCH_WRED_STATS_YELLOW_DROPPED:
      case SWITCH_WRED_STATS_YELLOW_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_YELLOW;
        break;
      case SWITCH_WRED_STATS_RED_DROPPED:
      case SWITCH_WRED_STATS_RED_ECN_MARKED:
        packet_color = SWITCH_METER_COUNTER_RED;
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    status = switch_pd_wred_stats_clear(
        device, counter_id, queue_info->wred_stats_handles[packet_color]);
    if (status != SWITCH_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

cleanup:
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("wred stats failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

static void switch_api_populate_wred_info(
    switch_color_t color,
    switch_api_wred_profile_info_t *profile_info,
    switch_api_wred_info_t *api_info) {
  double prob = 0;
  api_info->enable = profile_info->enable[color];
  api_info->ecn_mark = profile_info->ecn_mark[color];
  api_info->min_threshold = profile_info->min_threshold[color];
  api_info->max_threshold = profile_info->max_threshold[color];
  prob = (double)profile_info->probability[color] / 100.0;
  api_info->max_probability = prob;
}

switch_status_t switch_api_wred_profile_create_internal(
    switch_device_t device,
    switch_api_wred_profile_info_t *api_info,
    switch_handle_t *profile_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_profile_info_t *profile_info = NULL;
  switch_color_t color_index;

  switch_api_wred_info_t api_wred_info;
  *profile_handle = switch_wred_profile_handle_create(device);
  if (*profile_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "Failed to create wred profile on device %d: handle create failed",
        device);
    return SWITCH_STATUS_NO_MEMORY;
  }

  status = switch_wred_profile_get(device, *profile_handle, &profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to create wred profile on device %d: profile get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (color_index = SWITCH_COLOR_GREEN; color_index < SWITCH_COLOR_MAX;
       color_index++) {
    if (api_info->enable[color_index]) {
      SWITCH_MEMSET(&api_wred_info, 0, sizeof(switch_api_wred_info_t));
      switch_api_populate_wred_info(color_index, api_info, &api_wred_info);
      status = switch_api_wred_create(
          device, &api_wred_info, &(profile_info->wred_handles[color_index]));
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "Failed to create wred profile on device %d: wred create failed "
            "for prof_handle 0x%lx: %s",
            device,
            *profile_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }
  SWITCH_MEMCPY(&profile_info->api_info,
                api_info,
                sizeof(switch_api_wred_profile_info_t));
  return status;
}

switch_status_t switch_api_wred_profile_delete_internal(
    switch_device_t device, switch_handle_t profile_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_profile_info_t *profile_info = NULL;
  switch_color_t color_index;

  status = switch_wred_profile_get(device, profile_handle, &profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to delete wred profile on device %d: profile get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (color_index = SWITCH_COLOR_GREEN; color_index < SWITCH_COLOR_MAX;
       color_index++) {
    if (profile_info->api_info.enable[color_index] &&
        (profile_info->wred_handles[color_index] !=
         SWITCH_API_INVALID_HANDLE)) {
      status = switch_api_wred_delete(device,
                                      profile_info->wred_handles[color_index]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "Failed to delete wred profile on device %d: wred delete failed "
            "for prof_handle 0x%lx: %s",
            device,
            profile_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }
  return status;
}

switch_status_t switch_api_wred_profile_get_internal(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_api_wred_profile_info_t *api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_profile_info_t *profile_info = NULL;

  status = switch_wred_profile_get(device, profile_handle, &profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get wred profile on device %d: profile get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(api_info,
                &profile_info->api_info,
                sizeof(switch_api_wred_profile_info_t));
  return status;
}
switch_status_t switch_api_wred_profile_set_internal(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_color_t color,
    switch_api_wred_profile_info_t *profile_api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_profile_info_t *profile_info = NULL;
  switch_api_wred_info_t api_info;

  status = switch_wred_profile_get(device, profile_handle, &profile_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get wred profile on device %d: profile get failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&api_info, 0, sizeof(switch_api_wred_info_t));
  switch_api_populate_wred_info(color, profile_api_info, &api_info);

  if (profile_api_info->enable[color] &&
      !profile_info->api_info.enable[color]) {
    /*
     * Enable WRED profile for a color.
     */
    status = switch_api_wred_create(
        device, &api_info, &(profile_info->wred_handles[color]));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to update wred profile on device %d: wred create failed for "
          "prof_handle 0x%lx: %s",
          device,
          profile_handle,
          switch_error_to_string(status));
      return status;
    }
  } else if (!profile_api_info->enable[color]) {
    status = switch_api_wred_delete(device, profile_info->wred_handles[color]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to update wred profile on device %d: wred delete failed for "
          "prof handle 0x%lx: %s",
          device,
          profile_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    status = switch_api_wred_update(
        device, profile_info->wred_handles[color], &api_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to update wred profile on device %d: wred update failed for "
          "prof handle 0x%lx: %s",
          device,
          profile_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  SWITCH_MEMCPY(&profile_info->api_info,
                profile_api_info,
                sizeof(switch_api_wred_profile_info_t));
  return status;
}

switch_status_t switch_api_queue_wred_profile_set_internal(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_handle_t queue_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_wred_profile_info_t *profile_info = NULL;
  bool attach = true;
  switch_color_t color_index;
  switch_queue_info_t *queue_info = NULL;

  if (profile_handle == SWITCH_API_INVALID_HANDLE) {
    attach = false;
  } else {
    SWITCH_ASSERT(SWITCH_WRED_PROFILE_HANDLE(profile_handle));
    status = switch_wred_profile_get(device, profile_handle, &profile_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set wred profile to a queue 0x%lx on device %d: profile "
          "get failed for handle 0x%lx%s",
          queue_handle,
          device,
          profile_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set wred profile to a queue on device %d: queue get failed "
        "for handle 0x%lx",
        device,
        queue_handle);
    return status;
  }
  for (color_index = SWITCH_COLOR_GREEN; color_index < SWITCH_COLOR_MAX;
       color_index++) {
    if (profile_info &&
        profile_info->wred_handles[color_index] != SWITCH_API_INVALID_HANDLE) {
      if (attach) {
        status =
            switch_api_wred_attach(device,
                                   queue_handle,
                                   color_index,
                                   profile_info->wred_handles[color_index]);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to set wred profile to a queue 0x%lx on device %d: color "
              "%d profile attach failed for queue 0x%lx",
              device,
              color_index,
              queue_handle);
          return status;
        }
      }
    } else {
      if (queue_info->wred_handles[color_index]) {
        status = switch_api_wred_detach(device, queue_handle, color_index);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to set wred profile to a queue on device %d: wred detach "
              "failed for queue 0x%lx",
              device,
              queue_handle);
          return status;
        }
      }
      queue_info->wred_handles[color_index] = SWITCH_API_INVALID_HANDLE;
    }
  }
  queue_info->wred_profile_handle = profile_handle;
  return status;
}

switch_status_t switch_api_queue_wred_profile_get_internal(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *profile_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get wred profile queue on device %d: queue get failed for "
        "handle 0x%lx",
        device,
        queue_handle);
    return status;
  }
  *profile_handle = queue_info->wred_profile_handle;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_wred_delete(switch_device_t device,
                                       switch_handle_t wred_handle) {
  SWITCH_MT_WRAP(switch_api_wred_delete_internal(device, wred_handle))
}

switch_status_t switch_api_wred_get(switch_device_t device,
                                    switch_handle_t wred_handle,
                                    switch_api_wred_info_t *api_wred_info) {
  SWITCH_MT_WRAP(
      switch_api_wred_get_internal(device, wred_handle, api_wred_info))
}

switch_status_t switch_api_wred_create(switch_device_t device,
                                       switch_api_wred_info_t *api_wred_info,
                                       switch_handle_t *wred_handle) {
  SWITCH_MT_WRAP(
      switch_api_wred_create_internal(device, api_wred_info, wred_handle))
}

switch_status_t switch_api_wred_detach(switch_device_t device,
                                       switch_handle_t queue_handle,
                                       switch_meter_counter_t packet_color) {
  SWITCH_MT_WRAP(
      switch_api_wred_detach_internal(device, queue_handle, packet_color))
}

switch_status_t switch_api_wred_attach(switch_device_t device,
                                       switch_handle_t queue_handle,
                                       switch_meter_counter_t packet_color,
                                       switch_handle_t wred_handle) {
  SWITCH_MT_WRAP(switch_api_wred_attach_internal(
      device, queue_handle, packet_color, wred_handle))
}

switch_status_t switch_api_wred_update(switch_device_t device,
                                       switch_handle_t wred_handle,
                                       switch_api_wred_info_t *api_wred_info) {
  SWITCH_MT_WRAP(
      switch_api_wred_update_internal(device, wred_handle, api_wred_info))
}

switch_status_t switch_api_wred_stats_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          switch_uint8_t num_entries,
                                          switch_wred_counter_t *counter_ids,
                                          switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_wred_stats_get_internal(
      device, queue_handle, num_entries, counter_ids, counters))
}

switch_status_t switch_api_wred_stats_clear(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_uint8_t num_entries,
    switch_wred_counter_t *counter_ids) {
  SWITCH_MT_WRAP(switch_api_wred_stats_clear_internal(
      device, queue_handle, num_entries, counter_ids))
}

switch_status_t switch_api_wred_profile_create(
    switch_device_t device,
    switch_api_wred_profile_info_t *api_info,
    switch_handle_t *profile_handle) {
  SWITCH_MT_WRAP(
      switch_api_wred_profile_create_internal(device, api_info, profile_handle))
}

switch_status_t switch_api_wred_profile_delete(switch_device_t device,
                                               switch_handle_t profile_handle) {
  SWITCH_MT_WRAP(
      switch_api_wred_profile_delete_internal(device, profile_handle))
}

switch_status_t switch_api_wred_profile_get(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_api_wred_profile_info_t *api_info) {
  SWITCH_MT_WRAP(
      switch_api_wred_profile_get_internal(device, profile_handle, api_info))
}

switch_status_t switch_api_wred_profile_set(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_color_t color,
    switch_api_wred_profile_info_t *api_info) {
  SWITCH_MT_WRAP(switch_api_wred_profile_set_internal(
      device, profile_handle, color, api_info))
}

switch_status_t switch_api_queue_wred_profile_set(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_handle_t queue_handle) {
  SWITCH_MT_WRAP(switch_api_queue_wred_profile_set_internal(
      device, profile_handle, queue_handle))
}

switch_status_t switch_api_queue_wred_profile_get(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *profile_handle) {
  SWITCH_MT_WRAP(switch_api_queue_wred_profile_get_internal(
      device, queue_handle, profile_handle))
}
