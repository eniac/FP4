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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_RPF

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_rpf_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_RPF_GROUP, 64 * 1024);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("rpf init failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;
cleanup:
  return status;
}

switch_status_t switch_rpf_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RPF_GROUP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("rpf free failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_rpf_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_rpf_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_rpf_group_create_internal(
    switch_device_t device,
    switch_rpf_type_t rpf_type,
    switch_mcast_mode_t pim_mode,
    switch_handle_t *rpf_group_handle) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(rpf_group_handle != NULL);
  if (!rpf_group_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "parameters invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_rpf_group_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "rpf group get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  rpf_info->rpf_type = rpf_type;
  rpf_info->pim_mode = pim_mode;
  *rpf_group_handle = handle;

  SWITCH_LOG_DEBUG(
      "rpf group created on device %d handle %lx\n", device, handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rpf_group_delete_internal(
    switch_device_t device, switch_handle_t rpf_group_handle) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "rpf group handle %lx: "
        "rpf handle invalid(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "rpf group handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_handle_delete(device, rpf_group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf group create failed on device %d: "
        "rpf group handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "rpf group deleted on device %d handle %lx\n", device, rpf_group_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rpf_member_add_internal(
    switch_device_t device,
    switch_handle_t rpf_group_handle,
    switch_handle_t rif_handle) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_rpf_entry_t *rpf_entry = NULL;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_node_t *node = NULL;
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf member add failed on device %d "
        "rpf group handle %lx rif handle %lx: "
        "rpf group handle invalid(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf group add failed on device %d: "
        "rpf group handle %lx rif handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rpf_info->rpf_list, node) {
    rpf_entry = node->data;
    if (rpf_entry->rif_handle == rif_handle) {
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      SWITCH_LOG_ERROR(
          "rpf member add failed on device %d "
          "rpf group handle %lx rif handle %lx: "
          "rif handle already exists(%s)\n",
          device,
          rpf_group_handle,
          rif_handle,
          switch_error_to_string(status));
      return status;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (rpf_info->pim_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
    if (SWITCH_LIST_COUNT(&rpf_info->rpf_list) == 1) {
      SWITCH_LOG_ERROR(
          "rpf group add failed on device %d: "
          "rpf group handle %lx rif handle %lx: "
          "one rpf member allowed for pim sm(%s)\n",
          device,
          rpf_group_handle,
          rif_handle,
          switch_error_to_string(status));
      return status;
    }

    status = switch_bd_handle_get(device, rif_handle, &bd_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "rpf group add failed on device %d: "
          "rpf group handle %lx rif handle %lx: "
          "bd handle get failed(%s)\n",
          device,
          rpf_group_handle,
          rif_handle,
          switch_error_to_string(status));
      return status;
    }
    rpf_info->rpf_group = handle_to_id(bd_handle);
  }

  if (rpf_info->pim_mode == SWITCH_API_MCAST_IPMC_PIM_BIDIR) {
    rpf_info->rpf_group = handle_to_id(rif_handle);
    status = switch_rif_get(device, rif_handle, &rif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "rpf group add failed on device %d: "
          "rpf group handle %lx rif handle %lx: "
          "interface get failed(%s)\n",
          device,
          rpf_group_handle,
          rif_handle,
          switch_error_to_string(status));
      status = SWITCH_STATUS_SUCCESS;
    }

    if (rpf_info->rpf_type & SWITCH_RPF_TYPE_OUTER) {
      if (!(SWITCH_HW_FLAG_ISSET(rpf_entry, SWITCH_RPF_OUTER_PD_ENTRY))) {
        status =
            switch_pd_multicast_rpf_entry_add(device,
                                              SWITCH_RPF_TYPE_OUTER,
                                              handle_to_id(rpf_group_handle),
                                              handle_to_id(rif_info->bd_handle),
                                              &rpf_entry->outer_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "rpf member add failed on device %d "
              "rpf group handle %lx rif handle %lx: "
              "outer mcast rpf entry add failed(%s)\n",
              device,
              rpf_group_handle,
              rif_handle,
              switch_error_to_string(status));
          return status;
        }
        SWITCH_HW_FLAG_SET(rpf_entry, SWITCH_RPF_OUTER_PD_ENTRY);
      }
    }

    if (rpf_info->rpf_type & SWITCH_RPF_TYPE_INNER) {
      if (!(SWITCH_HW_FLAG_ISSET(rpf_entry, SWITCH_RPF_INNER_PD_ENTRY))) {
        status =
            switch_pd_multicast_rpf_entry_add(device,
                                              SWITCH_RPF_TYPE_INNER,
                                              handle_to_id(rpf_group_handle),
                                              handle_to_id(rif_info->bd_handle),
                                              &rpf_entry->inner_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "rpf member add failed on device %d "
              "rpf group handle %lx rif handle %lx: "
              "inner mcast rpf entry add failed(%s)\n",
              device,
              rpf_group_handle,
              rif_handle,
              switch_error_to_string(status));
          return status;
        }
        SWITCH_HW_FLAG_SET(rpf_entry, SWITCH_RPF_INNER_PD_ENTRY);
      }
    }
  }

  rpf_entry = SWITCH_MALLOC(device, sizeof(switch_rpf_entry_t), 0x1);
  if (!rpf_entry) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf member add failed on device %d "
        "rpf group handle %lx rif handle %lx: "
        "rif handle invalid(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(rpf_entry, 0x0, sizeof(switch_rpf_entry_t));
  rpf_entry->rif_handle = rif_handle;

  status =
      SWITCH_LIST_INSERT(&rpf_info->rpf_list, &(rpf_entry->node), rpf_entry);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "rpf group member added on device %d "
      "rpf group handle %lx rif handle %lx\n",
      device,
      rpf_group_handle,
      rif_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rpf_member_delete_internal(
    switch_device_t device,
    switch_handle_t rpf_group_handle,
    switch_handle_t rif_handle) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_rpf_entry_t *rpf_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf member delete failed on device %d "
        "rpf group handle %lx rif handle %lx: "
        "rpf group handle invalid(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf group delete failed on device %d "
        "rpf group handle %lx rif handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  FOR_EACH_IN_LIST(rpf_info->rpf_list, node) {
    rpf_entry = node->data;
    if (rpf_entry->rif_handle == rif_handle) {
      status = SWITCH_STATUS_SUCCESS;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (rpf_info->pim_mode == SWITCH_API_MCAST_IPMC_PIM_BIDIR) {
    if (rpf_info->rpf_type & SWITCH_RPF_TYPE_OUTER) {
      if ((SWITCH_HW_FLAG_ISSET(rpf_entry, SWITCH_RPF_OUTER_PD_ENTRY))) {
        status = switch_pd_multicast_rpf_entry_delete(
            device, SWITCH_RPF_TYPE_OUTER, rpf_entry->outer_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "rpf member delete failed on device %d "
              "rpf group handle %lx rif handle %lx: "
              "outer mcast rpf entry delete failed(%s)\n",
              device,
              rpf_group_handle,
              rif_handle,
              switch_error_to_string(status));
          return status;
        }
        SWITCH_HW_FLAG_CLEAR(rpf_entry, SWITCH_RPF_OUTER_PD_ENTRY);
      }
    }

    if (rpf_info->rpf_type & SWITCH_RPF_TYPE_INNER) {
      if ((SWITCH_HW_FLAG_ISSET(rpf_entry, SWITCH_RPF_INNER_PD_ENTRY))) {
        status = switch_pd_multicast_rpf_entry_delete(
            device, SWITCH_RPF_TYPE_INNER, rpf_entry->inner_pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "rpf member delete failed on device %d "
              "rpf group handle %lx rif handle %lx: "
              "inner mcast rpf entry delete failed(%s)\n",
              device,
              rpf_group_handle,
              rif_handle,
              switch_error_to_string(status));
          return status;
        }
        SWITCH_HW_FLAG_CLEAR(rpf_entry, SWITCH_RPF_INNER_PD_ENTRY);
      }
    }
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf member delete failed on device %d "
        "rpf group handle %lx rif handle %lx: "
        "rif handle not found(%s)\n",
        device,
        rpf_group_handle,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&rpf_info->rpf_list, &(rpf_entry->node));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_FREE(device, rpf_entry);

  SWITCH_LOG_DEBUG(
      "rpf group member deleted on device %d "
      "rpf group handle %lx rif handle %lx\n",
      device,
      rpf_group_handle,
      rif_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rpf_members_get_internal(
    switch_device_t device,
    switch_handle_t rpf_group_handle,
    switch_size_t *num_entries,
    switch_handle_t **rif_handles) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_rpf_entry_t *rpf_entry = NULL;
  switch_node_t *node = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(num_entries && rif_handles);
  if (!num_entries || !rif_handles) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rpf member get failed on device %d "
        "rpf group handle %lx: "
        "parameters invalid(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf member get failed on device %d "
        "rpf group handle %lx: "
        "parameters invalid(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf member get failed on device %d "
        "rpf group handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  *num_entries = 0;
  *rif_handles = NULL;
  if (!SWITCH_LIST_COUNT(&rpf_info->rpf_list)) {
    return status;
  }

  *rif_handles = SWITCH_MALLOC(
      device, sizeof(switch_handle_t), SWITCH_LIST_COUNT(&rpf_info->rpf_list));
  if (!(*rif_handles)) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "rpf member get failed on device %d "
        "rpf group handle %lx: "
        "rif handles malloc failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  *num_entries = SWITCH_LIST_COUNT(&rpf_info->rpf_list);
  FOR_EACH_IN_LIST(rpf_info->rpf_list, node) {
    rpf_entry = node->data;
    *rif_handles[index++] = rpf_entry->rif_handle;
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_rpf_group_id_get(switch_device_t device,
                                        switch_handle_t rpf_group_handle,
                                        switch_rpf_group_t *rpf_group) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rpf id get failed on device %d "
        "rpf group handle %lx: "
        "parameters invalid(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf id get failed on device %d "
        "rpf group handle %lx: "
        "rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  *rpf_group = rpf_info->rpf_group;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_rpf_group_create(switch_device_t device,
                                            switch_rpf_type_t rpf_type,
                                            switch_mcast_mode_t pim_mode,
                                            switch_handle_t *rpf_group_handle) {
  SWITCH_MT_WRAP(switch_api_rpf_group_create_internal(
      device, rpf_type, pim_mode, rpf_group_handle))
}

switch_status_t switch_api_rpf_member_add(switch_device_t device,
                                          switch_handle_t rpf_group_handle,
                                          switch_handle_t rif_handle) {
  SWITCH_MT_WRAP(
      switch_api_rpf_member_add_internal(device, rpf_group_handle, rif_handle))
}

switch_status_t switch_api_rpf_group_delete(switch_device_t device,
                                            switch_handle_t rpf_group_handle) {
  SWITCH_MT_WRAP(switch_api_rpf_group_delete_internal(device, rpf_group_handle))
}

switch_status_t switch_api_rpf_member_delete(switch_device_t device,
                                             switch_handle_t rpf_group_handle,
                                             switch_handle_t rif_handle) {
  SWITCH_MT_WRAP(switch_api_rpf_member_delete_internal(
      device, rpf_group_handle, rif_handle))
}

switch_status_t switch_api_rpf_members_get(switch_device_t device,
                                           switch_handle_t rpf_group_handle,
                                           switch_size_t *num_entries,
                                           switch_handle_t **rif_handles) {
  SWITCH_MT_WRAP(switch_api_rpf_members_get_internal(
      device, rpf_group_handle, num_entries, rif_handles))
}
