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

#include <saischedulergroup.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_port.h>
#include <switchapi/switch_scheduler.h>
#include <switchapi/switch_queue.h>

static sai_api_t api_id = SAI_API_SCHEDULER_GROUP;

static void sai_scheduler_group_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    switch_scheduler_group_api_info_t *api_info) {
  const sai_attribute_t *attribute;
  uint32_t i = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
        break;
      case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
        break;
      case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
        api_info->port_handle = attribute->value.oid;
        break;
      case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
        if (attribute->value.u32 == 1) {
          api_info->group_type = SWITCH_SCHEDULER_GROUP_TYPE_PORT;
        } else {
          api_info->group_type = SWITCH_SCHEDULER_GROUP_TYPE_QUEUE;
        }
        break;
      case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
        api_info->scheduler_handle = attribute->value.oid;
        break;
    }
  }
}

/**
 * @brief  Create Scheduler group
 *
 * @param[out] scheduler_group_id Scheudler group id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_create_scheduler_group(
    _Out_ sai_object_id_t *scheduler_group_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_scheduler_group_api_info_t api_info;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;

  memset(&api_info, 0, sizeof(switch_scheduler_group_api_info_t));

  sai_scheduler_group_attribute_parse(attr_count, attr_list, &api_info);

  switch_status = switch_api_scheduler_group_create(device, &api_info, &handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create scheduler group: %s",
                  sai_status_to_string(status));
    return status;
  }
  *scheduler_group_id = handle;

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Remove Scheduler group
 *
 * @param[in] scheduler_group_id Scheudler group id
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_remove_scheduler_group(_In_ sai_object_id_t
                                            scheduler_group_id) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(scheduler_group_id) ==
             SAI_OBJECT_TYPE_SCHEDULER_GROUP);

  switch_status = switch_api_scheduler_group_delete(device, scheduler_group_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to remove scheduler group 0x%lx: %s",
                  scheduler_group_id,
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}
/**
 * @brief  Set Scheduler group Attribute
 *
 * @param[in] scheduler_group_id Scheudler group id
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_set_scheduler_group_attribute(
    _In_ sai_object_id_t scheduler_group_id, _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t scheduler_profile;

  SAI_ASSERT(sai_object_type_query(scheduler_group_id) ==
             SAI_OBJECT_TYPE_SCHEDULER_GROUP);

  switch (attr->id) {
    case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
      scheduler_profile = attr->value.oid;
      switch_status = switch_api_scheduler_group_profile_set(
          device, scheduler_group_id, scheduler_profile);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR(
            "Failed to set scheduler profile to scheduler group 0x%lx: %s",
            scheduler_group_id,
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
 * @brief  Get Scheduler Group attribute
 *
 * @param[in] scheduler_group_id - scheduler group id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */

sai_status_t sai_get_scheduler_group_attribute(
    _In_ sai_object_id_t scheduler_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr;
  switch_uint32_t child_count = 0;
  switch_handle_t *child_handles = NULL;
  unsigned int i, index = 0;
  switch_handle_t profile_handle = SAI_NULL_OBJECT_ID;
  switch_scheduler_group_api_info_t api_info;
  sai_object_list_t *objlist = NULL;

  SAI_ASSERT(sai_object_type_query(scheduler_group_id) ==
             SAI_OBJECT_TYPE_SCHEDULER_GROUP);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
      case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:

        switch_status = switch_api_scheduler_group_child_count_get(
            device, scheduler_group_id, &child_count);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get scheduler group child count for handle 0x%lx",
              scheduler_group_id);
          return status;
        }

        if (attr->id == SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT) {
          attr->value.u32 = child_count;
          break;
        }

        child_handles = SAI_MALLOC(child_count * sizeof(switch_handle_t));
        switch_status = switch_api_scheduler_group_child_handle_get(
            device, scheduler_group_id, child_handles);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get scheduler group child handle for handle 0x%lx",
              scheduler_group_id);
          return status;
        }
        objlist = &attr->value.objlist;
        objlist->count = child_count;
        for (index = 0; index < child_count; index++) {
          objlist->list[index] = (sai_object_id_t)child_handles[index];
        }
        SAI_FREE(child_handles);
        break;

      case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
        switch_status = switch_api_scheduler_group_profile_get(
            device, scheduler_group_id, &profile_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get scheduler profile handle for group 0x%lx",
              scheduler_group_id);
          return status;
        }
        attr->value.oid = profile_handle;
        break;

      case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
        memset(&api_info, 0x0, sizeof(api_info));
        switch_status = switch_api_scheduler_group_config_get(
            device, scheduler_group_id, &api_info);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get scheduler profile handle for group 0x%lx",
              scheduler_group_id);
          return status;
        }
        attr->value.oid = api_info.port_handle;
        break;

      default:
        break;
    }
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Scheduler Group methods table retrieved with sai_api_query()
*/
sai_scheduler_group_api_t scheduler_group_api = {
    .create_scheduler_group = sai_create_scheduler_group,
    .remove_scheduler_group = sai_remove_scheduler_group,
    .set_scheduler_group_attribute = sai_set_scheduler_group_attribute,
    .get_scheduler_group_attribute = sai_get_scheduler_group_attribute,
};

sai_status_t sai_scheduler_group_initialize(
    sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing scheulder group");
  sai_api_service->scheduler_group_api = scheduler_group_api;
  return SAI_STATUS_SUCCESS;
}
