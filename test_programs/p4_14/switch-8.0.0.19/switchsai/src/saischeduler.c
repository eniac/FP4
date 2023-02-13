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

#include <saischeduler.h>
#include "saiinternal.h"
#include <switchapi/switch_scheduler.h>

static sai_api_t api_id = SAI_API_SCHEDULER;

switch_scheduler_type_t sai_scheduler_type_to_switch_type(uint32_t sch_type) {
  switch (sch_type) {
    case SAI_SCHEDULING_TYPE_STRICT:
      return SWITCH_SCHEDULER_MODE_STRICT;
    case SAI_SCHEDULING_TYPE_DWRR:
      return SWITCH_SCHEDULER_MODE_DWRR;
    default:
      SAI_LOG_ERROR("Unsupported scheduler type");
      return SWITCH_SCHEDULER_MODE_DWRR;
  }
}

sai_scheduling_type_t switch_scheduler_type_to_sai_type(
    switch_scheduler_type_t sch_type) {
  switch (sch_type) {
    case SWITCH_SCHEDULER_MODE_STRICT:
      return SAI_SCHEDULING_TYPE_STRICT;
    case SWITCH_SCHEDULER_MODE_DWRR:
      return SAI_SCHEDULING_TYPE_DWRR;
    default:
      SAI_LOG_ERROR("Unsupported scheduler type");
      return SAI_SCHEDULING_TYPE_DWRR;
  }
}

static void sai_scheduler_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    switch_scheduler_api_info_t *api_info) {
  const sai_attribute_t *attribute;
  uint32_t i = 0;

  /*
   * SAI default types are BPS and DWRR.
   */
  api_info->shaper_type = SWITCH_SCHEDULER_BPS;
  api_info->scheduler_type = SWITCH_SCHEDULER_MODE_DWRR;
  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
        api_info->scheduler_type =
            sai_scheduler_type_to_switch_type(attribute->value.u32);
        break;
      case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
        api_info->weight = attribute->value.u8;
        break;
      case SAI_SCHEDULER_ATTR_METER_TYPE:
        if (attribute->value.u32 == SAI_METER_TYPE_PACKETS) {
          api_info->shaper_type = SWITCH_SCHEDULER_PPS;
        } else {
          api_info->shaper_type = SWITCH_SCHEDULER_BPS;
        }
        break;
      case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
        api_info->min_rate = attribute->value.u64;
        break;
      case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
        api_info->min_burst_size = attribute->value.u64;
        break;
      case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
        api_info->max_rate = attribute->value.u64;
        break;
      case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
        api_info->max_burst_size = attribute->value.u64;
        break;
      default:
        break;
    }
  }
}

static void sai_convert_rate_to_bps(switch_uint64_t *rate_in_bytes) {
  *rate_in_bytes *= 8;
}

static void sai_convert_rate_to_bytes(switch_uint64_t *rate_in_bps) {
  *rate_in_bps /= 8;
}

/**
 * @brief  Create Scheduler Profile
 *
 * @param[out] scheduler_id Scheduler id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_create_scheduler_profile(
    _Out_ sai_object_id_t *scheduler_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_scheduler_api_info_t api_info;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;

  memset(&api_info, 0x0, sizeof(api_info));
  sai_scheduler_attribute_parse(attr_count, attr_list, &api_info);

  if (api_info.shaper_type == SWITCH_SCHEDULER_BPS) {
    /*
     * SAI sends the rate and burst in bytes. Convert that to BPS.
     */
    sai_convert_rate_to_bps(&api_info.min_rate);
    sai_convert_rate_to_bps(&api_info.max_rate);
  }
  switch_status = switch_api_scheduler_create(device, &api_info, &handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create scheduler: %s",
                  sai_status_to_string(status));
    return status;
  }

  *scheduler_id = handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Remove Scheduler profile
 *
 * @param[in] scheduler_id Scheduler id
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_remove_scheduler_profile(_In_ sai_object_id_t scheduler_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(scheduler_id) == SAI_OBJECT_TYPE_SCHEDULER);

  status = switch_api_scheduler_delete(device, scheduler_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove scheduler: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Set Scheduler Attribute
 *
 * @param[in] scheduler_id Scheduler id
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_set_scheduler_attribute(_In_ sai_object_id_t scheduler_id,
                                         _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_api_info_t api_info;

  memset(&api_info, 0, sizeof(switch_scheduler_api_info_t));

  SAI_ASSERT(sai_object_type_query(scheduler_id) == SAI_OBJECT_TYPE_SCHEDULER);
  /*
   * Get the existing scheduler API info and update only
   * the modified attribute.
   */
  switch_api_scheduler_config_get(device, scheduler_id, &api_info);
  sai_scheduler_attribute_parse(1, attr, &api_info);
  if (api_info.shaper_type == SWITCH_SCHEDULER_BPS) {
    /*
     * SAI sends the rate and burst in bytes. Convert that to BPS.
     */
    sai_convert_rate_to_bps(&api_info.min_rate);
    sai_convert_rate_to_bps(&api_info.max_rate);
  }
  switch_status =
      switch_api_scheduler_config_set(device, scheduler_id, &api_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to set scheduler attribute for handle 0x%lx: %s",
                  scheduler_id,
                  sai_status_to_string(status));
    return status;
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Get Scheduler attribute
 *
 * @param[in] scheduler_id - scheduler id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */

sai_status_t sai_get_scheduler_attribute(_In_ sai_object_id_t scheduler_id,
                                         _In_ uint32_t attr_count,
                                         _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_scheduler_api_info_t api_info;
  sai_attribute_t *attr;
  unsigned int i = 0;

  SAI_ASSERT(sai_object_type_query(scheduler_id) == SAI_OBJECT_TYPE_SCHEDULER);
  memset(&api_info, 0, sizeof(switch_scheduler_api_info_t));

  switch_status =
      switch_api_scheduler_config_get(device, scheduler_id, &api_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get scheduler info for handle 0x%lx",
                  scheduler_id);
    return status;
  }
  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
        attr->value.u32 =
            switch_scheduler_type_to_sai_type(api_info.scheduler_type);
        break;
      case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
        attr->value.u32 = api_info.weight;
        break;

      case SAI_SCHEDULER_ATTR_METER_TYPE:
        attr->value.u32 = (api_info.shaper_type == SWITCH_SCHEDULER_PPS)
                              ? SAI_METER_TYPE_PACKETS
                              : SAI_METER_TYPE_BYTES;
        break;

      case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
        sai_convert_rate_to_bytes(&api_info.min_rate);
        attr->value.u64 = api_info.min_rate;
        break;

      case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
        attr->value.u64 = api_info.min_burst_size;
        break;

      case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
        sai_convert_rate_to_bytes(&api_info.max_rate);
        attr->value.u64 = api_info.max_rate;
        break;

      case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
        attr->value.u64 = api_info.max_burst_size;
        break;

      default:
        SAI_LOG_ERROR("Unsupported scheduler attribute %d", attr->id);
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Scheduler methods table retrieved with sai_api_query()
*/
sai_scheduler_api_t scheduler_api = {
    .create_scheduler = sai_create_scheduler_profile,
    .remove_scheduler = sai_remove_scheduler_profile,
    .set_scheduler_attribute = sai_set_scheduler_attribute,
    .get_scheduler_attribute = sai_get_scheduler_attribute};

sai_status_t sai_scheduler_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing scheulder");
  sai_api_service->scheduler_api = scheduler_api;
  return SAI_STATUS_SUCCESS;
}
