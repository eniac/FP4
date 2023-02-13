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

#include <saiqueue.h>
#include "saiinternal.h"
#include <switchapi/switch_queue.h>
#include <switchapi/switch_buffer.h>
#include <switchapi/switch_wred.h>

static sai_api_t api_id = SAI_API_QUEUE;

/**
 * @brief Set attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_set_queue_attribute(_In_ sai_object_id_t queue_id,
                                     _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;

  SAI_ASSERT(sai_object_type_query(queue_id) == SAI_OBJECT_TYPE_QUEUE);
  switch (attr->id) {
    case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
      handle = attr->value.oid;
      switch_status =
          switch_api_queue_buffer_profile_set(device, queue_id, handle);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set buffer profile for queue:%s",
                      sai_status_to_string(status));
        return status;
      }
      break;
    case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
      handle = attr->value.oid;
      switch_status =
          switch_api_queue_wred_profile_set(device, handle, queue_id);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set queue WRED profile for queue:%s",
                      sai_status_to_string(status));
        return status;
      }
      break;
    default:
      break;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Get attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_get_queue_attribute(_In_ sai_object_id_t queue_id,
                                     _In_ uint32_t attr_count,
                                     _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  uint32_t i = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t queue_index = 0;

  SAI_ASSERT(sai_object_type_query(queue_id) == SAI_OBJECT_TYPE_QUEUE);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    memset(&(attr->value), 0, sizeof(attr->value));
    switch (attr->id) {
      case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
        switch_status = switch_api_queue_buffer_profile_get(
            device, (switch_handle_t)queue_id, &handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get queue buffer profile for queue 0x%lx",
                        queue_id);
          return status;
        }
        attr->value.oid = handle;
        break;

      case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
        switch_status = switch_api_queue_wred_profile_get(
            device, (switch_handle_t)queue_id, &handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get queue wred profile for queue 0x%lx",
                        queue_id);
          return status;
        }
        attr->value.oid = handle;
        break;
      case SAI_QUEUE_ATTR_INDEX:
        switch_status = switch_api_queue_index_get(
            device, (switch_handle_t)queue_id, &queue_index);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get queue index for queue handle 0x%lx",
                        queue_id);
          return status;
        }
        attr->value.u8 = queue_index;
        break;
      case SAI_QUEUE_ATTR_PORT:
        switch_status = switch_api_queue_port_get(
            device, (switch_handle_t)queue_id, &handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get queue port handle for queue 0x%lx",
                        queue_id);
          return status;
        }
        attr->value.oid = handle;
        break;
      case SAI_QUEUE_ATTR_TYPE:
        attr->value.s32 = SAI_QUEUE_TYPE_ALL;
        break;

      default:
        break;
    }
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief   Get queue statistics counters.
 *
 * @param[in] queue_id Queue id
 * @param[in] counter_ids specifies the array of counter ids
 * @param[in] number_of_counters number of counters in the array
 * @param[out] counters array of resulting counter values.
 *
 * @return SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
sai_status_t sai_get_queue_stats(_In_ sai_object_id_t queue_id,
                                 _In_ uint32_t number_of_counters,
                                 _In_ const sai_queue_stat_t *counter_ids,
                                 _Out_ uint64_t *counters) {
  SAI_LOG_ENTER();

  uint32_t index = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(queue_id) == SAI_OBJECT_TYPE_QUEUE);

  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_QUEUE_STAT_DROPPED_PACKETS:
        switch_status =
            switch_api_queue_drop_get(device, queue_id, &counters[index]);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("queue drop count get failed for queue 0x%lx\n",
                        queue_id);
        }
        break;
      default:
        counters[index] = 0;
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief   Clear queue statistics counters.
 *
 * @param[in] queue_id Queue id
 * @param[in] counter_ids specifies the array of counter ids
 * @param[in] number_of_counters number of counters in the array
 *
 * @return SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
sai_status_t sai_clear_queue_stats(_In_ sai_object_id_t queue_id,
                                   _In_ uint32_t number_of_counters,
                                   _In_ const sai_queue_stat_t *counter_ids) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(queue_id) == SAI_OBJECT_TYPE_QUEUE);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Queue  methods table retrieved with sai_api_query()
*/
sai_queue_api_t queue_api = {.set_queue_attribute = sai_set_queue_attribute,
                             .get_queue_attribute = sai_get_queue_attribute,
                             .get_queue_stats = sai_get_queue_stats,
                             .clear_queue_stats = sai_clear_queue_stats};

sai_status_t sai_queue_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing queue map");
  sai_api_service->queue_api = queue_api;
  return SAI_STATUS_SUCCESS;
}
