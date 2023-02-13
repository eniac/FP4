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

#include "switchapi/switch_meter.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_METER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Routine Description:
 *   @brief add meter default entries
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_meter_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_meter_index_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter default entry add failed on device %d "
        "meter index table default entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_meter_action_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter default entry add failed on device %d:"
        "meter action table default entry add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("meter default entries added on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete meter default entries
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_meter_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_meter_index_table_default_entry_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter default entry delete failed on device %d "
        "meter index table default entry delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_meter_action_table_default_entry_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter default entry delete failed on device %d: %s"
        "meter action table default entry delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("meter default entries deleted on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief initialize meter structs
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_meter_init(switch_device_t device) {
  switch_size_t meter_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_METER_INDEX, &meter_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter init failed on device %d: "
        "meter index table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_METER, meter_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter init failed on device %d: "
        "meter handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("meter init done successfully on device %d\n", device);

  SWITCH_LOG_EXIT();

  return SWITCH_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *   @brief  free meter structs
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_meter_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_METER
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_METER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter free failed for device %d: "
        "meter handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("meter free done successfully on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief create a meter
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] api_meter_info - meter parameters (cir, pir, cbs, pbs)
 *   @param[out] meter_handle - meter handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_meter_create_internal(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle) {
  switch_meter_info_t *meter_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(meter_handle && api_meter_info);
  if (!meter_handle || !api_meter_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "meter create failed for device %d: "
        "parameters null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_meter_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "meter create failed for device %d: "
        "meter handle create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "meter create failed for device %d: "
        "meter handle get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      &meter_info->api_meter_info, api_meter_info, sizeof(switch_api_meter_t));

  if (api_meter_info->meter_mode != SWITCH_METER_MODE_STORM_CONTROL) {
    status =
        switch_pd_meter_index_table_entry_add(device,
                                              handle_to_id(handle),
                                              meter_info,
                                              &meter_info->meter_idx_pd_hdl);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "meter create failed for device %d: "
          "meter index table add failed(%s)\n",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }
  }
  status = switch_pd_meter_action_table_entry_add(
      device, handle_to_id(handle), meter_info, meter_info->action_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter create failed for device %d: "
        "meter action table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    status = SWITCH_STATUS_SUCCESS;
    *meter_handle = handle;
    goto cleanup;
  }

  SWITCH_LOG_DEBUG(
      "meter created on device %d meter handle 0x%lx\n", device, meter_handle);

  SWITCH_LOG_DETAIL(
      "meter created on device %d meter handle 0x%lx "
      "mode %s type %s source %s "
      "cbs %" PRId64 " pbs %" PRId64 " cir %" PRId64 " pir %" PRId64
      " "
      "action green(%s) yellow(%s) red(%s)\n",
      device,
      handle,
      switch_meter_mode_to_string(api_meter_info->meter_mode),
      switch_meter_type_to_string(api_meter_info->meter_type),
      switch_meter_color_source_to_string(api_meter_info->color_source),
      api_meter_info->cbs,
      api_meter_info->pbs,
      api_meter_info->cir,
      api_meter_info->pir,
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_GREEN]),
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_YELLOW]),
      switch_packet_action_to_string(api_meter_info->action[SWITCH_COLOR_RED]));

  *meter_handle = handle;

cleanup:

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief update a meter
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] meter_handle - meter handle
 *   @param[in] flags - meter attributes
 *   @param[in] api_meter_info - meter parameters (cir, pir, cbs, pbs)
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_meter_update_internal(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint64_t flags,
    const switch_api_meter_t *api_meter_info) {
  switch_meter_info_t *meter_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!api_meter_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "meter update failed on device %d handle 0x%lx: "
        "parameters null(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "meter update failed on device %d handle 0x%lx: "
        "meter handle invalid(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter update failed on device %d handle 0x%lx: "
        "meter get failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_METER_ATTR_CBS) {
    meter_info->api_meter_info.cbs = api_meter_info->cbs;
  }

  if (flags & SWITCH_METER_ATTR_PBS) {
    meter_info->api_meter_info.pbs = api_meter_info->pbs;
  }

  if (flags & SWITCH_METER_ATTR_CIR) {
    meter_info->api_meter_info.cir = api_meter_info->cir;
  }

  if (flags & SWITCH_METER_ATTR_PIR) {
    meter_info->api_meter_info.pir = api_meter_info->pir;
  }

  if (api_meter_info->meter_mode != SWITCH_METER_MODE_STORM_CONTROL) {
    status =
        switch_pd_meter_index_table_entry_update(device,
                                                 handle_to_id(meter_handle),
                                                 meter_info,
                                                 meter_info->meter_idx_pd_hdl);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "meter update failed on device %d handle 0x%lx: "
          "meter index table update failed(%s)\n",
          device,
          meter_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  status = switch_pd_meter_action_table_entry_update(device,
                                                     handle_to_id(meter_handle),
                                                     meter_info,
                                                     meter_info->action_pd_hdl);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter update failed on device %d handle 0x%lx: "
        "meter action table update failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG(
      "meter updated on device %d meter handle 0x%lx\n", device, meter_handle);

  if (meter_info->meter_type == SWITCH_METER_TYPE_COPP) {
    status = switch_pd_hostif_meter_set(
        device, handle_to_id(meter_handle), meter_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("meter update failed on device %d handle 0x%lx",
                       "copp meter update failed",
                       device,
                       meter_handle);
      goto cleanup;
    }
  }

  SWITCH_LOG_DETAIL(
      "meter updated on device %d meter handle 0x%lx "
      "mode %s type %s source %s "
      "cbs %" PRId64 " pbs %" PRId64 " cir %" PRId64 " pir %" PRId64
      " "
      "action green(%s) yellow(%s) red(%s)\n",
      device,
      meter_handle,
      switch_meter_mode_to_string(api_meter_info->meter_mode),
      switch_meter_type_to_string(api_meter_info->meter_type),
      switch_meter_color_source_to_string(api_meter_info->color_source),
      api_meter_info->cbs,
      api_meter_info->pbs,
      api_meter_info->cir,
      api_meter_info->pir,
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_GREEN]),
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_YELLOW]),
      switch_packet_action_to_string(api_meter_info->action[SWITCH_COLOR_RED]));

  SWITCH_LOG_EXIT();

  return status;

cleanup:

  return status;
}

/*
 * Routine Description:
 *   @brief delete a meter
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] meter_handle - meter handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_meter_delete_internal(
    const switch_device_t device, const switch_handle_t meter_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t *meter_info = NULL;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "meter delete failed on device %d meter handle 0x%lx: "
        "meter handle invalid(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter delete failed on device %d meter handle 0x%lx: "
        "meter get failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  if (meter_info->api_meter_info.meter_mode !=
      SWITCH_METER_MODE_STORM_CONTROL) {
    status = switch_pd_meter_index_table_entry_delete(
        device, meter_info->meter_idx_pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "meter delete failed on device %d meter handle 0x%lx: "
          "meter index table delete failed(%s)\n",
          device,
          meter_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_pd_meter_action_table_entry_delete(device,
                                                     meter_info->action_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter delete failed on device %d meter handle 0x%lx: "
        "meter action table delete failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_handle_delete(device, meter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter delete failed on device %d meter handle 0x%lx: "
        "meter handle delete failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "meter deleted on device %d meter handle 0x%lx\n", device, meter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief get a meter
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] meter_handle - meter handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_meter_get_internal(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    switch_api_meter_t *api_meter_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t *meter_info = NULL;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "meter get failed on device %d meter handle 0x%lx: "
        "meter handle invalid(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter get failed on device %d meter handle 0x%lx: "
        "meter get failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      api_meter_info, &meter_info->api_meter_info, sizeof(switch_api_meter_t));

  SWITCH_LOG_DEBUG(
      "meter get on device %d meter handle 0x%lx\n", device, meter_handle);

  SWITCH_LOG_DETAIL(
      "meter get on device %d meter handle 0x%lx "
      "mode %s type %s source %s "
      "cbs %" PRId64 " pbs %" PRId64 " cir %" PRId64 " pir %" PRId64
      " "
      "action green(%s) yellow(%s) red(%s)\n",
      device,
      meter_handle,
      switch_meter_mode_to_string(api_meter_info->meter_mode),
      switch_meter_type_to_string(api_meter_info->meter_type),
      switch_meter_color_source_to_string(api_meter_info->color_source),
      api_meter_info->cbs,
      api_meter_info->pbs,
      api_meter_info->cir,
      api_meter_info->pir,
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_GREEN]),
      switch_packet_action_to_string(
          api_meter_info->action[SWITCH_COLOR_YELLOW]),
      switch_packet_action_to_string(api_meter_info->action[SWITCH_COLOR_RED]));

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief get a meter
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] meter_handle - meter handle
 *   @param[in] num_counters - number of counters
 *   @param[in] counters_ids - meter counter ids
 *   @param[out] counters - counter values
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_meter_counters_get_internal(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint8_t num_counters,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters) {
  switch_meter_info_t *meter_info = NULL;
  switch_meter_counter_t counter_id = 0;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);

  if (!counter_ids || !counters) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "meter counter get failed on device %d meter handle 0x%lx: "
        "parameters null(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "meter counter get failed on device %d meter handle 0x%lx: "
        "meter handle invalid(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter counter get failed on device %d meter handle 0x%lx: "
        "meter get failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_meter_counters_get(device, meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter counter get failed on device %d meter handle 0x%lx: "
        "meter pd counters get failed(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < num_counters; index++) {
    counter_id = counter_ids[index];
    counters[index] = meter_info->counters[counter_id];
    SWITCH_LOG_DETAIL(
        "meter counter get on device %d meter handle 0x%lx "
        "counter %s value 0x%lx\n",
        device,
        meter_handle,
        switch_meter_counter_id_to_string(counter_id),
        counters[index]);
  }

  SWITCH_LOG_DEBUG("meter counters get on device %d meter handle 0x%lx\n",
                   device,
                   meter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_meter_counters_get(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint8_t count,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_meter_counters_get_internal(
      device, meter_handle, count, counter_ids, counters))
}

switch_status_t switch_api_meter_update(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint64_t flags,
    const switch_api_meter_t *api_meter_info) {
  SWITCH_MT_WRAP(switch_api_meter_update_internal(
      device, meter_handle, flags, api_meter_info))
}

switch_status_t switch_api_meter_delete(const switch_device_t device,
                                        const switch_handle_t meter_handle) {
  SWITCH_MT_WRAP(switch_api_meter_delete_internal(device, meter_handle))
}

switch_status_t switch_api_meter_get(const switch_device_t device,
                                     const switch_handle_t meter_handle,
                                     switch_api_meter_t *meter_info) {
  SWITCH_MT_WRAP(
      switch_api_meter_get_internal(device, meter_handle, meter_info))
}

switch_status_t switch_api_meter_create(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle) {
  SWITCH_MT_WRAP(
      switch_api_meter_create_internal(device, api_meter_info, meter_handle))
}
