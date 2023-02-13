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

#include "switchapi/switch_interface.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_INTERFACE

switch_status_t switch_interface_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_interface_default_entries_delete(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

/*
 * Routine Description:
 *   @brief initialize interface device context
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_interface_init(switch_device_t device) {
  switch_interface_context_t *intf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  intf_ctx = SWITCH_MALLOC(device, sizeof(switch_interface_context_t), 0x1);
  if (!intf_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "interface init failed on device %d: "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(intf_ctx, 0x0, sizeof(switch_interface_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_INTERFACE, (void *)intf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface init failed on device %d: "
        "interface context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_INTERFACE
   */
  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_INTERFACE, SWITCH_INTERFACE_MAX);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface init failed on device %d: "
        "interface handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("interface init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief free interface device context
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_interface_free(switch_device_t device) {
  switch_interface_context_t *intf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_INTERFACE, (void **)&intf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface free failed on device %d: "
        "interface context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_INTERFACE
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_INTERFACE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface free failed on device %d: "
        "interface handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, intf_ctx);
  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_INTERFACE, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("interface free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_handle_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t *handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface get type failed on device %d :"
        "interface handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface get type failed for device %d :"
        "interface get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *handle = SWITCH_INTF_ATTR_HANDLE(intf_info);

  return status;
}

switch_status_t switch_api_interface_vlan_id_get(switch_device_t device,
                                                 switch_handle_t intf_handle,
                                                 switch_vlan_t *outer_vlan,
                                                 switch_vlan_t *inner_vlan) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(outer_vlan != NULL && inner_vlan != NULL);
  if (!outer_vlan || !inner_vlan) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface vlan id get failed on device %d :"
        "parameters null(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *outer_vlan = 0;
  *inner_vlan = 0;

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface vlan id get failed on device %d :"
        "interface handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface vlan id get failed on device %d :"
        "interface get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if ((SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_PORT_VLAN)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface vlan id get failed on device %d :"
        "interface type invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *inner_vlan = SWITCH_INTF_ATTR_VLAN_ID(intf_info);

  return status;
}

switch_status_t switch_interface_port_bind_mode_get(
    switch_device_t device,
    switch_handle_t handle,
    switch_port_bind_mode_t *bind_mode) {
  switch_port_info_t *port_info = NULL;
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_PORT_HANDLE(handle) && !SWITCH_LAG_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface bind_mode get failed on device %d "
        "handle %lx: handle invalid(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_PORT_HANDLE(handle)) {
    status = switch_port_get(device, handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface bind_mode get failed on device %d "
          "handle %lx: port get failed(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
    *bind_mode = port_info->bind_mode;
  } else if (SWITCH_LAG_HANDLE(handle)) {
    status = switch_lag_get(device, handle, &lag_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface bind_mode get failed on device %d "
          "handle %lx: lag get failed(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
    *bind_mode = lag_info->bind_mode;
  } else {
    status = SWITCH_STATUS_INVALID_HANDLE;
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface bind_mode get failed on device %d "
          "handle %lx: handle invalid(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
  }
  return status;
}

switch_status_t switch_interface_validate(
    switch_device_t device, switch_api_interface_info_t *api_intf_info) {
  switch_array_t *array = NULL;
  switch_handle_t *tmp_intf_handle = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_port_bind_mode_t port_bind_mode;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(tmp_intf_handle);

  SWITCH_ASSERT(api_intf_info != NULL);

  // front facing interfaces, make sure valid port_lag assigned to handle..
  // get list of interfaces using port/lag
  switch (api_intf_info->type) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
    case SWITCH_INTERFACE_TYPE_TRUNK:
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      SWITCH_ASSERT(SWITCH_PORT_HANDLE(api_intf_info->handle) ||
                    SWITCH_LAG_HANDLE(api_intf_info->handle));
      if (!SWITCH_PORT_HANDLE(api_intf_info->handle) &&
          !SWITCH_LAG_HANDLE(api_intf_info->handle)) {
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle %lx intf type %s"
            "invalid handle(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }

      status =
          switch_interface_array_get(device, api_intf_info->handle, &array);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle %lx intf type %s"
            "interface array get failed(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }

      status = switch_interface_port_bind_mode_get(
          device, api_intf_info->handle, &port_bind_mode);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle %lx intf type %s"
            "interface array get failed(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }

      break;

    case SWITCH_INTERFACE_TYPE_TUNNEL:
      SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(api_intf_info->handle));
      if (!SWITCH_TUNNEL_HANDLE(api_intf_info->handle)) {
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle %lx intf type %s"
            "invalid handle(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }

      status = switch_tunnel_get(device, api_intf_info->handle, &tunnel_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      if (SWITCH_INTERFACE_HANDLE(tunnel_info->intf_handle)) {
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle 0x%lx intf type %s"
            "tunnel interface already exists:(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }
      break;

    default:
      break;
  }

  // check vtag value
  switch (api_intf_info->type) {
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      SWITCH_ASSERT(SWITCH_VLAN_ID_VALID(api_intf_info->vlan));
      if (!SWITCH_VLAN_ID_VALID(api_intf_info->vlan)) {
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "handle %lx intf type %s"
            "vlan id invalid(%s)\n",
            device,
            api_intf_info->handle,
            switch_interface_type_to_string(api_intf_info->type),
            switch_error_to_string(status));
        return status;
      }
      break;
    default:
      break;
  }

  // if interface fully owns a port/lag, check only one
  switch (api_intf_info->type) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
    case SWITCH_INTERFACE_TYPE_TRUNK:
      if (SWITCH_ARRAY_COUNT(array) != 0 ||
          port_bind_mode != SWITCH_PORT_BIND_MODE_PORT) {
        status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d handle %lx: "
            "interface already exist on this handle(%s)\n",
            device,
            api_intf_info->handle,
            switch_error_to_string(status));
        return status;
      }
      break;

    // if port_vlan, check only single one on the same vlan (vtag) from same
    // port/lag.
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      if (port_bind_mode != SWITCH_PORT_BIND_MODE_PORT_VLAN) {
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR(
            "interface validate failed on device %d: "
            "interface get failed(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
      FOR_EACH_IN_ARRAY(
          intf_handle, (*array), switch_handle_t, tmp_intf_handle) {
        status = switch_interface_get(device, intf_handle, &intf_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          status = SWITCH_STATUS_INVALID_PARAMETER;
          SWITCH_LOG_ERROR(
              "interface validate failed on device %d: "
              "interface get failed(%s)\n",
              device,
              switch_error_to_string(status));
          return status;
        }

        if (SWITCH_INTF_TYPE(intf_info) == api_intf_info->type &&
            SWITCH_INTF_ATTR_VLAN_ID(intf_info) == api_intf_info->vlan) {
          status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
          SWITCH_LOG_ERROR(
              "interface validate failed on device %d handle %lx: "
              "interface already exist on this handle(%s)\n",
              device,
              api_intf_info->handle,
              switch_error_to_string(status));
          return status;
        }
      }
      FOR_EACH_IN_ARRAY_END();
      break;

    default:
      break;
  }

  SWITCH_LOG_DETAIL(
      "interface validated on device %d intf type %s "
      "handle %lx vlan id %d\n",
      device,
      switch_interface_type_to_string(api_intf_info->type),
      api_intf_info->handle,
      api_intf_info->vlan);

  return status;
}

switch_status_t switch_interface_ifindex_allocate(switch_device_t device,
                                                  switch_handle_t intf_handle) {
  switch_interface_context_t *intf_ctx = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_ifindex_t ifindex = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_INTERFACE, (void **)&intf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex allocate failed on device %d "
        "intf handle %lx: interface context get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface ifindex allocate failed on device %d "
        "intf handle %lx: interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex allocate failed on device %d "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_ifindex_allocate(device, &ifindex);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex allocate failed on device %d:"
        "intf handle %lx: handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_INSERT(
      &intf_ctx->ifindex_array, ifindex, (void *)intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex allocate failed on device %d:"
        "intf handle %lx: ifindex array insert failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "interface ifindex allocated on device %d "
      "intf handle %lx ifindex 0x%x\n",
      device,
      intf_handle,
      ifindex);

  SWITCH_ASSERT(ifindex != 0);
  SWITCH_IFINDEX_SET(device, intf_handle, ifindex, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_interface_ifindex_deallocate(
    switch_device_t device, switch_handle_t intf_handle) {
  switch_interface_context_t *intf_ctx = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_INTERFACE, (void **)&intf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: interface context get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(&intf_ctx->ifindex_array, intf_info->ifindex);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: ifindex array delete failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_ifindex_deallocate(device, intf_info->ifindex);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on "
        "device %d intf handle %lx: "
        "ifindex array deallocate failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "interface ifindex deallocated on device %d "
      "intf handle %lx ifindex 0x%x\n",
      device,
      intf_handle,
      intf_info->ifindex);

  SWITCH_IFINDEX_SET(device, intf_handle, intf_info->ifindex, status);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_interface_create_internal(
    switch_device_t device,
    switch_api_interface_info_t *api_intf_info,
    switch_handle_t *intf_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  CHECK_RET(!intf_handle, status);
  CHECK_RET(!api_intf_info, status);

  status = switch_interface_validate(device, api_intf_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  handle = switch_interface_handle_create(device);
  CHECK_RET(handle == SWITCH_API_INVALID_HANDLE, SWITCH_STATUS_NO_MEMORY);

  status = switch_interface_get(device, handle, &intf_info);
  CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_MEMSET(intf_info, 0x0, sizeof(switch_interface_info_t));
  intf_info->api_intf_info = *api_intf_info;

  // if port/lag facing / if not tunnel
  switch (SWITCH_INTF_TYPE(intf_info)) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
    case SWITCH_INTERFACE_TYPE_TRUNK:
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
    case SWITCH_INTERFACE_TYPE_TUNNEL:

      status = switch_interface_ifindex_allocate(device, handle);
      CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

      switch (switch_handle_type_get(api_intf_info->handle)) {
        case SWITCH_HANDLE_TYPE_PORT:
          status = switch_port_get(device, api_intf_info->handle, &port_info);
          CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
          intf_info->port_lag_index = port_info->port_lag_index;
          break;
        case SWITCH_HANDLE_TYPE_LAG:
          status = switch_lag_get(device, api_intf_info->handle, &lag_info);
          CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
          intf_info->port_lag_index = lag_info->port_lag_index;
          break;
        case SWITCH_HANDLE_TYPE_TUNNEL:
          status =
              switch_tunnel_get(device, api_intf_info->handle, &tunnel_info);
          CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
          tunnel_info->intf_handle = handle;
          break;
        default:
          SWITCH_LOG_ERROR("port/lag handle is neither.");
          return SWITCH_STATUS_INVALID_PARAMETER;
          break;
      }

      if (SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_TUNNEL) {
        status = switch_interface_array_insert(
            device, api_intf_info->handle, handle);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
      }

      if (SWITCH_RIF_HANDLE(api_intf_info->rif_handle)) {
        status = switch_api_rif_attach_intf(
            device, api_intf_info->rif_handle, handle);
        CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);
      }

      break;
    default:
      break;
  }

  *intf_handle = handle;

  SWITCH_LOG_DEBUG(
      "interface created on device %d "
      "intf handle %lx type %s\n",
      device,
      handle,
      switch_interface_type_to_string(api_intf_info->type));

  SWITCH_LOG_EXIT();

  return status;

clean:
  return status;
}

switch_status_t switch_api_interface_delete_internal(
    switch_device_t device, switch_handle_t intf_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_api_interface_info_t *api_intf_info = NULL;
  switch_tunnel_info_t *tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface delete failed on device %d "
        "intf handle %lx: interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface delete failed on device %d "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  api_intf_info = &intf_info->api_intf_info;

  // if port/lag facing, remove from intf array
  switch (SWITCH_INTF_TYPE(intf_info)) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
    case SWITCH_INTERFACE_TYPE_TRUNK:
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:

      if (intf_info->api_intf_info.rif_handle != SWITCH_API_INVALID_HANDLE) {
        status = switch_api_rif_dettach_intf(
            device, intf_info->api_intf_info.rif_handle);
        CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      }
      status = switch_interface_array_delete(
          device, intf_info->api_intf_info.handle, intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "interface l2 delete failed on device %d :"
            "interface array insert failed(%s)\n",
            device,
            switch_error_to_string(device));
        return status;
      }

      status = switch_interface_ifindex_deallocate(device, intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "interface delete failed on device %d "
            "intf handle %lx: ifindex deallocation failed(%s)\n",
            device,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }
      break;

    case SWITCH_INTERFACE_TYPE_TUNNEL:
      status = switch_tunnel_get(device, api_intf_info->handle, &tunnel_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      tunnel_info->intf_handle = SWITCH_API_INVALID_HANDLE;
      break;
    default:
      break;
  }

  status = switch_interface_handle_delete(device, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface delete failed on device %d "
        "intf handle %lx: interface handle delete failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "interface deleted on device %d intf handle %lx\n", device, intf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_interface_array_get(switch_device_t device,
                                           switch_handle_t handle,
                                           switch_array_t **array) {
  switch_port_info_t *port_info = NULL;
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(handle) || SWITCH_LAG_HANDLE(handle));
  if (!SWITCH_PORT_HANDLE(handle) && !SWITCH_LAG_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface array get failed on device %d "
        "handle %lx: handle invalid(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(array != NULL);
  if (!array) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface array get failed on device %d "
        "handle %lx: array null(%s)\n",
        device,
        handle,
        switch_error_to_string(status));
    return status;
  }

  *array = NULL;

  if (SWITCH_PORT_HANDLE(handle)) {
    status = switch_port_get(device, handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface array get failed on device %d "
          "handle %lx: port get failed(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
    *array = &port_info->intf_array;
  } else if (SWITCH_LAG_HANDLE(handle)) {
    status = switch_lag_get(device, handle, &lag_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface array get failed on device %d "
          "handle %lx: lag get failed(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
    *array = &lag_info->intf_array;
  } else {
    status = SWITCH_STATUS_INVALID_HANDLE;
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface array get failed on device %d "
          "handle %lx: handle invalid(%s)\n",
          device,
          handle,
          switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_interface_array_insert(switch_device_t device,
                                              switch_handle_t handle,
                                              switch_handle_t intf_handle) {
  switch_array_t *array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface array insert failed on device %d "
        "handle %lx interface handle %lx "
        "interface handle invalid(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_array_get(device, handle, &array);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface array insert failed on device %d "
        "handle %lx interface handle %lx "
        "handle array get failed(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }
  status = SWITCH_ARRAY_INSERT(array, intf_handle, (void *)intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface array insert failed on device %d "
        "handle %lx interface handle %lx "
        "handle array insert failed(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "interface array inserted on device %d "
      "handle %lx intf handle %lx\n",
      device,
      handle,
      intf_handle);
  return status;
}

switch_status_t switch_interface_array_delete(switch_device_t device,
                                              switch_handle_t handle,
                                              switch_handle_t intf_handle) {
  switch_array_t *array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "interface array delete failed on device %d "
        "handle %lx interface handle %lx "
        "interface handle invalid(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_array_get(device, handle, &array);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface array delete failed on device %d "
        "handle %lx interface handle %lx "
        "handle array get failed(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(array, intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface array delete failed on device %d "
        "handle %lx interface handle %lx "
        "handle array delete failed(%s)\n",
        device,
        handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "interface array deleted on device %d "
      "handle %lx intf handle %lx\n",
      device,
      handle,
      intf_handle);
  return status;
}

switch_status_t switch_interface_handle_get(switch_device_t device,
                                            switch_ifindex_t ifindex,
                                            switch_handle_t *intf_handle) {
  switch_interface_context_t *intf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_INTERFACE, (void **)&intf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("interface handle get failed on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(intf_handle != NULL);

  status =
      SWITCH_ARRAY_GET(&intf_ctx->ifindex_array, ifindex, (void *)intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("interface handle get failed on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(*intf_handle));
  if (!(SWITCH_INTERFACE_HANDLE(*intf_handle))) {
    SWITCH_LOG_ERROR("interface handle get failed on device %d",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_interface_by_type_get_internal(
    switch_device_t device,
    switch_handle_t handle,
    switch_interface_type_t intf_type,
    switch_handle_t *intf_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_array_t *array = NULL;
  switch_handle_t *tmp_intf_handle2 = NULL;
  switch_handle_t tmp_intf_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(handle) || SWITCH_LAG_HANDLE(handle));
  if (!SWITCH_PORT_HANDLE(handle) && !SWITCH_LAG_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("interface by type get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(intf_handle != NULL);

  SWITCH_ASSERT(intf_type < SWITCH_INTERFACE_TYPE_MAX);
  if (intf_type >= SWITCH_INTERFACE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("interface by type get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_interface_array_get(device, handle, &array);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("interface by type get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  *intf_handle = SWITCH_API_INVALID_HANDLE;
  FOR_EACH_IN_ARRAY(
      tmp_intf_handle1, (*array), switch_handle_t, tmp_intf_handle2) {
    UNUSED(tmp_intf_handle2);
    status = switch_interface_get(device, tmp_intf_handle1, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("interface by type get failed for device %d:",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    if (SWITCH_INTF_TYPE(intf_info) == intf_type) {
      *intf_handle = tmp_intf_handle1;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_ARRAY_END();

  return status;
}

switch_status_t switch_api_interface_rewrite_mac_get(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_mac_addr_t *mac) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_interface_native_vlan_set_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t vlan_handle,
    switch_handle_t *member_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_handle_t native_vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface native vlan set failed on device %d "
        "intf handle %lx vlan handle %lx: "
        "interface handle invalid(%s)\n",
        device,
        intf_handle,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  if (vlan_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
    if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
      SWITCH_LOG_ERROR(
          "interface native vlan set failed on device %d "
          "intf handle %lx vlan handle %lx: "
          "vlan handle invalid(%s)\n",
          device,
          intf_handle,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan set failed on device %d "
        "intf handle %lx vlan handle %lx: "
        "interface get failed(%s)\n",
        device,
        intf_handle,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  native_vlan_handle = SWITCH_INTF_NATIVE_VLAN_HANDLE(intf_info);
  if (native_vlan_handle == vlan_handle) {
    return status;
  }

  if (SWITCH_VLAN_HANDLE(native_vlan_handle)) {
    status =
        switch_api_vlan_member_remove(device, native_vlan_handle, intf_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface native vlan set failed on device %d "
          "intf handle %lx vlan handle %lx: "
          "vlan member delete failed(%s)\n",
          device,
          intf_handle,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  if (SWITCH_VLAN_HANDLE(vlan_handle)) {
    SWITCH_INTF_NATIVE_VLAN_HANDLE(intf_info) = vlan_handle;
    status = switch_api_vlan_member_add(
        device, vlan_handle, intf_handle, member_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface native vlan set failed on device %d "
          "intf handle %lx vlan handle %lx: "
          "vlan member add failed(%s)\n",
          device,
          intf_handle,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "interface native vlan set on device %d "
      "intf handle %lx vlan handle %lx\n",
      device,
      intf_handle,
      vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_native_vlan_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t *vlan_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface native vlan get failed on device %d "
        "intf handle %lx: "
        "interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan get failed on device %d "
        "intf handle %lx: "
        "interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  *vlan_handle = SWITCH_INTF_NATIVE_VLAN_HANDLE(intf_info);

  SWITCH_LOG_DEBUG(
      "interface native vlan get on device %d "
      "intf handle %lx vlan handle %lx\n",
      device,
      intf_handle,
      *vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_native_vlan_id_set_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_vlan_t vlan_id,
    switch_handle_t *member_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface native vlan id set failed on device %d "
        "intf handle %lx: "
        "interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan id set failed on device %d "
        "intf handle %lx: "
        "interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (vlan_id != 0) {
    status = switch_api_vlan_id_to_handle_get(device, vlan_id, &vlan_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface native vlan id set failed on device %d "
          "intf handle %lx: "
          "vlan id to handle get failed(%s)\n",
          device,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }

    if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
      SWITCH_LOG_ERROR(
          "interface native vlan id set failed on device %d "
          "intf handle %lx: "
          "vlan id invalid(%s)\n",
          device,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_api_interface_native_vlan_set(
      device, intf_handle, vlan_handle, member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan id set failed on device %d "
        "intf handle %lx: "
        "interface native vlan handle set failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_native_vlan_id_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_vlan_t *vlan_id) {
  switch_interface_info_t *intf_info = NULL;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface native vlan id get failed on device %d "
        "intf handle %lx: "
        "interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface native vlan id get failed on device %d "
        "intf handle %lx: "
        "interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  vlan_handle = SWITCH_INTF_NATIVE_VLAN_HANDLE(intf_info);
  *vlan_id = 0;

  if (SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = switch_api_vlan_handle_to_id_get(device, vlan_handle, vlan_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "interface native vlan id get failed on device %d "
          "intf handle %lx vlan handle %lx: "
          "interface get failed(%s)\n",
          device,
          intf_handle,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "interface native vlan id get on device %d "
      "intf handle %lx vlan handle %lx\n",
      device,
      intf_handle,
      *vlan_id);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_attribute_set_internal(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const switch_uint64_t intf_flags,
    const switch_api_interface_info_t *api_intf_info) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface attribute set failed on device %d "
        "interface handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface attribute set failed on device %d "
        "interface get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "interface attribute set on device %d "
      "intf flags 0x%x intf handle %lx\n",
      device,
      intf_flags,
      intf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_interface_attribute_get_internal(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const switch_uint64_t intf_flags,
    switch_api_interface_info_t *api_intf_info) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_intf_info != NULL);
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "interface attribute get failed on device %d "
        "intf handle %lx: "
        "intf handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface attribute get failed on device %d "
        "intf handle %lx: "
        "intf get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  *api_intf_info = intf_info->api_intf_info;

  return status;
}

switch_status_t switch_api_interface_ifindex_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_ifindex_t *ifindex) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: interface handle invalid(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface ifindex deallocate failed on device %d "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  *ifindex = intf_info->ifindex;
  return status;
}

switch_status_t switch_api_interface_ln_handle_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t *ln_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  *ln_handle = SWITCH_API_INVALID_HANDLE;
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device %d: %s\n", device, switch_error_to_string(status));
    return status;
  }
  *ln_handle = intf_info->ln_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_interface_stats_get_internal(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  switch_interface_info_t *intf_info = NULL;
  switch_handle_type_t handle_type = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface stats get failed on device %d "
        "intf handle %lx: interface get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, sizeof(switch_counter_t) * num_entries);

  handle = intf_info->api_intf_info.handle;
  handle_type = switch_handle_type_get(handle);

  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_PORT:
      status = switch_api_interface_port_stats_get(
          device, handle, num_entries, counter_id, counters);
      break;
    case SWITCH_HANDLE_TYPE_LAG:
      status = switch_api_interface_lag_stats_get(
          device, handle, num_entries, counter_id, counters);
      break;
    default:
      break;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface stats get failed on device %d intf handle 0x%lx: "
        "stats get failed(%s)\n",
        device,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_interface_native_vlan_id_set(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_vlan_t vlan_id,
    switch_handle_t *member_handle) {
  SWITCH_MT_WRAP(switch_api_interface_native_vlan_id_set_internal(
      device, intf_handle, vlan_id, member_handle))
}

switch_status_t switch_api_interface_create(
    switch_device_t device,
    switch_api_interface_info_t *intf_info,
    switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(
      switch_api_interface_create_internal(device, intf_info, intf_handle))
}

switch_status_t switch_api_interface_handle_get(switch_device_t device,
                                                switch_handle_t intf_handle,
                                                switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(switch_api_interface_handle_get_internal(
      device, intf_handle, port_handle))
}

switch_status_t switch_api_interface_ifindex_get(
    switch_device_t device,
    switch_handle_t interface_handle,
    switch_ifindex_t *ifindex) {
  SWITCH_MT_WRAP(switch_api_interface_ifindex_get_internal(
      device, interface_handle, ifindex))
}

switch_status_t switch_api_interface_native_vlan_get(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t *vlan_handle) {
  SWITCH_MT_WRAP(switch_api_interface_native_vlan_get_internal(
      device, intf_handle, vlan_handle))
}

switch_status_t switch_api_interface_ln_handle_get(switch_device_t device,
                                                   switch_handle_t intf_handle,
                                                   switch_handle_t *ln_handle) {
  SWITCH_MT_WRAP(switch_api_interface_ln_handle_get_internal(
      device, intf_handle, ln_handle))
}

switch_status_t switch_api_interface_native_vlan_id_get(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_vlan_t *vlan_id) {
  SWITCH_MT_WRAP(switch_api_interface_native_vlan_id_get_internal(
      device, intf_handle, vlan_id))
}

switch_status_t switch_api_interface_attribute_set(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const switch_uint64_t intf_flags,
    const switch_api_interface_info_t *api_intf_info) {
  SWITCH_MT_WRAP(switch_api_interface_attribute_set_internal(
      device, intf_handle, intf_flags, api_intf_info))
}

switch_status_t switch_api_interface_attribute_get(
    const switch_device_t device,
    const switch_handle_t intf_handle,
    const switch_uint64_t intf_flags,
    switch_api_interface_info_t *api_intf_info) {
  SWITCH_MT_WRAP(switch_api_interface_attribute_get_internal(
      device, intf_handle, intf_flags, api_intf_info))
}

switch_status_t switch_api_interface_by_type_get(
    switch_device_t device,
    switch_handle_t handle,
    switch_interface_type_t intf_type,
    switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(switch_api_interface_by_type_get_internal(
      device, handle, intf_type, intf_handle))
}

switch_status_t switch_api_interface_native_vlan_set(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_handle_t vlan_handle,
    switch_handle_t *member_handle) {
  SWITCH_MT_WRAP(switch_api_interface_native_vlan_set_internal(
      device, intf_handle, vlan_handle, member_handle))
}

switch_status_t switch_api_interface_delete(switch_device_t device,
                                            switch_handle_t interface_handle) {
  SWITCH_MT_WRAP(switch_api_interface_delete_internal(device, interface_handle))
}

switch_status_t switch_api_interface_stats_get(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_interface_stats_get_internal(
      device, intf_handle, num_entries, counter_id, counters));
}
