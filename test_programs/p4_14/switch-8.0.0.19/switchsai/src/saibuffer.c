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

#include <saibuffer.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_port.h>
#include <switchapi/switch_queue.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_buffer.h>

static sai_api_t api_id = SAI_API_BUFFER;
/**
 * @brief Set ingress priority group attribute
 * @param[in] ingress_pg_id ingress priority group id
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_set_ingress_priority_group_attribute(
    _In_ sai_object_id_t ingress_pg_id, _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(ingress_pg_id) ==
             SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP);
  SAI_ASSERT(sai_object_type_query(attr->value.oid) ==
             SAI_OBJECT_TYPE_BUFFER_PROFILE);

  if (attr->id == SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE) {
    status = switch_api_priority_group_buffer_profile_set(
        device, ingress_pg_id, attr->value.oid);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to set pg buffer profile :%s",
                    sai_status_to_string(status));
    }
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get ingress priority group attributes
 * @param[in] ingress_pg_id ingress priority group id
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_get_ingress_priority_group_attribute(
    _In_ sai_object_id_t ingress_pg_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  uint32_t i = 0;
  sai_attribute_t *sai_attr;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t ppg_index = 0;

  for (i = 0, sai_attr = attr_list; i < attr_count; i++, sai_attr++) {
    switch (sai_attr->id) {
      case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
        switch_status = switch_api_priority_group_buffer_profile_get(
            device, ingress_pg_id, &handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get buffer profile for ingress pg handl 0x%lx: %s",
              ingress_pg_id,
              sai_status_to_string(status));
          return status;
        }
        sai_attr->value.oid = handle;
        break;

      case SAI_INGRESS_PRIORITY_GROUP_ATTR_PORT:
        switch_status =
            switch_api_priority_group_port_get(device, ingress_pg_id, &handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get buffer profile for ingress pg handl 0x%lx: %s",
              ingress_pg_id,
              sai_status_to_string(status));
          return status;
        }
        sai_attr->value.oid = handle;
        break;

      case SAI_INGRESS_PRIORITY_GROUP_ATTR_INDEX:
        switch_status = switch_api_priority_group_index_get(
            device, ingress_pg_id, &ppg_index);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get buffer profile for ingress pg handl 0x%lx: %s",
              ingress_pg_id,
              sai_status_to_string(status));
          return status;
        }
        sai_attr->value.u32 = ppg_index;
        break;
      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get ingress priority group statistics counters.
 *
 * @param[in] ingress_priority_group_id Ingress priority group id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_ingress_priority_group_stats(
    _In_ sai_object_id_t ingress_pg_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_ingress_priority_group_stat_t *counter_ids,
    _Out_ uint64_t *counters) {
  SAI_LOG_ENTER();

  uint32_t index = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(ingress_pg_id) ==
             SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP);

  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_INGRESS_PRIORITY_GROUP_STAT_DROPPED_PACKETS:
        switch_status =
            switch_api_ppg_drop_get(device, ingress_pg_id, &counters[index]);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("ppg drop count get failed for ppg 0x%lx\n",
                        ingress_pg_id);
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

static void sai_buffer_pool_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ switch_api_buffer_pool_t *api_buffer_pool) {
  const sai_attribute_t *attribute;
  uint32_t i = 0;

  api_buffer_pool->threshold_mode = SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;
  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
        break;
      case SAI_BUFFER_POOL_ATTR_TYPE:
        if (attribute->value.u32 == SAI_BUFFER_POOL_TYPE_INGRESS) {
          api_buffer_pool->direction = SWITCH_API_DIRECTION_INGRESS;
        } else {
          api_buffer_pool->direction = SWITCH_API_DIRECTION_EGRESS;
        }
        break;
      case SAI_BUFFER_POOL_ATTR_SIZE:
        api_buffer_pool->pool_size = attribute->value.u32;
        break;
      case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
        if (attribute->value.u32 == SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC) {
          api_buffer_pool->threshold_mode = SWITCH_BUFFER_THRESHOLD_MODE_STATIC;
        } else {
          api_buffer_pool->threshold_mode =
              SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;
        }
        break;
      case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
        api_buffer_pool->xoff_size = attribute->value.u32;
        break;

      default:
        break;
    }
  }
}

/**
 * @brief Create buffer pool
 * @param[out] pool_id buffer pool id
 * @param[in] switch_id Switch id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_create_buffer_pool(_Out_ sai_object_id_t *pool_id,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_buffer_pool_t api_buffer_pool;
  switch_handle_t pool_handle = SWITCH_API_INVALID_HANDLE;
  *pool_id = SAI_NULL_OBJECT_ID;

  memset(&api_buffer_pool, 0, sizeof(switch_api_buffer_pool_t));
  sai_buffer_pool_attribute_parse(attr_count, attr_list, &api_buffer_pool);

  switch_status =
      switch_api_buffer_pool_create(device, api_buffer_pool, &pool_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create buffer pool: %s",
                  sai_status_to_string(status));
  }

  *pool_id = pool_handle;
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Remove buffer pool
 * @param[in] pool_id buffer pool id
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_remove_buffer_pool(_In_ sai_object_id_t pool_id) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(pool_id) == SAI_OBJECT_TYPE_BUFFER_POOL);

  switch_status =
      switch_api_buffer_pool_delete(device, (switch_handle_t)pool_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete buffer pool: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Set buffer pool attribute
 * @param[in] pool_id buffer pool id
 * @param[in] attr attribute
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_set_buffer_pool_attribute(_In_ sai_object_id_t pool_id,
                                           _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(pool_id) == SAI_OBJECT_TYPE_BUFFER_POOL);

  switch (attr->id) {
    case SAI_BUFFER_POOL_ATTR_SIZE:
      switch_status = switch_api_buffer_pool_size_set(
          device, (switch_handle_t)pool_id, (switch_uint32_t)attr->value.u32);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set size of buffer pool: %s",
                      sai_status_to_string(status));
        return status;
      }
      break;

    case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
      switch_status = switch_api_buffer_pool_xoff_size_set(
          device, (switch_handle_t)pool_id, (switch_uint32_t)attr->value.u32);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set xoff size of buffer pool: %s",
                      sai_status_to_string(status));
        return status;
      }
      break;

    default:
      break;
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get buffer pool attributes
 * @param[in] pool_id buffer pool id
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_get_buffer_pool_attribute(_In_ sai_object_id_t pool_id,
                                           _In_ uint32_t attr_count,
                                           _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *sai_attr;
  switch_buffer_threshold_mode_t threshold_mode;
  switch_uint32_t pool_size = 0;
  switch_uint32_t xoff_size = 0;
  switch_direction_t dir;
  uint32_t i;

  SAI_ASSERT(sai_object_type_query(pool_id) == SAI_OBJECT_TYPE_BUFFER_POOL);
  for (i = 0, sai_attr = attr_list; i < attr_count; i++, sai_attr++) {
    switch (sai_attr->id) {
      case SAI_BUFFER_POOL_ATTR_TYPE:
        switch_status = switch_api_buffer_pool_type_get(device, pool_id, &dir);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get buffer pool size for pool handle 0x%lx",
                        pool_id);
          return status;
        }
        sai_attr->value.u32 = (dir == SWITCH_API_DIRECTION_INGRESS)
                                  ? SAI_BUFFER_POOL_TYPE_INGRESS
                                  : SAI_BUFFER_POOL_TYPE_EGRESS;
        break;

      case SAI_BUFFER_POOL_ATTR_SIZE:
        switch_status =
            switch_api_buffer_pool_size_get(device, pool_id, &pool_size);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get buffer pool size for pool handle 0x%lx",
                        pool_id);
          return status;
        }
        sai_attr->value.u32 = pool_size;
        break;

      case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
        switch_status = switch_api_buffer_pool_threshold_mode_get(
            device, pool_id, &threshold_mode);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get buffer pool threshold mode for pool handle 0x%lx",
              pool_id);
          return status;
        }
        sai_attr->value.u32 =
            (threshold_mode == SWITCH_BUFFER_THRESHOLD_MODE_STATIC)
                ? SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC
                : SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;
        break;
      case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
        switch_status =
            switch_api_buffer_pool_xoff_size_get(device, pool_id, &xoff_size);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get xoff size for pool handle 0x%lx",
                        pool_id);
          return status;
        }
        sai_attr->value.u32 = xoff_size;
        break;

      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get buffer pool statistics counters.
 *
 * @param[in] buffer_pool_id Buffer pool id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_buffer_pool_stats(
    _In_ sai_object_id_t buffer_pool_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_buffer_pool_stat_t *counter_ids,
    _Out_ uint64_t *counters) {
  uint32_t index = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t co_bytes = 0;
  uint32_t wm_bytes = 0;

  SAI_ASSERT(sai_object_type_query(buffer_pool_id) ==
             SAI_OBJECT_TYPE_BUFFER_POOL);

  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES:
        switch_status = switch_api_buffer_pool_usage_get(
            device, buffer_pool_id, &co_bytes, &wm_bytes);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("pool usage count get failed for pool 0x%lx\n",
                        buffer_pool_id);
        }
        counters[index] = co_bytes;
        break;
      default:
        counters[index] = 0;
        break;
    }
  }

  return status;

  SAI_LOG_EXIT();
}

sai_status_t sai_buffer_profile_get_switch_threshold_mode(
    sai_object_id_t pool_handle,
    sai_buffer_profile_threshold_mode_t th_mode,
    switch_buffer_threshold_mode_t *switch_threshold_mode) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  if (th_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_INHERIT_BUFFER_POOL_MODE) {
    switch_status = switch_api_buffer_pool_threshold_mode_get(
        device, pool_handle, switch_threshold_mode);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR(
          "Failed to get buffer pool threshold mode for pool handle 0x%lx",
          pool_handle);
      return status;
    }
  } else {
    *switch_threshold_mode =
        (th_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC)
            ? SWITCH_BUFFER_THRESHOLD_MODE_STATIC
            : SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;
  }
  return status;
}

static void sai_buffer_profile_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    switch_api_buffer_profile_t *buffer_profile_info) {
  const sai_attribute_t *attribute;
  uint32_t i = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
        buffer_profile_info->pool_handle = attribute->value.oid;
        break;
      case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
        buffer_profile_info->buffer_size = attribute->value.u32;
        break;
      case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
        buffer_profile_info->threshold = attribute->value.u32;
        buffer_profile_info->threshold_mode =
            SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;
        break;
      case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
        buffer_profile_info->threshold = attribute->value.u32;
        buffer_profile_info->threshold_mode =
            SWITCH_BUFFER_THRESHOLD_MODE_STATIC;
        break;
      case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
        buffer_profile_info->xoff_threshold = attribute->value.u32;
        break;
      case SAI_BUFFER_PROFILE_ATTR_XON_TH:
        buffer_profile_info->xon_threshold = attribute->value.u32;
        break;
      default:
        break;
    }
  }
}

sai_status_t sai_buffer_profile_update_threshold(
    uint32_t attr_count,
    const sai_attribute_t *attr_list,
    switch_api_buffer_profile_t *profile_info) {
  const sai_attribute_t *attribute;
  switch_buffer_threshold_mode_t threshold_mode;
  uint32_t i = 0;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_buffer_profile_threshold_mode_t sai_threshold_mode =
      SAI_BUFFER_PROFILE_THRESHOLD_MODE_INHERIT_BUFFER_POOL_MODE;

  SAI_ASSERT(profile_info->pool_handle != SAI_NULL_OBJECT_ID);

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    if (attribute->id == SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE) {
      sai_threshold_mode = attribute->value.u32;
      profile_info->threshold_mode = sai_threshold_mode;
      return status;
    }
  }

  /*
   * Default buffer profile mode is INHERIT_MODE from buffer pool.
   */
  status = sai_buffer_profile_get_switch_threshold_mode(
      profile_info->pool_handle, sai_threshold_mode, &threshold_mode);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to get threshold mode for buffer pool handle 0x%lx: %s",
        profile_info->pool_handle,
        sai_status_to_string(status));
    return status;
  }
  SAI_ASSERT(profile_info->threshold_mode == threshold_mode);
  return status;
}

/**
 * @brief Create buffer profile
 * @param[out] buffer_profile_id buffer profile id
 * @param[in] switch_id Switch id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_create_buffer_profile(_Out_ sai_object_id_t *buffer_profile_id,
                                       _In_ sai_object_id_t switch_id,
                                       _In_ uint32_t attr_count,
                                       _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_buffer_profile_t buffer_profile_info;
  switch_handle_t buffer_profile_handle = SWITCH_API_INVALID_HANDLE;
  *buffer_profile_id = SAI_NULL_OBJECT_ID;

  memset(&buffer_profile_info, 0x0, sizeof(buffer_profile_info));

  sai_buffer_profile_attribute_parse(
      attr_count, attr_list, &buffer_profile_info);

  /*
   * When SAI buffer threshold mode is INHERIT_BUFFER_POOL_MODE, buffer pool
   * handle
   * is required. So updating the threshold mode in a separate call after
   * retrieving the
   * buffer pool handle from the attribute list.
   */
  status = sai_buffer_profile_update_threshold(
      attr_count, attr_list, &buffer_profile_info);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to update buffer profile threshold: %s",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_buffer_profile_create(
      device, &buffer_profile_info, &buffer_profile_handle);

  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create buffer profile: %s",
                  sai_status_to_string(status));
    return status;
  }

  *buffer_profile_id = buffer_profile_handle;
  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Remove buffer profile
 * @param[in] buffer_profile_id buffer profile id
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_remove_buffer_profile(_In_ sai_object_id_t buffer_profile_id) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(buffer_profile_id) ==
             SAI_OBJECT_TYPE_BUFFER_PROFILE);

  switch_status = switch_api_buffer_profile_delete(device, buffer_profile_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove buffer profile: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Set buffer profile attribute
 * @param[in] buffer_profile_id buffer profile id
 * @param[in] attr attribute
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_set_buffer_profile_attribute(
    _In_ sai_object_id_t buffer_profile_id, _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_buffer_profile_t buffer_profile_info;
  memset(&buffer_profile_info, 0x0, sizeof(buffer_profile_info));

  SAI_ASSERT(sai_object_type_query(buffer_profile_id) ==
             SAI_OBJECT_TYPE_BUFFER_PROFILE);

  sai_buffer_profile_attribute_parse(1, attr, &buffer_profile_info);

  if (attr->id == SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE) {
    sai_buffer_profile_update_threshold(1, attr, &buffer_profile_info);
  }

  switch_status = switch_api_buffer_profile_info_set(
      device, buffer_profile_id, &buffer_profile_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to update buffer profile attributes for profile_handle 0x%lx: "
        "%s",
        buffer_profile_id,
        sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return status;
}

/**
 * @brief Get buffer profile attributes
 * @param[in] buffer_profile_id buffer profile id
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list array of attributes
 * @return SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_get_buffer_profile_attribute(
    _In_ sai_object_id_t buffer_profile_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_buffer_profile_t buffer_profile_info;
  uint32_t i = 0;
  sai_attribute_t *sai_attr;

  memset(&buffer_profile_info, 0x0, sizeof(buffer_profile_info));

  SAI_ASSERT(sai_object_type_query(buffer_profile_id) ==
             SAI_OBJECT_TYPE_BUFFER_PROFILE);

  switch_status = switch_api_buffer_profile_info_get(
      device, buffer_profile_id, &buffer_profile_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to get buffer profile info for profile handle 0x%lx: %s",
        buffer_profile_id,
        sai_status_to_string(status));
    return status;
  }

  for (i = 0, sai_attr = attr_list; i < attr_count; i++, sai_attr++) {
    switch (sai_attr->id) {
      case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
        sai_attr->value.oid = buffer_profile_info.pool_handle;
        break;

      case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
        sai_attr->value.u32 = buffer_profile_info.buffer_size;
        break;

      case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
      case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
        sai_attr->value.u32 = buffer_profile_info.threshold;
        break;

      case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
        sai_attr->value.u32 = buffer_profile_info.xoff_threshold;
        break;

      case SAI_BUFFER_PROFILE_ATTR_XON_TH:
        sai_attr->value.u32 = buffer_profile_info.xon_threshold;
        break;

      case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
        sai_attr->value.u32 = (buffer_profile_info.threshold_mode ==
                               SWITCH_BUFFER_THRESHOLD_MODE_STATIC)
                                  ? SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC
                                  : SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC;
        break;

      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return status;
}

sai_status_t sai_buffer_profiles_set(switch_handle_t port_handle,
                                     switch_handle_t ingress_profile_handle,
                                     switch_handle_t egress_profile_handle) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_uint32_t max_queues = 0;
  switch_handle_t *queue_handles = NULL;
  switch_handle_t *ppg_handles = NULL;
  switch_uint8_t num_ppgs = 0;
  switch_uint32_t index = 0;

  switch_status =
      switch_api_port_max_queues_get(device, port_handle, &max_queues);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get max queues for port %d : %s",
                  port_handle & 0xFFFF,
                  sai_status_to_string(status));
    return status;
  }
  queue_handles = SAI_MALLOC(sizeof(switch_handle_t) * max_queues);
  memset(queue_handles, 0, sizeof(switch_handle_t) * max_queues);
  if (!queue_handles) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("Failed to create list of queue handles: %s",
                  sai_status_to_string(status));
    return status;
  }
  switch_status =
      switch_api_queues_get(device, port_handle, &max_queues, queue_handles);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get queue handles for port 0x%lx: %s",
                  (switch_handle_t)port_handle,
                  sai_status_to_string(status));
    SAI_FREE(queue_handles);
    return status;
  }

  for (index = 0; index < max_queues; index++) {
    if (queue_handles[index] == SAI_NULL_OBJECT_ID) {
      break;
    }
    status = switch_api_queue_buffer_profile_set(
        device, queue_handles[index], egress_profile_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SAI_LOG_ERROR("Failed to set buffer profile for queue 0x%lx",
                    queue_handles[index]);
      SAI_FREE(queue_handles);
      return status;
    }
  }

  SAI_FREE(queue_handles);

  switch_status = switch_api_port_max_ppg_get(device, port_handle, &num_ppgs);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get port's max PPG for port 0x%lx: %s",
                  port_handle,
                  sai_status_to_string(status));
    return status;
  }
  if (num_ppgs == 0) {
    SAI_LOG_ERROR("Num ppg is zero, may be internal ports");
    return status;
  }

  ppg_handles =
      (switch_handle_t *)SAI_MALLOC(num_ppgs * sizeof(switch_handle_t));
  switch_status =
      switch_api_port_ppg_get(device, port_handle, &num_ppgs, ppg_handles);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get port's PPG handles for port 0x%lx: %s",
                  port_handle,
                  sai_status_to_string(status));
    SAI_FREE(ppg_handles);
    return status;
  }
  for (index = 0; index < num_ppgs; index++) {
    status = switch_api_priority_group_buffer_profile_set(
        device, ppg_handles[index], ingress_profile_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SAI_LOG_ERROR("Failed to set buffer profile to PPG 0x%lx",
                    ppg_handles[index]);
      SAI_FREE(ppg_handles);
      return status;
    }
  }
  SAI_FREE(ppg_handles);
  return status;
}

/*
 * Configure default buffer pool size as 100,000 cells.
 */
#define SAI_SWITCH_INGRESS_DEFAULT_BUFFER_SIZE (100000 * 80)
#define SAI_SWITCH_EGRESS_DEFAULT_BUFFER_SIZE (100000 * 80)
#define SAI_SWITCH_DEFAULT_BUFFER_THRESHOLD 0xff
/*
*  Buffer methods table retrieved with sai_api_query()
*/
sai_buffer_api_t buffer_api = {
    .create_buffer_pool = sai_create_buffer_pool,
    .remove_buffer_pool = sai_remove_buffer_pool,
    .set_buffer_pool_attribute = sai_set_buffer_pool_attribute,
    .get_buffer_pool_attribute = sai_get_buffer_pool_attribute,
    .get_buffer_pool_stats = sai_get_buffer_pool_stats,
    .set_ingress_priority_group_attribute =
        sai_set_ingress_priority_group_attribute,
    .get_ingress_priority_group_attribute =
        sai_get_ingress_priority_group_attribute,
    .get_ingress_priority_group_stats = sai_get_ingress_priority_group_stats,
    .create_buffer_profile = sai_create_buffer_profile,
    .remove_buffer_profile = sai_remove_buffer_profile,
    .set_buffer_profile_attribute = sai_set_buffer_profile_attribute,
    .get_buffer_profile_attribute = sai_get_buffer_profile_attribute};

sai_status_t sai_buffer_initialize(sai_api_service_t *sai_api_service) {
  switch_api_device_info_t api_info;
  switch_uint64_t flags = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t port_index = 0;

  switch_handle_t ingress_profile_handle, egress_profile_handle;
  switch_handle_t ingress_pool_handle, egress_pool_handle;
  switch_api_buffer_profile_t buffer_profile;
  switch_api_buffer_pool_t buffer_pool;
  switch_handle_t cpu_port_handle;

  sai_status_t status = SAI_STATUS_SUCCESS;

  /*
   * SAI expects a default buffer pools and profiles for each queue and PPG.
   */

  for (int i = 0; i < 2; i++) {
    SWITCH_MEMSET(&buffer_pool, 0, sizeof(switch_api_buffer_pool_t));
    SWITCH_MEMSET(&buffer_profile, 0, sizeof(switch_api_buffer_profile_t));

    /*
     * Use the default buffer pool size from switchapi.
     */
    buffer_pool.threshold_mode = SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;

    buffer_profile.threshold_mode = SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC;
    buffer_profile.threshold = SAI_SWITCH_DEFAULT_BUFFER_THRESHOLD;

    if (i == 0) {
      buffer_pool.direction = SWITCH_API_DIRECTION_INGRESS;
      buffer_pool.pool_size = SAI_SWITCH_INGRESS_DEFAULT_BUFFER_SIZE;
      buffer_profile.buffer_size = SAI_SWITCH_INGRESS_DEFAULT_BUFFER_SIZE;
      status = switch_api_buffer_pool_create(
          device, buffer_pool, &ingress_pool_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Default ingress buffer pool create failed");
        return status;
      }

      buffer_profile.pool_handle = ingress_pool_handle;
      status = switch_api_buffer_profile_create(
          device, &buffer_profile, &ingress_profile_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Default ingress buffer profile create failed");
        return status;
      }
    } else {
      buffer_profile.buffer_size = SAI_SWITCH_EGRESS_DEFAULT_BUFFER_SIZE;
      buffer_pool.direction = SWITCH_API_DIRECTION_EGRESS;
      buffer_pool.pool_size = SAI_SWITCH_EGRESS_DEFAULT_BUFFER_SIZE;
      status = switch_api_buffer_pool_create(
          device, buffer_pool, &egress_pool_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Default egress buffer pool create failed");
        return status;
      }
      buffer_profile.pool_handle = egress_pool_handle;
      status = switch_api_buffer_profile_create(
          device, &buffer_profile, &egress_profile_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Default egress buffer profile create failed");
        return status;
      }
    }
  }
  memset(&api_info, 0x0, sizeof(switch_api_device_info_t));

  flags = (SWITCH_DEVICE_ATTR_MAX_PORTS | SWITCH_DEVICE_ATTR_PORT_LIST);

  switch_status = switch_api_device_attribute_get(device, flags, &api_info);
  for (port_index = 0; port_index < api_info.port_list.num_handles;
       port_index++) {
    status = sai_buffer_profiles_set(api_info.port_list.handles[port_index],
                                     ingress_profile_handle,
                                     egress_profile_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("Failed to set queue/buffer profiles for the port 0x%lx",
                    api_info.port_list.handles[port_index]);
      return status;
    }
  }
  switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);
  status = sai_buffer_profiles_set(
      cpu_port_handle, ingress_profile_handle, egress_profile_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to set queue/buffer profiles for cpu port");
    return status;
  }

  SAI_LOG_DEBUG("Initializing buffer");
  sai_api_service->buffer_api = buffer_api;
  return SAI_STATUS_SUCCESS;
}
