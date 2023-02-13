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

#include "switchapi/switch_mpls.h"

#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_MPLS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Routine Description:
 *   @brief add default entries for mpls
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mpls_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief delete default entries for mpls
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mpls_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief initilize mpls structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mpls_init(switch_device_t device) {
  switch_mpls_context_t *mpls_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  mpls_ctx = SWITCH_MALLOC(device, sizeof(switch_mpls_context_t), 0x1);
  if (!mpls_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mpls init failed on device %d "
        "mpls device context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_MPLS, (void *)mpls_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls init failed on device %d "
        "mpls device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_MPLS, SWITCH_MPLS_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls init failed on device %d: "
        "mpls handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK,
                                   SWITCH_LABEL_STACK_HANDLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls init failed on device %d: "
        "label stack handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("mpls init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

/*
 * Routine Description:
 *   @brief free mpls structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_mpls_free(switch_device_t device) {
  switch_mpls_context_t *mpls_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MPLS, (void **)&mpls_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls free failed on device %d "
        "mpls device context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MPLS);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls free failed on device %d: "
        "mpls handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls free failed on device %d: "
        "label stack handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, mpls_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_MPLS, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_mpls_tunnel_type_ingress_get(
    switch_mpls_tunnel_type_t tunnel_type,
    bool l2,
    uint16_t label_count,
    switch_mpls_tunnel_type_ingress_t *ingress_tunnel_type,
    switch_mpls_tunnel_subtype_ingress_t *mpls_tunnel_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (tunnel_type == SWITCH_MPLS_TUNNEL_TYPE_MPLS) {
    *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS;
  } else {
    *ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_UDP;
  }

  if (l2) {
    switch (label_count) {
      case 0:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_NONE;
        break;

      case 1:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_1;
        break;

      case 2:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_2;
        break;

      case 3:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_3;
        break;
      default:
        status = SWITCH_STATUS_NOT_SUPPORTED;
        break;
    }
  } else {
    switch (label_count) {
      case 0:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_NONE;
        break;

      case 1:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_1;
        break;

      case 2:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_2;
        break;

      case 3:
        *mpls_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_3;
        break;
      default:
        status = SWITCH_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  return status;
}

switch_status_t switch_mpls_tunnel_type_egress_get(
    switch_mpls_tunnel_type_t tunnel_type,
    bool l2,
    switch_mpls_tunnel_type_egress_t *egress_tunnel_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (l2) {
    *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  } else {
    *egress_tunnel_type = SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  }

  return status;
}

switch_status_t switch_api_mpls_tunnel_create_internal(
    switch_device_t device,
    switch_api_mpls_info_t *api_mpls_info,
    switch_handle_t *mpls_handle) {
  switch_mpls_info_t *mpls_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_interface_info_t *intf_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  bool l2 = TRUE;
  switch_ifindex_t ifindex = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_mpls_info);

  switch (api_mpls_info->mpls_type) {
    case SWITCH_MPLS_TYPE_EOMPLS:
    case SWITCH_MPLS_TYPE_VPLS:
      l2 = TRUE;
      break;
    case SWITCH_MPLS_TYPE_IPV4_MPLS:
    case SWITCH_MPLS_TYPE_IPV6_MPLS:
      l2 = FALSE;
      break;
    default:
      break;
  }

  switch (api_mpls_info->mpls_mode) {
    case SWITCH_MPLS_MODE_INITIATE:
    case SWITCH_MPLS_MODE_TERMINATE: {
      switch (api_mpls_info->mpls_type) {
        case SWITCH_MPLS_TYPE_EOMPLS:
        case SWITCH_MPLS_TYPE_VPLS:
          SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(api_mpls_info->network_handle));
          status = switch_bd_handle_get(
              device, api_mpls_info->network_handle, &bd_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "mpls tunnel create failed on device %d nw handle 0x%lx: "
                "bd handle get failed:(%s)\n",
                device,
                api_mpls_info->network_handle,
                switch_error_to_string(status));
            return status;
          }
          break;

        case SWITCH_MPLS_TYPE_IPV4_MPLS:
        case SWITCH_MPLS_TYPE_IPV6_MPLS:
          SWITCH_ASSERT(SWITCH_VRF_HANDLE(api_mpls_info->vrf_handle));
          status = switch_bd_handle_get(
              device, api_mpls_info->vrf_handle, &bd_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "mpls tunnel create failed on device %d vrf handle 0x%lx: "
                "bd handle get failed:(%s)\n",
                device,
                api_mpls_info->vrf_handle,
                switch_error_to_string(status));
            return status;
          }
          break;
        default:
          status = SWITCH_STATUS_INVALID_PARAMETER;
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "mpls tunnel create failed on device %d mpls type %d: "
                "mpls type invalid:(%s)\n",
                device,
                api_mpls_info->mpls_type,
                switch_error_to_string(status));
            return status;
          }
          break;
      }
    } break;

    default:
      break;
  }

  if (SWITCH_INTERFACE_HANDLE(api_mpls_info->intf_handle)) {
    status =
        switch_interface_get(device, api_mpls_info->intf_handle, &intf_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "interface get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  handle = switch_mpls_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mpls tunnel create failed on device %d: "
        "mpls handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mpls_get(device, handle, &mpls_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls tunnel create failed on device %d: "
        "mpls get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      &mpls_info->api_mpls_info, api_mpls_info, sizeof(switch_api_mpls_info_t));

  if (SWITCH_BD_HANDLE(bd_handle)) {
    status = switch_bd_get(device, bd_handle, &bd_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    mpls_info->bd_handle = bd_handle;
  }

  if (api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_TERMINATE ||
      api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_TRANSIT) {
    status =
        switch_mpls_tunnel_type_ingress_get(api_mpls_info->tunnel_type,
                                            l2,
                                            api_mpls_info->pop_count,
                                            &mpls_info->ingress_tunnel_type,
                                            &mpls_info->mpls_tunnel_type);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls tunnel type ingress get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_mpls_table_entry_add(device,
                                            mpls_info->ingress_tunnel_type,
                                            mpls_info->mpls_tunnel_type,
                                            handle_to_id(bd_handle),
                                            api_mpls_info,
                                            bd_info,
                                            api_mpls_info->pop_label,
                                            ifindex,
                                            &mpls_info->pd_hdl[0]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls tunnel table add failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  } else if (api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_INITIATE) {
    status = switch_mpls_tunnel_type_egress_get(
        api_mpls_info->tunnel_type, l2, &mpls_info->egress_tunnel_type);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls tunnel type egress get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    status = switch_neighbor_tunnel_dmac_rewrite_add(
        device, &api_mpls_info->mac_addr, &mpls_info->tunnel_dmac_index);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls neighbor tunnel mac table add failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  *mpls_handle = handle;

  return status;
}

switch_status_t switch_api_mpls_tunnel_delete_internal(
    switch_device_t device, switch_handle_t mpls_handle) {
  switch_mpls_info_t *mpls_info = NULL;
  switch_api_mpls_info_t *api_mpls_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MPLS_HANDLE(mpls_handle));
  status = switch_mpls_get(device, mpls_handle, &mpls_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls tunnel create failed on device %d: "
        "mpls get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  api_mpls_info = &mpls_info->api_mpls_info;

  if ((api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_TERMINATE) ||
      (api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_TRANSIT)) {
    status = switch_pd_mpls_table_entry_delete(device, mpls_info->pd_hdl[0]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls pd entry delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    if (api_mpls_info->mpls_type == SWITCH_MPLS_TYPE_EOMPLS) {
      status = switch_pd_mpls_table_entry_delete(device, mpls_info->pd_hdl[1]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mpls tunnel create failed on device %d: "
            "mpls pd entry delete failed:(%s)\n",
            device,
            switch_error_to_string(status));
        return status;
      }
    }
  } else if (api_mpls_info->mpls_mode == SWITCH_MPLS_MODE_INITIATE) {
    status = switch_neighbor_tunnel_dmac_rewrite_delete(
        device, &api_mpls_info->mac_addr);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mpls tunnel create failed on device %d: "
          "mpls pd entry delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_mpls_handle_delete(device, mpls_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return 0;
}

switch_status_t switch_api_mpls_label_stack_create_internal(
    switch_device_t device,
    switch_mpls_label_stack_t *label_stack,
    switch_handle_t *label_stack_handle) {
  switch_mpls_label_stack_info_t *label_stack_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(label_stack != NULL);

  handle = switch_mpls_label_stack_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mpls label stack create failed on device %d: "
        "label stack handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mpls_label_stack_get(device, handle, &label_stack_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls label stack create failed on device %d: "
        "label stack get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  tunnel_handle = switch_tunnel_handle_create(device);
  if (tunnel_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "mpls label stack create failed on device %d: "
        "tunnel handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  label_stack_info->tunnel_handle = tunnel_handle;
  SWITCH_MEMCPY(&label_stack_info->label_stack,
                label_stack,
                sizeof(switch_mpls_label_stack_t));

  status = switch_pd_tunnel_rewrite_table_mpls_entry_add(
      device,
      handle_to_id(tunnel_handle),
      label_stack->num_labels,
      label_stack->label_list,
      &label_stack_info->rw_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls label stack create failed on device %d: "
        "tunnel rewrite table add failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *label_stack_handle = handle;

  SWITCH_LOG_DEBUG(
      "label stack created on device %d handle 0x%lx\n", device, handle);

  return status;
}

switch_status_t switch_api_mpls_label_stack_delete_internal(
    switch_device_t device, switch_handle_t label_stack_handle) {
  switch_mpls_label_stack_info_t *label_stack_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MPLS_LABEL_STACK_HANDLE(label_stack_handle));
  status = switch_mpls_label_stack_get(
      device, label_stack_handle, &label_stack_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls label stack delete failed on device %d stack handle 0x%lx: "
        "mpls label stack get failed:(%s)\n",
        device,
        label_stack_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_rewrite_table_entry_delete(
      device, label_stack_info->rw_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mpls label stack delete failed on device %d stack handle 0x%lx: "
        "mpls label stack get failed:(%s)\n",
        device,
        label_stack_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_handle_delete(device, label_stack_info->tunnel_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_mpls_label_stack_handle_delete(device, label_stack_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("label stack deleted on device %d handle 0x%lx\n",
                   device,
                   label_stack_handle);

  return status;
}

switch_status_t switch_api_mpls_tunnel_create(
    switch_device_t device,
    switch_api_mpls_info_t *api_mpls_info,
    switch_handle_t *mpls_handle) {
  SWITCH_MT_WRAP(switch_api_mpls_tunnel_create_internal(
      device, api_mpls_info, mpls_handle));
}

switch_status_t switch_api_mpls_tunnel_delete(switch_device_t device,
                                              switch_handle_t mpls_handle) {
  SWITCH_MT_WRAP(switch_api_mpls_tunnel_delete_internal(device, mpls_handle));
}

switch_status_t switch_api_mpls_label_stack_create(
    switch_device_t device,
    switch_mpls_label_stack_t *label_stack,
    switch_handle_t *label_stack_handle) {
  SWITCH_MT_WRAP(switch_api_mpls_label_stack_create_internal(
      device, label_stack, label_stack_handle));
}

switch_status_t switch_api_mpls_label_stack_delete(
    switch_device_t device, switch_handle_t label_stack_handle) {
  SWITCH_MT_WRAP(
      switch_api_mpls_label_stack_delete_internal(device, label_stack_handle));
}

#ifdef __cplusplus
}
#endif
