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

#include <saipolicer.h>
#include "saiinternal.h"
#include <switchapi/switch_meter.h>

static sai_api_t api_id = SAI_API_POLICER;

static switch_meter_mode_t sai_meter_mode_to_switch_meter_mode(
    _In_ sai_policer_mode_t policer_mode) {
  switch (policer_mode) {
    case SAI_POLICER_MODE_TR_TCM:
      return SWITCH_METER_MODE_TWO_RATE_THREE_COLOR;
    case SAI_POLICER_MODE_STORM_CONTROL:
      return SWITCH_METER_MODE_STORM_CONTROL;
    case SAI_POLICER_MODE_SR_TCM:
    default:
      return SWITCH_METER_MODE_NONE;
  }
}

sai_policer_mode_t switch_meter_mode_to_sai_meter_mode(
    switch_meter_mode_t meter_mode) {
  switch (meter_mode) {
    case SWITCH_METER_MODE_NONE:
      return SAI_POLICER_MODE_SR_TCM;
    case SWITCH_METER_MODE_TWO_RATE_THREE_COLOR:
      return SAI_POLICER_MODE_TR_TCM;
    case SWITCH_METER_MODE_STORM_CONTROL:
      return SAI_POLICER_MODE_STORM_CONTROL;
    default:
      return SWITCH_METER_MODE_NONE;
  }
}

static switch_meter_color_source_t sai_color_source_to_switch_color_source(
    _In_ sai_policer_color_source_t color_source) {
  switch (color_source) {
    case SAI_POLICER_COLOR_SOURCE_BLIND:
      return SWITCH_METER_COLOR_SOURCE_BLIND;
    case SAI_POLICER_COLOR_SOURCE_AWARE:
      return SWITCH_METER_COLOR_SOURCE_AWARE;
    default:
      return SWITCH_METER_COLOR_SOURCE_NONE;
  }
}

sai_policer_color_source_t switch_color_source_to_sai_color_source(
    switch_meter_color_source_t color_source) {
  switch (color_source) {
    case SWITCH_METER_COLOR_SOURCE_BLIND:
      return SAI_POLICER_COLOR_SOURCE_BLIND;
    case SWITCH_METER_COLOR_SOURCE_AWARE:
      return SAI_POLICER_COLOR_SOURCE_AWARE;
    default:
      return SAI_POLICER_COLOR_SOURCE_BLIND;
  }
}

static switch_meter_type_t sai_meter_type_to_switch_meter_type(
    _In_ sai_meter_type_t meter_type) {
  switch (meter_type) {
    case SAI_METER_TYPE_PACKETS:
      return SWITCH_METER_TYPE_PACKETS;
    case SAI_METER_TYPE_BYTES:
      return SWITCH_METER_TYPE_BYTES;
    default:
      return SWITCH_METER_TYPE_NONE;
  }
}

sai_meter_type_t switch_meter_type_to_sai_meter_type(
    switch_meter_type_t switch_meter) {
  switch (switch_meter) {
    case SWITCH_METER_TYPE_PACKETS:
      return SAI_METER_TYPE_PACKETS;
    case SWITCH_METER_TYPE_BYTES:
      return SAI_METER_TYPE_BYTES;
    default:
      assert(0);
  }
}

sai_status_t sai_policer_attr_parse(_In_ const int attr_count,
                                    _In_ const sai_attribute_t *attr_list,
                                    _Out_ switch_api_meter_t *api_meter_info) {
  int index = 0;
  const sai_attribute_t *attribute = NULL;
  // init the defaults
  api_meter_info->color_source = SAI_POLICER_COLOR_SOURCE_AWARE;
  api_meter_info->action[SWITCH_COLOR_GREEN] =
      sai_packet_action_to_switch_packet_action(SAI_PACKET_ACTION_FORWARD);
  api_meter_info->action[SWITCH_COLOR_YELLOW] =
      sai_packet_action_to_switch_packet_action(SAI_PACKET_ACTION_FORWARD);
  api_meter_info->action[SWITCH_COLOR_RED] =
      sai_packet_action_to_switch_packet_action(SAI_PACKET_ACTION_FORWARD);

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    //    printf("policer attr %d\n", attribute->id);
    switch (attribute->id) {
      case SAI_POLICER_ATTR_METER_TYPE:
        api_meter_info->meter_type =
            sai_meter_type_to_switch_meter_type(attribute->value.s32);
        break;
      case SAI_POLICER_ATTR_MODE:
        api_meter_info->meter_mode =
            sai_meter_mode_to_switch_meter_mode(attribute->value.s32);
        break;
      case SAI_POLICER_ATTR_COLOR_SOURCE:
        api_meter_info->color_source =
            sai_color_source_to_switch_color_source(attribute->value.s32);
        break;
      case SAI_POLICER_ATTR_CBS:
        api_meter_info->cbs = attribute->value.u64;
        break;
      case SAI_POLICER_ATTR_CIR:
        api_meter_info->cir = attribute->value.u64;
        break;
      case SAI_POLICER_ATTR_PBS:
        api_meter_info->pbs = attribute->value.u64;
        break;
      case SAI_POLICER_ATTR_PIR:
        api_meter_info->pir = attribute->value.u64;
        break;
      case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
        api_meter_info->action[SWITCH_COLOR_GREEN] =
            sai_packet_action_to_switch_packet_action(attribute->value.s32);
        break;
      case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
        api_meter_info->action[SWITCH_COLOR_YELLOW] =
            sai_packet_action_to_switch_packet_action(attribute->value.s32);
        break;
      case SAI_POLICER_ATTR_RED_PACKET_ACTION:
        api_meter_info->action[SWITCH_COLOR_RED] =
            sai_packet_action_to_switch_packet_action(attribute->value.s32);
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create Policer
 *
 * @param[out] policer_id - the policer id
 * @param[in] switch_id Switch id
 * @param[in] attr_count - number of attributes
 * @param[in] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
sai_status_t sai_create_policer(_Out_ sai_object_id_t *policer_id,
                                _In_ sai_object_id_t switch_id,
                                _In_ uint32_t attr_count,
                                _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_meter_t api_meter_info = {0};
  switch_handle_t policer_handle = SWITCH_API_INVALID_HANDLE;
  *policer_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&api_meter_info, 0, sizeof(switch_api_meter_t));
  /*
   * Set the default policer action for all the colors as these are not
   * mandatory attributes for SAI policer.
   */
  api_meter_info.action[SWITCH_COLOR_GREEN] = SWITCH_ACL_ACTION_PERMIT;
  api_meter_info.action[SWITCH_COLOR_YELLOW] = SWITCH_ACL_ACTION_PERMIT;
  api_meter_info.action[SWITCH_COLOR_RED] = SWITCH_ACL_ACTION_PERMIT;
  sai_policer_attr_parse(attr_count, attr_list, &api_meter_info);

  // if SR_TCM, copy cir to pir
  if (api_meter_info.meter_mode == SWITCH_METER_MODE_NONE) {
    api_meter_info.pir = api_meter_info.cir;
    api_meter_info.pbs = api_meter_info.cbs;
  }

  status = switch_api_meter_create(device, &api_meter_info, &policer_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create policer: %s", sai_status_to_string(status));
  }
  *policer_id = policer_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Delete policer
 *
 * @param[in] policer_id - Policer id
 *
 * @return  SAI_STATUS_SUCCESS on success
 *         Failure status code on error
 */
sai_status_t sai_remove_policer(_In_ sai_object_id_t policer_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(policer_id) == SAI_OBJECT_TYPE_POLICER);

  switch_status = switch_api_meter_delete(device, policer_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to delete policer %lx: %s",
                  policer_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Set Policer attribute
 *
 * @param[in] policer_id - Policer id
 * @param[in] attr - attribute
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */
sai_status_t sai_set_policer_attribute(_In_ sai_object_id_t policer_id,
                                       _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(policer_id) == SAI_OBJECT_TYPE_POLICER);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

/**
 * @brief  Get Policer attribute
 *
 * @param[in] policer_id - policer id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */
sai_status_t sai_get_policer_attribute(_In_ sai_object_id_t policer_id,
                                       _In_ uint32_t attr_count,
                                       _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_meter_t switch_meter_info;
  unsigned int i = 0;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(policer_id) == SAI_OBJECT_TYPE_POLICER);
  memset(&switch_meter_info, 0, sizeof(switch_api_meter_t));
  switch_status = switch_api_meter_get(device, policer_id, &switch_meter_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get switch meter info %lx: %s",
                  policer_id,
                  sai_status_to_string(status));
    return status;
  }
  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_POLICER_ATTR_METER_TYPE:
        attr->value.u32 =
            switch_meter_type_to_sai_meter_type(switch_meter_info.meter_type);
        break;

      case SAI_POLICER_ATTR_MODE:
        attr->value.u32 =
            switch_meter_mode_to_sai_meter_mode(switch_meter_info.meter_mode);
        break;

      case SAI_POLICER_ATTR_COLOR_SOURCE:
        attr->value.u32 = switch_color_source_to_sai_color_source(
            switch_meter_info.color_source);
        break;

      case SAI_POLICER_ATTR_CBS:
        attr->value.u64 = switch_meter_info.cbs;
        break;

      case SAI_POLICER_ATTR_CIR:
        attr->value.u64 = switch_meter_info.cir;
        break;

      case SAI_POLICER_ATTR_PBS:
        attr->value.u64 = switch_meter_info.pbs;
        break;

      case SAI_POLICER_ATTR_PIR:
        attr->value.u64 = switch_meter_info.pir;
        break;

      case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
        attr->value.u32 = switch_packet_action_to_sai_packet_action(
            switch_meter_info.action[SWITCH_COLOR_GREEN]);
        break;

      case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
        attr->value.u32 = switch_packet_action_to_sai_packet_action(
            switch_meter_info.action[SWITCH_COLOR_YELLOW]);
        break;

      case SAI_POLICER_ATTR_RED_PACKET_ACTION:
        attr->value.u32 = switch_packet_action_to_sai_packet_action(
            switch_meter_info.action[SWITCH_COLOR_RED]);
        break;

      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

static sai_status_t switch_meter_counters_to_sai_meter_counters(
    _In_ uint32_t number_of_counters,
    _In_ const sai_policer_stat_t *counter_ids,
    _In_ switch_counter_t *switch_counters,
    _Out_ uint64_t *counters) {
  uint32_t index = 0;
  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_POLICER_STAT_PACKETS:
        counters[index] =
            switch_counters[SWITCH_METER_COUNTER_GREEN].num_packets +
            switch_counters[SWITCH_METER_COUNTER_YELLOW].num_packets +
            switch_counters[SWITCH_METER_COUNTER_RED].num_packets;
        break;
      case SAI_POLICER_STAT_ATTR_BYTES:
        counters[index] =
            switch_counters[SWITCH_METER_COUNTER_GREEN].num_bytes +
            switch_counters[SWITCH_METER_COUNTER_YELLOW].num_bytes +
            switch_counters[SWITCH_METER_COUNTER_RED].num_bytes;
        break;
      case SAI_POLICER_STAT_GREEN_PACKETS:
        counters[index] =
            switch_counters[SWITCH_METER_COUNTER_GREEN].num_packets;
        break;
      case SAI_POLICER_STAT_GREEN_BYTES:
        counters[index] = switch_counters[SWITCH_METER_COUNTER_GREEN].num_bytes;
        break;
      case SAI_POLICER_STAT_YELLOW_PACKETS:
        counters[index] =
            switch_counters[SWITCH_METER_COUNTER_YELLOW].num_packets;
        break;
      case SAI_POLICER_STAT_YELLOW_BYTES:
        counters[index] =
            switch_counters[SWITCH_METER_COUNTER_YELLOW].num_bytes;
        break;
      case SAI_POLICER_STAT_RED_PACKETS:
        counters[index] = switch_counters[SWITCH_METER_COUNTER_RED].num_packets;
        break;
      case SAI_POLICER_STAT_RED_BYTES:
        counters[index] = switch_counters[SWITCH_METER_COUNTER_RED].num_bytes;
        break;
      default:
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_policer_statistics(
    _In_ sai_object_id_t policer_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_policer_stat_t *counter_ids,
    _Out_ uint64_t *counters) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_counter_t *switch_counters = NULL;
  switch_meter_counter_t *meter_stat_ids = NULL;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  switch_counters =
      SAI_MALLOC(sizeof(switch_counter_t) * SWITCH_METER_COUNTER_MAX);
  if (!switch_counters) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get meter stats %lx: %s",
                  policer_id,
                  sai_status_to_string(status));
    return status;
  }

  meter_stat_ids =
      SAI_MALLOC(sizeof(switch_meter_counter_t) * SWITCH_METER_COUNTER_MAX);
  if (!meter_stat_ids) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get meter stats %lx: %s",
                  policer_id,
                  sai_status_to_string(status));
    SAI_FREE(switch_counters);
    return status;
  }

  for (index = 0; index < SWITCH_METER_COUNTER_MAX; index++) {
    meter_stat_ids[index] = index;
  }

  switch_status = switch_api_meter_counters_get(device,
                                                policer_id,
                                                SWITCH_METER_COUNTER_MAX,
                                                meter_stat_ids,
                                                switch_counters);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SWITCH_STATUS_SUCCESS) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get meter stats %lx: %s",
                  policer_id,
                  sai_status_to_string(status));
    SAI_FREE(meter_stat_ids);
    SAI_FREE(switch_counters);
    return status;
  }

  switch_meter_counters_to_sai_meter_counters(
      number_of_counters, counter_ids, switch_counters, counters);

  SAI_FREE(meter_stat_ids);
  SAI_FREE(switch_counters);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Policer methods table retrieved with sai_api_query()
*/
sai_policer_api_t policer_api = {
    .create_policer = sai_create_policer,
    .remove_policer = sai_remove_policer,
    .set_policer_attribute = sai_set_policer_attribute,
    .get_policer_attribute = sai_get_policer_attribute,
    .get_policer_stats = sai_get_policer_statistics};

sai_status_t sai_policer_initialize(sai_api_service_t *sai_api_service) {
  sai_api_service->policer_api = policer_api;
  return SAI_STATUS_SUCCESS;
}
