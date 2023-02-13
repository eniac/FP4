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

#include "switchapi/switch_stp.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_STP

#ifdef __cplusplus
extern "C" {
#endif /* cplusplus */

/*
 * Routine Description:
 *   @brief initialize stp structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_stp_init(switch_device_t device) {
  switch_size_t stp_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_api_table_size_get(device, SWITCH_TABLE_STP, &stp_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp init failed on device %d: "
        "table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_STP, stp_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp init failed on device %d: "
        "stp handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("stp init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return SWITCH_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *   @brief free stp structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_stp_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_STP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp free failed on device %d: "
        "stp handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("stp free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief stp group create
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] stp_mode - spanning tree mode
 *   @param[out] stp_handle - spanning tree group handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_stp_group_create_internal(
    const switch_device_t device,
    const switch_stp_mode_t stp_mode,
    switch_handle_t *stp_handle) {
  switch_stp_info_t *stp_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  handle = switch_stp_handle_create(device);
  if (stp_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "stp group create failed on device %d: "
        "stp handle create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "stp group create failed on device %d: "
        "stp get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_INIT(&(stp_info->network_list));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = SWITCH_LIST_INIT(&(stp_info->intf_list));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  *stp_handle = handle;

  SWITCH_LOG_DEBUG(
      "stp group created on device %d stp handle 0x%lx\n", device, handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief stp group delete
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] stp_handle - spanning tree group handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_stp_group_delete_internal(
    const switch_device_t device, const switch_handle_t stp_handle) {
  switch_stp_info_t *stp_info = NULL;
  switch_stp_intf_entry_t *intf_entry = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp group delete failed on device %d stp handle 0x%lx: "
        "stp handle invalid(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp group delete failed on device %d stp handle 0x%lx: "
        "stp get failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(stp_info->intf_list, node) {
    intf_entry = (switch_stp_intf_entry_t *)node->data;
    status = switch_api_stp_interface_state_set(device,
                                                stp_handle,
                                                intf_entry->intf_handle,
                                                SWITCH_PORT_STP_STATE_NONE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp group delete failed on device %d "
          "stp handle 0x%lx port handle 0x%lx: "
          "stp port state set failed(%s)\n",
          device,
          stp_handle,
          intf_entry->intf_handle,
          switch_error_to_string(status));
    }
  }
  FOR_EACH_IN_LIST_END();

  FOR_EACH_IN_LIST(stp_info->network_list, node) {
    network_entry = (switch_stp_network_entry_t *)node->data;
    status = switch_api_stp_group_member_remove(
        device, stp_handle, network_entry->handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp group delete failed on device %d "
          "stp handle 0x%lx network handle 0x%lx: "
          "stp network remove failed(%s)\n",
          device,
          stp_handle,
          network_entry->handle,
          switch_error_to_string(status));
    }
  }
  FOR_EACH_IN_LIST_END();

  status = switch_stp_handle_delete(device, stp_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp group delete failed on device %d stp handle 0x%lx: "
        "stp handle delete failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "stp group deleted on device %d stp handle 0x%lx\n", device, stp_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_stp_group_member_add_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle) {
  switch_stp_info_t *stp_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_handle_t bd_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "stp handle invalid(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "stp get failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle)) {
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "network handle invalid(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_bd_handle_get(device, network_handle, &bd_handle);

  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  network_entry = SWITCH_MALLOC(device, sizeof(switch_stp_network_entry_t), 1);
  if (!network_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "memory allocation failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(network_entry, 0x0, sizeof(switch_stp_network_entry_t));

  network_entry->handle = network_handle;
  bd_info->stp_handle = stp_handle;
  bd_info->bd_flags |= SWITCH_BD_ATTR_STP_HANDLE;

  status = switch_pd_bd_table_entry_update(
      device, handle_to_id(bd_handle), bd_info, bd_info->bd_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network add failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "bd table update failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LIST_INSERT(
      &(stp_info->network_list), &(network_entry->node), network_entry);

  SWITCH_LOG_DEBUG(
      "stp network added on device %d "
      "stp handle 0x%lx network handle 0x%lxn",
      device,
      stp_handle,
      network_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_stp_group_member_remove_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle) {
  switch_stp_info_t *stp_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_node_t *node = NULL;
  switch_handle_t bd_handle = 0;
  bool entry_found = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "stp handle invalid(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "stp get failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle)) {
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "network handle invalid(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }
  status = switch_bd_handle_get(device, network_handle, &bd_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx: "
        "bd get failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        switch_error_to_string(status));
    return status;
  }

  entry_found = FALSE;
  FOR_EACH_IN_LIST(stp_info->network_list, node) {
    network_entry = (switch_stp_network_entry_t *)node->data;
    if (network_entry->handle == network_handle) {
      entry_found = TRUE;
      break;
    }
    node = node->next;
  }
  FOR_EACH_IN_LIST_END();

  if (!entry_found) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "network not found(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_info->stp_handle = SWITCH_API_INVALID_HANDLE;
  status = switch_pd_bd_table_entry_update(
      device, handle_to_id(bd_handle), bd_info, bd_info->bd_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "bd update failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&stp_info->network_list, node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network remove failed on device %d "
        "stp handle 0x%lx network handle 0x%lx bd handle 0x%lx: "
        "bd update failed(%s)\n",
        device,
        stp_handle,
        network_handle,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  bd_info->stp_handle = SWITCH_API_INVALID_HANDLE;
  SWITCH_FREE(device, network_entry);

  SWITCH_LOG_DEBUG(
      "stp network removed on device %d "
      "stp handle 0x%lx network handle 0x%lx\n",
      device,
      stp_handle,
      network_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_stp_intf_entry_find(
    switch_device_t device,
    switch_handle_t stp_handle,
    switch_handle_t handle,
    switch_stp_intf_entry_t **intf_entry) {
  switch_stp_info_t *stp_info = NULL;
  switch_stp_intf_entry_t *tmp_intf_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(intf_entry != NULL);

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp port entry find failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp handle invalid",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port entry find failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp get failed",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  *intf_entry = NULL;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  FOR_EACH_IN_LIST(stp_info->intf_list, node) {
    tmp_intf_entry = (switch_stp_intf_entry_t *)node->data;
    if (tmp_intf_entry->intf_handle == handle) {
      *intf_entry = tmp_intf_entry;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DETAIL(
      "stp port entry found on device %d "
      "stp handle 0x%lx handle 0x%lx\n",
      device,
      stp_handle,
      handle);

  return status;
}

/*
 * Routine Description:
 *   @brief stp port state set
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] stp_handle - spanning tree group handle
 *   @param[in] handle - interface handle
 *   @param[in] stp_state - spanning tree state
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_stp_interface_state_set_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t handle,
    const switch_stp_state_t stp_state) {
  switch_stp_info_t *stp_info = NULL;
  switch_stp_intf_entry_t *intf_entry = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_interface_info_t *interface_info = NULL;
  switch_ifindex_t ifindex = 0;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp port state set failed on device %d "
        "stp handle 0x%lx handle 0x%lx stp state: "
        "stp handle invalid(%s)\n",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(handle));
  if (!SWITCH_INTERFACE_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp port state set failed on device %d "
        "stp handle 0x%lx handle 0x%lx stp state: "
        "interface handle invalid(%s)\n",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port state set failed on device %d "
        "stp handle 0x%lx handle 0x%lx stp state: "
        "stp get failed(%s)\n",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, handle, &interface_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port state set failed on device %d "
        "stp handle 0x%lx handle 0x%lx stp state: "
        "port get failed",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  ifindex = interface_info->ifindex;

  status = switch_stp_intf_entry_find(device, stp_handle, handle, &intf_entry);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "stp port state set failed on device %d "
        "stp handle 0x%lx handle 0x%lx stp state: "
        "stp port entry find failed(%s)\n",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    if (stp_state == SWITCH_PORT_STP_STATE_NONE) {
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "stp port entry does not exist(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      return status;
    }

    intf_entry = SWITCH_MALLOC(device, sizeof(switch_stp_intf_entry_t), 0x1);
    if (!intf_entry) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "memory allocation failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(intf_entry, 0x0, sizeof(switch_stp_intf_entry_t));
    status =
        SWITCH_LIST_INSERT(&stp_info->intf_list, &intf_entry->node, intf_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "stp port list insertion failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      goto cleanup;
    }

    intf_entry->hw_entry = SWITCH_PD_INVALID_HANDLE;
  }

  if (stp_state == SWITCH_PORT_STP_STATE_NONE) {
    if (SWITCH_HW_FLAG_ISSET(intf_entry, SWITCH_STP_PD_INTF_ENTRY)) {
      status = switch_pd_spanning_tree_table_entry_delete(device,
                                                          intf_entry->hw_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "stp port state set failed on device %d "
            "stp handle 0x%lx handle 0x%lx stp state: "
            "stp entry delete failed(%s)\n",
            device,
            stp_handle,
            handle,
            switch_stp_state_to_string(stp_state),
            switch_error_to_string(status));
        return status;
      }
      SWITCH_HW_FLAG_CLEAR(intf_entry, SWITCH_STP_PD_INTF_ENTRY);
    }

    status = SWITCH_LIST_DELETE(&stp_info->intf_list, &intf_entry->node);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "stp port list delete failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      return status;
    }

    SWITCH_FREE(device, intf_entry);
    return status;
  }

  if (stp_state == intf_entry->stp_state) {
    SWITCH_LOG_DEBUG(
        "stp port state set on device %d "
        "stp handle 0x%lx handle 0x%lx stp state %s: ",
        device,
        stp_handle,
        handle,
        switch_stp_state_to_string(stp_state));
    return status;
  }

  intf_entry->stp_state = stp_state;
  intf_entry->intf_handle = handle;

  if (!SWITCH_HW_FLAG_ISSET(intf_entry, SWITCH_STP_PD_INTF_ENTRY)) {
    status = switch_pd_spanning_tree_table_entry_add(device,
                                                     handle_to_id(stp_handle),
                                                     ifindex,
                                                     intf_entry->stp_state,
                                                     &intf_entry->hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "stp table add failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(intf_entry, SWITCH_STP_PD_INTF_ENTRY);
  } else {
    status =
        switch_pd_spanning_tree_table_entry_update(device,
                                                   handle_to_id(stp_handle),
                                                   ifindex,
                                                   intf_entry->stp_state,
                                                   intf_entry->hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "stp table update failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  /* If interface not part of vlan/ln, then skip and process next network */
  FOR_EACH_IN_LIST(stp_info->network_list, node) {
    network_entry = (switch_stp_network_entry_t *)node->data;
    status = switch_bd_member_stp_state_set(
        device, network_entry->handle, handle, stp_state);
    if (status != SWITCH_STATUS_SUCCESS) {
      if (status == SWITCH_STATUS_ITEM_NOT_FOUND) continue;
      SWITCH_LOG_ERROR(
          "stp port state set failed on device %d "
          "stp handle 0x%lx handle 0x%lx stp state: "
          "bd member state set failed(%s)\n",
          device,
          stp_handle,
          handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      goto cleanup;
    }
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DEBUG(
      "stp port state set on device %d "
      "stp handle 0x%lx handle 0x%lx stp state %s\n",
      device,
      stp_handle,
      handle,
      switch_stp_state_to_string(stp_state));

  return status;

cleanup:
  return status;
}

/*
 * Routine Description:
 *   @brief stp port state get
 *
 * Arguments:
 *   @param[in] device - device id
 *   @param[in] stp_handle - spanning tree group handle
 *   @param[in] handle - port handle
 *   @param[out] stp_state - spanning tree state
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_stp_interface_state_get_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t handle,
    switch_stp_state_t *stp_state) {
  switch_stp_info_t *stp_info = NULL;
  switch_stp_intf_entry_t *intf_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp port state get failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp handle invalid(%s)\n",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(handle));
  if (!SWITCH_INTERFACE_HANDLE(handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "stp port state get failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "port handle invalid(%s)\n",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(stp_state != NULL);
  if (!stp_state) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp port state get failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp state is null(%s)\n",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port state get failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp get failed(%s)\n",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  *stp_state = SWITCH_PORT_STP_STATE_NONE;

  status = switch_stp_intf_entry_find(device, stp_handle, handle, &intf_entry);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port state get failed on device %d "
        "stp handle 0x%lx handle 0x%lx: "
        "stp port entry find failed(%s)\n",
        device,
        stp_handle,
        handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "stp port state get on device %d "
      "stp handle 0x%lx handle 0x%lx stp state %s\n",
      device,
      stp_handle,
      handle,
      switch_stp_state_to_string(intf_entry->stp_state));

  *stp_state = intf_entry->stp_state;

  return status;
}

switch_status_t switch_api_stp_group_members_get_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    switch_uint16_t *num_entries,
    switch_handle_t **network_handles) {
  switch_stp_info_t *stp_info = NULL;
  switch_stp_network_entry_t *network_entry = NULL;
  switch_node_t *node = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(num_entries != NULL);
  SWITCH_ASSERT(network_handles != NULL);
  if (!num_entries || !network_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp network members get failed on device %d "
        "stp handle 0x%lx: parameters invalid (%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  *num_entries = 0;
  *network_handles = NULL;

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp network members get failed on device %d "
        "stp handle 0x%lx: parameters invalid (%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp network members get failed on device %d "
        "stp handle 0x%lx: stp get failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_LIST_COUNT(&stp_info->network_list)) {
    SWITCH_LOG_ERROR(
        "stp network members get failed on device %d "
        "stp handle 0x%lx: network member list empty(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  *network_handles = SWITCH_MALLOC(device,
                                   sizeof(switch_handle_t),
                                   SWITCH_LIST_COUNT(&stp_info->network_list));
  if (!(*network_handles)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "stp network members get failed on device %d "
        "stp handle 0x%lx:  mmemory allocation failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(stp_info->network_list, node) {
    network_entry = (switch_stp_network_entry_t *)node->data;
    (*network_handles)[index++] = network_entry->handle;
  }
  FOR_EACH_IN_LIST_END();
  *num_entries = index;

  return status;
}

switch_status_t switch_api_stp_interfaces_get_internal(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    switch_uint16_t *num_entries,
    switch_handle_t **intf_handles)

{
  switch_stp_info_t *stp_info = NULL;
  switch_stp_intf_entry_t *intf_entry = NULL;
  switch_node_t *node = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(num_entries != NULL);
  SWITCH_ASSERT(intf_handles != NULL);
  if (!num_entries || !intf_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp port members get failed on device %d "
        "stp handle 0x%lx: parameters invalid (%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  *num_entries = 0;
  *intf_handles = NULL;

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "stp port members get failed on device %d "
        "stp handle 0x%lx: parameters invalid (%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_stp_get(device, stp_handle, &stp_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "stp port members get failed on device %d "
        "stp handle 0x%lx: stp get failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_LIST_COUNT(&stp_info->intf_list)) {
    SWITCH_LOG_ERROR(
        "stp port members get failed on device %d "
        "stp handle 0x%lx: port member list empty(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  *intf_handles = SWITCH_MALLOC(
      device, sizeof(switch_handle_t), SWITCH_LIST_COUNT(&stp_info->intf_list));
  if (!(*intf_handles)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "stp port members get failed on device %d "
        "stp handle 0x%lx:  mmemory allocation failed(%s)\n",
        device,
        stp_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(stp_info->intf_list, node) {
    intf_entry = (switch_stp_intf_entry_t *)node->data;
    (*intf_handles)[index++] = intf_entry->intf_handle;
  }
  FOR_EACH_IN_LIST_END();
  *num_entries = index;

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_stp_interface_state_get(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t intf_handle,
    switch_stp_state_t *state) {
  SWITCH_MT_WRAP(switch_api_stp_interface_state_get_internal(
      device, stp_handle, intf_handle, state))
}

switch_status_t switch_api_stp_group_delete(const switch_device_t device,
                                            const switch_handle_t stp_handle) {
  SWITCH_MT_WRAP(switch_api_stp_group_delete_internal(device, stp_handle))
}

switch_status_t switch_api_stp_interfaces_get(const switch_device_t device,
                                              const switch_handle_t stp_handle,
                                              switch_uint16_t *num_entries,
                                              switch_handle_t **port_handles) {
  SWITCH_MT_WRAP(switch_api_stp_interfaces_get_internal(
      device, stp_handle, num_entries, port_handles))
}

switch_status_t switch_api_stp_group_members_get(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    switch_uint16_t *num_entries,
    switch_handle_t **network_handles) {
  SWITCH_MT_WRAP(switch_api_stp_group_members_get_internal(
      device, stp_handle, num_entries, network_handles))
}

switch_status_t switch_api_stp_group_create(const switch_device_t device,
                                            const switch_stp_mode_t stp_mode,
                                            switch_handle_t *stp_handle) {
  SWITCH_MT_WRAP(
      switch_api_stp_group_create_internal(device, stp_mode, stp_handle))
}

switch_status_t switch_api_stp_group_member_add(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle) {
  SWITCH_MT_WRAP(switch_api_stp_group_member_add_internal(
      device, stp_handle, network_handle))
}

switch_status_t switch_api_stp_interface_state_set(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t handle,
    const switch_stp_state_t state) {
  SWITCH_MT_WRAP(switch_api_stp_interface_state_set_internal(
      device, stp_handle, handle, state))
}

switch_status_t switch_api_stp_group_member_remove(
    const switch_device_t device,
    const switch_handle_t stp_handle,
    const switch_handle_t network_handle) {
  SWITCH_MT_WRAP(switch_api_stp_group_member_remove_internal(
      device, stp_handle, network_handle))
}
