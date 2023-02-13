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

#include "switchapi/switch_scheduler.h"
#include "switchapi/switch_queue.h"

/* Local header includes */
#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_port_shaping_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    bool pps,
    uint32_t burst_size,
    switch_uint64_t rate) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port shaping set failed on device: %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "port shaping set failed on device %d "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_shaping_set(
      device, port_info->dev_port, pps, burst_size, rate);
  return status;
}

switch_status_t switch_scheduler_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_SCHEDULER, SWITCH_SCHEDULER_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "scheduler init failed for device %d: "
        "scheduler handle init failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_SCHEDULER_GROUP,
                                   SWITCH_SCHEDULER_GROUP_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "scheduler init failed for device %d: "
        "scheduler group handle init failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_scheduler_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_SCHEDULER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "scheduler free failed for device %d: "
        "scheduler handle free failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_SCHEDULER_GROUP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "scheduler free failed for device %d: "
        "scheduler group handle free failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_scheduler_create_internal(
    switch_device_t device,
    const switch_scheduler_api_info_t *api_info,
    switch_handle_t *scheduler_handle) {
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *scheduler_handle = switch_scheduler_handle_create(device);
  if (scheduler_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "scheduler create failed on device %d: handle create failed: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_scheduler_get(device, *scheduler_handle, &scheduler_info);
  if (!scheduler_info) {
    SWITCH_LOG_ERROR(
        "scheduler create failed on device %d: scheduler info get failed %s",
        device,
        switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }
  SWITCH_MEMCPY(
      &scheduler_info->api_info, api_info, sizeof(switch_scheduler_api_info_t));
  SWITCH_LIST_INIT(&(scheduler_info->scheduler_group_list));
  return status;
}

switch_status_t switch_api_scheduler_delete_internal(
    switch_device_t device, switch_handle_t scheduler_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_scheduler_handle_delete(device, scheduler_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to delete scheduler on device %d: handle delete failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  /*
   * TODO: Driver API is required to reset all the queues to the default
   * scheduler settings.
   */
  return status;
}

switch_status_t switch_api_scheduler_config_set_internal(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    const switch_scheduler_api_info_t *api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info;
  switch_node_t *node = NULL;
  switch_scheduler_group_entry_t *group_entry = NULL;
  switch_handle_t queue_handle = SWITCH_API_INVALID_HANDLE;
  switch_scheduler_group_info_t *scheduler_group_info = NULL;

  SWITCH_ASSERT(SWITCH_SCHEDULER_HANDLE(scheduler_handle));
  status = switch_scheduler_get(device, scheduler_handle, &scheduler_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set scheduler config on device %d: scheduler info get "
        "failed for  handle 0x%lx: %s",
        device,
        scheduler_handle,
        switch_error_to_string(status));
    return status;
  }

  /*
   * When scheduler parameters are modified, update all the port and queue
   * handles
   * in the scheduler-group that has the same scheduler profile.
   */
  FOR_EACH_IN_LIST(scheduler_info->scheduler_group_list, node) {
    group_entry = (switch_scheduler_group_entry_t *)node->data;

    status = switch_scheduler_group_get(
        device, group_entry->scheduler_group_handle, &scheduler_group_info);
    if (!scheduler_group_info) {
      SWITCH_LOG_ERROR(
          "scheduler config set failed on device %d: scheduler group info "
          "get failed %s",
          device,
          switch_error_to_string(status));
      return SWITCH_API_INVALID_HANDLE;
    }
    if (scheduler_group_info->api_info.group_type ==
        SWITCH_SCHEDULER_GROUP_TYPE_PORT) {
      if (api_info->max_rate) {
        status = switch_api_port_shaping_set(device,
                                             group_entry->port_handle,
                                             false,
                                             api_info->max_burst_size,
                                             api_info->max_rate);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to set profile to scheduler group on device %d: port "
              "shaping set failed for handle 0x%lx: %s",
              device,
              group_entry->port_handle,
              switch_error_to_string(status));
          return status;
        }
      }
    } else {
      queue_handle = group_entry->queue_handle;

      if (scheduler_info->api_info.scheduler_type != api_info->scheduler_type) {
        if (api_info->scheduler_type == SWITCH_SCHEDULER_MODE_STRICT) {
          status =
              switch_api_queue_strict_priority_set(device, queue_handle, true);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "FAiled to update scheduler info on device %d: queue strict "
                "priority set failed for handle 0x%lx: %s",
                device,
                queue_handle,
                switch_error_to_string(status));
            return status;
          }

        } else if (api_info->scheduler_type == SWITCH_SCHEDULER_MODE_DWRR) {
          SWITCH_ASSERT(api_info->weight);
          status = switch_api_queue_dwrr_weight_set(
              device, queue_handle, api_info->weight);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "FAiled to update scheduler info on device %d: queue dwrr set "
                "failed for handle 0x%lx: %s",
                device,
                queue_handle,
                switch_error_to_string(status));
            return status;
          }
          status =
              switch_api_queue_strict_priority_set(device, queue_handle, false);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "FAiled to update scheduler info on device %d: queue normal "
                "priority set failed for handle 0x%lx: %s",
                device,
                queue_handle,
                switch_error_to_string(status));
            return status;
          }
        }
      }
      if (scheduler_info->api_info.max_rate != api_info->max_rate) {
        if (api_info->max_rate == 0) {
          /*
           * TODO: do we need to set to default rate??
           */
        } else {
          status = switch_api_queue_shaping_set(
              device,
              queue_handle,
              (api_info->shaper_type == SWITCH_SCHEDULER_PPS),
              api_info->max_burst_size,
              api_info->max_rate);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "Failed to update scheduler info on device %d: queue shaping "
                "set "
                "failed for handle 0x%lx: %s",
                device,
                queue_handle,
                switch_error_to_string(status));
            return status;
          }
        }
      }
      if (scheduler_info->api_info.min_rate != api_info->min_rate) {
        status = switch_api_queue_guaranteed_rate_set(
            device,
            queue_handle,
            (api_info->shaper_type == SWITCH_SCHEDULER_PPS),
            scheduler_info->api_info.min_burst_size,
            scheduler_info->api_info.min_rate);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to update scheduler info on device %d: queue guarantted "
              "set failed for handle 0x%lx: %s",
              device,
              queue_handle,
              switch_error_to_string(status));
          return status;
        }
      }
    }
  }
  FOR_EACH_IN_LIST_END();
  SWITCH_MEMCPY(
      &scheduler_info->api_info, api_info, sizeof(switch_scheduler_api_info_t));
  return status;
}

switch_status_t switch_api_scheduler_config_get_internal(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    switch_scheduler_api_info_t *api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info;

  SWITCH_ASSERT(SWITCH_SCHEDULER_HANDLE(scheduler_handle));
  status = switch_scheduler_get(device, scheduler_handle, &scheduler_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set scheduler config on device %d: scheduler info get "
        "failed for  handle 0x%lx: %s",
        device,
        scheduler_handle,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(
      api_info, &scheduler_info->api_info, sizeof(switch_scheduler_api_info_t));
  return status;
}

switch_status_t switch_api_scheduler_group_config_get_internal(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_scheduler_group_api_info_t *api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_group_info_t *scheduler_group_info = NULL;

  SWITCH_ASSERT(SWITCH_SCHEDULER_GROUP_HANDLE(scheduler_group_handle));
  status = switch_scheduler_group_get(
      device, scheduler_group_handle, &scheduler_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get scheduler config on device %d: scheduler info get "
        "failed for  handle 0x%lx: %s",
        device,
        scheduler_group_handle,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(api_info,
                &scheduler_group_info->api_info,
                sizeof(switch_scheduler_group_api_info_t));

  return status;
}

static switch_status_t switch_api_scheduer_group_profile_update(
    switch_device_t device,
    switch_handle_t group_handle,
    switch_handle_t port_handle,
    switch_handle_t queue_handle,
    switch_handle_t scheduler_handle) {
  switch_scheduler_group_info_t *scheduler_group_info = NULL;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_group_entry_t *scheduler_group_entry;

  status =
      switch_scheduler_group_get(device, group_handle, &scheduler_group_info);
  if (!scheduler_group_info) {
    SWITCH_LOG_ERROR(
        "scheduler_group profile set failed on device %d: scheduler group info "
        "get failed %s",
        device,
        switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  if (scheduler_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_SCHEDULER_HANDLE(scheduler_handle));
    status = switch_scheduler_get(device, scheduler_handle, &scheduler_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set profile to scheduler group on device %d: scheduler "
          "get failed: %s",
          device,
          switch_error_to_string(status));
      return status;
    }
  }
  if (scheduler_group_info->api_info.group_type ==
      SWITCH_SCHEDULER_GROUP_TYPE_PORT) {
    /*
     * For port level scheduler group, update max rate and max burst.
     */
    if (scheduler_info && scheduler_info->api_info.max_rate) {
      status =
          switch_api_port_shaping_set(device,
                                      port_handle,
                                      false,
                                      scheduler_info->api_info.max_burst_size,
                                      scheduler_info->api_info.max_rate);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "Failed to set profile to scheduler group on device %d: port "
            "shaping set failed for handle 0x%lx: %s",
            device,
            port_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  } else {
    if (queue_handle != SWITCH_API_INVALID_HANDLE) {
      SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
      if (scheduler_info && scheduler_info->api_info.max_rate) {
        status = switch_api_queue_shaping_set(
            device,
            queue_handle,
            false,
            scheduler_info->api_info.max_burst_size,
            scheduler_info->api_info.max_rate);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to set profile to scheduler group on device %d: queue "
              "shaping set failed for handle 0x%lx: %s",
              device,
              port_handle,
              switch_error_to_string(status));
          return status;
        }
      }

      if (scheduler_info &&
          scheduler_info->api_info.scheduler_type ==
              SWITCH_SCHEDULER_MODE_STRICT) {
        status =
            switch_api_queue_strict_priority_set(device, queue_handle, true);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "FAiled to set profile to scheduler group on device %d: queue "
              "strict priority set failed for handle 0x%lx: %s",
              device,
              queue_handle,
              switch_error_to_string(status));
          return status;
        }
      } else if (scheduler_info &&
                 scheduler_info->api_info.scheduler_type ==
                     SWITCH_SCHEDULER_MODE_DWRR) {
        SWITCH_ASSERT(scheduler_info->api_info.weight);
        status = switch_api_queue_dwrr_weight_set(
            device, queue_handle, scheduler_info->api_info.weight);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "FAiled to set profile to scheduler group on device %d: queue "
              "dwrr set failed for handle 0x%lx: %s",
              device,
              queue_handle,
              switch_error_to_string(status));
          return status;
        }
        status =
            switch_api_queue_strict_priority_set(device, queue_handle, false);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "FAiled to set profile to scheduler group on device %d: queue "
              "normal priority set failed for handle 0x%lx: %s",
              device,
              queue_handle,
              switch_error_to_string(status));
          return status;
        }
      }
      if (scheduler_info && scheduler_info->api_info.min_rate != 0) {
        status = switch_api_queue_guaranteed_rate_set(
            device,
            queue_handle,
            false,
            scheduler_info->api_info.min_burst_size,
            scheduler_info->api_info.min_rate);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "Failed to set profile to scheduler group on device %d: queue "
              "guarantted set failed for handle 0x%lx: %s",
              device,
              queue_handle,
              switch_error_to_string(status));
          return status;
        }
      }
    }
  }

  scheduler_group_entry =
      SWITCH_MALLOC(device, sizeof(switch_scheduler_group_entry_t), 1);
  SWITCH_MEMSET(
      scheduler_group_entry, 0, sizeof(switch_scheduler_group_entry_t));
  scheduler_group_entry->scheduler_group_handle = group_handle;
  if (scheduler_group_info->api_info.group_type ==
      SWITCH_SCHEDULER_GROUP_TYPE_PORT) {
    scheduler_group_entry->port_handle = port_handle;
  } else {
    scheduler_group_entry->queue_handle = queue_handle;
  }
  if (scheduler_info) {
    /*
     * Insert the scheduler group handle to scheduler_info list. This will be
     * used when
     * scheduler profile parameters are updated.
     */
    SWITCH_LIST_INSERT(&scheduler_info->scheduler_group_list,
                       &scheduler_group_entry->node,
                       scheduler_group_entry);
  }
  return status;
}

switch_status_t switch_api_scheduler_group_create_internal(
    switch_device_t device,
    switch_scheduler_group_api_info_t *api_info,
    switch_handle_t *scheduler_group_handle) {
  switch_scheduler_group_info_t *scheduler_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t port_handle;
  switch_handle_t queue_handle;
  switch_handle_t scheduler_handle;
  switch_port_info_t *port_info = NULL;

  *scheduler_group_handle = switch_scheduler_group_handle_create(device);
  if (scheduler_group_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "scheduler_group create failed on device %d: handle create failed: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_scheduler_group_get(
      device, *scheduler_group_handle, &scheduler_group_info);
  if (!scheduler_group_info) {
    SWITCH_LOG_ERROR(
        "scheduler_group create failed on device %d: scheduler group info get "
        "failed %s",
        device,
        switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }
  port_handle = api_info->port_handle;
  queue_handle = api_info->queue_handle;
  scheduler_handle = api_info->scheduler_handle;

  if (port_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
    status = switch_port_get(device, port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to create scheduler group on device %d: port info get failed "
          "for port 0x%lx: %s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  if (api_info->group_type == SWITCH_SCHEDULER_GROUP_TYPE_PORT) {
    if (port_handle != SWITCH_API_INVALID_HANDLE) {
      SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
      if (port_info) {
        port_info->port_scheduler_group_handle = *scheduler_group_handle;
      }
    }
  } else {
    if (port_info && port_info->queue_scheduler_group_handles) {
      port_info->queue_scheduler_group_handles[(handle_to_id(queue_handle)) %
                                               SWITCH_MAX_QUEUE] =
          *scheduler_group_handle;
    }
  }
  status = switch_api_scheduer_group_profile_update(device,
                                                    *scheduler_group_handle,
                                                    port_handle,
                                                    queue_handle,
                                                    scheduler_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to create scheduler profile on device %d: set scheduler group "
        "profile: %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(&scheduler_group_info->api_info,
                api_info,
                sizeof(switch_scheduler_group_api_info_t));
  return status;
}

switch_status_t switch_api_scheduler_group_delete_internal(
    switch_device_t device, switch_handle_t group_handle) {
  switch_scheduler_group_info_t *scheduler_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t scheduler_handle = SWITCH_API_INVALID_HANDLE;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_node_t *node = NULL;
  switch_scheduler_group_entry_t *group_entry;

  status =
      switch_scheduler_group_get(device, group_handle, &scheduler_group_info);
  if (!scheduler_group_info) {
    SWITCH_LOG_ERROR(
        "scheduler_group delete failed on device %d: scheduler group info get "
        "failed %s",
        device,
        switch_error_to_string(status));
    return status;
  }
  scheduler_handle = scheduler_group_info->api_info.scheduler_handle;

  if (scheduler_handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_DEBUG(
        "No scheduler profile configured for the scheduler group 0x%lx",
        group_handle);
    return status;
  }
  if (scheduler_handle != SWITCH_API_INVALID_HANDLE) {
    status = switch_scheduler_get(device, scheduler_handle, &scheduler_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "scheduler_group delete failed on device %d: scheduler get failed "
          "for handle 0x%lx:%s",
          device,
          scheduler_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  FOR_EACH_IN_LIST(scheduler_info->scheduler_group_list, node) {
    group_entry = (switch_scheduler_group_entry_t *)node->data;
    if (group_entry->scheduler_group_handle == group_handle) {
      SWITCH_LIST_DELETE(&scheduler_info->scheduler_group_list, node);
      SWITCH_FREE(device, group_entry);
      break;
    }
  }
  FOR_EACH_IN_LIST_END();
  return status;
}

#if 0
switch_status_t switch_api_queue_scheduling_enable(
    switch_device_t device, switch_handle_t scheduler_handle, bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_queue_info_t *queue_info = NULL;

  scheduler_info = switch_scheduler_info_get(scheduler_handle);
  if (!scheduler_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  queue_info = switch_queue_info_get(scheduler_info->queue_handle);
  if (!queue_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_pd_queue_scheduling_enable(
      device, queue_info->port_handle, queue_info->queue_id, enable);
  return status;
}

switch_status_t switch_api_queue_scheduling_strict_priority_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    uint32_t priority) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_queue_info_t *queue_info = NULL;

  scheduler_info = switch_scheduler_info_get(scheduler_handle);
  if (!scheduler_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  queue_info = switch_queue_info_get(scheduler_info->queue_handle);
  if (!queue_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  scheduler_info->priority = priority;

  status = switch_pd_queue_scheduling_strict_priority_set(
      device, queue_info->port_handle, queue_info->queue_id, priority);
  return status;
}

switch_status_t switch_api_queue_scheduling_remaining_bw_priority_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    uint32_t priority) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_queue_info_t *queue_info = NULL;

  scheduler_info = switch_scheduler_info_get(scheduler_handle);
  if (!scheduler_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  queue_info = switch_queue_info_get(scheduler_info->queue_handle);
  if (!queue_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  scheduler_info->rem_bw_priority = priority;

  status = switch_pd_queue_scheduling_remaining_bw_priority_set(
      device, queue_info->port_handle, queue_info->queue_id, priority);
  return status;
}

switch_status_t switch_api_queue_scheduling_guaranteed_shaping_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    bool pps,
    uint32_t burst_size,
    uint32_t rate) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_info_t *scheduler_info = NULL;
  switch_queue_info_t *queue_info = NULL;

  scheduler_info = switch_scheduler_info_get(scheduler_handle);
  if (!scheduler_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  queue_info = switch_queue_info_get(scheduler_info->queue_handle);
  if (!queue_info) {
    SWITCH_LOG_ERROR("queue scheduling enable failed");
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  scheduler_info->min_rate = rate;
  scheduler_info->min_burst_size = burst_size;

  status =
      switch_pd_queue_scheduling_guaranteed_shaping_set(device,
                                                        queue_info->port_handle,
                                                        queue_info->queue_id,
                                                        pps,
                                                        burst_size,
                                                        rate);
  return status;
}
#endif

switch_status_t switch_api_scheduler_group_child_handle_get_internal(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *child_handles) {
  switch_scheduler_group_info_t *group_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;
  switch_uint32_t i = 0;

  SWITCH_ASSERT(SWITCH_SCHEDULER_GROUP_HANDLE(scheduler_group_handle));
  status =
      switch_scheduler_group_get(device, scheduler_group_handle, &group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get child handles from scheduler group on device %d: group "
        "get failed for 0x%lx: %s",
        device,
        scheduler_group_handle,
        switch_error_to_string(status));
    return status;
  }
  if (group_info->api_info.group_type == SWITCH_SCHEDULER_GROUP_TYPE_QUEUE) {
    child_handles[0] = group_info->api_info.queue_handle;
  } else {
    status =
        switch_port_get(device, group_info->api_info.port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to child handles on device %d: port info get failed for port "
          "0x%lx: %s",
          device,
          group_info->api_info.port_handle,
          switch_error_to_string(status));
      return status;
    }
    for (i = 0; i < SWITCH_MAX_QUEUE; i++) {
      child_handles[i] = port_info->queue_scheduler_group_handles[i];
    }
  }
  return status;
}
switch_status_t switch_api_scheduler_group_child_count_get_internal(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_uint32_t *child_count) {
  switch_scheduler_group_info_t *group_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_SCHEDULER_GROUP_HANDLE(scheduler_group_handle));
  status =
      switch_scheduler_group_get(device, scheduler_group_handle, &group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get child count from scheduler group on device %d: group "
        "get failed for 0x%lx: %s",
        device,
        scheduler_group_handle,
        switch_error_to_string(status));
    return status;
  }
  if (group_info->api_info.group_type == SWITCH_SCHEDULER_GROUP_TYPE_QUEUE) {
    *child_count = SWITCH_SCHEDULER_GROUP_CHILD_COUNT;
  } else {
    *child_count = SWITCH_MAX_QUEUE;
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_scheduler_group_profile_get_internal(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *profile_handle) {
  switch_scheduler_group_info_t *group_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_SCHEDULER_GROUP_HANDLE(scheduler_group_handle));
  status =
      switch_scheduler_group_get(device, scheduler_group_handle, &group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to get queue_handle from scheduler group on device %d: group "
        "get failed for 0x%lx: %s",
        device,
        scheduler_group_handle,
        switch_error_to_string(status));
    return status;
  }
  *profile_handle = group_info->api_info.scheduler_handle;
  return status;
}

switch_status_t switch_api_scheduler_group_profile_set_internal(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t profile_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_group_info_t *group_info;

  SWITCH_ASSERT(SWITCH_SCHEDULER_GROUP_HANDLE(scheduler_group_handle));
  status =
      switch_scheduler_group_get(device, scheduler_group_handle, &group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to set scheduler group profile on device %d: group get failed "
        "for 0x%lx: %s",
        device,
        scheduler_group_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_api_scheduer_group_profile_update(
      device,
      scheduler_group_handle,
      group_info->api_info.port_handle,
      group_info->api_info.queue_handle,
      profile_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to update scheduler profile to the group on device %d: profile "
        "update for handle 0x%lx",
        device,
        scheduler_group_handle);
    return status;
  }
  group_info->api_info.scheduler_handle = profile_handle;

  return status;
}
#ifdef __cplusplus
}
#endif

switch_status_t switch_api_scheduler_create(
    switch_device_t device,
    const switch_scheduler_api_info_t *api_info,
    switch_handle_t *scheduler_handle) {
  SWITCH_MT_WRAP(
      switch_api_scheduler_create_internal(device, api_info, scheduler_handle))
}

switch_status_t switch_api_scheduler_delete(switch_device_t device,
                                            switch_handle_t scheduler_handle) {
  SWITCH_MT_WRAP(switch_api_scheduler_delete_internal(device, scheduler_handle))
}

switch_status_t switch_api_port_shaping_set(switch_device_t device,
                                            switch_handle_t port_handle,
                                            bool pps,
                                            uint32_t burst_size,
                                            switch_uint64_t rate) {
  SWITCH_MT_WRAP(switch_api_port_shaping_set_internal(
      device, port_handle, pps, burst_size, rate))
}

switch_status_t switch_api_scheduler_group_create(
    switch_device_t device,
    switch_scheduler_group_api_info_t *api_info,
    switch_handle_t *scheduler_group_handle) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_create_internal(
      device, api_info, scheduler_group_handle))
}

switch_status_t switch_api_scheduler_group_delete(
    switch_device_t device, switch_handle_t scheduler_group_handle) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_delete_internal(
      device, scheduler_group_handle))
}

switch_status_t switch_api_scheduler_group_child_handle_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *child_handles) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_child_handle_get_internal(
      device, scheduler_group_handle, child_handles))
}

switch_status_t switch_api_scheduler_group_child_count_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_uint32_t *child_count) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_child_count_get_internal(
      device, scheduler_group_handle, child_count))
}

switch_status_t switch_api_scheduler_group_profile_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *profile_handle) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_profile_get_internal(
      device, scheduler_group_handle, profile_handle))
}

switch_status_t switch_api_scheduler_group_profile_set(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t profile_handle) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_profile_set_internal(
      device, scheduler_group_handle, profile_handle))
}

switch_status_t switch_api_scheduler_config_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    const switch_scheduler_api_info_t *api_info) {
  SWITCH_MT_WRAP(switch_api_scheduler_config_set_internal(
      device, scheduler_handle, api_info))
}

switch_status_t switch_api_scheduler_config_get(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    switch_scheduler_api_info_t *api_info) {
  SWITCH_MT_WRAP(switch_api_scheduler_config_get_internal(
      device, scheduler_handle, api_info))
}

switch_status_t switch_api_scheduler_group_config_get(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    switch_scheduler_group_api_info_t *api_info) {
  SWITCH_MT_WRAP(switch_api_scheduler_group_config_get_internal(
      device, scheduler_handle, api_info))
}
