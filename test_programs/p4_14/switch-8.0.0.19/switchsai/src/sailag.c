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

#include <sailag.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_lag.h>

static sai_api_t api_id = SAI_API_LAG;

sai_status_t sai_create_lag_entry(_Out_ sai_object_id_t *lag_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list);

sai_status_t sai_remove_lag_entry(_In_ sai_object_id_t lag_id);

sai_status_t sai_set_lag_entry_attribute(_In_ sai_object_id_t lag_id,
                                         _In_ const sai_attribute_t *attr);

sai_status_t sai_add_ports_to_lag(_In_ sai_object_id_t lag_id,
                                  _In_ const sai_object_list_t *port_list);

/*
    \brief Create LAG
    \param[out] lag_id LAG id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_create_lag_entry(_Out_ sai_object_id_t *lag_id,
                                  _In_ sai_object_id_t switch_id,

                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t lag_handle = SWITCH_API_INVALID_HANDLE;
  *lag_id = SAI_NULL_OBJECT_ID;

  if (attr_count && !attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_lag_create(device, &lag_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  status = (lag_handle == SWITCH_API_INVALID_HANDLE) ? SAI_STATUS_FAILURE
                                                     : SAI_STATUS_SUCCESS;

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create lag: %s", sai_status_to_string(status));
    return status;
  }
  *lag_id = lag_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Remove LAG
    \param[in] lag_id LAG id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_remove_lag_entry(_In_ sai_object_id_t lag_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(lag_id) == SAI_OBJECT_TYPE_LAG);

  switch_status = switch_api_lag_delete(device, (switch_handle_t)lag_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "failed to remove lag %lx: %s", lag_id, sai_status_to_string(status));
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Set LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_set_lag_attribute(_In_ sai_object_id_t lag_id,
                                   _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t acl_table_id = SWITCH_API_INVALID_HANDLE;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(lag_id) == SAI_OBJECT_TYPE_LAG);

  switch (attr->id) {
    case SAI_LAG_ATTR_INGRESS_ACL:
    case SAI_LAG_ATTR_EGRESS_ACL:
      acl_table_id = (switch_handle_t)attr->value.oid;
      if (acl_table_id == SAI_NULL_OBJECT_ID) {
        if (attr->id == SAI_LAG_ATTR_INGRESS_ACL) {
          switch_status = switch_api_lag_ingress_acl_group_get(
              device, lag_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to unbind lag to acl for port %d: %s",
                          (lag_id & 0xFFFF),
                          sai_status_to_string(status));
            return status;
          }
        } else {
          switch_status = switch_api_lag_egress_acl_group_get(
              device, lag_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to unbind lag to acl for port %d: %s",
                          (lag_id & 0xFFFF),
                          sai_status_to_string(status));
            return status;
          }
        }
        switch_status =
            switch_api_acl_dereference(device, acl_table_id, lag_id);
      } else {
        switch_status = switch_api_acl_reference(device, acl_table_id, lag_id);
      }
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to bind lag to acl for lag %d: %s",
                      (lag_id & 0xFFFF),
                      sai_status_to_string(status));
        return status;
      }
      break;

    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Get LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be
   get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_get_lag_attribute(_In_ sai_object_id_t lag_id,
                                   _In_ uint32_t attr_count,
                                   _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_handle_t acl_handle;
  switch_uint32_t member_count = 0;
  unsigned int i = 0;
  switch_handle_t *member_handles = NULL;
  sai_object_list_t *obj_list = NULL;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(lag_id) == SAI_OBJECT_TYPE_LAG);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_LAG_ATTR_INGRESS_ACL:
        switch_status =
            switch_api_lag_ingress_acl_group_get(device, lag_id, &acl_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attr->value.oid = (acl_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : acl_handle;
        break;
      case SAI_LAG_ATTR_EGRESS_ACL:
        break;
      case SAI_LAG_ATTR_PORT_LIST:
        obj_list = &attr_list->value.objlist;
        switch_status =
            switch_api_lag_member_count_get(device, lag_id, &member_count);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get max member count: %s",
                        sai_status_to_string(status));
          return status;
        }
        if (member_count) {
          member_handles =
              (switch_handle_t *)malloc(member_count * sizeof(switch_handle_t));
          switch_status =
              switch_api_lag_members_get(device, lag_id, member_handles);
          status = sai_switch_status_to_sai_status(switch_status);
          if (status != SWITCH_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to get member handles: %s",
                          sai_status_to_string(status));
            return status;
          }
          obj_list->count = member_count;
          obj_list = &attr_list->value.objlist;
          for (i = 0; i < member_count; i++) {
            obj_list->list[i] = member_handles[i];
          }
        } else {
          obj_list->count = 0;
        }
        free(member_handles);
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

sai_status_t sai_lag_member_entry_parse(_In_ const sai_attribute_t *attr_list,
                                        _In_ uint32_t attr_count,
                                        _Out_ sai_object_id_t *lag_id,
                                        _Out_ sai_object_id_t *port_id) {
  const sai_attribute_t *attribute;
  uint32_t index = 0;

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_LAG_MEMBER_ATTR_LAG_ID:
        *lag_id = attribute->value.oid;
        break;
      case SAI_LAG_MEMBER_ATTR_PORT_ID:
        *port_id = attribute->value.oid;
        break;
      case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
        break;
      case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

/*
    \brief Create LAG Member
    \param[out] lag_member_id LAG Member id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_create_lag_member(_Out_ sai_object_id_t *lag_member_id,
                                   _In_ sai_object_id_t switch_id,

                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_direction_t direction = SWITCH_API_DIRECTION_BOTH;
  sai_object_id_t lag_id = 0;
  sai_object_id_t port_id = 0;
  switch_handle_t lag_member_handle = SWITCH_API_INVALID_HANDLE;
  *lag_member_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null port list: %s", sai_status_to_string(status));
    return status;
  }

  status = sai_lag_member_entry_parse(attr_list, attr_count, &lag_id, &port_id);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to parse lag member attributes: %s",
                  sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(lag_id) == SAI_OBJECT_TYPE_LAG);
  SAI_ASSERT(sai_object_type_query(port_id) == SAI_OBJECT_TYPE_PORT);

  status = switch_api_lag_member_create(
      device, (switch_handle_t)lag_id, direction, port_id, &lag_member_handle);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create lag: %s", sai_status_to_string(status));
    return status;
  }
  *lag_member_id = lag_member_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Remove LAG Member
    \param[in] lag_member_id LAG Member id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_remove_lag_member(_In_ sai_object_id_t lag_member_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(lag_member_id) ==
             SAI_OBJECT_TYPE_LAG_MEMBER);

  switch_status =
      switch_api_lag_member_remove(device, (switch_handle_t)lag_member_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove lag member %lx : %s",
                  lag_member_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Set LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t sai_set_lag_member_attribute(_In_ sai_object_id_t lag_member_id,
                                          _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t lag_handle = SWITCH_API_INVALID_HANDLE;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  switch (attr->id) {
    case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
    case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
      switch_status = swich_api_lag_handle_from_lag_member_get(
          device, (switch_handle_t)(lag_member_id), &lag_handle);
      if ((status = sai_switch_status_to_sai_status(switch_status)) !=
          SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to get lag handle for port %d error: %s",
                      lag_member_id,
                      sai_status_to_string(status));
        return status;
      }
      switch_status = switch_api_lag_member_port_handle_get(
          device, (switch_handle_t)lag_member_id, &port_handle);
      if ((status = sai_switch_status_to_sai_status(switch_status)) !=
          SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR(
            "failed to get lag member port handle for port %d error: %s",
            lag_member_id,
            sai_status_to_string(status));
        return status;
      }
      if (attr->value.booldata) {
        switch_status = switch_api_lag_member_delete(
            device,
            lag_handle,
            (attr->id == SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE
                 ? SWITCH_API_DIRECTION_INGRESS
                 : SWITCH_API_DIRECTION_EGRESS),
            port_handle);
      } else {
        switch_status = switch_api_lag_member_add(
            device,
            lag_handle,
            (attr->id == SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE
                 ? SWITCH_API_DIRECTION_INGRESS
                 : SWITCH_API_DIRECTION_EGRESS),
            port_handle);
      }
      if ((status = sai_switch_status_to_sai_status(switch_status)) !=
          SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR(
            "failed to update ingress/egress disable for port %d error: %s",
            lag_member_id,
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

/*
    \brief Get LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be
   get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/

sai_status_t sai_get_lag_member_attribute(_In_ sai_object_id_t lag_member_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_handle_t lag_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;

  unsigned int i = 0;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(lag_member_id) ==
             SAI_OBJECT_TYPE_LAG_MEMBER);
  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_LAG_MEMBER_ATTR_LAG_ID:
        switch_status = swich_api_lag_handle_from_lag_member_get(
            device, (switch_handle_t)(lag_member_id), &lag_handle);
        if ((status = sai_switch_status_to_sai_status(switch_status)) !=
            SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get lag handle for port %d error: %s",
                        lag_member_id,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.oid = lag_handle;
        break;

      case SAI_LAG_MEMBER_ATTR_PORT_ID:
        switch_status = switch_api_lag_member_port_handle_get(
            device, (switch_handle_t)lag_member_id, &port_handle);
        if ((status = sai_switch_status_to_sai_status(switch_status)) !=
            SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "failed to get lag member port handle for port %d error: %s",
              lag_member_id,
              sai_status_to_string(status));
          return status;
        }
        attr->value.oid = port_handle;
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  LAG methods table retrieved with sai_api_query()
*/
sai_lag_api_t lag_api = {
    .create_lag = sai_create_lag_entry,
    .remove_lag = sai_remove_lag_entry,
    .set_lag_attribute = sai_set_lag_attribute,
    .get_lag_attribute = sai_get_lag_attribute,
    .create_lag_member = sai_create_lag_member,
    .remove_lag_member = sai_remove_lag_member,
    .set_lag_member_attribute = sai_set_lag_member_attribute,
    .get_lag_member_attribute = sai_get_lag_member_attribute,
};

sai_status_t sai_lag_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing lag");
  sai_api_service->lag_api = lag_api;
  return SAI_STATUS_SUCCESS;
}
