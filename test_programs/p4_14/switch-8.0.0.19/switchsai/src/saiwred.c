/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2017 Barefoot Networks, Inc.

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

#include <saiwred.h>
#include "saiinternal.h"
#include <switchapi/switch_wred.h>

static sai_api_t api_id = SAI_API_WRED;

void sai_switch_ecn_mark_mode(sai_ecn_mark_mode_t ecn_mode,
                              switch_api_wred_profile_info_t *api_info) {
  switch (ecn_mode) {
    case SAI_ECN_MARK_MODE_NONE:
      api_info->ecn_mark[SWITCH_COLOR_GREEN] = 0;
      api_info->ecn_mark[SWITCH_COLOR_YELLOW] = 0;
      api_info->ecn_mark[SWITCH_COLOR_RED] = 0;
      break;
    case SAI_ECN_MARK_MODE_GREEN:
      api_info->ecn_mark[SWITCH_COLOR_GREEN] = 1;
      break;
    case SAI_ECN_MARK_MODE_YELLOW:
      api_info->ecn_mark[SWITCH_COLOR_YELLOW] = 1;
      break;
    case SAI_ECN_MARK_MODE_RED:
      api_info->ecn_mark[SWITCH_COLOR_RED] = 1;
      break;
    case SAI_ECN_MARK_MODE_GREEN_YELLOW:
      api_info->ecn_mark[SWITCH_COLOR_GREEN] = 1;
      api_info->ecn_mark[SWITCH_COLOR_YELLOW] = 1;
      break;
    case SAI_ECN_MARK_MODE_GREEN_RED:
      api_info->ecn_mark[SWITCH_COLOR_GREEN] = 1;
      api_info->ecn_mark[SWITCH_COLOR_RED] = 1;
      break;
    case SAI_ECN_MARK_MODE_YELLOW_RED:
      api_info->ecn_mark[SWITCH_COLOR_YELLOW] = 1;
      api_info->ecn_mark[SWITCH_COLOR_RED] = 1;
      break;
    case SAI_ECN_MARK_MODE_ALL:
      api_info->ecn_mark[SWITCH_COLOR_GREEN] = 1;
      api_info->ecn_mark[SWITCH_COLOR_YELLOW] = 1;
      api_info->ecn_mark[SWITCH_COLOR_RED] = 1;
      break;
    default:
      break;
  }
}

sai_status_t sai_wred_profile_attr_parse(
    const int attr_count,
    const sai_attribute_t *attr_list,
    switch_api_wred_profile_info_t *api_info) {
  int index = 0;
  const sai_attribute_t *attr;

  for (index = 0; index < attr_count; index++) {
    attr = &(attr_list[index]);
    switch (attr->id) {
      case SAI_WRED_ATTR_GREEN_ENABLE:
        api_info->enable[SWITCH_COLOR_GREEN] = attr->value.booldata;
        break;
      case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
        api_info->min_threshold[SWITCH_COLOR_GREEN] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
        api_info->max_threshold[SWITCH_COLOR_GREEN] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
        api_info->probability[SWITCH_COLOR_GREEN] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_YELLOW_ENABLE:
        api_info->enable[SWITCH_COLOR_YELLOW] = attr->value.booldata;
        break;
      case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
        api_info->min_threshold[SWITCH_COLOR_YELLOW] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
        api_info->max_threshold[SWITCH_COLOR_YELLOW] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
        api_info->probability[SWITCH_COLOR_YELLOW] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_RED_ENABLE:
        api_info->enable[SWITCH_COLOR_RED] = attr->value.booldata;
        break;
      case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
        api_info->min_threshold[SWITCH_COLOR_RED] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
        api_info->max_threshold[SWITCH_COLOR_RED] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
        api_info->probability[SWITCH_COLOR_RED] = attr->value.u32;
        break;
      case SAI_WRED_ATTR_ECN_MARK_MODE:
        sai_switch_ecn_mark_mode(attr->value.u32, api_info);
        break;
      default:
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_wred_profile(_Out_ sai_object_id_t *wred_id,
                                     _In_ sai_object_id_t switch_id,
                                     _In_ uint32_t attr_count,
                                     _In_ const sai_attribute_t *attr_list) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_wred_profile_info_t wred_profile;
  switch_handle_t wred_handle = SWITCH_API_INVALID_HANDLE;

  memset(&wred_profile, 0, sizeof(switch_api_wred_profile_info_t));
  sai_wred_profile_attr_parse(attr_count, attr_list, &wred_profile);

  switch_status =
      switch_api_wred_profile_create(device, &wred_profile, &wred_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to create wred profile: %s",
                  sai_status_to_string(status));
    return status;
  }
  *wred_id = wred_handle;
  return status;
}

sai_status_t sai_remove_wred_profile(_In_ sai_object_id_t wred_id) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  switch_status =
      switch_api_wred_profile_delete(device, (switch_handle_t)wred_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to delete wred profile 0x%lx : %s",
                  wred_id,
                  sai_status_to_string(status));
    return status;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_wred_profile(_In_ sai_object_id_t wred_id,
                                  _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_wred_profile_info_t profile_info;
  switch_color_t update_color[SWITCH_COLOR_MAX];
  int color_index;

  memset(&profile_info, 0, sizeof(switch_api_wred_profile_info_t));
  switch_status = switch_api_wred_profile_get(device, wred_id, &profile_info);

  sai_wred_profile_attr_parse(1, attr, &profile_info);

  switch (attr->id) {
    case SAI_WRED_ATTR_GREEN_ENABLE:
    case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
    case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
    case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
      update_color[SWITCH_COLOR_GREEN] = 1;
      break;

    case SAI_WRED_ATTR_YELLOW_ENABLE:
    case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
    case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
    case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
      update_color[SWITCH_COLOR_YELLOW] = 1;
      break;

    case SAI_WRED_ATTR_RED_ENABLE:
    case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
    case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
    case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
      update_color[SWITCH_COLOR_RED] = 1;
      break;

    case SAI_WRED_ATTR_ECN_MARK_MODE:
      if (profile_info.ecn_mark[SWITCH_COLOR_GREEN]) {
        update_color[SWITCH_COLOR_GREEN] = 1;
      }
      if (profile_info.ecn_mark[SWITCH_COLOR_YELLOW]) {
        update_color[SWITCH_COLOR_YELLOW] = 1;
      }

      if (profile_info.ecn_mark[SWITCH_COLOR_RED]) {
        update_color[SWITCH_COLOR_RED] = 1;
      }
      break;
    default:
      break;
  }

  for (color_index = SWITCH_COLOR_GREEN; color_index < SWITCH_COLOR_MAX;
       color_index++) {
    if (!update_color[color_index]) {
      continue;
    }
    switch_status = switch_api_wred_profile_set(
        device, wred_id, color_index, &profile_info);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("Failed to update wred profile 0x%lx: %s",
                    wred_id,
                    sai_status_to_string(status));
      return status;
    }
  }

  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_wred_profile(_In_ sai_object_id_t wred_id,
                                  _In_ uint32_t attr_count,
                                  _Inout_ sai_attribute_t *attr_list) {
  unsigned int i = 0;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_wred_profile_info_t profile_info;

  memset(&profile_info, 0, sizeof(switch_api_wred_profile_info_t));
  switch_status = switch_api_wred_profile_get(
      device, (switch_handle_t)wred_id, &profile_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to get wred profile for handle 0x%lx: %s",
                  wred_id,
                  sai_status_to_string(status));
    return status;
  }

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_WRED_ATTR_GREEN_ENABLE:
        attr->value.booldata = profile_info.enable[SWITCH_COLOR_GREEN];
        break;
      case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
        attr->value.u32 = profile_info.min_threshold[SWITCH_COLOR_GREEN];
        break;
      case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
        attr->value.u32 = profile_info.max_threshold[SWITCH_COLOR_GREEN];
        break;
      case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
        attr->value.u32 = profile_info.probability[SWITCH_COLOR_GREEN];
        break;

      case SAI_WRED_ATTR_YELLOW_ENABLE:
        attr->value.booldata = profile_info.enable[SWITCH_COLOR_YELLOW];
        break;
      case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
        attr->value.u32 = profile_info.min_threshold[SWITCH_COLOR_YELLOW];
        break;
      case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
        attr->value.u32 = profile_info.max_threshold[SWITCH_COLOR_YELLOW];
        break;
      case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
        attr->value.u32 = profile_info.probability[SWITCH_COLOR_YELLOW];
        break;

      case SAI_WRED_ATTR_RED_ENABLE:
        attr->value.booldata = profile_info.enable[SWITCH_COLOR_RED];
        break;
      case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
        attr->value.u32 = profile_info.min_threshold[SWITCH_COLOR_RED];
        break;
      case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
        attr->value.u32 = profile_info.max_threshold[SWITCH_COLOR_RED];
        break;
      case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
        attr->value.u32 = profile_info.probability[SWITCH_COLOR_RED];
        break;

      case SAI_WRED_ATTR_ECN_MARK_MODE:
        break;
      default:
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

sai_wred_api_t wred_api = {.create_wred = sai_create_wred_profile,
                           .remove_wred = sai_remove_wred_profile,
                           .set_wred_attribute = sai_set_wred_profile,
                           .get_wred_attribute = sai_get_wred_profile};

sai_status_t sai_wred_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_ENTER();
  SAI_LOG_DEBUG("Initializing WRED");
  sai_api_service->wred_api = wred_api;
  SAI_LOG_EXIT();
  return SAI_STATUS_SUCCESS;
}
