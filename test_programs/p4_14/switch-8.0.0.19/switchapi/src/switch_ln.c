/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switchapi/switch_ln.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_LOGICAL_NETWORK

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_ln_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_ln_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_ln_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK, SWITCH_LN_HANDLE_SIZE);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln init failed on device %d: "
        "ln handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("ln init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_ln_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln free failed on device %d: "
        "ln handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("ln free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_logical_network_create_internal(
    const switch_device_t device, switch_handle_t *ln_handle) {
  switch_ln_info_t *ln_info = NULL;
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(ln_handle != NULL);
  if (!ln_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln create failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_ln_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    SWITCH_LOG_ERROR(
        "ln create failed on device %d: "
        "ln handle create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln create failed on device %d: "
        "ln get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_info.bd_type = SWITCH_BD_TYPE_LN;
  bd_info.handle = handle;

  bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_flags |= SWITCH_BD_ATTR_UUC_FLOODING_ENABLED;
  bd_flags |= SWITCH_BD_ATTR_UMC_FLOODING_ENABLED;
  bd_flags |= SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED;
  status = switch_api_multicast_index_create(device, &bd_info.flood_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln create failed on device %d: "
        "unknown ucast index create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  bd_flags |= SWITCH_BD_ATTR_LEARNING;
  bd_info.learning = TRUE;

  status = switch_bd_create(device, bd_flags, &bd_info, &bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln create failed for device %d: "
        "bd create failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_bd_stats_enable(device, bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln create failed for device %d: "
        "bd stats enable failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  ln_info->bd_handle = bd_handle;
  *ln_handle = handle;

  SWITCH_LOG_DEBUG("ln created on device %d ln handle 0x%lx bd handle 0x%lx\n",
                   device,
                   handle,
                   bd_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_logical_network_delete_internal(
    switch_device_t device, switch_handle_t ln_handle) {
  switch_ln_info_t *ln_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, ln_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_multicast_index_delete(device, bd_info->flood_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "ucast mc index free failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_disable(device, ln_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "bd stats disable failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_delete(device, ln_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln delete failed on device %d ln handle 0x%lx: "
        "ln bd delete failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_handle_delete(device, ln_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "ln deleted on device %d ln handle 0x%lx\n", device, ln_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_logical_network_member_add_internal(
    switch_device_t device,
    switch_handle_t ln_handle,
    switch_handle_t intf_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_ln_info_t *ln_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_mcast_member_t mcast_member;
  bool mcast_member_add = true;
  switch_handle_t member_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_vlan_t vlan_id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_DEBUG(
      "ln member add on device %d ln handle 0x%lx intf handle 0x%lx\n",
      device,
      ln_handle,
      intf_handle);

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_LN_HANDLE(ln_handle) || !SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "handle invalid(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "ln get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "interface get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if ((SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_ACCESS) &&
      (SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_PORT_VLAN)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "interface type is not l2:(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_handle = ln_info->bd_handle;

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "bd member already exists(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (SWITCH_INTF_TYPE(intf_info)) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
      vlan_id = 0;
      break;
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      vlan_id = SWITCH_INTF_ATTR_VLAN_ID(intf_info);
      break;
    default:
      status = SWITCH_STATUS_UNSUPPORTED_TYPE;
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "logical network member add failed "
            "on device %d ln handle 0x%lx "
            "intf handle 0x%lx: "
            "ln member add failed(%s)",
            device,
            ln_handle,
            intf_handle,
            switch_error_to_string(status));
        return status;
      }
  }

  status = switch_pv_member_add(
      device, bd_handle, intf_handle, 0x0, vlan_id, 0x0, &member_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member add failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "pv member add failed(%s)",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (mcast_member_add) {
    mcast_member.handle = intf_handle;
    mcast_member.network_handle = ln_handle;
    status = switch_api_multicast_member_add(
        device, bd_info->flood_handle, 0x1, &mcast_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "logical network member add failed on device %d "
          "ln handle 0x%lx intf handle 0x%lx: "
          "uc flood mcast member add failed(%s)\n",
          device,
          ln_handle,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  intf_info->ln_handle = ln_handle;
  SWITCH_LOG_DEBUG(
      "ln member added on device %d "
      "ln handle 0x%lx intf handle 0x%lx",
      device,
      ln_handle,
      intf_handle);

  SWITCH_LOG_EXIT();

  return status;
cleanup:
  return status;
}

switch_status_t switch_api_logical_network_member_remove_internal(
    switch_device_t device,
    switch_handle_t ln_handle,
    switch_handle_t intf_handle) {
  switch_interface_info_t *intf_info = NULL;
  switch_ln_info_t *ln_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_mcast_member_t mcast_member;
  bool mcast_member_delete = true;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_DEBUG(
      "ln member remove on device %d ln handle 0x%lx intf handle 0x%lx\n",
      device,
      ln_handle,
      intf_handle);

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_LN_HANDLE(ln_handle) || !SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "handle invalid(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "ln get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "interface get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if ((SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_ACCESS) &&
      (SWITCH_INTF_TYPE(intf_info) != SWITCH_INTERFACE_TYPE_PORT_VLAN)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "interface type is not l2(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_handle = ln_info->bd_handle;

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "bd member not found(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (mcast_member_delete) {
    mcast_member.handle = intf_handle;
    mcast_member.network_handle = ln_handle;
    status = switch_api_multicast_member_delete(
        device, bd_info->flood_handle, 0x1, &mcast_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "logical network member remove failed on device %d "
          "ln handle 0x%lx intf handle 0x%lx: "
          "uc flood mcast member remove failed(%s)\n",
          device,
          ln_handle,
          intf_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_pv_member_delete(device, bd_handle, intf_handle, 0x0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "logical network member remove failed on device %d "
        "ln handle 0x%lx intf handle 0x%lx: "
        "pv member delete failed(%s)\n",
        device,
        ln_handle,
        intf_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  intf_info->ln_handle = SWITCH_API_INVALID_HANDLE;
  SWITCH_LOG_DEBUG(
      "ln member removed on device %d "
      "ln handle 0x%lx intf handle 0x%lx",
      device,
      ln_handle,
      intf_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_logical_network_learning_set_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const bool enable) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln learning enabled set failed on device %d "
        "ln handle 0x%lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln learning enabled set failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_learning_set(device, ln_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln learning enabled set failed on device %d "
        "ln handle 0x%lx: learn enable bd set failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ln learning enabled set on device %d "
      "ln handle 0x%lx enable %d\n",
      device,
      ln_handle,
      enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_logical_network_learning_get_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    bool *enable) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln learning enabled get failed on device %d "
        "ln handle 0x%lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln learning enabled get failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_learning_get(device, ln_info->bd_handle, enable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln learning enabled get failed on device %d "
        "ln handle 0x%lx: learn enable bd get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ln learning enabled get on device %d "
      "ln handle 0x%lx enable %d\n",
      device,
      ln_handle,
      *enable);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_logical_network_attribute_set_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    const switch_api_ln_info_t *api_ln_info) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln attribute set failed on device %d "
        "ln handle 0x%lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln attribute set failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_logical_network_attribute_get_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    switch_api_ln_info_t *api_ln_info) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln attribute get failed on device %d "
        "ln handle 0x%lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln attribute get failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_logical_network_bd_get_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    switch_uint32_t *bd) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln bd get failed on device %d "
        "ln handle 0x%lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln bd get failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  *bd = handle_to_id(ln_info->bd_handle);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_logical_network_learning_set(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const bool enable) {
  SWITCH_MT_WRAP(switch_api_logical_network_learning_set_internal(
      device, ln_handle, enable))
}

switch_status_t switch_api_logical_network_member_add(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_handle_t intf_handle) {
  SWITCH_MT_WRAP(switch_api_logical_network_member_add_internal(
      device, ln_handle, intf_handle))
}

switch_status_t switch_api_logical_network_create(const switch_device_t device,
                                                  switch_handle_t *ln_handle) {
  SWITCH_MT_WRAP(switch_api_logical_network_create_internal(device, ln_handle))
}

switch_status_t switch_api_logical_network_member_remove(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_handle_t intf_handle) {
  SWITCH_MT_WRAP(switch_api_logical_network_member_remove_internal(
      device, ln_handle, intf_handle))
}

switch_status_t switch_api_logical_network_learning_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    bool *enable) {
  SWITCH_MT_WRAP(switch_api_logical_network_learning_get_internal(
      device, ln_handle, enable))
}

switch_status_t switch_api_logical_network_attribute_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    switch_api_ln_info_t *api_ln_info) {
  SWITCH_MT_WRAP(switch_api_logical_network_attribute_get_internal(
      device, ln_handle, flags, api_ln_info))
}

switch_status_t switch_api_logical_network_delete(
    const switch_device_t device, const switch_handle_t ln_handle) {
  SWITCH_MT_WRAP(switch_api_logical_network_delete_internal(device, ln_handle))
}

switch_status_t switch_api_logical_network_attribute_set(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    const switch_api_ln_info_t *api_ln_info) {
  SWITCH_MT_WRAP(switch_api_logical_network_attribute_set_internal(
      device, ln_handle, flags, api_ln_info))
}

switch_status_t switch_api_logical_network_bd_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    switch_uint32_t *bd) {
  SWITCH_MT_WRAP(
      switch_api_logical_network_bd_get_internal(device, ln_handle, bd))
}

switch_status_t switch_api_logical_network_members_get(
    switch_device_t device,
    switch_handle_t ln_handle,
    switch_uint16_t *mbr_count,
    switch_handle_t **mbrs) {
  switch_ln_info_t *ln_info = NULL;
  switch_node_t *node = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_uint16_t mbr_count_max = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_bd_info_t *bd_info = NULL;

  SWITCH_ASSERT(mbr_count != NULL);
  SWITCH_ASSERT(mbrs != NULL);
  if (!mbr_count || !mbrs) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("LN members get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln members get failed on device %d "
        "ln handle %lx: ln handle invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln attribute get failed on device %d "
        "ln handle %lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, ln_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan interfaces get failed for device %d(%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *mbrs = NULL;
  *mbr_count = 0;
  FOR_EACH_IN_LIST(bd_info->members, node) {
    bd_member = (switch_bd_member_t *)node->data;

    if (mbr_count_max == *mbr_count) {
      mbr_count_max += 16;
      *mbrs = SWITCH_REALLOC(
          device, *mbrs, (sizeof(switch_handle_t) * mbr_count_max));
    }
    (*mbrs)[*mbr_count] = bd_member->member_handle;
    (*mbr_count)++;
  }
  FOR_EACH_IN_LIST_END();

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_logical_network_stats_get_internal(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint8_t count,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters) {
  switch_ln_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);

  if (!counters || !counter_ids) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln stats get failed on device %d ln handle 0x%lx: "
        "parameters invalid:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ln stats get failed on device %d ln handle 0x%lx: "
        "ln handle invalid:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, count * sizeof(switch_counter_t));
  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln stats get failed on device %d ln handle 0x%lx: "
        "ln get failed:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_get(
      device, ln_info->bd_handle, count, counter_ids, counters);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln stats get failed on device %d ln handle 0x%lx: "
        "bd stats get failed:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_logical_network_stats_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint8_t num_entries,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_logical_network_stats_get_internal(
      device, ln_handle, num_entries, counter_ids, counters));
}

switch_status_t switch_api_logical_network_stats_clear_internal(
    const switch_device_t device, const switch_handle_t ln_handle) {
  switch_vlan_info_t *ln_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "ln stats clear failed on device %d ln handle 0x%lx: "
        "ln handle invalid:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln stats clear failed on device %d ln handle 0x%lx: "
        "ln get failed:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_stats_clear(device, ln_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln stats clear failed on device %d ln handle 0x%lx: "
        "bd stats clear failed:(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  return status;
}
switch_status_t switch_api_logical_network_stats_clear(
    const switch_device_t device, const switch_handle_t ln_handle) {
  SWITCH_MT_WRAP(
      switch_api_logical_network_stats_clear_internal(device, ln_handle));
}
