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

#include "switchapi/switch_vrf.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_VRF

/*
 * Routine Description:
 *   @brief add vrf default entries
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vrf_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

/*
 * Routine Description:
 *   @brief remove vrf default entries
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vrf_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

/*
 * Routine Description:
 *   @brief initialize vrf context and structs
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vrf_init(switch_device_t device) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t max_vrf = 0;

  SWITCH_LOG_ENTER();

  vrf_ctx = SWITCH_MALLOC(device, sizeof(switch_vrf_context_t), 0x1);
  if (!vrf_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "vrf init failed on device %d: "
        "vrf context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(vrf_ctx, 0x0, sizeof(switch_vrf_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_VRF, (void *)vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf init failed on device %d: "
        "vrf context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_device_vrf_max_get(device, &max_vrf);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_ASSERT(max_vrf != 0);

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_VRF, max_vrf);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf init failed on device %d: "
        "vrf handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("vrf init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief uninitialize vrf context and structs
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_vrf_free(switch_device_t device) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VRF, (void **)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf free failed on device %d: "
        "vrf context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_VRF);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf free failed on device %d: "
        "vrf handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, vrf_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_VRF, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("vrf free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief vrf create
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] vrf_id - vrf id
 *   @param[out] vrf_handle - vrf handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vrf_create_internal(const switch_device_t device,
                                               const switch_vrf_t vrf_id,
                                               switch_handle_t *vrf_handle) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(vrf_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VRF, (void **)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf context get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  *vrf_handle = SWITCH_API_INVALID_HANDLE;

  if (vrf_id) {
    status = SWITCH_ARRAY_GET(&vrf_ctx->vrf_id_array, vrf_id, (void *)&handle);
    if (status != SWITCH_STATUS_ITEM_NOT_FOUND &&
        status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vrf create failed on device %d vrf id %d: "
          "vrf array get failed(%s)\n",
          device,
          vrf_id,
          switch_error_to_string(status));
      return status;
    }

    if (status == SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DEBUG("vrf id %x (handle: 0x%lx) already exists on device %d",
                       vrf_id,
                       handle,
                       device);
      *vrf_handle = handle;
      return status;
    }
  }

  handle = switch_vrf_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf handle create failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(switch_bd_info_t));
  bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_info.bd_type = SWITCH_BD_TYPE_VRF;
  bd_info.handle = handle;

  bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
  bd_info.ipv4_unicast = TRUE;

  bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
  bd_info.ipv6_unicast = TRUE;

  bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;
  bd_info.vrf_handle = handle;

  status = switch_bd_create(device, bd_flags, &bd_info, &bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d "
        "vrf id %d: bd create failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  bd_vrf_handle = SWITCH_BD_HANDLE_TO_VRF_HANDLE(bd_handle);

  if (vrf_id) {
    status = SWITCH_ARRAY_INSERT(&vrf_ctx->vrf_id_array,
                                 vrf_id,
                                 (void *)((switch_uint64_t)bd_vrf_handle));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vrf create failed on device %d vrf id %d: "
          "vrf array insert failed(%s)\n",
          device,
          vrf_id,
          switch_error_to_string(status));
      tmp_status = switch_vrf_handle_delete(device, handle);
      SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
      return status;
    }
  }

  status = SWITCH_ARRAY_INSERT(
      &vrf_ctx->vrf_array, bd_vrf_handle, (void *)((switch_uint64_t)handle));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf bd array insert failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    tmp_status = switch_vrf_handle_delete(device, handle);
    SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
    return status;
  }

  switch_vrf_get(device, bd_vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    status = switch_vrf_handle_delete(device, handle);
    return status;
  }

  SWITCH_LIST_INIT(&vrf_info->ipv4_routes);
  SWITCH_LIST_INIT(&vrf_info->ipv6_routes);
  vrf_info->bd_handle = bd_handle;

  status = switch_api_device_default_rmac_handle_get(device, &rmac_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_bd_rmac_handle_set(device, bd_handle, rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf bd rmac handle set failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
  }

  vrf_info->rmac_handle = rmac_handle;
  vrf_info->vrf_handle = handle;
  vrf_info->bd_vrf_handle = bd_vrf_handle;
  vrf_info->vrf_id = vrf_id;

  status = switch_lpm_trie_create(
      device, SWITCH_IPV4_PREFIX_LENGTH, TRUE, &vrf_info->ipv4_lpm_trie);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_lpm_trie_create(
      device, SWITCH_IPV6_PREFIX_LENGTH, TRUE, &vrf_info->ipv6_lpm_trie);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_l3_default_route_entries_add(device, bd_vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf l3 default route entries add failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    tmp_status = switch_vrf_handle_delete(device, handle);
    SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
    return status;
  }

  status = switch_bd_vrf_handle_set(device, bd_handle, bd_vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf create failed on device %d vrf id %d: "
        "vrf bd vrf handle set failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
  }

  *vrf_handle = bd_vrf_handle;

  SWITCH_LOG_DEBUG(
      "vrf created on device %d vrf id %d handle 0x%lx "
      "bd vrf handle 0x%lx\n",
      device,
      vrf_id,
      handle,
      bd_vrf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete vrf by vrf handle
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] vrf_handle - vrf handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vrf_delete_internal(
    const switch_device_t device, const switch_handle_t vrf_handle) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_vrf_info_t *vrf_info = NULL;
  switch_vrf_t default_vrf = 0;
  switch_handle_t default_vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "vrf handle invalid(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VRF, (void **)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "vrf context get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  if (vrf_info->vrf_id) {
    status = switch_api_device_default_vrf_get(
        device, &default_vrf, &default_vrf_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vrf delete failed on device %d vrf handle 0x%lx vrf id %d: "
          "default vrf get failed(%s)\n",
          device,
          vrf_handle,
          vrf_info->vrf_id,
          switch_error_to_string(status));
      return status;
    }

    if (vrf_info->vrf_id == default_vrf) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "vrf delete failed on device %d vrf handle 0x%lx vrf id %d: "
          "default vrf cannot be deleted(%s)\n",
          device,
          vrf_handle,
          vrf_info->vrf_id,
          switch_error_to_string(status));
      return status;
    }

    status = SWITCH_ARRAY_DELETE(&vrf_ctx->vrf_id_array, vrf_info->vrf_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vrf delete failed on device %d vrf handle 0x%lx vrf id %d: "
          "vrf array delete failed(%s)\n",
          device,
          vrf_handle,
          vrf_info->vrf_id,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_l3_default_route_entries_delete(device, vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "vrf l3 default route entries delete failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(&vrf_ctx->vrf_array, vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx vrf id %d: "
        "bd vrf array delete failed(%s)\n",
        device,
        vrf_handle,
        vrf_info->vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_lpm_trie_destroy(device, vrf_info->ipv4_lpm_trie);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_lpm_trie_destroy(device, vrf_info->ipv6_lpm_trie);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_bd_delete(device, vrf_info->bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "bd delete failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vrf_handle_delete(device, vrf_info->vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf handle 0x%lx: "
        "vrf handle delete failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "vrf deleted on device %d vrf handle 0x%lx\n", device, vrf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete vrf by vrf id
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] vrf_id - vrf id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vrf_id_delete_internal(switch_device_t device,
                                                  switch_vrf_t vrf_id) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(vrf_id != 0);

  if (vrf_id == 0) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf id %d: "
        "vrf id invalid(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VRF, (void **)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf id %d: "
        "vrf context get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_ARRAY_GET(&vrf_ctx->vrf_id_array, vrf_id, (void *)&vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf id %d: "
        "vrf array get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vrf_delete(device, vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf delete failed on device %d vrf id %d vrf handle 0x%lx: "
        "vrf delete failed(%s)\n",
        device,
        vrf_id,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief get vrf handle from vrf id
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] vrf_id - vrf id
 *   @param[out] vrf_handle - vrf handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vrf_id_to_handle_get_internal(
    const switch_device_t device,
    const switch_vrf_t vrf_id,
    switch_handle_t *vrf_handle) {
  switch_vrf_context_t *vrf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(vrf_id != 0);
  SWITCH_ASSERT(vrf_handle != NULL);
  if (vrf_id == 0 || vrf_handle == NULL) {
    SWITCH_LOG_ERROR(
        "vrf id to handle get failed on device %d vrf id %d: "
        "parameters invalid(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_VRF, (void **)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf id to handle get failed on device %d vrf id %d: "
        "vrf context get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_GET(&vrf_ctx->vrf_id_array, vrf_id, (void *)vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf id to handle get failed on device %d vrf id %d: "
        "vrf array get failed(%s)\n",
        device,
        vrf_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(*vrf_handle));

  SWITCH_LOG_DEBUG(
      "vrf id to vrf handle get on device %d "
      "vrf id %d vrf handle 0x%lx\n",
      device,
      vrf_id,
      *vrf_handle);

  SWITCH_LOG_EXIT();

  return SWITCH_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *   @brief get vrf id from vrf handle
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] vrf_handle - vrf handle
 *   @param[out] vrf_id - vrf id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_vrf_handle_to_id_get_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_vrf_t *vrf_id) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  SWITCH_ASSERT(vrf_id != NULL);
  if (!SWITCH_VRF_HANDLE(vrf_handle) || vrf_id == NULL) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vrf handle to id get failed on device %d vrf handle 0x%lx: "
        "parameters invalid(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf id to handle get failed on device %d vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  *vrf_id = vrf_info->vrf_id;

  SWITCH_LOG_DEBUG(
      "vrf handle to vrf id on device %d "
      "vrf handle 0x%lx vrf id %d\n",
      device,
      vrf_handle,
      *vrf_id);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vrf_rmac_handle_set_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_handle_t rmac_handle) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf rmac set failed on device %d vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_rmac_handle_set(device, vrf_info->bd_handle, rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf rmac set failed on device %d vrf handle 0x%lx: "
        "vrf bd rmac set failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  vrf_info->rmac_handle = rmac_handle;

  return status;
}

switch_status_t switch_api_vrf_rmac_handle_get_internal(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_handle_t *rmac_handle) {
  switch_vrf_info_t *vrf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf rmac set failed on device %d vrf handle 0x%lx: "
        "vrf get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_rmac_handle_get(device, vrf_info->bd_handle, rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vrf rmac set failed on device %d vrf handle 0x%lx: "
        "vrf bd rmac get failed(%s)\n",
        device,
        vrf_handle,
        switch_error_to_string(status));
    return status;
  }

  *rmac_handle = vrf_info->rmac_handle;

  return status;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

switch_status_t switch_api_vrf_id_to_handle_get(const switch_device_t device,
                                                const switch_vrf_t vrf_id,
                                                switch_handle_t *vrf_handle) {
  SWITCH_MT_WRAP(
      switch_api_vrf_id_to_handle_get_internal(device, vrf_id, vrf_handle))
}

switch_status_t switch_api_vrf_delete(const switch_device_t device,
                                      const switch_handle_t vrf_handle) {
  SWITCH_MT_WRAP(switch_api_vrf_delete_internal(device, vrf_handle))
}

switch_status_t switch_api_vrf_handle_to_id_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_vrf_t *vrf_id) {
  SWITCH_MT_WRAP(
      switch_api_vrf_handle_to_id_get_internal(device, vrf_handle, vrf_id))
}

switch_status_t switch_api_vrf_create(const switch_device_t device,
                                      const switch_vrf_t vrf_id,
                                      switch_handle_t *vrf_handle) {
  SWITCH_MT_WRAP(switch_api_vrf_create_internal(device, vrf_id, vrf_handle))
}

switch_status_t switch_api_vrf_id_delete(const switch_device_t device,
                                         const switch_vrf_t vrf_id) {
  SWITCH_MT_WRAP(switch_api_vrf_id_delete_internal(device, vrf_id))
}

switch_status_t switch_api_vrf_rmac_handle_set(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_handle_t rmac_handle) {
  SWITCH_MT_WRAP(
      switch_api_vrf_rmac_handle_set_internal(device, vrf_handle, rmac_handle));
}

switch_status_t switch_api_vrf_rmac_handle_get(const switch_device_t device,
                                               const switch_handle_t vrf_handle,
                                               switch_handle_t *rmac_handle) {
  SWITCH_MT_WRAP(
      switch_api_vrf_rmac_handle_get_internal(device, vrf_handle, rmac_handle));
}
