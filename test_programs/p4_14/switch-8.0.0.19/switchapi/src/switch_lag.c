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

#include "switchapi/switch_lag.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_LAG

switch_status_t switch_lag_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_lag_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag default entry add failed on device %d "
        "lag table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_lag_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_lag_init(switch_device_t device) {
  switch_size_t lag_table_size = 0;
  switch_size_t lag_member_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_LAG_GROUP, &lag_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag init failed on device %d: "
        "lag table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(lag_table_size != 0);
  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_LAG, lag_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag init failed on device %d: "
        "lag handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_LAG_SELECT, &lag_member_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag init failed on device %d: "
        "lag member table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG);
    return status;
  }

  SWITCH_ASSERT(lag_member_table_size != 0);

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_LAG_MEMBER, lag_member_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag init failed on device %d "
        "lag member handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG);
    return status;
  }

  status = switch_pd_lag_action_profile_set_fallback_member(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag init failed on device %d "
        "lag fallback member set failed(%s)\n",
        device,
        switch_error_to_string(status));
    switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG);
    return status;
  }

  SWITCH_LOG_DEBUG("lag init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_lag_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag free failed on device %d: "
        "lag handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG_MEMBER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag free failed on device %d: "
        "lag member handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_LOG_DEBUG("lag free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_lag_prune_mask_table_update(switch_device_t device,
                                                   switch_handle_t lag_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_mc_port_map_t port_map;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag prune mask update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag prune mask update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(port_map, 0x0, sizeof(switch_mc_port_map_t));

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;

    switch_port_info_t *port_info = NULL;
    status = switch_port_get(device, lag_member->port_handle, &port_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag prune mask update failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_MC_PORT_MAP_SET(port_map, port_info->dev_port);
  }
  FOR_EACH_IN_LIST_END();

  status = switch_pd_prune_mask_table_update(device, lag_info->yid, port_map);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag prune mask update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_lag_mcast_port_map_update(switch_device_t device,
                                                 switch_handle_t lag_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_mc_port_map_t port_map;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag mcast port map update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag mcast port map update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(port_map, 0x0, sizeof(switch_mc_port_map_t));

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    SWITCH_ASSERT(SWITCH_PORT_HANDLE(lag_member->port_handle));
    status = switch_port_get(device, lag_member->port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    SWITCH_MC_PORT_MAP_SET(port_map, port_info->dev_port);
  }
  FOR_EACH_IN_LIST_END();

  status = switch_pd_lag_mcast_port_map_update(
      device, handle_to_id(lag_handle), port_map);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag mcast port map update failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_lag_create_internal(switch_device_t device,
                                               switch_handle_t *lag_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  handle = switch_lag_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag handle create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(lag_info, 0x0, sizeof(switch_lag_info_t));
  lag_info->type = SWITCH_API_LAG_SIMPLE;
  lag_info->ingress_port_lag_label = 0;
  lag_info->egress_port_lag_label = 0;
  lag_info->port_lag_index =
      SWITCH_COMPUTE_PORT_LAG_INDEX(handle, SWITCH_PORT_LAG_INDEX_TYPE_LAG);
  lag_info->bind_mode = SWITCH_PORT_BIND_MODE_PORT;

  status = SWITCH_LIST_INIT(&lag_info->members);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag group table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = SWITCH_ARRAY_INIT(&lag_info->intf_array);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag group table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_lag_group_create(device, &lag_info->pd_group_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag group table add failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_lag_group_register_callback(device, (void *)handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag create failed on device %d: "
        "lag group callback register failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG(
      "lag handle created on device %d handle 0x%lx "
      "port_lag_index 0x%x yid %d\n",
      device,
      handle,
      lag_info->port_lag_index,
      lag_info->yid);

  *lag_handle = handle;

  SWITCH_LOG_EXIT();

  return status;

cleanup:

  switch_lag_handle_delete(device, handle);
  return status;
}

switch_status_t switch_api_lag_delete_internal(switch_device_t device,
                                               switch_handle_t lag_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "lag delete failed on device %d: "
        "lag handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag delete failed on device %d: "
        "lag get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_ARRAY_COUNT(&lag_info->intf_array)) {
    status = SWITCH_STATUS_PORT_IN_USE;
    SWITCH_LOG_ERROR(
        "lag delete failed on device %d lag handle 0x%lx: "
        "interface still referenced(%s)\n",
        device,
        lag_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    status = switch_api_lag_member_delete(
        device, lag_handle, lag_member->direction, lag_member->port_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "lag delete failed on device %d: "
          "lag member delete failed(%s)\n",
          device,
          switch_error_to_string(status));
    }
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }
  FOR_EACH_IN_LIST_END();

  status = switch_pd_lag_group_delete(device, lag_info->pd_group_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag delete failed on device %d: "
        "lag group table delete failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_lag_handle_delete(device, lag_handle);

  SWITCH_LOG_DEBUG(
      "lag deleted on device %d lag handle 0x%lx\n", device, lag_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_lag_member_search(switch_device_t device,
                                         switch_handle_t lag_handle,
                                         switch_handle_t port_handle,
                                         switch_lag_member_t **lag_member) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *tmp_lag_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(lag_member != NULL);

  if (!lag_member) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("lag member search failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *lag_member = NULL;

  status = switch_lag_get(device, lag_handle, &lag_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member search failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  FOR_EACH_IN_LIST(lag_info->members, node) {
    tmp_lag_member = (switch_lag_member_t *)node->data;
    if (tmp_lag_member->port_handle == port_handle) {
      *lag_member = tmp_lag_member;
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_api_lag_member_add_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_direction_t direction,
    switch_handle_t port_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_port_info_t *port_info = NULL;
  switch_handle_t lag_member_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool ingress_enable = FALSE;
  bool egress_enable = FALSE;
  bool new_lag_member = TRUE;
  switch_port_lag_label_t port_lag_label = 0;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));

  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_lag_member_search(device, lag_handle, port_handle, &lag_member);

  if (status != SWITCH_STATUS_ITEM_NOT_FOUND &&
      status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    lag_member_handle = switch_lag_member_handle_create(device);
    status = switch_lag_member_get(device, lag_member_handle, &lag_member);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    lag_member->port_handle = port_handle;
    lag_member->lag_member_handle = lag_member_handle;
    lag_member->lag_handle = lag_handle;
    lag_member->direction = direction;
    lag_member->active = TRUE;
    new_lag_member = TRUE;

    ingress_enable = (direction == SWITCH_API_DIRECTION_BOTH ||
                      direction == SWITCH_API_DIRECTION_INGRESS)
                         ? TRUE
                         : FALSE;
    egress_enable = (direction == SWITCH_API_DIRECTION_BOTH ||
                     direction == SWITCH_API_DIRECTION_EGRESS)
                        ? TRUE
                        : FALSE;
  } else {
    if (lag_member->direction == SWITCH_API_DIRECTION_BOTH ||
        lag_member->direction == direction) {
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    if ((lag_member->direction == SWITCH_API_DIRECTION_INGRESS) &&
        (direction == SWITCH_API_DIRECTION_EGRESS ||
         direction == SWITCH_API_DIRECTION_BOTH)) {
      egress_enable = TRUE;
    }

    if ((lag_member->direction == SWITCH_API_DIRECTION_EGRESS) &&
        (direction == SWITCH_API_DIRECTION_INGRESS ||
         direction == SWITCH_API_DIRECTION_BOTH)) {
      ingress_enable = TRUE;
    }
  }

  SWITCH_ASSERT(ingress_enable || egress_enable);

  status =
      SWITCH_LIST_INSERT(&lag_info->members, &lag_member->node, lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  if (egress_enable) {
    status = switch_pd_lag_member_add(device,
                                      lag_info->pd_group_hdl,
                                      port_info->dev_port,
                                      &(lag_member->mbr_hdl));

    if (lag_info->member_count == 0) {
      status =
          switch_pd_lag_group_table_with_selector_add(device,
                                                      lag_info->port_lag_index,
                                                      lag_info->pd_group_hdl,
                                                      &lag_info->hw_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_lag_mcast_port_map_update(device, lag_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    /*
     * Use the LAG ACL label when it is valid. If not, use port's ACL Label.
     */
    if (lag_info->egress_port_lag_label) {
      port_lag_label = lag_info->egress_port_lag_label;
    } else {
      port_lag_label = port_info->egress_port_lag_label;
    }

    status = switch_pd_egress_port_mapping_table_entry_update(
        device,
        port_info->dev_port,
        port_lag_label,
        port_info->port_type,
        port_info->egress_qos_group,
        port_info->egress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (ingress_enable) {
    status = switch_port_prune_mask_table_update(device, port_info, TRUE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_yid_free(device, port_info->yid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    port_info->yid = SWITCH_YID_INVALID;

    if (lag_info->member_count == 0) {
      status = switch_yid_allocate(device, &lag_info->yid);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    status = switch_lag_prune_mask_table_update(device, lag_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_ingress_port_mapping_table_entry_update(
        device,
        port_info->dev_port,
        lag_info->port_lag_index,
        port_info->port_type,
        port_info->ingress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    /*
     * Use the LAG ACL label when it is valid. If not, use port's ACL Label.
     */
    if (lag_info->ingress_port_lag_label) {
      port_lag_label = lag_info->ingress_port_lag_label;
    } else {
      port_lag_label = port_info->ingress_port_lag_label;
    }

    status = switch_pd_ingress_port_properties_table_entry_update(
        device,
        lag_info->yid,
        port_info,
        port_lag_label,
        port_info->ingress_prop_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  port_info->lag_handle = lag_handle;

  if (new_lag_member) {
    lag_info->member_count++;
  }

  SWITCH_LOG_DEBUG(
      "lag member add success on device %d "
      "direction %s lag handle 0x%lx"
      "port handle 0x%lx lag member handle 0x%lx\n",
      device,
      switch_direction_to_string(direction),
      lag_handle,
      port_handle,
      lag_member->lag_member_handle);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  /*
   * Check with Kram for a cleaner way of deleting entries
   */
  return status;
}

switch_status_t switch_api_lag_member_delete_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_direction_t direction,
    switch_handle_t port_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_port_info_t *port_info = NULL;
  bool egress_disable = FALSE;
  bool ingress_disable = FALSE;
  bool delete_lag_member = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));

  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_lag_member_search(device, lag_handle, port_handle, &lag_member);

  if (status |= SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  /*
   * -----------------------------------------------
   * delete direction | existing direction | action
   * -----------------------------------------------
   *     Ingress      |     Egress         | Reject
   *     Egress       |     Ingress        | Reject
   *     Both         |     Ingress        | Reject
   *     Both         |     Egerss         | Reject
   *     Ingress      |     Ingress        | Accept
   *     Ingress      |     Both           | Accept
   *     Egress       |     Egress         | Accept
   *     Egress       |     Both           | Accept
   */
  if (((direction == SWITCH_API_DIRECTION_INGRESS ||
        direction == SWITCH_API_DIRECTION_BOTH) &&
       lag_member->direction == SWITCH_API_DIRECTION_EGRESS) ||
      ((direction == SWITCH_API_DIRECTION_EGRESS ||
        direction == SWITCH_API_DIRECTION_BOTH) &&
       lag_member->direction == SWITCH_API_DIRECTION_INGRESS)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (direction == SWITCH_API_DIRECTION_BOTH ||
      direction == SWITCH_API_DIRECTION_INGRESS) {
    ingress_disable = TRUE;
  }

  if (direction == SWITCH_API_DIRECTION_BOTH ||
      direction == SWITCH_API_DIRECTION_EGRESS) {
    egress_disable = TRUE;
  }

  SWITCH_ASSERT(ingress_disable || egress_disable);

  /*
   * To delete lag member,
   * ------------------------------------------------
   *  delete direction | existing direction | action
   *  -----------------------------------------------
   *       Ingress     |     Ingress        | Delete
   *       Egress      |     Egress         | Delete
   *       Both        |     Both           | Delete
   *       Ingress     |     Both           | Don't Delete
   *       Egress      |     Both           | Don't Delete
   */
  if (direction == SWITCH_API_DIRECTION_BOTH ||
      (direction == SWITCH_API_DIRECTION_INGRESS &&
       lag_member->direction == SWITCH_API_DIRECTION_INGRESS) ||
      (direction == SWITCH_API_DIRECTION_EGRESS &&
       lag_member->direction == SWITCH_API_DIRECTION_EGRESS)) {
    delete_lag_member = TRUE;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  if (egress_disable) {
    if (lag_info->member_count == 1) {
      status = switch_pd_lag_group_table_entry_delete(
          device, FALSE, lag_info->hw_entry, 0x0);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }

    status = switch_pd_lag_member_delete(
        device, lag_info->pd_group_hdl, lag_member->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_egress_port_mapping_table_entry_update(
        device,
        port_info->dev_port,
        port_info->egress_port_lag_label,
        port_info->port_type,
        port_info->egress_qos_group,
        port_info->egress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (ingress_disable) {
    status = switch_lag_prune_mask_table_update(device, lag_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (lag_info->member_count == 1) {
      status = switch_yid_free(device, lag_info->yid);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        goto cleanup;
      }
      lag_info->yid = SWITCH_YID_INVALID;
    }

    status = switch_yid_allocate(device, &port_info->yid);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_port_prune_mask_table_update(device, port_info, FALSE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_ingress_port_mapping_table_entry_update(
        device,
        port_info->dev_port,
        port_info->port_lag_index,
        port_info->port_type,
        port_info->ingress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_pd_ingress_port_properties_table_entry_update(
        device,
        port_info->yid,
        port_info,
        port_info->ingress_port_lag_label,
        port_info->ingress_prop_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (delete_lag_member) {
    status = SWITCH_LIST_DELETE(&lag_info->members, &lag_member->node);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  if (egress_disable) {
    status = switch_lag_mcast_port_map_update(device, lag_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  port_info->lag_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_LOG_DEBUG(
      "lag member delete success on device %d "
      "direction %s lag handle 0x%lx"
      "port handle 0x%lx lag member handle 0x%lx\n",
      device,
      switch_direction_to_string(direction),
      lag_handle,
      port_handle,
      lag_member->lag_member_handle);

  if (delete_lag_member) {
    lag_info->member_count--;
    status =
        switch_lag_member_handle_delete(device, lag_member->lag_member_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  return status;
}

switch_status_t switch_api_lag_member_create_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_direction_t direction,
    switch_handle_t port_handle,
    switch_handle_t *lag_member_handle) {
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));

  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status =
      switch_api_lag_member_add(device, lag_handle, direction, port_handle);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status =
      switch_lag_member_search(device, lag_handle, port_handle, &lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member create failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  SWITCH_LOG_DEBUG(
      "lag member add success on device %d "
      "direction %s lag handle 0x%lx"
      "port handle 0x%lx lag member handle 0x%lx\n",
      device,
      switch_direction_to_string(direction),
      lag_handle,
      port_handle,
      lag_member->lag_member_handle);

  *lag_member_handle = lag_member->lag_member_handle;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_lag_member_remove_internal(
    switch_device_t device, switch_handle_t lag_member_handle) {
  switch_lag_member_t *lag_member = NULL;
  switch_direction_t direction = 0;
  switch_handle_t port_handle = 0;
  switch_handle_t lag_handle = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_MEMBER_HANDLE(lag_member_handle));

  if (!SWITCH_LAG_MEMBER_HANDLE(lag_member_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_member_get(device, lag_member_handle, &lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  direction = lag_member->direction;
  port_handle = lag_member->port_handle;
  lag_handle = lag_member->lag_handle;

  status = switch_api_lag_member_delete(device,
                                        lag_member->lag_handle,
                                        lag_member->direction,
                                        lag_member->port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  lag_member = NULL;

  SWITCH_LOG_DEBUG(
      "lag member delete success on device %d "
      "direction %s lag handle 0x%lx"
      "port handle 0x%lx lag member handle 0x%lx\n",
      device,
      switch_direction_to_string(direction),
      lag_handle,
      port_handle,
      lag_member_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_lag_member_activate(switch_device_t device,
                                           switch_handle_t lag_handle,
                                           switch_handle_t port_handle,
                                           bool activate) {
  switch_lag_info_t *lag_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    if (handle_to_id(lag_member->port_handle) == port_handle) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!lag_member) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (!lag_member->active && activate) {
    status = switch_pd_lag_member_activate(
        device, lag_info->pd_group_hdl, lag_member->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_port_get(device, port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }

    status = switch_pd_pktgen_clear_port_down(device, port_info->port);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  } else if (lag_member->active && !activate) {
    status = switch_pd_lag_member_deactivate(
        device, lag_info->pd_group_hdl, lag_member->mbr_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("lag member activate failed on device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  lag_member->active = activate;

  return status;
}

switch_status_t switch_api_lag_member_activate_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t port_handle) {
  return switch_lag_member_activate(device, lag_handle, port_handle, TRUE);
}

switch_status_t switch_api_lag_member_deactivate_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t port_handle) {
  return switch_lag_member_activate(device, lag_handle, port_handle, FALSE);
}
switch_status_t switch_api_lag_member_count(switch_device_t device,
                                            switch_handle_t lag_handle,
                                            switch_size_t *member_count) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  SWITCH_ASSERT(member_count != NULL);

  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member count get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (!member_count) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("lag member count get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_handle, &lag_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member count get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *member_count = lag_info->member_count;

  SWITCH_LOG_DEBUG(
      "lag member count on device %d: %d\n", device, *member_count);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_lag_ingress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_port_lag_label_t label) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;
  switch_node_t *node = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_lag_get(device, lag_handle, &lag_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      lag_info->ingress_port_lag_label = label;
      break;
    case SWITCH_HANDLE_TYPE_LAG:
      lag_info->ingress_port_lag_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }
  lag_info->ingress_acl_group_handle = acl_group;

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = node->data;
    status = switch_port_get(device, lag_member->port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    };
    status = switch_pd_ingress_port_properties_table_entry_update(
        device,
        lag_info->yid,
        port_info,
        lag_info->ingress_port_lag_label,
        port_info->ingress_prop_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    };
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DETAIL(
      "lag label set device %d, lag_handle 0x%x "
      "bp_type %d, label %d\n",
      device,
      lag_handle,
      bp_type,
      label);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_ingress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t acl_group) {
  return switch_lag_ingress_acl_group_label_set(
      device, lag_handle, SWITCH_HANDLE_TYPE_LAG, acl_group, 0);
}

switch_status_t switch_api_lag_ingress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t label) {
  return switch_lag_ingress_acl_group_label_set(
      device, lag_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_lag_egress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_port_lag_label_t label) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;
  switch_node_t *node = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_port_info_t *port_info = NULL;

  status = switch_lag_get(device, lag_handle, &lag_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      lag_info->egress_port_lag_label = label;
      break;
    case SWITCH_HANDLE_TYPE_LAG:
      lag_info->egress_port_lag_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }

  lag_info->egress_acl_group_handle = acl_group;

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = node->data;
    status = switch_port_get(device, lag_member->port_handle, &port_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    };

    status = switch_pd_egress_port_mapping_table_entry_update(
        device,
        port_info->dev_port,
        lag_info->egress_port_lag_label,
        port_info->port_type,
        port_info->egress_qos_group,
        port_info->egress_mapping_hw_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DETAIL(
      "lag label set device %d, lag_handle 0x%x "
      "bp_type %d, label %d\n",
      device,
      lag_handle,
      bp_type,
      label);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_egress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t acl_group) {
  return switch_api_lag_egress_acl_group_label_set(
      device, lag_handle, SWITCH_HANDLE_TYPE_LAG, acl_group, 0);
}

switch_status_t switch_api_lag_egress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t label) {
  return switch_api_lag_egress_acl_group_label_set(
      device, lag_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_lag_ingress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *acl_group) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *acl_group = lag_info->ingress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_ingress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t *label) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *label = lag_info->ingress_port_lag_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_egress_acl_group_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *acl_group) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *acl_group = lag_info->egress_acl_group_handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_egress_acl_label_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t *label) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *label = lag_info->egress_port_lag_label;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_bind_mode_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_bind_mode_t *bind_mode) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  *bind_mode = lag_info->bind_mode;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_handle_from_lag_member_get_internal(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *lag_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_member_t *lag_member = NULL;
  status = switch_lag_member_get(device, lag_member_handle, &lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member add failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *lag_handle = lag_member->lag_handle;
  return status;
}

switch_status_t switch_api_lag_bind_mode_set_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_bind_mode_t bind_mode) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  };

  if (lag_info->bind_mode == bind_mode) {
    return SWITCH_STATUS_SUCCESS;
  } else if (SWITCH_ARRAY_COUNT(&lag_info->intf_array) != 0) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  lag_info->bind_mode = bind_mode;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_lag_members_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *members_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_node_t *node = NULL;
  switch_uint32_t i = 0;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag members get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag members get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    members_handle[i++] = lag_member->lag_member_handle;
  }
  FOR_EACH_IN_LIST_END();
  return status;
}

switch_status_t switch_api_lag_member_count_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_uint32_t *member_count) {
  switch_lag_info_t *lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_LAG_HANDLE(lag_handle));
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member count get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member count get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *member_count = lag_info->member_count;
  return status;
}

switch_status_t switch_api_lag_member_port_handle_get_internal(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *port_handle) {
  switch_status_t status;
  switch_lag_member_t *lag_member = NULL;
  SWITCH_ASSERT(SWITCH_LAG_MEMBER_HANDLE(lag_member_handle));

  if (!SWITCH_LAG_MEMBER_HANDLE(lag_member_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("lag member port handle get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_lag_member_get(device, lag_member_handle, &lag_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag member delete failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  *port_handle = lag_member->port_handle;
  return status;
}

switch_status_t switch_api_interface_lag_stats_get_internal(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  switch_lag_info_t *lag_info = NULL;
  switch_node_t *node = NULL;
  switch_lag_member_t *lag_member = NULL;
  switch_counter_t port_counters[SWITCH_PORT_STAT_MAX];
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_lag_get(device, lag_handle, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "lag stats get failed on device %d lag handle 0x%lx: "
        "lag get failed:(%s)\n",
        device,
        lag_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, sizeof(switch_counter_t) * num_entries);

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    SWITCH_MEMSET(port_counters, 0x0, sizeof(port_counters));
    status = switch_api_interface_port_stats_get(device,
                                                 lag_member->port_handle,
                                                 num_entries,
                                                 counter_id,
                                                 port_counters);
    if (status != SWITCH_STATUS_SUCCESS) {
      continue;
    }

    for (index = 0; index < num_entries; index++) {
      counters[index].num_packets += port_counters[index].num_packets;
      counters[index].num_bytes += port_counters[index].num_bytes;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_lag_create(switch_device_t device,
                                      switch_handle_t *lag_handle) {
  SWITCH_MT_WRAP(switch_api_lag_create_internal(device, lag_handle))
}

switch_status_t switch_api_lag_delete(switch_device_t device,
                                      switch_handle_t lag_handle) {
  SWITCH_MT_WRAP(switch_api_lag_delete_internal(device, lag_handle))
}

switch_status_t switch_api_lag_member_remove(
    switch_device_t device, switch_handle_t lag_member_handle) {
  SWITCH_MT_WRAP(
      switch_api_lag_member_remove_internal(device, lag_member_handle))
}

switch_status_t switch_api_lag_member_delete(switch_device_t device,
                                             switch_handle_t lag_handle,
                                             switch_direction_t direction,
                                             switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_lag_member_delete_internal(
      device, lag_handle, direction, port_handle))
}

switch_status_t switch_api_lag_member_activate(switch_device_t device,
                                               switch_handle_t lag_handle,
                                               switch_handle_t port_handle) {
  SWITCH_MT_WRAP(
      switch_api_lag_member_activate_internal(device, lag_handle, port_handle))
}

switch_status_t switch_api_lag_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_lag_ingress_acl_group_get_internal(
      device, lag_handle, acl_group))
}

switch_status_t switch_api_lag_ingress_acl_label_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t *label) {
  SWITCH_MT_WRAP(
      switch_api_lag_ingress_acl_label_get_internal(device, lag_handle, label))
}

switch_status_t switch_api_lag_egress_acl_group_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t *acl_group) {
  SWITCH_MT_WRAP(switch_api_lag_egress_acl_group_get_internal(
      device, lag_handle, acl_group))
}

switch_status_t switch_api_lag_egress_acl_label_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t *label) {
  SWITCH_MT_WRAP(
      switch_api_lag_egress_acl_label_get_internal(device, lag_handle, label))
}

switch_status_t switch_api_lag_member_deactivate(switch_device_t device,
                                                 switch_handle_t lag_handle,
                                                 switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_lag_member_deactivate_internal(
      device, lag_handle, port_handle))
}

switch_status_t switch_api_lag_member_add(switch_device_t device,
                                          switch_handle_t lag_handle,
                                          switch_direction_t direction,
                                          switch_handle_t port_handle) {
  SWITCH_MT_WRAP(switch_api_lag_member_add_internal(
      device, lag_handle, direction, port_handle))
}

switch_status_t switch_api_lag_member_create(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_direction_t direction,
    switch_handle_t port_handle,
    switch_handle_t *lag_member_handle) {
  SWITCH_MT_WRAP(switch_api_lag_member_create_internal(
      device, lag_handle, direction, port_handle, lag_member_handle))
}

switch_status_t switch_api_lag_ingress_acl_group_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_handle_t acl_group) {
  SWITCH_MT_WRAP(switch_api_lag_ingress_acl_group_set_internal(
      device, lag_handle, acl_group))
}

switch_status_t switch_api_lag_ingress_acl_label_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_lag_ingress_acl_label_set_internal(device, lag_handle, label))
}

switch_status_t switch_api_lag_egress_acl_group_set(switch_device_t device,
                                                    switch_handle_t lag_handle,
                                                    switch_handle_t acl_group) {
  SWITCH_MT_WRAP(switch_api_lag_egress_acl_group_set_internal(
      device, lag_handle, acl_group))
}

switch_status_t switch_api_lag_egress_acl_label_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_lag_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_lag_egress_acl_label_set_internal(device, lag_handle, label))
}

switch_status_t switch_api_lag_bind_mode_set(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_bind_mode_t bind_mode) {
  SWITCH_MT_WRAP(
      switch_api_lag_bind_mode_set_internal(device, lag_handle, bind_mode))
}

switch_status_t switch_api_lag_bind_mode_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_port_bind_mode_t *bind_mode) {
  SWITCH_MT_WRAP(
      switch_api_lag_bind_mode_get_internal(device, lag_handle, bind_mode))
}

switch_status_t swich_api_lag_handle_from_lag_member_get_internal(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *lag_handle) {
  SWITCH_MT_WRAP(switch_api_lag_handle_from_lag_member_get_internal(
      device, lag_member_handle, lag_handle))
}

switch_status_t switch_api_lag_members_get(switch_device_t device,
                                           switch_handle_t lag_handle,
                                           switch_handle_t *members_handle) {
  SWITCH_MT_WRAP(
      switch_api_lag_members_get_internal(device, lag_handle, members_handle))
}

switch_status_t switch_api_lag_member_count_get(switch_device_t device,
                                                switch_handle_t lag_handle,
                                                switch_uint32_t *member_count) {
  SWITCH_MT_WRAP(switch_api_lag_member_count_get_internal(
      device, lag_handle, member_count))
}

switch_status_t switch_api_lag_member_port_handle_get(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *port_handle) {
  SWITCH_MT_WRAP(switch_api_lag_member_port_handle_get_internal(
      device, lag_member_handle, port_handle))
}

switch_status_t swich_api_lag_handle_from_lag_member_get(
    switch_device_t device,
    switch_handle_t lag_member_handle,
    switch_handle_t *lag_handle) {
  SWITCH_MT_WRAP(swich_api_lag_handle_from_lag_member_get_internal(
      device, lag_member_handle, lag_handle))
}

switch_status_t switch_api_interface_lag_stats_get(
    switch_device_t device,
    switch_handle_t lag_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters) {
  SWITCH_MT_WRAP(switch_api_interface_lag_stats_get_internal(
      device, lag_handle, num_entries, counter_id, counters));
}
