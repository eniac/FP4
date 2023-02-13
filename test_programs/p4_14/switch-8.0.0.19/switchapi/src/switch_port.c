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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"
#include "switchapi/switch_port.h"
#include "switchapi/switch_scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_PORT

switch_handle_t cpu_port_handle = 0;

/*
 * Routine Description:
 *   @brief add default entries for port
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_storm_control_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port default entry add failed on device %d: "
        "storm control default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_port_mapping_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port default entry add failed on device %d: "
        "egress port mapping default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ingress_port_mirror_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port default entry add failed on device %d: "
        "ingress_port_mirror default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_port_mirror_table_entry_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port default entry add failed on device %d: "
        "egress_port_mirror default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("port default entries added successfully on device %d\n",
                    device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete default entries for port
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  SWITCH_LOG_DETAIL("port default entries deleted successfully on device %d\n",
                    device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief initialize port structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_init(switch_device_t device) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_uint16_t num_ports = SWITCH_MAX_PORTS;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  port_ctx = SWITCH_MALLOC(device, sizeof(switch_port_context_t), 0x1);
  if (!port_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "port init failed for device %d: "
        "port device context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(port_ctx, 0x0, sizeof(switch_port_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_PORT, (void *)port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port init failed for device %d: "
        "port device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(num_ports != 0);

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_PORT, num_ports);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port init failed for device %d: "
        "port handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_PRIORITY_GROUP, SWITCH_PPG_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port init failed for device %d: "
        "ppg handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_ports; index++) {
    port_ctx->port_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  status = switch_api_id_allocator_new(
      device, SWITCH_YID_MAX, TRUE, &port_ctx->yid_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port init failed for device: %d "
        "port yid allocator allcoation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_port_state_change_notification_register(device, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port init failed for device: %d "
        "port state change notification registration failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG("port init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

/*
 * Routine Description:
 *   @brief free port structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_free(switch_device_t device) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_uint16_t num_ports = SWITCH_MAX_PORTS;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port free failed for device %d: "
        "port device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_destroy(device, port_ctx->yid_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port free failed for device %d: "
        "port yid allocator destroy failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_PORT);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port free failed for device: %d "
        "port handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_PRIORITY_GROUP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port free failed for device: %d "
        "ppg handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_ports; index++) {
    port_ctx->port_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  status = switch_device_api_context_set(device, SWITCH_API_TYPE_PORT, NULL);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port free failed for device: %d "
        "port device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, port_ctx);

  SWITCH_LOG_DEBUG("port free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_yid_allocate(switch_device_t device, switch_yid_t *yid) {
  switch_port_context_t *port_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "yid allocation failed on device %d: "
        "port context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_allocate(device, port_ctx->yid_allocator, yid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "yid allocation failed on device %d: "
        "yid allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("yid %d allocated on device %d", yid, device);

  return status;
}

switch_status_t switch_yid_free(switch_device_t device, switch_yid_t yid) {
  switch_port_context_t *port_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "yid release failed on device %d: "
        "port context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_release(device, port_ctx->yid_allocator, yid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "yid release failed on device %d: "
        "yid release failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("yid %d freed on device %d", yid, device);

  return status;
}

switch_status_t switch_api_port_event_notification_register_internal(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_event_notification_fn cb_fn) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port event app register failed on device %d: "
        "port context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_EVENT_REGISTRATION_MAX; index++) {
    if (port_ctx->event_app_list[index].valid) {
      if (port_ctx->event_app_list[index].app_id == app_id) {
        port_ctx->event_app_list[index].cb_fn = cb_fn;
        return status;
      }
    }
  }

  for (index = 0; index < SWITCH_PORT_EVENT_REGISTRATION_MAX; index++) {
    if (!port_ctx->event_app_list[index].valid) {
      port_ctx->event_app_list[index].cb_fn = cb_fn;
      port_ctx->event_app_list[index].valid = TRUE;
      port_ctx->event_app_list[index].app_id = app_id;
      return status;
    }
  }

  return SWITCH_STATUS_INSUFFICIENT_RESOURCES;
}

switch_status_t switch_api_port_event_notification_deregister_internal(
    switch_device_t device, switch_app_id_t app_id) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port event app deregister failed on device %d: "
        "port context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_EVENT_REGISTRATION_MAX; index++) {
    if (port_ctx->event_app_list[index].app_id == app_id) {
      port_ctx->event_app_list[index].cb_fn = NULL;
      port_ctx->event_app_list[index].valid = FALSE;
      port_ctx->event_app_list[index].app_id = 0;
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_port_state_change_notification_register_internal(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_state_change_notification_fn cb_fn) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port event app register failed on device %d: "
        "port context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX; index++) {
    if (port_ctx->sc_app_list[index].valid) {
      if (port_ctx->sc_app_list[index].app_id == app_id) {
        port_ctx->sc_app_list[index].cb_fn = cb_fn;
        return status;
      }
    }
  }

  for (index = 0; index < SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX; index++) {
    if (!port_ctx->sc_app_list[index].valid) {
      port_ctx->sc_app_list[index].cb_fn = cb_fn;
      port_ctx->sc_app_list[index].valid = TRUE;
      port_ctx->sc_app_list[index].app_id = app_id;
      return status;
    }
  }

  return SWITCH_STATUS_INSUFFICIENT_RESOURCES;
}

switch_status_t switch_api_port_state_change_notification_deregister_internal(
    switch_device_t device, switch_app_id_t app_id) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port event app deregister failed on device %d: "
        "port context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX; index++) {
    if (port_ctx->sc_app_list[index].app_id == app_id) {
      port_ctx->sc_app_list[index].cb_fn = NULL;
      port_ctx->sc_app_list[index].valid = FALSE;
      port_ctx->sc_app_list[index].app_id = 0;
      return status;
    }
  }

  return status;
}

/*
 * Routine Description:
 *   @brief port app event notification
 *   Registered applications will be notified of
 *   the port events - port add/port delete
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] handle - port handle
 *   @param[in] port_event - port event(add/delete)
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_event_app_notify(switch_device_t device,
                                             switch_handle_t handle,
                                             switch_port_event_t port_event) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(handle));
  if (!SWITCH_PORT_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port event app notify failed on device %d: "
        "port handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_event < SWITCH_PORT_EVENT_MAX);
  if (port_event >= SWITCH_PORT_EVENT_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port event app notify failed on device %d: "
        "port event invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port event app notify failed on device %d: "
        "port device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_EVENT_REGISTRATION_MAX; index++) {
    if (port_ctx->event_app_list[index].valid) {
      port_ctx->event_app_list[index].cb_fn(
          device, handle, port_event, port_ctx->event_app_list[index].app_data);

      SWITCH_LOG_DEBUG(
          "port event app notification on device: %d app_id 0x%lx"
          "port handle  0x%lx port event %s\n",
          device,
          port_ctx->event_app_list[index].app_id,
          handle,
          switch_port_event_to_string(port_event));
    }
  }

  SWITCH_LOG_DEBUG(
      "port event app notify on device: %d "
      "port handle 0x%lx port event %s\n",
      device,
      handle,
      switch_port_event_to_string(port_event));

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief port state change event notification
 *   Registered applications will be notified of
 *   the port state change - port up/port down
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] handle - port handle
 *   @param[in] port_event - port state change(up/down)
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_port_state_change_app_notify(
    switch_device_t device,
    switch_handle_t handle,
    switch_port_oper_status_t oper_status) {
  switch_port_context_t *port_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(handle));
  if (!SWITCH_PORT_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port state change app notify failed on device %d "
        "port handle 0x%lx: port handle invalid(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(oper_status < SWITCH_PORT_OPER_STATUS_MAX);
  if (oper_status >= SWITCH_PORT_OPER_STATUS_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port state change app notify failed on device %d "
        "port handle 0x%lx: port state invalid(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port state change app notify failed on device %d "
        "port handle 0x%lx: port device context get failed(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX; index++) {
    if (port_ctx->sc_app_list[index].valid) {
      port_ctx->sc_app_list[index].cb_fn(
          device, handle, oper_status, port_ctx->sc_app_list[index].app_data);

      SWITCH_LOG_DEBUG(
          "port event app notification on device: %d app_id 0x%lx"
          "port handle 0x%lx port state %s\n",
          device,
          port_ctx->sc_app_list[index].app_id,
          handle,
          switch_port_oper_status_to_string(oper_status));
    }
  }

  SWITCH_LOG_DETAIL(
      "port state change app notify on device: %d "
      "port handle 0x%lx port state %s\n",
      device,
      handle,
      switch_port_oper_status_to_string(oper_status));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_port_state_change(switch_device_t device,
                                         switch_handle_t port_handle,
                                         switch_port_oper_status_t oper_status,
                                         void *cookie) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MT_LOCK(device);

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port state change failed on device %d "
        "port handle 0x%lx invalid port handle:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    SWITCH_MT_UNLOCK(device);
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port state change failed on device %d "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    SWITCH_MT_UNLOCK(device);
    return status;
  }

  port_info->oper_status = oper_status;

/*
 * Walk through the list of all nexthops and deactivate the corresponding
 * ecmp members on port down event.
 */

#ifdef ECMP_MEMBER_FAST_DEACTIVATION
  PWord_t *PValue = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;

  if (oper_status == SWITCH_PORT_OPER_STATUS_DOWN) {
    JLF(PValue, SWITCH_PORT_NHOP_REF_LIST(port_info), nhop_handle);
    while (PValue != NULL) {
      SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
      status = switch_nhop_get(device, nhop_handle, &nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        status = SWITCH_STATUS_INVALID_HANDLE;
        SWITCH_LOG_ERROR(
            "neighbor handle get failed for"
            "device %d handle 0x%lx: %s",
            device,
            nhop_handle,
            switch_error_to_string(status));
        SWITCH_MT_UNLOCK(device);
        return status;
      }

      status = switch_nhop_ecmp_members_deactivate(device, nhop_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("ecmp member deactivate failed on device %d: %s",
                         device,
                         switch_error_to_string(status));
        SWITCH_MT_UNLOCK(device);
        return status;
      }

      JLN(PValue, SWITCH_PORT_NHOP_REF_LIST(port_info), nhop_handle);
    }
  }
#endif /* ECMP_MEMBER_FAST_DEACTIVATION */

  SWITCH_MT_UNLOCK(device);

  status =
      switch_port_state_change_app_notify(device, port_handle, oper_status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port state change failed on device %d "
        "port handle 0x%lx: port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

void switch_api_port_speed_max_ppg_get(switch_port_speed_t port_speed,
                                       switch_uint8_t *max_ppg) {
  switch (port_speed) {
    case SWITCH_PORT_SPEED_10G:
      *max_ppg = SWITCH_MAX_PPG_10G;
      break;
    case SWITCH_PORT_SPEED_25G:
    case SWITCH_PORT_SPEED_40G:
    case SWITCH_PORT_SPEED_50G:
    case SWITCH_PORT_SPEED_100G:
      *max_ppg = SWITCH_MAX_PPG_100G;
      break;
    default:
      *max_ppg = SWITCH_MAX_PPG_10G;
      break;
  }
}

static switch_status_t switch_api_port_queue_scheduler_create(
    switch_device_t device, switch_handle_t port_handle) {
  switch_scheduler_group_api_info_t scheduler_group_api_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t scheduler_group_id;
  switch_port_info_t *port_info = NULL;
  switch_uint32_t queue_index;

  SWITCH_LOG_DEBUG("Creating port/queue scheduler groups for port 0x%lx",
                   port_handle);
  status = switch_port_get(device, port_handle, &port_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (switch_device_recirc_port(device, port_info->port)) {
    SWITCH_LOG_DEBUG("Recirc port, skip creating scheduler groups");
    return SWITCH_STATUS_SUCCESS;
  }
  memset(
      &scheduler_group_api_info, 0, sizeof(switch_scheduler_group_api_info_t));
  scheduler_group_api_info.port_handle = port_handle;
  scheduler_group_api_info.group_type = SWITCH_SCHEDULER_GROUP_TYPE_PORT;
  status = switch_api_scheduler_group_create(
      device, &scheduler_group_api_info, &scheduler_group_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to create port level scheduler group for port 0x%lx",
        port_handle);
    return status;
  }

  for (queue_index = 0; queue_index < port_info->max_queues; queue_index++) {
    if (port_info->queue_handles[queue_index] == SWITCH_API_INVALID_HANDLE) {
      break;
    }
    scheduler_group_api_info.queue_handle =
        port_info->queue_handles[queue_index];
    scheduler_group_api_info.group_type = SWITCH_SCHEDULER_GROUP_TYPE_QUEUE;
    status = switch_api_scheduler_group_create(
        device, &scheduler_group_api_info, &scheduler_group_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to create queue level scheduler group for port 0x%lx, qid "
          "%d, queue handle 0x%lx",
          (switch_handle_t)port_handle,
          queue_index,
          port_info->queue_handles[queue_index]);
      return status;
    }
    SWITCH_LOG_DEBUG("Queue scheduler group handle 0x%lx for port 0x%lx",
                     scheduler_group_id,
                     (switch_handle_t)port_handle);
  }
  return status;
}

static switch_status_t switch_api_port_queue_scheduler_delete(
    switch_device_t device, switch_handle_t port_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;
  switch_uint32_t queue_index = 0;

  status = switch_port_get(device, port_handle, &port_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (switch_device_recirc_port(device, port_info->port)) {
    SWITCH_LOG_DEBUG("Recirc port, skip deleting scheduler groups");
    return SWITCH_STATUS_SUCCESS;
  }
  status = switch_api_scheduler_group_delete(
      device, port_info->port_scheduler_group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Failed to delete port scheduler group for port 0x%lx",
                     port_handle);
    return status;
  }
  for (queue_index = 0; queue_index < port_info->max_queues; queue_index++) {
    status = switch_api_scheduler_group_delete(
        device, port_info->queue_scheduler_group_handles[queue_index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to delete queue scheduler groups for port 0x%lx, index %d",
          port_handle,
          queue_index);
      return status;
    }
  }
  return status;
}

/*
 * Routine Description:
 *   @brief create a port on a device
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] port - port number
 *   @param[out] port_handle - port handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_port_add_internal(
    switch_device_t device,
    switch_api_port_info_t *api_port_info,
    switch_handle_t *port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_port_t cpu_port = SWITCH_CPU_PORT_DEFAULT;
  switch_port_t port = SWITCH_PORT_INVALID;
  switch_port_speed_t port_speed = SWITCH_PORT_SPEED_NONE;
  switch_dev_port_t dev_port = 0;
  switch_dev_port_t cpu_eth_dev_port = 0;
  switch_uint16_t index = 0;
  switch_uint64_t flags = 0;
  switch_handle_t queue_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_queue_info_t api_queue_info;
  switch_uint32_t rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
  switch_uint32_t tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
  switch_port_fec_mode_t fec_mode = SWITCH_PORT_FEC_MODE_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_port_info != NULL);
  SWITCH_ASSERT(SWITCH_PORT_VALID(api_port_info->port));
  if (!SWITCH_PORT_VALID(api_port_info->port)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d: "
        "lane number max exceeded(%s)\n",
        device,
        api_port_info->port,
        switch_error_to_string(status));
    return status;
  }

  port = api_port_info->port;
  port_speed = api_port_info->port_speed;

  if (port_speed == SWITCH_PORT_SPEED_NONE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d: "
        "port speed %s invalid(%s)\n",
        device,
        port,
        switch_port_speed_to_string(port_speed),
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d: "
        "port device context get failed(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_PORT_HANDLE(port_ctx->port_handles[port])) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    *port_handle = port_ctx->port_handles[port];
    SWITCH_LOG_DETAIL(
        "port add failed on device %d port %d port handle 0x%lx: "
        "port already created(%s)\n",
        device,
        port,
        *port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_dev_port_get(device, port, &dev_port);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  if (api_port_info->rx_mtu) {
    rx_mtu = api_port_info->rx_mtu;
  }

  if (api_port_info->tx_mtu) {
    tx_mtu = api_port_info->tx_mtu;
  }

  if (!SWITCH_PORT_INTERNAL(port) && !switch_device_recirc_port(device, port)) {
    status = switch_pd_port_add(device, dev_port, port_speed);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port add failed on device %d port %d dev port %d: "
          "port pd add failed(%s)\n",
          device,
          port,
          dev_port,
          switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_port_mtu_set(device, dev_port, tx_mtu, rx_mtu);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port add failed on device %d port %d dev port %d: "
          "port pd mtu set failed(%s)\n",
          device,
          port,
          dev_port,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (api_port_info->initial_admin_state) {
      status = switch_pd_port_enable(device, dev_port);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port add failed on device %d port %d dev port %d: "
            "port pd enable failed(%s)\n",
            device,
            port,
            dev_port,
            switch_error_to_string(status));
        goto cleanup;
      }
    }

    fec_mode = api_port_info->fec_mode;
    status = switch_device_cpu_eth_dev_port_get(device, &cpu_eth_dev_port);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (cpu_eth_dev_port == dev_port) {
      fec_mode = SWITCH_PORT_FEC_MODE_NONE;
    }

    if (fec_mode != SWITCH_PORT_FEC_MODE_NONE) {
      status = switch_pd_port_fec_set(device, dev_port, fec_mode);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port add failed on device %d port %d dev port %d: "
            "port fec set failed(%s) \n",
            device,
            port,
            dev_port,
            switch_error_to_string(status));
        return status;
      }
    }
    switch_device_active_ports_increment(device);
  }

  *port_handle = switch_port_handle_create(device);
  if (port_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "port add failed for device %d port %d dev port %d: "
        "port handle create failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_port_get(device, *port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port get failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_PORT_LANE_MAPPING(port, port_speed, port_info->lane_list, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  port_info->port_speed = port_speed;
  port_info->ingress_port_lag_label = 0;
  port_info->egress_port_lag_label = 0;
  port_info->lag_handle = SWITCH_API_INVALID_HANDLE;
  port_info->port = port;
  port_info->yid = SWITCH_YID_INVALID;
  port_info->dev_port = dev_port;
  port_info->admin_state = api_port_info->initial_admin_state;
  port_info->an_mode = SWITCH_PORT_AUTO_NEG_MODE_DEFAULT;
  port_info->tx_mtu = tx_mtu;
  port_info->rx_mtu = rx_mtu;
  port_info->fec_mode = api_port_info->fec_mode;
  port_info->learning_enabled = TRUE;

  port_ctx->port_handles[port] = *port_handle;

  SWITCH_PORT_NHOP_REF_LIST(port_info) = (Pvoid_t)NULL;

  status = switch_api_id_allocator_allocate(
      device, port_ctx->yid_allocator, &port_info->yid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port yid allocation failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_device_cpu_port_get(device, &cpu_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d: "
        "cpu port get failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (!switch_device_recirc_port(device, port)) {
    switch_api_port_speed_max_ppg_get(port_info->port_speed,
                                      &port_info->max_ppg);
    status = switch_api_ppg_create(device, *port_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port ppg create failed on device %d for port 0x%lx\n",
                       device,
                       *port_handle);
    }
  }

  port_info->port_type = SWITCH_PORT_TYPE_NORMAL;
  port_info->num_queues = 0;
  port_info->max_queues =
      (port == cpu_port ? SWITCH_MAX_CPU_QUEUE : SWITCH_MAX_PORT_QUEUE);
  SWITCH_ASSERT(port_info->max_queues != 0);

  port_info->queue_handles =
      SWITCH_MALLOC(device, sizeof(switch_handle_t), port_info->max_queues);
  if (!port_info->queue_handles) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port queue handle memory allocation failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(port_info->queue_handles,
                0x0,
                sizeof(switch_handle_t) * port_info->max_queues);

  SWITCH_MEMSET(&api_queue_info, 0x0, sizeof(api_queue_info));
  api_queue_info.port_handle = *port_handle;
  flags |= SWITCH_QUEUE_ATTR_PORT_HANDLE;
  flags |= SWITCH_QUEUE_ATTR_QUEUE_ID;

  for (index = 0; index < port_info->max_queues; index++) {
    api_queue_info.queue_id = index;
    status =
        switch_api_queue_create(device, flags, &api_queue_info, &queue_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port add failed on device %d port %d dev port %d: "
          "queue create failed:(%s)\n",
          device,
          port,
          dev_port,
          switch_error_to_string(status));
      goto cleanup;
    }
    port_info->queue_handles[index] = queue_handle;
  }

  port_info->lag_handle = SWITCH_API_INVALID_HANDLE;
  port_info->ingress_acl_group_handle = SWITCH_API_INVALID_HANDLE;
  port_info->egress_acl_group_handle = SWITCH_API_INVALID_HANDLE;

  port_info->port_lag_index = SWITCH_COMPUTE_PORT_LAG_INDEX(
      *port_handle, SWITCH_PORT_LAG_INDEX_TYPE_PORT);

  port_info->bind_mode = SWITCH_PORT_BIND_MODE_PORT;

  status = SWITCH_ARRAY_INIT(&port_info->intf_array);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev_port %d: "
        "port lag group table add failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_lag_group_table_entry_add(device,
                                               port_info->dev_port,
                                               port_info->port_lag_index,
                                               &(port_info->mbr_hdl),
                                               &(port_info->lg_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d: "
        "port lag group table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_LAG_GROUP_ENTRY);
  SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_LAG_MEMBER_ENTRY);

  status = switch_pd_ingress_port_mapping_table_entry_add(
      device,
      port_info->dev_port,
      port_info->port_lag_index,
      port_info->port_type,
      &port_info->ingress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port ingress port mapping table add failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_INGRESS_PORT_MAPPING_ENTRY);

  status = switch_pd_ingress_port_properties_table_entry_add(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      &port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port ingress port properties table add failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_INGRESS_PORT_PROPERTIES_ENTRY);

  status = switch_pd_egress_port_mapping_table_entry_add(
      device,
      port_info->dev_port,
      port_info->egress_port_lag_label,
      port_info->port_type,
      port_info->egress_qos_group,
      &port_info->egress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port egress port mapping table add failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_EGRESS_PORT_MAPPING_ENTRY);

  status = switch_port_prune_mask_table_update(device, port_info, FALSE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port add failed on device %d port %d dev port %d: "
        "port prune mask table add failed(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_port_queue_scheduler_create(device, *port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to create port/queue scheduler groups for port 0x%lx",
        *port_handle);
    return status;
  }
  SWITCH_LOG_DEBUG(
      "port created successfully on device %d: "
      "port number %d  dev port %d port handle 0x%lx "
      "port speed %s tx mtu %d rx mtu %d port lag index %d\n",
      device,
      port,
      *port_handle,
      dev_port,
      switch_port_speed_to_string(port_speed),
      tx_mtu,
      rx_mtu,
      port_info->port_lag_index);

  SWITCH_LOG_EXIT();

  return status;
cleanup:
  return status;
}

/*
 * Routine Description:
 *   @brief delete a port on a device
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] port_handle - port handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_port_delete_internal(switch_device_t device,
                                                switch_handle_t port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_uint16_t index = 0;
  switch_port_t port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port delete failed on device %d port_handle 0x%lx: "
        "port handle invalid(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port delete failed on device: %d port_handle 0x%lx: "
        "port device context get failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port delete failed on device %d port_handle 0x%lx: "
        "port get failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_ARRAY_COUNT(&port_info->intf_array)) {
    status = SWITCH_STATUS_PORT_IN_USE;
    SWITCH_LOG_ERROR(
        "port delete failed on device %d port %d "
        "dev port %d port_handle 0x%lx: "
        "interface still referenced(%s)\n",
        device,
        port_info->port,
        port_info->dev_port,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  port = port_info->port;

  status = switch_api_port_queue_scheduler_delete(device, port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to delete port/queue scheduler groups for device %d, port "
        "0x%lx",
        device,
        port_handle);
    return status;
  }
  for (index = 0; index < port_info->max_queues; index++) {
    status = switch_api_queue_delete(device, port_info->queue_handles[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port_handle 0x%lx: "
          "queue delete failed(%s).\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    port_info->queue_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  if (port_info->max_queues) {
    SWITCH_FREE(device, port_info->queue_handles);
    port_info->queue_handles = NULL;
  }

  if (SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_INGRESS_PORT_MAPPING_ENTRY)) {
    status = switch_pd_ingress_port_mapping_table_entry_delete(
        device, port_info->ingress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port_handle 0x%lx: "
          "ingress port mapping table delete failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(port_info, SWITCH_PORT_INGRESS_PORT_MAPPING_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(port_info,
                           SWITCH_PORT_INGRESS_PORT_PROPERTIES_ENTRY)) {
    status = switch_pd_ingress_port_properties_table_entry_delete(
        device, port_info->ingress_prop_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device: %d port_handle 0x%lx: "
          "ingress port prroperties table delete failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(port_info, SWITCH_PORT_INGRESS_PORT_PROPERTIES_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_EGRESS_PORT_MAPPING_ENTRY)) {
    status = switch_pd_egress_port_mapping_table_entry_delete(
        device, port_info->egress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port_handle 0x%lx: "
          "egress port mapping table delete failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(port_info, SWITCH_PORT_EGRESS_PORT_MAPPING_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_LAG_GROUP_ENTRY)) {
    status = switch_pd_lag_group_table_entry_delete(
        device, TRUE, port_info->lg_entry, port_info->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port_handle 0x%lx: "
          "lag group table delete failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_CLEAR(port_info, SWITCH_PORT_LAG_GROUP_ENTRY);
  }

  if (port_info->yid != SWITCH_YID_INVALID) {
    status = switch_api_id_allocator_release(
        device, port_ctx->yid_allocator, port_info->yid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port_handle 0x%lx: "
          "port yid deallocation failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    port_info->yid = SWITCH_YID_INVALID;
  }
  status = switch_api_ppg_delete(device, port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port ppg delete failed on device %d for port 0x%lx\n",
                     device,
                     port_handle);
  }

  port_info->ingress_port_lag_label = 0;
  port_info->egress_port_lag_label = 0;
  if (!SWITCH_PORT_INTERNAL(port_info->port)) {
    status = switch_pd_port_disable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port %d "
          "dev port %d port handle 0x%lx: "
          "port pd disable failed(%s)\n",
          device,
          port_info->port,
          port_info->dev_port,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_port_delete(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port delete failed on device %d port %d: "
          "dev port %d port handle 0x%lx: "
          "port pd delete failed(%s)\n",
          device,
          port_info->port,
          port_info->dev_port,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (!SWITCH_PORT_INTERNAL(port_info->port) &&
      !switch_device_recirc_port(device, port_info->port)) {
    switch_device_active_ports_decrement(device);
  }

  status = switch_port_handle_delete(device, port_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  port_ctx->port_handles[port] = SWITCH_API_INVALID_HANDLE;

  SWITCH_LOG_DEBUG(
      "port deleted successfully on device %d "
      "port_handle 0x%lx\n",
      device,
      port_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_port_prune_mask_table_update(
    switch_device_t device, switch_port_info_t *port_info, bool prune) {
  switch_mc_port_map_t port_map;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_MEMSET(port_map, 0x0, sizeof(switch_mc_port_map_t));

  if (!prune) {
    SWITCH_MC_PORT_MAP_SET(port_map, port_info->dev_port);
  }

  status = switch_pd_prune_mask_table_update(device, port_info->yid, port_map);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port prune table update failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

/*
 * Routine Description:
 *   @brief enable storm control on port
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] port_handle - port handle
 *   @param[in] pkt_type - packet type (ucast/mcast/bcast)
 *   @param[in] meter_handle - storm control meter handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_port_storm_control_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_packet_type_t pkt_type,
    switch_handle_t meter_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  bool hw_set = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "port storm control set failed on device: %d "
        "port_handle 0x%lx meter_handle 0x%lx: "
        "port handle invalid %s\n",
        device,
        port_handle,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port storm control set failed on device %d "
        "port_handle 0x%lx meter_handle 0x%lx: "
        "port device context get failed(%s)\n",
        device,
        port_handle,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port storm control set failed on device: %d "
        "port_handle: 0x%lx meter_handle 0x%lx "
        "port get failed %s\n",
        device,
        port_handle,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  if (meter_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
    if (!SWITCH_METER_HANDLE(meter_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "port storm control set failed on device: %d "
          "port_handle 0x%lx meter_handle 0x%lx: "
          "meter handle invalid(%s)\n",
          device,
          port_handle,
          meter_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_ASSERT(pkt_type < SWITCH_PACKET_TYPE_MAX);
  if (pkt_type >= SWITCH_PACKET_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port storm control set failed on device: %d "
        "port_handle: 0x%lx meter_handle 0x%lx "
        "packet type invalid(%s)\n",
        device,
        port_handle,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  if (pkt_type == SWITCH_PACKET_TYPE_UNICAST) {
    hw_set = SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_SC_UCAST_ENTRY);
  } else if (pkt_type == SWITCH_PACKET_TYPE_MULTICAST) {
    hw_set = SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_SC_MCAST_ENTRY);
  } else {
    hw_set = SWITCH_HW_FLAG_ISSET(port_info, SWITCH_PORT_SC_BCAST_ENTRY);
  }

  if (meter_handle != SWITCH_API_INVALID_HANDLE) {
    if (hw_set) {
      status = switch_pd_storm_control_table_entry_update(
          device,
          port_info->dev_port,
          SWITCH_PRIORITY_DEFAULT,
          pkt_type,
          handle_to_id(meter_handle),
          port_info->meter_pd_hdl[pkt_type]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port storm control set failed on device: %d "
            "port_handle: 0x%lx meter_handle 0x%lx "
            "port storm control table add failed(%s)\n",
            device,
            port_handle,
            meter_handle,
            switch_error_to_string(status));
        return status;
      }
    } else {
      status = switch_pd_storm_control_table_entry_add(
          device,
          port_info->dev_port,
          SWITCH_PRIORITY_DEFAULT,
          pkt_type,
          handle_to_id(meter_handle),
          &port_info->meter_pd_hdl[pkt_type]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port storm control set failed on device: %d "
            "port_handle: 0x%lx meter_handle 0x%lx "
            "port storm control table add failed(%s)",
            device,
            port_handle,
            switch_error_to_string(status));
        return status;
      }
      if (pkt_type == SWITCH_PACKET_TYPE_UNICAST) {
        SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_SC_UCAST_ENTRY);
      } else if (pkt_type == SWITCH_PACKET_TYPE_MULTICAST) {
        SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_SC_MCAST_ENTRY);
      } else {
        SWITCH_HW_FLAG_SET(port_info, SWITCH_PORT_SC_BCAST_ENTRY);
      }
    }
  } else {
    if (hw_set) {
      status = switch_pd_storm_control_table_entry_delete(
          device, port_info->meter_pd_hdl[pkt_type]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port storm control set failed on device: %d "
            "port_handle: 0x%lx meter_handle 0x%lx "
            "port storm control table delete failed(%s)\n",
            device,
            port_handle,
            meter_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  port_info->meter_handle[pkt_type] = meter_handle;

  SWITCH_LOG_DEBUG(
      "port storm control set successfully on device %d "
      "port handle 0x%lx packet type %s meter handle 0x%lx\n",
      device,
      port_handle,
      switch_packet_type_to_string(pkt_type),
      meter_handle);

  return status;
}

switch_status_t switch_api_ppg_create_internal(switch_device_t device,
                                               switch_handle_t port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_port_priority_group_t *ppg_info = NULL;
  switch_handle_t ppg_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("ppg create failed for device %d, invalid port handle:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ppg create failed for device %d, port context get failed:%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ppg create failed for device %d, port get failed for 0x%lx:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_info->max_ppg != 0);

  port_info->ppg_handles =
      SWITCH_MALLOC(device, sizeof(switch_handle_t), port_info->max_ppg);

  if (!port_info->ppg_handles) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("ppg create failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < port_info->max_ppg; index++) {
    ppg_handle = switch_ppg_handle_create(device);

    if (ppg_handle == SWITCH_API_INVALID_HANDLE) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "ppg create failed for device %d, invalid handle for port 0x%lx :%s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_ppg_get(device, ppg_handle, &ppg_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ppg create failed for device %d, ppg get failed for 0x%lx :%s",
          device,
          ppg_handle,
          switch_error_to_string(status));
      return status;
    }

    port_info->ppg_handles[index] = ppg_handle;
    ppg_info->port_handle = port_handle;
    ppg_info->ppg_handle = ppg_handle;
    ppg_info->ppg_index = index;

    /*
     * TM model is not fully initialized and PPG create fails on Model.
     * TODO: Remove this when driver can handle PPG create in model.
     */
    status = switch_pd_ppg_create(
        device, port_info->dev_port, &ppg_info->tm_ppg_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ppg create failed for device %d, pd_ppg failed for port 0x%lx:%s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_ppg_delete_internal(switch_device_t device,
                                               switch_handle_t port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_port_priority_group_t *ppg_info = NULL;
  switch_handle_t ppg_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t buffer_profile_handle = SWITCH_API_INVALID_HANDLE;
  switch_buffer_profile_t *buffer_profile_info = NULL;
  switch_node_t *node = NULL;
  switch_buffer_ppg_entry_t *buffer_ppg_entry = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("ppg delete failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg delete failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg delete failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_info->max_ppg != 0);
  for (index = 0; index < port_info->max_ppg; index++) {
    ppg_handle = port_info->ppg_handles[index];
    status = switch_ppg_get(device, ppg_handle, &ppg_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ppg delete failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_pd_ppg_delete(device, ppg_info->tm_ppg_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("ppg delete failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    buffer_profile_handle = ppg_info->buffer_profile_handle;
    if (buffer_profile_handle != SWITCH_API_INVALID_HANDLE) {
      status = switch_buffer_profile_get(
          device, buffer_profile_handle, &buffer_profile_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "ppg delete failed for device %d: buffer profile get failed for "
            "handle 0x%lx: %s",
            device,
            buffer_profile_handle,
            switch_error_to_string(status));
        return status;
      }

      FOR_EACH_IN_LIST(buffer_profile_info->ppg_handle_list, node) {
        buffer_ppg_entry = (switch_buffer_ppg_entry_t *)node->data;
        if (buffer_ppg_entry->handle == ppg_handle) {
          status =
              SWITCH_LIST_DELETE(&(buffer_profile_info->ppg_handle_list), node);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_FREE(device, buffer_ppg_entry);
        }
      }
      FOR_EACH_IN_LIST_END();
    }
    switch_ppg_handle_delete(device, ppg_handle);
    port_info->ppg_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  return status;
}

switch_status_t switch_api_port_ppg_get_internal(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_uint8_t *num_ppg,
                                                 switch_handle_t *ppg_handles) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("ppg get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_info->max_ppg != 0);
  SWITCH_ASSERT(num_ppg != NULL);
  SWITCH_ASSERT(ppg_handles != NULL);
  if (!num_ppg || !ppg_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < port_info->max_ppg; index++) {
    ppg_handles[index] = port_info->ppg_handles[index];
  }

  *num_ppg = port_info->max_ppg;

  return status;
}

switch_status_t switch_api_port_max_ppg_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint8_t *num_ppgs) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg get failed for device %d: port get failed :%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *num_ppgs = port_info->max_ppg;
  return status;
}

switch_status_t switch_api_port_cos_mapping_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t ppg_handle,
    switch_uint8_t cos_bitmap) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if ((!SWITCH_PORT_HANDLE(port_handle)) || (!SWITCH_PPG_HANDLE(ppg_handle))) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg cos mapping failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg cos mapping failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_ppg_icos_mapping(
      device, ppg_info->tm_ppg_handle, cos_bitmap);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg cos mapping failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_ppg_lossless_enable_internal(
    switch_device_t device, switch_handle_t ppg_handle, bool enable) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (!SWITCH_PPG_HANDLE(ppg_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg lossless enable failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg lossless enable failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  ppg_info->lossless_enabled = enable;

  status =
      switch_pd_ppg_lossless_enable(device, ppg_info->tm_ppg_handle, enable);
  return status;
}

switch_status_t switch_api_ppg_drop_get_internal(switch_device_t device,
                                                 switch_handle_t ppg_handle,
                                                 uint64_t *num_packets) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (!SWITCH_PPG_HANDLE(ppg_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg drop get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_ppg_get(device, ppg_handle, &ppg_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg drop get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ppg_drop_count_get(
      device, ppg_info->tm_ppg_handle, num_packets);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg drop get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}
switch_status_t switch_api_port_qos_group_ingress_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->ingress_qos_group = 0;

  if (qos_map_handle) {
    SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
    if (!SWITCH_QOS_MAP_HANDLE(qos_map_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    port_info->ingress_qos_group = qos_map_list->qos_group;
  }
  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  port_info->ingress_qos_handle = qos_map_handle;

  return status;
}

switch_status_t switch_api_port_pfc_priority_to_ppg_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_qos_map_t qos_map;
  switch_node_t *node = NULL;
  switch_uint8_t pfc_prio_map[SWITCH_MAX_PPG];
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_uint32_t index = 0;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(pfc_prio_map, 0, sizeof(pfc_prio_map));
  if (qos_map_handle) {
    SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
    status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    FOR_EACH_IN_LIST(qos_map_list->qos_map_list, node) {
      SWITCH_MEMSET(&qos_map, 0, sizeof(switch_qos_map_t));
      qos_map_info = NULL;
      qos_map_info = (switch_qos_map_info_t *)node->data;
      SWITCH_ASSERT(qos_map_info);
      SWITCH_MEMCPY(&qos_map, &qos_map_info->qos_map, sizeof(switch_qos_map_t));
      pfc_prio_map[qos_map.ppg] |= (1 << qos_map.pfc_priority);
    }
    FOR_EACH_IN_LIST_END();
  }

  for (index = 0; index < SWITCH_MAX_PPG; index++) {
    if (pfc_prio_map[index] == 0) {
      continue;
    }
    status = switch_api_port_cos_mapping(device,
                                         port_handle,
                                         port_info->ppg_handles[index],
                                         pfc_prio_map[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set PFC cos to priority group mapping for device %d, for "
          "port 0x%lx: %s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  port_info->ingress_pfc_ppg_handle = qos_map_handle;
  return status;
}

switch_status_t switch_api_port_pfc_priority_to_ppg_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *qos_map_handle = port_info->ingress_pfc_ppg_handle;
  return status;
}

switch_status_t switch_api_port_pfc_priority_to_queue_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_qos_map_t qos_map;
  switch_node_t *node = NULL;
  switch_uint32_t pfc_prio_map[SWITCH_MAX_QUEUE];
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_uint32_t index = 0;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port pfc_prio to queue set failed: port 0x%lx get failed for device "
        "%d:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(pfc_prio_map, 0, sizeof(pfc_prio_map));
  if (qos_map_handle) {
    SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
    status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port pfc_prio to queue set failed: qos handle 0x%lx get failed for "
          "device %d:%s",
          qos_map_handle,
          device,
          switch_error_to_string(status));
      return status;
    }
    FOR_EACH_IN_LIST(qos_map_list->qos_map_list, node) {
      SWITCH_MEMSET(&qos_map, 0, sizeof(switch_qos_map_t));
      qos_map_info = NULL;
      qos_map_info = (switch_qos_map_info_t *)node->data;
      SWITCH_ASSERT(qos_map_info);
      SWITCH_MEMCPY(&qos_map, &qos_map_info->qos_map, sizeof(switch_qos_map_t));
      pfc_prio_map[qos_map.qid] |= (1 << qos_map.pfc_priority);
    }
    FOR_EACH_IN_LIST_END();
  }

  for (index = 0; index < SWITCH_MAX_QUEUE; index++) {
    if (pfc_prio_map[index] == 0) {
      continue;
    }
    status =
        switch_api_queue_pfc_cos_mapping(device,
                                         port_info->queue_handles[index],
                                         (switch_uint8_t)pfc_prio_map[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set PFC cos to queue mapping for device %d, for port "
          "0x%lx: %s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  port_info->egress_pfc_queue_handle = qos_map_handle;
  return status;
}

switch_status_t switch_api_port_pfc_priority_to_queue_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port pfc_prio to queue get failed: port 0x%lx get failed for device "
        "%d:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  *qos_map_handle = port_info->egress_pfc_queue_handle;
  return status;
}

switch_status_t switch_api_port_storm_control_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_packet_type_t pkt_type,
    switch_handle_t *meter_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("port get failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *meter_handle = port_info->meter_handle[pkt_type];
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_qos_group_tc_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_qos_map_t qos_map;
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_node_t *node = NULL;
  switch_uint8_t icos_ppg_map[SWITCH_MAX_PPG];
  switch_uint32_t index = 0;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->tc_qos_group = 0;

  if (qos_map_handle) {
    SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
    if (!SWITCH_QOS_MAP_HANDLE(qos_map_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    if (qos_map_list->ingress_qos_map_type ==
        SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS) {
      SWITCH_MEMSET(icos_ppg_map, 0, sizeof(icos_ppg_map));
      switch_uint8_t ppg_index = 0;
      switch_uint8_t ppg_usage_bmap = 0;
      FOR_EACH_IN_LIST(qos_map_list->qos_map_list, node) {
        SWITCH_MEMSET(&qos_map, 0, sizeof(switch_qos_map_t));
        qos_map_info = NULL;
        qos_map_info = (switch_qos_map_info_t *)node->data;
        SWITCH_ASSERT(qos_map_info);
        SWITCH_MEMCPY(
            &qos_map, &qos_map_info->qos_map, sizeof(switch_qos_map_t));
        if (ppg_usage_bmap & (1 << qos_map.icos)) {
          SWITCH_LOG_DEBUG("icos %d is already mapped to a PPG", qos_map.icos);
          continue;
        }
        icos_ppg_map[ppg_index++] = (1 << qos_map.icos);
        ppg_usage_bmap |= (1 << qos_map.icos);
      }
      FOR_EACH_IN_LIST_END();

      for (index = 0; index < port_info->max_ppg; index++) {
        status = switch_api_port_cos_mapping(device,
                                             port_handle,
                                             port_info->ppg_handles[index],
                                             icos_ppg_map[index]);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("port qos ingress map set failed for device %d: %s:",
                           "icos to ppg mapping set failed for port 0x%lx",
                           device,
                           switch_error_to_string(status),
                           port_handle);
          return status;
        }
      }
      port_info->tc_ppg_handle = qos_map_handle;
    } else {
      port_info->tc_queue_handle = qos_map_handle;
    }
    port_info->tc_qos_group = qos_map_list->qos_group;
  }

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port qos ingress map set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_qos_group_egress_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  switch_port_info_t *port_info = NULL;
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port qos egress map set failed for device %d, invalid port_handle "
        "0x%lx:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port qos egress map set failed for device %d, port get failed "
        "0x%lx:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  port_info->egress_qos_group = 0;

  if (qos_map_handle) {
    SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
    if (!SWITCH_QOS_MAP_HANDLE(qos_map_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "port qos egress map set failed for device %d, invalid qos handle "
          "0x%lx :%s",
          device,
          qos_map_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port qos egress map set failed for device %d, qos get failed for "
          "handle %lx:%s",
          device,
          qos_map_handle,
          switch_error_to_string(status));
      return status;
    }
    port_info->egress_qos_group = qos_map_list->qos_group;
  }

  status = switch_pd_egress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->egress_port_lag_label,
      port_info->port_type,
      port_info->egress_qos_group,
      port_info->egress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port qos egress map set failed for device %d, table update failed "
        "for handle 0x%lx:%s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  port_info->egress_qos_handle = qos_map_handle;
  return status;
}

switch_status_t switch_api_storm_control_counters_get_internal(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint16_t count,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);

  if (!counter_ids || !counters) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("storm control stats failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_meter_counters_get(
      device, meter_handle, count, counter_ids, counters);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("storm control stats failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_port_tc_default_set_internal(
    switch_device_t device, switch_handle_t port_handle, switch_tc_t tc) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port tc default set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port tc default set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->tc = tc;

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("storm control stats failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_color_default_set_internal(
    switch_device_t device, switch_handle_t port_handle, switch_color_t color) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port tc default set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port tc default set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->color = color;

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("storm control stats failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_trust_dscp_set_internal(
    switch_device_t device, switch_handle_t port_handle, bool trust_dscp) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port dscp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port dscp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (port_info->trust_pcp && trust_dscp) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port dscp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->trust_dscp = trust_dscp;

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port dscp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_trust_pcp_set_internal(
    switch_device_t device, switch_handle_t port_handle, bool trust_pcp) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port pcp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port pcp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (port_info->trust_dscp && trust_pcp) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port pcp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->trust_pcp = trust_pcp;

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port pcp trust set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_learning_enabled_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    bool learning_enabled) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port learning enabled set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port learning enabled set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->learning_enabled = learning_enabled;

  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port learning enabled set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_ppg_guaranteed_limit_set_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_uint32_t num_bytes) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (!SWITCH_PORT_HANDLE(ppg_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg guaranteed limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ppg_guaranteed_limit_set(
      device, ppg_info->tm_ppg_handle, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg guaranteed limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_ppg_skid_limit_set_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_uint32_t num_bytes) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (!SWITCH_PORT_HANDLE(ppg_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg skid limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ppg_skid_limit_set(device, ppg_info->tm_ppg_handle, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg skid limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_ppg_skid_hysteresis_set_internal(
    switch_device_t device,
    switch_handle_t ppg_handle,
    switch_uint32_t num_bytes) {
  switch_port_priority_group_t *ppg_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PPG_HANDLE(ppg_handle));
  if (!SWITCH_PORT_HANDLE(ppg_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ppg skid hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ppg_skid_hysteresis_set(
      device, ppg_info->tm_ppg_handle, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ppg skid hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_drop_limit_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t num_bytes) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_drop_limit_set(device, port_handle, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_drop_hysteresis_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t num_bytes) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port drop hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_drop_hysteresis_set(device, port_handle, num_bytes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_pfc_cos_mapping_internal(
    switch_device_t device, switch_handle_t port_handle, uint8_t *cos_to_icos) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("port drop hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop limit set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_port_pfc_cos_mapping(device, port_info->dev_port, cos_to_icos);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port drop hysteresis set failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_port_flowcontrol_mode_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_flowcontrol_type_t flow_control) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port flow control mode set failed on device %d "
        "port handle 0x%lx invalid(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port flow control mode set failed on device %d "
        "port handle 0x%lx get failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_flowcontrol_mode_set(
      device, port_info->dev_port, flow_control);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port flow control mode set failed on device %d "
        "port handle 0x%lx pd port flow control set failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "port flow control mode set on device %d "
      "port handle 0x%lx flow control %s",
      device,
      port_handle,
      switch_port_flowcontrol_type_to_string(flow_control));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_port_id_to_handle_get_internal(
    switch_device_t device, switch_port_t port, switch_handle_t *port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(port <= SWITCH_MAX_PORTS);
  if (port > SWITCH_MAX_PORTS) {
    SWITCH_LOG_ERROR("port id to handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_handle != NULL);
  if (port_handle == NULL) {
    SWITCH_LOG_ERROR("port id to handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port id to handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  handle = port_ctx->port_handles[port];
  if (!SWITCH_PORT_HANDLE(handle)) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR(
        "port id to handle get failed on device %d port %d: "
        "port number invalid:(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  *port_handle = handle;

  return status;
}

switch_status_t switch_api_port_handle_to_id_get_internal(
    switch_device_t device, switch_handle_t port_handle, switch_port_t *port) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port handle to id get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port != NULL);
  if (port == NULL) {
    SWITCH_LOG_ERROR("port handle to id get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port = 0;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port handle to id get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port = port_info->port;

  return status;
}
switch_status_t switch_api_port_speed_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_speed_t *port_speed) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port speed get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(port_speed != NULL);
  if (port_speed == NULL) {
    SWITCH_LOG_ERROR("port speed get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port speed get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *port_speed = port_info->port_speed;

  return status;
}

switch_status_t switch_api_port_admin_state_set_internal(
    switch_device_t device, switch_handle_t port_handle, bool admin_state) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port admin state set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port admin state set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (admin_state && !port_info->admin_state) {
    status = switch_pd_port_enable(device, port_info->dev_port);
  } else if (!admin_state && port_info->admin_state) {
    status = switch_pd_port_disable(device, port_info->dev_port);
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port admin state set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  port_info->admin_state = admin_state;

  return status;
}

switch_status_t switch_api_port_admin_state_get_internal(
    switch_device_t device, switch_handle_t port_handle, bool *admin_state) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port admin get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port admin get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *admin_state = port_info->admin_state;

  return status;
}

switch_status_t switch_api_port_oper_status_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_oper_status_t *oper_status) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port oper get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port oper get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *oper_status = port_info->oper_status;

  return status;
}

switch_status_t switch_api_port_stats_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t num_entries,
    switch_port_counter_id_t *counter_ids,
    switch_uint64_t *counters) {
  switch_port_info_t *port_info = NULL;
  switch_handle_t cpu_port_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (port_handle == cpu_port_handle) return status;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d "
        "port handle 0x%lx invalid port handle : %s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(num_entries != 0);
  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);
  if (num_entries == 0 || !counter_ids || !counters) {
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d "
        "port handle 0x%lx invalid arguments: %s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(SWITCH_PORT_HANDLE(cpu_port_handle));
  if (cpu_port_handle == port_handle) {
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d "
        "port handle 0x%lx invalid arguments: %s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, sizeof(switch_uint64_t) * num_entries);

  status = switch_pd_port_stats_get(
      device, port_info->dev_port, num_entries, counter_ids, counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d "
        "port handle 0x%lx pd stats get failed: %s",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "port stats get done successfully on device %d "
      "for port handle 0x%lx",
      device,
      port_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_port_stats_counter_id_clear_internal(
    const switch_device_t device,
    const switch_handle_t port_handle,
    const switch_uint16_t num_counters,
    const switch_port_counter_id_t *counter_ids) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx invalid port handle:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_stats_counter_id_clear(
      device, port_info->dev_port, num_counters, counter_ids);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx port stats clear failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_stats_clear_internal(
    const switch_device_t device, const switch_handle_t port_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx invalid port handle:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_stats_clear_all(device, port_info->dev_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats clear failed on device %d "
        "port handle 0x%lx port stats clear failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_all_stats_clear_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_port_loopback_mode_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t lb_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port loopback set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port loopback set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(lb_mode < SWITCH_PORT_LOOPBACK_MODE_MAX);

  port_info->lb_mode = lb_mode;

  status =
      switch_pd_port_loopback_mode_set(device, port_info->dev_port, lb_mode);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port loopback set failed on device %d "
        "port handle 0x%lx port loopback mode set failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_port_loopback_mode_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t *lb_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port loopback get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port loopback get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *lb_mode = port_info->lb_mode;

  return status;
}

switch_status_t switch_api_port_attribute_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint64_t flags,
    switch_port_attribute_info_t *port_attribute_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_port_attribute_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint64_t flags,
    switch_port_attribute_info_t *port_attr_info) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "port attribute get failed on device %d "
        "port handle 0x%lx: invalid port handle(%s)",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port attribute get failed on device %d "
        "port handle 0x%lx: port get failed(%s)",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_PORT_ATTR_ADMIN_STATE) {
    port_attr_info->admin_state = port_info->admin_state;
  }

  if (flags & SWITCH_PORT_ATTR_SPEED) {
    port_attr_info->port_speed = port_info->port_speed;
  }

  if (flags & SWITCH_PORT_ATTR_OPER_STATUS) {
    port_attr_info->oper_status = port_info->oper_status;
  }

  if (flags & SWITCH_PORT_ATTR_LANE_LIST) {
    SWITCH_MEMCPY(&port_attr_info->lane_list,
                  &port_info->lane_list,
                  sizeof(switch_port_lane_list_t));
  }

  if (flags & SWITCH_PORT_ATTR_INGRESS_ACL_GROUP) {
    port_attr_info->ingress_acl_group_handle =
        port_info->ingress_acl_group_handle;
  }

  if (flags & SWITCH_PORT_ATTR_EGRESS_ACL_GROUP) {
    port_attr_info->egress_acl_group_handle =
        port_info->egress_acl_group_handle;
  }

  if (flags & SWITCH_PORT_ATTR_LOOPBACK_MODE) {
    port_attr_info->lb_mode = port_info->lb_mode;
  }

  if (flags & SWITCH_PORT_ATTR_AUTO_NEG_MODE) {
    port_attr_info->an_mode = port_info->an_mode;
  }

  return status;
}

switch_status_t switch_port_ingress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_port_lag_label_t label) {
  switch_status_t status;
  switch_port_info_t *port_info = NULL;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      port_info->ingress_port_lag_label = label;
      break;
    case SWITCH_HANDLE_TYPE_PORT:
      port_info->ingress_port_lag_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }

  port_info->ingress_acl_group_handle = acl_group;
  status = switch_pd_ingress_port_properties_table_entry_update(
      device,
      port_info->yid,
      port_info,
      port_info->ingress_port_lag_label,
      port_info->ingress_prop_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "Port acl label set device: %d, "
      "port_handle 0x%x bp_type %d, label %d \n",
      device,
      port_handle,
      bp_type,
      label);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_port_egress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_port_lag_label_t label) {
  switch_status_t status;
  switch_port_info_t *port_info = NULL;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      port_info->egress_port_lag_label = label;
      break;
    case SWITCH_HANDLE_TYPE_PORT:
      port_info->egress_port_lag_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }
  port_info->egress_acl_group_handle = acl_group;
  status = switch_pd_egress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->egress_port_lag_label,
      port_info->port_type,
      port_info->egress_qos_group,
      port_info->egress_mapping_hw_entry);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "Port acl label set device: %d, "
      "port_handle 0x%x bp_type %d, label %d \n",
      device,
      port_handle,
      bp_type,
      label);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_ingress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t acl_group) {
  return switch_port_ingress_acl_group_label_set(
      device, port_handle, SWITCH_HANDLE_TYPE_PORT, acl_group, 0);
}

switch_status_t switch_api_port_ingress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t label) {
  return switch_port_ingress_acl_group_label_set(
      device, port_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_port_egress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t acl_group) {
  return switch_port_egress_acl_group_label_set(
      device, port_handle, SWITCH_HANDLE_TYPE_PORT, acl_group, 0);
}

switch_status_t switch_api_port_egress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t label) {
  return switch_port_egress_acl_group_label_set(
      device, port_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_port_ingress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *acl_group = port_info->ingress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_ingress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t *label) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *label = port_info->ingress_port_lag_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_egress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *acl_group = port_info->egress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_egress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t *label) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  if (port_info == NULL) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *label = port_info->egress_port_lag_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_cpu_port_add(switch_device_t device,
                                    switch_port_t port,
                                    switch_handle_t *port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_api_port_info_t api_port_info;
  switch_handle_t handle = 0;
  switch_port_t fp_port = 0;
  switch_dev_port_t dev_port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;

  status = switch_device_dev_port_get(device, port, &dev_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d: "
        "dev port get failed:(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_front_port_get(device, dev_port, &fp_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d dev port %d: "
        "front port get failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d dev port %d: "
        "device context get failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  handle = port_ctx->port_handles[fp_port];
  if (!SWITCH_PORT_HANDLE(port_ctx->port_handles[fp_port])) {
    // SWITCH_ASSERT(fp_port == port);
    SWITCH_MEMSET(&api_port_info, 0x0, sizeof(api_port_info));
    api_port_info.port_speed = SWITCH_PORT_SPEED_100G;
    api_port_info.port = port;
    api_port_info.initial_admin_state = TRUE;
    api_port_info.rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
    api_port_info.tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
    status = switch_api_port_add(device, &api_port_info, &handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "cpu port create failed on device %d port %d dev port %d: "
          "device context get failed:(%s)\n",
          device,
          port,
          dev_port,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_port_get(device, handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d dev port %d: "
        "port get failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  port_ctx->port_handles[port] = handle;
  port_info->port_type = SWITCH_PORT_TYPE_CPU;
  port_info->port = port;

  status = switch_pd_ingress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->port_lag_index,
      port_info->port_type,
      port_info->ingress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d dev port %d: "
        "ingress port mapping table update failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->egress_port_lag_label,
      port_info->port_type,
      port_info->egress_qos_group,
      port_info->egress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "cpu port create failed on device %d port %d dev port %d: "
        "egress port mapping table update failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "cpu port created on device %d port %d dev port %d port handle 0x%lx\n",
      device,
      port,
      dev_port,
      handle);

  *port_handle = cpu_port_handle = handle;

  for (index = 0; index < port_info->max_queues; index++) {
    status = switch_pd_queue_color_drop_enable(
        device, port_info->dev_port, index, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port add failed on device %d: "
          "queue color drop enable failed for cpu port, qid %d",
          device,
          index);
      return status;
    }
    /*
     * Copp meter doesn't have drop action. so, set the minimum buffer for
     * Yellow and Red packets on CPU port.
     * Setting the limit of 0 for red and yellow corresponds to 12.5% of Green
     * buffers in hardware. This is the minimum limit in Tofino.
     *
     * ToDo: To drop all red/yellow packets, egress acl to be added for
     * '{color, reason-code} - drop'.
     */
    status = switch_pd_queue_color_limit_set(
        device, port_info->dev_port, index, SWITCH_COLOR_RED, 0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port add failed on device %d:",
                       "queue red color limit set failed for queue %d",
                       device,
                       index);
      return status;
    }
    status = switch_pd_queue_color_limit_set(
        device, port_info->dev_port, index, SWITCH_COLOR_YELLOW, 0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("port add failed on device %d:",
                       "queue yellow color limit set failed for queue %d",
                       device,
                       index);
      return status;
    }
  }
  return status;
}

switch_status_t switch_recirc_port_add(switch_device_t device,
                                       switch_port_t port,
                                       switch_handle_t *port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_info_t *port_info = NULL;
  switch_api_port_info_t api_port_info;
  switch_handle_t handle = 0;
  switch_port_t fp_port = 0;
  switch_dev_port_t dev_port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_dev_port_get(device, port, &dev_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "recirc port create failed on device %d port %d: "
        "dev port get failed:(%s)\n",
        device,
        port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "recirc port create failed on device %d port %d dev port %d: "
        "device context get failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  handle = port_ctx->port_handles[fp_port];
  if (!SWITCH_PORT_HANDLE(port_ctx->port_handles[port])) {
    SWITCH_MEMSET(&api_port_info, 0x0, sizeof(api_port_info));
    api_port_info.port_speed = SWITCH_PORT_SPEED_100G;
    api_port_info.port = port;
    api_port_info.initial_admin_state = TRUE;
    api_port_info.rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
    api_port_info.tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
    status = switch_api_port_add(device, &api_port_info, &handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "recirc port create failed on device %d port %d dev port %d: "
          "device context get failed:(%s)\n",
          device,
          port,
          dev_port,
          switch_error_to_string(status));
      return status;
    }
  }
  status = switch_port_get(device, handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "recirc port create failed on device %d port %d: "
        "port get failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  port_ctx->port_handles[port] = handle;
  port_info->port_type = SWITCH_PORT_TYPE_RECIRC;
  port_info->port = port;

  status = switch_pd_ingress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->port_lag_index,
      port_info->port_type,
      port_info->ingress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "recirc port create failed on device %d port %d dev port %d: "
        "ingress port mapping table update failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_egress_port_mapping_table_entry_update(
      device,
      port_info->dev_port,
      port_info->egress_port_lag_label,
      port_info->port_type,
      port_info->egress_qos_group,
      port_info->egress_mapping_hw_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "recirc port create failed on device %d port %d dev port %d: "
        "egress port mapping table update failed:(%s)\n",
        device,
        port,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "recirc port created on device %d port %d dev port %d "
      "port handle 0x%lx\n",
      device,
      port,
      dev_port,
      handle);

  *port_handle = handle;

  return status;
}

switch_status_t switch_api_port_bind_mode_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t *bind_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *bind_mode = port_info->bind_mode;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_bind_mode_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t bind_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  if (port_info->bind_mode == bind_mode) {
    return SWITCH_STATUS_SUCCESS;
  } else if (SWITCH_ARRAY_COUNT(&port_info->intf_array) != 0) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  port_info->bind_mode = bind_mode;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_auto_neg_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t an_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port auto neg set failed on device %d handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port auto neg set failed on device %d handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (port_info->an_mode == an_mode) {
    return status;
  }

  if (port_info->admin_state) {
    status = switch_pd_port_disable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port auto neg set failed on device %d handle 0x%lx: "
          "port disable failed:(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_pd_port_auto_neg_set(device, port_info->dev_port, an_mode);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port auto neg set failed on device %d handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (port_info->admin_state) {
    status = switch_pd_port_enable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port auto neg set failed on device %d handle 0x%lx: "
          "port enable failed:(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  port_info->an_mode = an_mode;

  SWITCH_LOG_DEBUG("port auto neg set on device %d port handle 0x%lx mode %d\n",
                   device,
                   port_handle,
                   an_mode);

  return status;
}

switch_status_t switch_api_port_auto_neg_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t *an_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port auto neg get failed on device %d handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port auto neg get failed on device %d handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *an_mode = port_info->an_mode;

  SWITCH_LOG_DEBUG("port auto neg get on device %d port handle 0x%lx mode %d\n",
                   device,
                   port_handle,
                   *an_mode);

  return status;
}

switch_status_t switch_api_port_pfc_get_internal(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 uint32_t *pfc_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port handle set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    *pfc_map = port_info->pfc_map;
  }

  return status;
}

switch_status_t switch_api_port_pfc_set_internal(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 uint32_t pfc_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port handle invalid on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port handle set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * Set same PFC cos-bitmap for both Rx and Tx.
   */
  status =
      switch_pd_port_pfc_set(device, port_info->dev_port, pfc_map, pfc_map);
  if (status == SWITCH_STATUS_SUCCESS) {
    port_info->pfc_map = pfc_map;
  }

  return status;
}

switch_status_t switch_api_port_link_pause_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    bool rx_pause_en,
    bool tx_pause_en) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port handle invalid on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port info get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * Set same PFC cos-bitmap for both Rx and Tx.
   */
  status = switch_pd_port_link_pause_set(
      device, port_info->dev_port, rx_pause_en, tx_pause_en);
  if (status == SWITCH_STATUS_SUCCESS) {
    port_info->rx_pause = rx_pause_en;
    port_info->tx_pause = tx_pause_en;
  }

  return status;
}

switch_status_t switch_api_port_link_pause_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    bool *rx_pause,
    bool *tx_pause) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR("port handle invalid on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port info get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_SUCCESS) {
    *rx_pause = port_info->rx_pause;
    *tx_pause = port_info->tx_pause;
  }

  return status;
}

switch_status_t switch_api_port_mtu_set_internal(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_uint32_t tx_mtu,
                                                 switch_uint32_t rx_mtu) {
  switch_uint32_t rx_mtu_int = SWITCH_PORT_RX_MTU_DEFAULT;
  switch_uint32_t tx_mtu_int = SWITCH_PORT_TX_MTU_DEFAULT;
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port mtu set failed on device %d port handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port mtu set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (rx_mtu) {
    rx_mtu_int = rx_mtu;
  }

  if (tx_mtu) {
    tx_mtu_int = tx_mtu;
  }

  if (port_info->rx_mtu == rx_mtu && port_info->tx_mtu == tx_mtu) {
    return status;
  }

  if (port_info->admin_state) {
    status = switch_pd_port_disable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port mtu set failed on device %d port handle 0x%lx: "
          "port pd disable failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_pd_port_mtu_set(
      device, port_info->dev_port, tx_mtu_int, rx_mtu_int);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port mtu set failed on device %d port handle 0x%lx: "
        "port pd mtu set failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (port_info->admin_state) {
    status = switch_pd_port_enable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port mtu set failed on device %d port handle 0x%lx: "
          "port pd enable failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  port_info->rx_mtu = rx_mtu_int;
  port_info->tx_mtu = tx_mtu_int;

  return status;
}

switch_status_t switch_api_port_mtu_get_internal(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_uint32_t *tx_mtu,
                                                 switch_uint32_t *rx_mtu) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port mtu get failed on device %d port handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port mtu get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *rx_mtu = port_info->rx_mtu;
  *tx_mtu = port_info->tx_mtu;

  return status;
}

switch_status_t switch_api_port_max_queues_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t *max_queues) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device %u, error %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *max_queues = port_info->max_queues;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_port_dev_port_to_handle_get(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_handle_t *port_handle) {
  switch_port_context_t *port_ctx = NULL;
  switch_port_t fp_port = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *port_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_device_front_port_get(device, dev_port, &fp_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "dev port to handle get failed on device %d port %d: "
        "front port get failed:(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_PORT, (void **)&port_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "dev port to handle get failed on device %d port %d: "
        "port context get failed:(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  handle = port_ctx->port_handles[fp_port];
  if (!SWITCH_PORT_HANDLE(handle)) {
    SWITCH_LOG_ERROR(
        "dev port to handle get failed on device %d port %d: "
        "port not created:(%s)\n",
        device,
        dev_port,
        switch_error_to_string(status));
    return status;
  }

  *port_handle = handle;
  return status;
}

switch_status_t switch_api_port_fec_mode_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_fec_mode_t fec_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port fec set failed on device %d port handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port fec set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_port_fec_set(device, port_info->dev_port, fec_mode);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port FEC mode set failed on device %d port handle 0x%lx: ",
        "PD fec set failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  port_info->fec_mode = fec_mode;
  return status;
}

switch_status_t switch_api_port_fec_mode_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_fec_mode_t *fec_mode) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port fec get failed on device %d port handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port fec get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  *fec_mode = port_info->fec_mode;
  return status;
}

switch_status_t switch_api_port_qos_group_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_qos_handle,
    switch_handle_t *tc_queue_handle,
    switch_handle_t *tc_ppg_handle,
    switch_handle_t *egress_qos_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port qosgroup get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  *ingress_qos_handle = port_info->ingress_qos_handle;
  *egress_qos_handle = port_info->egress_qos_handle;
  *tc_ppg_handle = port_info->tc_ppg_handle;
  *tc_queue_handle = port_info->tc_queue_handle;
  return status;
}

switch_status_t switch_api_port_qos_scheduler_group_handles_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *group_handles) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t index = 0;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port qosgroup get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(group_handles);

  for (index = 0; index < SWITCH_MAX_QUEUE; index++) {
    group_handles[index] = port_info->queue_scheduler_group_handles[index];
  }
  return status;
}

switch_status_t switch_api_port_queue_scheduler_group_handle_count_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t *count) {
  *count = SWITCH_MAX_QUEUE;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_port_ingress_mirror_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle) {
  switch_port_info_t *port_info = NULL;
  switch_pd_hdl_t pd_hdl = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port ingress mirror set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  if (mirror_handle == SWITCH_API_INVALID_HANDLE) {
    status = switch_pd_port_ingress_mirror_delete(
        device, port_info->dev_port, port_info->ingress_mirror_hw_entry);
  } else {
    SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
    status = switch_pd_port_ingress_mirror_set(
        device, port_info->dev_port, mirror_handle, &pd_hdl);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Ingress port session set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  port_info->ingress_mirror_handle = mirror_handle;
  port_info->ingress_mirror_hw_entry = pd_hdl;
  return status;
}

switch_status_t switch_api_port_ingress_mirror_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port ingress mirror get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *mirror_handle = port_info->ingress_mirror_handle;
  return status;
}

switch_status_t switch_api_port_egress_mirror_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle) {
  switch_port_info_t *port_info = NULL;
  switch_pd_hdl_t pd_hdl = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port egress mirror set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  if (mirror_handle == SWITCH_API_INVALID_HANDLE) {
    status = switch_pd_port_egress_mirror_delete(
        device, port_info->dev_port, port_info->egress_mirror_hw_entry);
  } else {
    SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
    status = switch_pd_port_egress_mirror_set(
        device, port_info->dev_port, mirror_handle, &pd_hdl);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Egress port session set failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  port_info->egress_mirror_handle = mirror_handle;
  port_info->egress_mirror_hw_entry = pd_hdl;
  return status;
}

switch_status_t switch_api_port_egress_mirror_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port egress mirror get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *mirror_handle = port_info->egress_mirror_handle;
  return status;
}

switch_status_t switch_api_port_ingress_sflow_handle_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_sflow_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port ingress sflow handle get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *ingress_sflow_handle = port_info->ingress_sflow_handle;
  return status;
}

switch_status_t switch_api_port_egress_sflow_handle_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *egress_sflow_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port egress sflow handle get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *egress_sflow_handle = port_info->egress_sflow_handle;
  return status;
}

switch_status_t switch_api_port_lane_list_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lane_list_t *lane_list) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port lane list get failed on device %d port handle 0x%lx "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      lane_list, &port_info->lane_list, sizeof(switch_port_lane_list_t));

  return status;
}

switch_status_t switch_api_port_speed_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_speed_t port_speed) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "port speed set failed on device %d port handle 0x%lx: "
        "port handle invalid:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port speed set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  if (port_info->port_speed == port_speed) {
    return status;
  }

  if (port_info->admin_state) {
    status = switch_pd_port_disable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port speed set failed on device %d port handle 0x%lx: "
          "port pd disable failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_pd_port_delete(device, port_info->dev_port);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port speed set failed on device %d port handle 0x%lx: "
        "port pd delete failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_add(device, port_info->dev_port, port_speed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port speed set failed on device %d port handle 0x%lx: "
        "port pd add failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_mtu_set(
      device, port_info->dev_port, port_info->tx_mtu, port_info->rx_mtu);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port speed set failed on device %d port handle 0x%lx: "
        "port pd add failed(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  // if old speed is 100G and new is 40G and RS FEC enabled - disable FEC
  // (hack?)
  if (port_info->port_speed == SWITCH_PORT_SPEED_100G &&
      port_speed == SWITCH_PORT_SPEED_40G &&
      port_info->fec_mode == SWITCH_PORT_FEC_MODE_RS) {
    port_info->fec_mode = SWITCH_PORT_FEC_MODE_NONE;
  }

  if (port_info->fec_mode != SWITCH_PORT_FEC_MODE_NONE) {
    status = switch_pd_port_fec_set(
        device, port_info->dev_port, port_info->fec_mode);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port speed set failed on device %d port handle 0x%lx: "
          "port fec set failed(%s) \n",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (port_info->admin_state) {
    status = switch_pd_port_enable(device, port_info->dev_port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "port speed set failed on device %d port handle 0x%lx: "
          "port pd enable failed(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_MEMSET(&port_info->lane_list, 0x0, sizeof(switch_port_lane_list_t));
  SWITCH_PORT_LANE_MAPPING(
      port_info->port, port_speed, port_info->lane_list, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  port_info->port_speed = port_speed;

  return status;
}

switch_status_t switch_api_port_scheduler_profile_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *scheduler_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port scheduler get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  *scheduler_handle = port_info->scheduler_handle;
  return status;
}

switch_status_t switch_api_port_scheduler_profile_set_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t scheduler_handle) {
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port scheduler set failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }
  /*
   * When scheduler profile is attached to a port, only port level shaper
   * is configured.
   */
  if (scheduler_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_SCHEDULER_HANDLE(scheduler_handle));
    status = switch_api_scheduler_group_profile_set(
        device, port_info->port_scheduler_group_handle, scheduler_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set port scheduler profile on device %d: port scheduler "
          "group update failed for port 0x%lx: %s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    /*
     * When the port shaper is disabled, configure default shape rate of 100G
     * and default burst size.
     */
    status =
        switch_api_port_shaping_set(device,
                                    port_handle,
                                    false,
                                    DEFAULT_BURST_SIZE,
                                    (switch_uint64_t)DEFAULT_PORT_SHAPE_RATE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "Failed to set port default shaping rate on device %d: default port "
          "shaping set failed for port 0x%lx: %s",
          device,
          port_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  port_info->scheduler_handle = scheduler_handle;
  return status;
}

switch_status_t switch_api_port_ppg_drop_get_internal(
    const switch_device_t device,
    const switch_handle_t port_handle,
    uint64_t *drop_count) {
  switch_port_info_t *port_info = NULL;
  uint64_t idrop_count = 0;
  uint64_t edrop_count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port ppg drop count get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_tm_drop_get(
      device, port_info->dev_port, &idrop_count, &edrop_count);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port ppg drop count get failed on device %d port handle 0x%lx: "
        "port pd tm drop get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *drop_count = idrop_count;
  return status;
}

switch_status_t switch_api_port_queue_drop_get_internal(
    const switch_device_t device,
    const switch_handle_t port_handle,
    uint64_t *drop_count) {
  switch_port_info_t *port_info = NULL;
  uint64_t idrop_count = 0;
  uint64_t edrop_count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port queue drop count get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_port_tm_drop_get(
      device, port_info->dev_port, &idrop_count, &edrop_count);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port queue drop count get failed on device %d port handle 0x%lx: "
        "port pd tm drop get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  *drop_count = edrop_count;
  return status;
}

switch_status_t switch_api_interface_port_stats_get_internal(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  switch_port_info_t *port_info = NULL;
  switch_uint16_t num_port_entries = 0;
  switch_port_counter_id_t port_counter_id[SWITCH_PORT_STAT_MAX];
  switch_uint64_t port_counters[SWITCH_PORT_STAT_MAX];
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d port handle 0x%lx: "
        "port get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_entries; index++) {
    switch (counter_id[index]) {
      case SWITCH_INTERFACE_COUNTER_IN_PACKETS:
        port_counter_id[index] = SWITCH_PORT_STAT_IN_ALL_PKTS;
        num_port_entries++;
        break;
      case SWITCH_INTERFACE_COUNTER_IN_BYTES:
        port_counter_id[index] = SWITCH_PORT_STAT_IN_ALL_OCTETS;
        num_port_entries++;
        break;
      case SWITCH_INTERFACE_COUNTER_OUT_PACKETS:
        port_counter_id[index] = SWITCH_PORT_STAT_OUT_ALL_PKTS;
        num_port_entries++;
      case SWITCH_INTERFACE_COUNTER_OUT_BYTES:
        port_counter_id[index] = SWITCH_PORT_STAT_OUT_ALL_OCTETS;
        num_port_entries++;
        break;
      default:
        break;
    }
  }

  if (num_port_entries == 0) {
    return status;
  }

  SWITCH_MEMSET(port_counters, 0x0, sizeof(port_counters));
  status = switch_api_port_stats_get(
      device, port_handle, num_port_entries, port_counter_id, port_counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "port stats get failed on device %d port handle 0x%lx: "
        "port stats get failed:(%s)\n",
        device,
        port_handle,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_port_entries; index++) {
    switch (port_counter_id[index]) {
      case SWITCH_PORT_STAT_IN_ALL_PKTS:
      case SWITCH_PORT_STAT_OUT_ALL_PKTS:
        counters[index].num_packets = port_counters[index];
        counters[index].num_bytes = 0;
        break;
      case SWITCH_PORT_STAT_IN_ALL_OCTETS:
      case SWITCH_PORT_STAT_OUT_ALL_OCTETS:
        counters[index].num_packets = 0;
        counters[index].num_bytes = port_counters[index];
        break;
      default:
        break;
    }
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_port_bind_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t bind_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_bind_mode_set_internal(device, port_handle, bind_mode))
}

switch_status_t switch_api_port_ingress_acl_group_set(
    switch_device_t device, switch_handle_t port_handle, switch_handle_t acl) {
  SWITCH_MT_WRAP(
      switch_api_port_ingress_acl_group_set_internal(device, port_handle, acl))
}

switch_status_t switch_api_port_egress_acl_group_set(
    switch_device_t device, switch_handle_t port_handle, switch_handle_t acl) {
  SWITCH_MT_WRAP(
      switch_api_port_egress_acl_group_set_internal(device, port_handle, acl))
}

switch_status_t switch_api_port_ingress_acl_label_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t label) {
  SWITCH_MT_WRAP(switch_api_port_ingress_acl_label_set_internal(
      device, port_handle, label))
}

switch_status_t switch_api_port_egress_acl_label_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lag_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_port_egress_acl_label_set_internal(device, port_handle, label))
}

switch_status_t switch_api_port_drop_hysteresis_set(switch_device_t device,
                                                    switch_handle_t port_handle,
                                                    uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_port_drop_hysteresis_set_internal(
      device, port_handle, num_bytes))
}

switch_status_t switch_api_port_qos_group_tc_set(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_handle_t qos_group) {
  SWITCH_MT_WRAP(
      switch_api_port_qos_group_tc_set_internal(device, port_handle, qos_group))
}

switch_status_t switch_api_port_color_default_set(switch_device_t device,
                                                  switch_handle_t port_handle,
                                                  switch_color_t color) {
  SWITCH_MT_WRAP(
      switch_api_port_color_default_set_internal(device, port_handle, color))
}

switch_status_t switch_api_port_storm_control_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_packet_type_t pkt_type,
    switch_handle_t *meter_handle) {
  SWITCH_MT_WRAP(switch_api_port_storm_control_get_internal(
      device, port_handle, pkt_type, meter_handle))
}

switch_status_t switch_api_port_ppg_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        uint8_t *num_ppgs,
                                        switch_handle_t *ppg_handles) {
  SWITCH_MT_WRAP(switch_api_port_ppg_get_internal(
      device, port_handle, num_ppgs, ppg_handles))
}

switch_status_t switch_api_port_admin_state_get(switch_device_t device,
                                                switch_handle_t port_handle,
                                                bool *admin_state) {
  SWITCH_MT_WRAP(switch_api_port_admin_state_get_internal(
      device, port_handle, admin_state))
}

switch_status_t switch_api_port_bind_mode_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t *bind_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_bind_mode_get_internal(device, port_handle, bind_mode))
}

switch_status_t switch_api_port_handle_to_id_get(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_port_t *port) {
  SWITCH_MT_WRAP(
      switch_api_port_handle_to_id_get_internal(device, port_handle, port))
}

switch_status_t switch_api_storm_control_counters_get(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint16_t num_counters,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_storm_control_counters_get_internal(
      device, meter_handle, num_counters, counter_ids, counters))
}

switch_status_t switch_api_port_flowcontrol_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_flowcontrol_type_t flow_control) {
  SWITCH_MT_WRAP(switch_api_port_flowcontrol_mode_set_internal(
      device, port_handle, flow_control))
}

switch_status_t switch_api_ppg_lossless_enable(switch_device_t device,
                                               switch_handle_t ppg_handle,
                                               bool enable) {
  SWITCH_MT_WRAP(
      switch_api_ppg_lossless_enable_internal(device, ppg_handle, enable))
}

switch_status_t switch_api_ppg_drop_get(switch_device_t device,
                                        switch_handle_t ppg_handle,
                                        uint64_t *num_packets) {
  SWITCH_MT_WRAP(
      switch_api_ppg_drop_get_internal(device, ppg_handle, num_packets))
}
switch_status_t switch_api_port_trust_pcp_set(switch_device_t device,
                                              switch_handle_t port_handle,
                                              bool trust_pcp) {
  SWITCH_MT_WRAP(
      switch_api_port_trust_pcp_set_internal(device, port_handle, trust_pcp))
}

switch_status_t switch_api_port_learning_enabled_set(
    switch_device_t device,
    switch_handle_t port_handle,
    bool learning_enabled) {
  SWITCH_MT_WRAP(switch_api_port_learning_enabled_set_internal(
      device, port_handle, learning_enabled))
}

switch_status_t switch_api_port_add(switch_device_t device,
                                    switch_api_port_info_t *api_port_info,
                                    switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(
      switch_api_port_add_internal(device, api_port_info, port_handle))
}

switch_status_t switch_api_port_speed_get(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_port_speed_t *speed) {
  SWITCH_MT_WRAP(switch_api_port_speed_get_internal(device, port_handle, speed))
}

switch_status_t switch_api_port_stats_get(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_uint16_t num_entries,
                                          switch_port_counter_id_t *counter_ids,
                                          switch_uint64_t *counters) {
  SWITCH_MT_WRAP(switch_api_port_stats_get_internal(
      device, port_handle, num_entries, counter_ids, counters))
}

switch_status_t switch_api_port_storm_control_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_packet_type_t pkt_type,
    switch_handle_t meter_handle) {
  SWITCH_MT_WRAP(switch_api_port_storm_control_set_internal(
      device, port_handle, pkt_type, meter_handle))
}

switch_status_t switch_api_port_id_to_handle_get(switch_device_t device,
                                                 switch_port_t port,
                                                 switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(
      switch_api_port_id_to_handle_get_internal(device, port, port_handle))
}

switch_status_t switch_api_port_stats_clear(switch_device_t device,
                                            switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_port_stats_clear_internal(device, port_handle))
}

switch_status_t switch_api_port_stats_counter_id_clear(
    const switch_device_t device,
    const switch_handle_t port_handle,
    const switch_uint16_t num_counters,
    const switch_port_counter_id_t *counter_ids) {
  SWITCH_MT_WRAP(switch_api_port_stats_counter_id_clear_internal(
      device, port_handle, num_counters, counter_ids))
}

switch_status_t switch_api_port_pfc_cos_mapping(switch_device_t device,
                                                switch_handle_t port_handle,
                                                uint8_t *cos_to_icos) {
  SWITCH_MT_WRAP(switch_api_port_pfc_cos_mapping_internal(
      device, port_handle, cos_to_icos))
}

switch_status_t switch_api_port_admin_state_set(switch_device_t device,
                                                switch_handle_t port_handle,
                                                bool admin_state) {
  SWITCH_MT_WRAP(switch_api_port_admin_state_set_internal(
      device, port_handle, admin_state))
}

switch_status_t switch_api_ppg_skid_hysteresis_set(switch_device_t device,
                                                   switch_handle_t ppg_handle,
                                                   uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_ppg_skid_hysteresis_set_internal(
      device, ppg_handle, num_bytes))
}

switch_status_t switch_api_port_all_stats_clear(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_port_all_stats_clear_internal(device))
}

switch_status_t switch_api_port_oper_status_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_oper_status_t *oper_status) {
  SWITCH_MT_WRAP(switch_api_port_oper_status_get_internal(
      device, port_handle, oper_status))
}

switch_status_t switch_api_port_qos_group_egress_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_group) {
  SWITCH_MT_WRAP(switch_api_port_qos_group_egress_set_internal(
      device, port_handle, qos_group))
}

switch_status_t switch_api_port_tc_default_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               uint16_t tc) {
  SWITCH_MT_WRAP(
      switch_api_port_tc_default_set_internal(device, port_handle, tc))
}

switch_status_t switch_api_port_cos_mapping(switch_device_t device,
                                            switch_handle_t port_handle,
                                            switch_handle_t ppg_handle,
                                            uint8_t cos_bitmap) {
  SWITCH_MT_WRAP(switch_api_port_cos_mapping_internal(
      device, port_handle, ppg_handle, cos_bitmap))
}

switch_status_t switch_api_port_delete(switch_device_t device,
                                       switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_port_delete_internal(device, port_handle))
}

switch_status_t switch_api_port_trust_dscp_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool trust_dscp) {
  SWITCH_MT_WRAP(
      switch_api_port_trust_dscp_set_internal(device, port_handle, trust_dscp))
}

switch_status_t switch_api_port_drop_limit_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               uint32_t num_bytes) {
  SWITCH_MT_WRAP(
      switch_api_port_drop_limit_set_internal(device, port_handle, num_bytes))
}

switch_status_t switch_api_ppg_skid_limit_set(switch_device_t device,
                                              switch_handle_t ppg_handle,
                                              uint32_t num_bytes) {
  SWITCH_MT_WRAP(
      switch_api_ppg_skid_limit_set_internal(device, ppg_handle, num_bytes))
}

switch_status_t switch_api_port_qos_group_ingress_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_group) {
  SWITCH_MT_WRAP(switch_api_port_qos_group_ingress_set_internal(
      device, port_handle, qos_group))
}

switch_status_t switch_api_port_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_port_ingress_acl_group_get_internal(
      device, port_handle, acl_group))
}

switch_status_t switch_api_port_ingress_acl_label_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t *label) {
  SWITCH_MT_WRAP(switch_api_port_ingress_acl_label_get_internal(
      device, port_handle, label))
}

switch_status_t switch_api_port_egress_acl_label_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t *label) {
  SWITCH_MT_WRAP(
      switch_api_port_egress_acl_label_get_internal(device, port_handle, label))
}

switch_status_t switch_api_port_egress_acl_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_port_egress_acl_group_get_internal(
      device, port_handle, acl_group))
}

switch_status_t switch_api_ppg_guaranteed_limit_set(switch_device_t device,
                                                    switch_handle_t ppg_handle,
                                                    uint32_t num_bytes) {
  SWITCH_MT_WRAP(switch_api_ppg_guaranteed_limit_set_internal(
      device, ppg_handle, num_bytes))
}

switch_status_t switch_api_port_auto_neg_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t an_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_auto_neg_set_internal(device, port_handle, an_mode));
}

switch_status_t switch_api_port_auto_neg_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t *an_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_auto_neg_get_internal(device, port_handle, an_mode));
}

switch_status_t switch_api_port_event_notification_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_event_notification_fn cb_fn) {
  SWITCH_MT_WRAP(switch_api_port_event_notification_register_internal(
      device, app_id, cb_fn));
}

switch_status_t switch_api_port_event_notification_deregister(
    switch_device_t device, switch_app_id_t app_id) {
  SWITCH_MT_WRAP(
      switch_api_port_event_notification_deregister_internal(device, app_id));
}

switch_status_t switch_api_port_state_change_notification_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_state_change_notification_fn cb_fn) {
  SWITCH_MT_WRAP(switch_api_port_state_change_notification_register_internal(
      device, app_id, cb_fn));
}

switch_status_t switch_api_port_state_change_notification_deregister(
    switch_device_t device, switch_app_id_t app_id) {
  SWITCH_MT_WRAP(switch_api_port_state_change_notification_deregister_internal(
      device, app_id));
}

switch_status_t switch_api_port_loopback_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t lb_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_loopback_mode_set_internal(device, port_handle, lb_mode));
}

switch_status_t switch_api_port_loopback_mode_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t *lb_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_loopback_mode_get_internal(device, port_handle, lb_mode));
}

switch_status_t switch_api_port_max_queues_get(switch_device_t device,
                                               switch_handle_t port_handle,
                                               switch_uint32_t *max_queues) {
  SWITCH_MT_WRAP(
      switch_api_port_max_queues_get_internal(device, port_handle, max_queues))
}

switch_status_t switch_api_port_pfc_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t *pfc_map) {
  SWITCH_MT_WRAP(switch_api_port_pfc_get_internal(device, port_handle, pfc_map))
}

switch_status_t switch_api_port_pfc_set(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t pfc_map) {
  SWITCH_MT_WRAP(switch_api_port_pfc_set_internal(device, port_handle, pfc_map))
}

switch_status_t switch_api_port_link_pause_get(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool *rx_pause_en,
                                               bool *tx_pause_en) {
  SWITCH_MT_WRAP(switch_api_port_link_pause_get_internal(
      device, port_handle, rx_pause_en, tx_pause_en))
}
switch_status_t switch_api_port_link_pause_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool rx_pause_en,
                                               bool tx_pause_en) {
  SWITCH_MT_WRAP(switch_api_port_link_pause_set_internal(
      device, port_handle, rx_pause_en, tx_pause_en))
}

switch_status_t switch_api_port_mtu_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t *tx_mtu,
                                        switch_uint32_t *rx_mtu) {
  SWITCH_MT_WRAP(
      switch_api_port_mtu_get_internal(device, port_handle, tx_mtu, rx_mtu))
}
switch_status_t switch_api_port_mtu_set(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t tx_mtu,
                                        switch_uint32_t rx_mtu) {
  SWITCH_MT_WRAP(
      switch_api_port_mtu_set_internal(device, port_handle, tx_mtu, rx_mtu))
}

switch_status_t switch_api_port_fec_mode_set(switch_device_t device,
                                             switch_handle_t port_handle,
                                             switch_port_fec_mode_t fec_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_fec_mode_set_internal(device, port_handle, fec_mode))
}

switch_status_t switch_api_port_fec_mode_get(switch_device_t device,
                                             switch_handle_t port_handle,
                                             switch_port_fec_mode_t *fec_mode) {
  SWITCH_MT_WRAP(
      switch_api_port_fec_mode_get_internal(device, port_handle, fec_mode))
}

switch_status_t switch_api_port_qos_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_qos_handle,
    switch_handle_t *tc_queue_handle,
    switch_handle_t *tc_ppg_handle,
    switch_handle_t *egress_qos_handle) {
  SWITCH_MT_WRAP(switch_api_port_qos_group_get_internal(device,
                                                        port_handle,
                                                        ingress_qos_handle,
                                                        tc_queue_handle,
                                                        tc_ppg_handle,
                                                        egress_qos_handle))
}

switch_status_t switch_api_port_max_ppg_get(switch_device_t device,
                                            switch_handle_t port_handle,
                                            switch_uint8_t *num_ppgs) {
  SWITCH_MT_WRAP(
      switch_api_port_max_ppg_get_internal(device, port_handle, num_ppgs))
}

switch_status_t switch_api_port_pfc_priority_to_ppg_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_port_pfc_priority_to_ppg_set_internal(
      device, port_handle, qos_map_handle))
}

switch_status_t switch_api_port_pfc_priority_to_ppg_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_port_pfc_priority_to_ppg_get_internal(
      device, port_handle, qos_map_handle))
}

switch_status_t switch_api_port_pfc_priority_to_queue_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_port_pfc_priority_to_queue_set_internal(
      device, port_handle, qos_map_handle))
}

switch_status_t switch_api_port_pfc_priority_to_queue_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_port_pfc_priority_to_queue_get_internal(
      device, port_handle, qos_map_handle))
}

switch_status_t switch_api_port_qos_scheduler_group_handles_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *group_handles) {
  SWITCH_MT_WRAP(switch_api_port_qos_scheduler_group_handles_get_internal(
      device, port_handle, group_handles))
}

switch_status_t switch_api_port_queue_scheduler_group_handle_count_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t *count) {
  SWITCH_MT_WRAP(
      switch_api_port_queue_scheduler_group_handle_count_get_internal(
          device, port_handle, count))
}

switch_status_t switch_api_port_ingress_mirror_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle) {
  SWITCH_MT_WRAP(switch_api_port_ingress_mirror_set_internal(
      device, port_handle, mirror_handle))
}

switch_status_t switch_api_port_ingress_mirror_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle) {
  SWITCH_MT_WRAP(switch_api_port_ingress_mirror_get_internal(
      device, port_handle, mirror_handle))
}

switch_status_t switch_api_port_egress_mirror_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle) {
  SWITCH_MT_WRAP(switch_api_port_egress_mirror_set_internal(
      device, port_handle, mirror_handle))
}

switch_status_t switch_api_port_egress_mirror_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle) {
  SWITCH_MT_WRAP(switch_api_port_egress_mirror_get_internal(
      device, port_handle, mirror_handle))
}

switch_status_t switch_api_port_ingress_sflow_handle_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_sflow_handle) {
  SWITCH_MT_WRAP(switch_api_port_ingress_sflow_handle_get_internal(
      device, port_handle, ingress_sflow_handle))
}

switch_status_t switch_api_port_egress_sflow_handle_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *egress_sflow_handle) {
  SWITCH_MT_WRAP(switch_api_port_egress_sflow_handle_get_internal(
      device, port_handle, egress_sflow_handle))
}

switch_status_t switch_api_port_speed_set(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_port_speed_t speed) {
  SWITCH_MT_WRAP(
      switch_api_port_speed_set_internal(device, port_handle, speed));
}

switch_status_t switch_api_port_scheduler_profile_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t scheduler_handle) {
  SWITCH_MT_WRAP(switch_api_port_scheduler_profile_set_internal(
      device, port_handle, scheduler_handle))
}

switch_status_t switch_api_port_scheduler_profile_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *scheduler_handle) {
  SWITCH_MT_WRAP(switch_api_port_scheduler_profile_get_internal(
      device, port_handle, scheduler_handle));
}

switch_status_t switch_api_port_lane_list_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lane_list_t *lane_list) {
  SWITCH_MT_WRAP(
      switch_api_port_lane_list_get_internal(device, port_handle, lane_list));
}

switch_status_t switch_api_ppg_create(switch_device_t device,
                                      switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_ppg_create_internal(device, port_handle));
}

switch_status_t switch_api_ppg_delete(switch_device_t device,
                                      switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_ppg_delete_internal(device, port_handle));
}

switch_status_t switch_api_port_ppg_drop_get(const switch_device_t device,
                                             const switch_handle_t port_handle,
                                             uint64_t *drop_count) {
  SWITCH_MT_WRAP(
      switch_api_port_ppg_drop_get_internal(device, port_handle, drop_count));
}

switch_status_t switch_api_port_queue_drop_get(
    const switch_device_t device,
    const switch_handle_t port_handle,
    uint64_t *drop_count) {
  SWITCH_MT_WRAP(
      switch_api_port_queue_drop_get_internal(device, port_handle, drop_count));
}

switch_status_t switch_api_interface_port_stats_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_interface_port_stats_get_internal(
      device, port_handle, num_entries, counter_id, counters));
}
