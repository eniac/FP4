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

#include "switchapi/switch_rif.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_rif_int.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_RIF

/*
 * Routine Description:
 *   @brief initialize rif context and structs
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rif_init_internal(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_RIF, SWITCH_RIF_MAX);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

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
switch_status_t switch_rif_free_internal(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RIF);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG("RIF free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

static switch_status_t switch_rif_setup_bd_info(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                switch_bd_info_t *bd_info,
                                                switch_uint64_t *bd_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_rif_info_t *rif_info = NULL;

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif create failed on device %d: "
        "rif get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(bd_info, 0, sizeof(*bd_info));
  *bd_flags = 0;

  *bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_info->bd_type = SWITCH_BD_TYPE_L3;
  bd_info->handle = rif_handle;

  *bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;
  bd_info->vrf_handle = rif_info->api_rif_info.vrf_handle;

  *bd_flags |= SWITCH_BD_ATTR_RMAC_HANDLE;
  bd_info->rmac_handle = rif_info->api_rif_info.rmac_handle;

  *bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
  bd_info->ipv4_unicast = rif_info->api_rif_info.ipv4_unicast;

  *bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
  bd_info->ipv6_unicast = rif_info->api_rif_info.ipv6_unicast;

  *bd_flags |= SWITCH_BD_ATTR_IPV4_MULTICAST;
  bd_info->ipv4_multicast = rif_info->api_rif_info.ipv4_multicast;

  *bd_flags |= SWITCH_BD_ATTR_IPV6_MULTICAST;
  bd_info->ipv6_multicast = rif_info->api_rif_info.ipv6_multicast;

  if (rif_info->api_rif_info.ipv4_urpf_mode != SWITCH_URPF_MODE_NONE) {
    *bd_flags |= SWITCH_BD_ATTR_IPV4_URPF_MODE;
    bd_info->ipv4_urpf_mode = rif_info->api_rif_info.ipv4_urpf_mode;
  }

  if (rif_info->api_rif_info.ipv6_urpf_mode != SWITCH_URPF_MODE_NONE) {
    *bd_flags |= SWITCH_BD_ATTR_IPV6_URPF_MODE;
    bd_info->ipv6_urpf_mode = rif_info->api_rif_info.ipv6_urpf_mode;
  }

  if (rif_info->api_rif_info.nat_mode != SWITCH_NAT_MODE_NONE) {
    *bd_flags |= SWITCH_BD_ATTR_NAT_MODE;
    bd_info->nat_mode = rif_info->api_rif_info.nat_mode;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_rif_attach_vlan(switch_device_t device,
                                           switch_handle_t rif_handle,
                                           switch_vlan_t vlan) {
  switch_rif_info_t *rif_info = NULL;
  switch_vlan_info_t *vlan_info = NULL;
  switch_handle_t vlan_handle = 0;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_api_vlan_id_to_handle_get(device, vlan, &vlan_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  CHECK_RET(vlan_handle == SWITCH_API_INVALID_HANDLE, status);

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  if (vlan_info->l3_intf_handle != SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "interface validate failed on device %d "
        "intf handle 0x%lx vlan handle 0x%lx: "
        "interface vlan already exists(%s)\n",
        device,
        vlan_handle,
        vlan_info->l3_intf_handle,
        switch_error_to_string(status));
    return status;
  }

  rif_info->bd_handle = vlan_info->bd_handle;

  status = switch_rif_setup_bd_info(device, rif_handle, &bd_info, &bd_flags);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, &bd_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  vlan_info->l3_intf_handle = rif_handle;

  return SWITCH_STATUS_SUCCESS;
}
switch_status_t switch_api_rif_dettach_vlan(switch_device_t device,
                                            switch_handle_t rif_handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_vlan_info_t *vlan_info = NULL;
  switch_handle_t vlan_handle = 0;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_api_vlan_id_to_handle_get(
      device, rif_info->api_rif_info.vlan, &vlan_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  vlan_info->l3_intf_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_info.bd_type = SWITCH_BD_TYPE_VLAN;
  bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;
  bd_flags |= SWITCH_BD_ATTR_RMAC_HANDLE;
  bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
  bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, &bd_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_rif_attach_ln_internal(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  switch_handle_t ln_handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_ln_info_t *ln_info = NULL;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_ln_get(device, ln_handle, &ln_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  if (ln_info->l3_intf_handle != SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "rif validate failed on device %d "
        "intf handle 0x%lx vlan handle 0x%lx: "
        "interface vlan already exists(%s)\n",
        device,
        ln_handle,
        ln_info->l3_intf_handle,
        switch_error_to_string(status));
    return status;
  }

  rif_info->bd_handle = ln_info->bd_handle;

  status = switch_rif_setup_bd_info(device, rif_handle, &bd_info, &bd_flags);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, &bd_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  ln_info->l3_intf_handle = rif_handle;

  rif_info->api_rif_info.ln_handle = ln_handle;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_rif_dettach_ln_internal(switch_device_t device,
                                                   switch_handle_t rif_handle) {
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info = {0};
  switch_rif_info_t *rif_info = NULL;
  switch_ln_info_t *ln_info = NULL;

  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_ln_get(device, rif_info->api_rif_info.ln_handle, &ln_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_info.bd_type = SWITCH_BD_TYPE_LN;
  bd_flags |= SWITCH_BD_ATTR_TYPE;
  bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;
  bd_flags |= SWITCH_BD_ATTR_RMAC_HANDLE;
  bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
  bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, &bd_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  ln_info->l3_intf_handle = SWITCH_API_INVALID_HANDLE;
  rif_info->api_rif_info.ln_handle = SWITCH_API_INVALID_HANDLE;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_rif_attach_intf_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t intf_handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_interface_info_t *intf_info = NULL;
  switch_vlan_t vlan_id = 0;
  switch_handle_t member_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint64_t bd_flags = 0;
  switch_bd_info_t bd_info = {0};

  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_interface_get(device, intf_handle, &intf_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  CHECK_RET((rif_info->api_rif_info.intf_handle != SWITCH_API_INVALID_HANDLE) &&
                (rif_info->api_rif_info.intf_handle != intf_handle),
            SWITCH_STATUS_ITEM_ALREADY_EXISTS);

  CHECK_RET(
      (intf_info->api_intf_info.rif_handle != SWITCH_API_INVALID_HANDLE) &&
          (intf_info->api_intf_info.rif_handle != rif_handle),
      SWITCH_STATUS_ITEM_ALREADY_EXISTS);

  status = switch_rif_setup_bd_info(device, rif_handle, &bd_info, &bd_flags);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_bd_create(device, bd_flags, &bd_info, &rif_info->bd_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  switch (SWITCH_INTF_TYPE(intf_info)) {
    case SWITCH_INTERFACE_TYPE_ACCESS:
      vlan_id = 0;
      break;
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      vlan_id = SWITCH_INTF_ATTR_VLAN_ID(intf_info);
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }

  status = switch_pv_member_add(device,
                                rif_info->bd_handle,
                                intf_handle,
                                0x0,
                                vlan_id,
                                0x0,
                                &member_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  rif_info->api_rif_info.intf_handle = intf_handle;
  intf_info->api_intf_info.rif_handle = rif_handle;

  return status;
}
switch_status_t switch_api_rif_dettach_intf_internal(
    switch_device_t device, switch_handle_t rif_handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_interface_info_t *intf_info = NULL;

  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif detach failed on device %d: "
        "rif get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_interface_get(
      device, rif_info->api_rif_info.intf_handle, &intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif detach failed on device %d: "
        "rif intf get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pv_member_delete(
      device, rif_info->bd_handle, rif_info->api_rif_info.intf_handle, 0x0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "interface l3 delete failed on device %d "
        "intf handle 0x%lx: pv member delete failed(%s)\n",
        device,
        rif_info->api_rif_info.intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_delete(device, rif_info->bd_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  rif_info->api_rif_info.intf_handle = SWITCH_API_INVALID_HANDLE;
  intf_info->api_intf_info.rif_handle = SWITCH_API_INVALID_HANDLE;
  return status;
}

switch_status_t switch_api_rif_create_internal(
    switch_device_t device,
    switch_api_rif_info_t *api_rif_info,
    switch_handle_t *rif_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_rif_info_t *rif_info = NULL;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;

  if (api_rif_info->rmac_handle == SWITCH_API_INVALID_HANDLE) {
    status = switch_api_device_default_rmac_handle_get(
        device, &api_rif_info->rmac_handle);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  }

  if (!SWITCH_RMAC_HANDLE(api_rif_info->rmac_handle) ||
      !SWITCH_VRF_HANDLE(api_rif_info->vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "rif create failed on device %d: "
        "rmac or vrf handle invalid(%s)\n",
        device,
        switch_error_to_string(device));
    return status;
  }

  *rif_handle = switch_rif_handle_create(device);
  CHECK_RET(*rif_handle == SWITCH_API_INVALID_HANDLE, SWITCH_STATUS_NO_MEMORY);

  status = switch_rif_get(device, *rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = SWITCH_LIST_INIT(&rif_info->ip_list);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  rif_info->api_rif_info = *api_rif_info;

  if (api_rif_info->rmac_handle == SWITCH_API_INVALID_HANDLE) {
    status = switch_api_device_default_rmac_handle_get(
        device, &api_rif_info->rmac_handle);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  }

  switch (api_rif_info->rif_type) {
    case SWITCH_RIF_TYPE_VLAN:
      status =
          switch_api_rif_attach_vlan(device, *rif_handle, api_rif_info->vlan);
      CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

      break;

    case SWITCH_RIF_TYPE_LN:

      if (api_rif_info->ln_handle != SWITCH_API_INVALID_HANDLE) {
        status = switch_api_rif_attach_ln(
            device, *rif_handle, api_rif_info->ln_handle);
        CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      }
      break;

    case SWITCH_RIF_TYPE_INTF:

      if (intf_handle != SWITCH_API_INVALID_HANDLE) {
        status = switch_api_rif_attach_intf(
            device, *rif_handle, api_rif_info->intf_handle);
        CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      }
      break;

    case SWITCH_RIF_TYPE_LOOPBACK:
      break;

    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  return status;
}

switch_status_t switch_api_rif_delete_internal(switch_device_t device,
                                               switch_handle_t rif_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_rif_info_t *rif_info = NULL;

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  switch (rif_info->api_rif_info.rif_type) {
    case SWITCH_RIF_TYPE_VLAN:
      status = switch_api_rif_dettach_vlan(device, rif_handle);
      CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      break;

    case SWITCH_RIF_TYPE_LN:
      if (rif_info->api_rif_info.ln_handle != SWITCH_API_INVALID_HANDLE) {
        status = switch_api_rif_dettach_ln(device, rif_handle);
        CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      }
      break;

    case SWITCH_RIF_TYPE_INTF:
      if (rif_info->api_rif_info.intf_handle != SWITCH_API_INVALID_HANDLE) {
        status = switch_api_rif_dettach_intf(device, rif_handle);
        CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      }
      break;

    case SWITCH_RIF_TYPE_LOOPBACK:
      break;

    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  status = switch_rif_handle_delete(device, rif_handle);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  return status;
}

switch_status_t switch_api_rif_attribute_set_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    const switch_api_rif_info_t *api_rif_info) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t bd_info = {0};
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif attribute set failed on device %d "
        "rif handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  if (rif_flags & SWITCH_RIF_ATTR_MTU_HANDLE) {
    bd_flags |= SWITCH_BD_ATTR_MTU_HANDLE;
    bd_info.mtu_handle = api_rif_info->mtu_handle;
    rif_info->api_rif_info.mtu_handle = api_rif_info->mtu_handle;
  }

  if (rif_flags & SWITCH_RIF_ATTR_IPV4_UNICAST) {
    bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
    bd_info.ipv4_unicast = api_rif_info->ipv4_unicast;
    rif_info->api_rif_info.ipv4_unicast = api_rif_info->ipv4_unicast;
  }

  if (rif_flags & SWITCH_RIF_ATTR_IPV6_UNICAST) {
    bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
    bd_info.ipv6_unicast = api_rif_info->ipv6_unicast;
    rif_info->api_rif_info.ipv6_unicast = api_rif_info->ipv6_unicast;
  }

  if (rif_flags & SWITCH_RIF_ATTR_IPV4_MULTICAST) {
    bd_flags |= SWITCH_BD_ATTR_IPV4_MULTICAST;
    bd_info.ipv4_multicast = api_rif_info->ipv4_multicast;
    rif_info->api_rif_info.ipv4_multicast = api_rif_info->ipv4_multicast;
  }

  if (rif_flags & SWITCH_RIF_ATTR_IPV6_MULTICAST) {
    bd_flags |= SWITCH_BD_ATTR_IPV6_MULTICAST;
    bd_info.ipv6_multicast = api_rif_info->ipv6_multicast;
    rif_info->api_rif_info.ipv6_multicast = api_rif_info->ipv6_multicast;
  }

  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif attribute set failed on device %d "
        "bd update failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "rif attribute set on device %d "
      "intf flags 0x%x intf handle 0x%lx\n",
      device,
      rif_flags,
      rif_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_attribute_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    switch_api_rif_info_t *api_rif_info) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif attribute get failed on device %d "
        "rif handle 0x%lx: "
        "rif handle invalid(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  /* just get all attributes? */
  *api_rif_info = rif_info->api_rif_info;

  return status;
}

switch_status_t switch_api_rif_mtu_set_internal(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                switch_handle_t mtu_handle) {
  switch_api_rif_info_t api_rif_info = {0};
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.mtu_handle = mtu_handle;
  rif_flags |= SWITCH_RIF_ATTR_MTU_HANDLE;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_mtu_get_internal(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                switch_handle_t *mtu_handle) {
  switch_api_rif_info_t api_rif_info = {0};
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));

  rif_flags |= SWITCH_RIF_ATTR_MTU_HANDLE;
  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *mtu_handle = api_rif_info.mtu_handle;

  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_rif_ipv4_unicast_set_internal(
    switch_device_t device, switch_handle_t rif_handle, bool ipv4_unicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.ipv4_unicast = ipv4_unicast;
  rif_flags |= SWITCH_RIF_ATTR_IPV4_UNICAST;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG(
      "interface ipv4 unicast set on device %d "
      "rif handle 0x%lx ipv4_unicast %s\n",
      device,
      rif_handle,
      ipv4_unicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv4_unicast_get_internal(
    switch_device_t device, switch_handle_t rif_handle, bool *ipv4_unicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_IPV4_UNICAST;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *ipv4_unicast = api_rif_info.ipv4_unicast;

  SWITCH_LOG_DEBUG(
      "interface ipv4 unicast get on device %d "
      "rif handle 0x%lx ipv4_unicast %s\n",
      device,
      rif_handle,
      *ipv4_unicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv6_unicast_set_internal(
    switch_device_t device, switch_handle_t rif_handle, bool ipv6_unicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.ipv6_unicast = ipv6_unicast;
  rif_flags |= SWITCH_RIF_ATTR_IPV6_UNICAST;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG(
      "interface ipv6 unicast set on device %d "
      "rif handle 0x%lx ipv6_unicast %s\n",
      device,
      rif_handle,
      ipv6_unicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv6_unicast_get_internal(
    switch_device_t device, switch_handle_t rif_handle, bool *ipv6_unicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_IPV6_UNICAST;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);

  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *ipv6_unicast = api_rif_info.ipv6_unicast;

  SWITCH_LOG_DEBUG(
      "rif ipv6 unicast get on device %d "
      "rif handle 0x%lx ipv6_unicast %s\n",
      device,
      rif_handle,
      *ipv6_unicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv4_multicast_set_internal(
    switch_device_t device, switch_handle_t rif_handle, bool ipv4_multicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.ipv4_multicast = ipv4_multicast;
  rif_flags |= SWITCH_RIF_ATTR_IPV4_MULTICAST;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG(
      "rif ipv6 multicast set on device %d "
      "rif handle 0x%lx ipv4_multicast %s\n",
      device,
      rif_handle,
      ipv4_multicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv4_multicast_get_internal(
    switch_device_t device, switch_handle_t rif_handle, bool *ipv4_multicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_IPV4_MULTICAST;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *ipv4_multicast = api_rif_info.ipv4_multicast;

  SWITCH_LOG_DEBUG(
      "rif ipv6 multicast get on device %d "
      "rif handle 0x%lx ipv4_multicast %s\n",
      device,
      rif_handle,
      *ipv4_multicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv6_multicast_set_internal(
    switch_device_t device, switch_handle_t rif_handle, bool ipv6_multicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.ipv6_multicast = ipv6_multicast;
  rif_flags |= SWITCH_RIF_ATTR_IPV6_MULTICAST;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG(
      "rif ipv6 multicast set on device %d "
      "rif handle 0x%lx ipv6_multicast %s\n",
      device,
      rif_handle,
      ipv6_multicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv6_multicast_get_internal(
    switch_device_t device, switch_handle_t rif_handle, bool *ipv6_multicast) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_IPV6_MULTICAST;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *ipv6_multicast = api_rif_info.ipv6_multicast;

  SWITCH_LOG_DEBUG(
      "rif ipv6 multicast set on device %d "
      "rif handle 0x%lx ipv6_multicast %s\n",
      device,
      rif_handle,
      *ipv6_multicast ? "true" : "false");

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_vrf_handle_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t vrf_handle) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  api_rif_info.vrf_handle = vrf_handle;
  rif_flags |= SWITCH_RIF_ATTR_VRF_HANDLE;

  status = switch_api_rif_attribute_set(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  SWITCH_LOG_DEBUG(
      "rif vrf handle set on device %d "
      "rif handle 0x%lx vrf handle 0x%lx\n",
      device,
      rif_handle,
      vrf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_vrf_handle_get_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t *vrf_handle) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_VRF_HANDLE;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *vrf_handle = api_rif_info.vrf_handle;

  SWITCH_LOG_DEBUG(
      "rif vrf handle get on device %d "
      "rif handle 0x%lx vrf handle 0x%lx\n",
      device,
      rif_handle,
      *vrf_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_rmac_handle_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *rmac_handle) {
  switch_api_rif_info_t api_rif_info;
  switch_uint64_t rif_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));

  rif_flags |= SWITCH_RIF_ATTR_RMAC_HANDLE;

  status = switch_api_rif_attribute_get(
      device, rif_handle, rif_flags, &api_rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *rmac_handle = api_rif_info.rmac_handle;

  SWITCH_LOG_DEBUG(
      "interface rmac handle get on device %d "
      "rif handle 0x%lx rmac handle 0x%lx\n",
      device,
      rif_handle,
      api_rif_info.rmac_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rif_ipv4_urpf_mode_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_urpf_mode_t urpf_mode) {
  switch_status_t status = SWITCH_STATUS_NOT_SUPPORTED;
  return status;
}

switch_status_t switch_api_rif_ipv6_urpf_mode_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_urpf_mode_t urpf_mode) {
  switch_status_t status = SWITCH_STATUS_NOT_SUPPORTED;
  return status;
}
switch_status_t switch_api_rif_intf_handle_get_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t *intf_handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif attribute set failed on device %d "
        "rif handle invalid(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *intf_handle = rif_info->api_rif_info.intf_handle;
  return status;
}

switch_status_t switch_api_rif_type_get_internal(switch_device_t device,
                                                 switch_handle_t rif_handle,
                                                 switch_rif_type_t *type) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  CHECK_RET(!SWITCH_RIF_HANDLE(rif_handle), SWITCH_STATUS_INVALID_HANDLE);
  status = switch_rif_get(device, rif_handle, &rif_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *type = rif_info->api_rif_info.rif_type;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_rif_rewrite_smac_index_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  switch_id_t *smac_index) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(smac_index != NULL);

  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("interface rewrite smac index get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("interface rewrite smac index get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *smac_index = 0;
  status =
      switch_bd_rewrite_smac_index_get(device, rif_info->bd_handle, smac_index);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("interface rewrite smac index get failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_rif_attr_handle_get(switch_device_t device,
                                           switch_handle_t rif_handle,
                                           switch_handle_t *handle) {
  switch_rif_info_t *rif_info = NULL;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  if (!SWITCH_RIF_HANDLE(rif_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif handle invalid:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif get failed:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  if (rif_info->api_rif_info.rif_type != SWITCH_RIF_TYPE_INTF) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif type not an interface:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_rif_intf_handle_get(device, rif_handle, &intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif interface handle get failed:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_interface_handle_get(device, intf_handle, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif port handle get failed:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  if (!SWITCH_PORT_HANDLE(port_handle)) {
    SWITCH_LOG_ERROR(
        "rif attribute handle get failed on device %d rif handle 0x%lx: "
        "rif handle invalid:(%s)\n",
        device,
        rif_handle,
        switch_error_to_string(status));
    return status;
  }

  *handle = port_handle;
  return status;
}

switch_status_t switch_rif_ingress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_bd_label_t label) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  ;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      bd_info->ingress_bd_label = label;
      break;
    case SWITCH_HANDLE_TYPE_RIF:
      bd_info->ingress_bd_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }

  bd_info->ingress_acl_group_handle = acl_group;
  bd_flags |= SWITCH_BD_ATTR_INGRESS_LABEL;

  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd label set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        rif_info->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd acl label set on device %d "
      "bd handle %lx bd label %d bp_type %d\n",
      device,
      rif_info->bd_handle,
      bd_info->ingress_bd_label,
      bp_type);

  return status;
}

switch_status_t switch_api_rif_egress_acl_group_label_set(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_type_t bp_type,
    switch_handle_t acl_group,
    switch_bd_label_t label) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));

  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch (bp_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      bd_info->egress_bd_label = label;
      break;
    case SWITCH_HANDLE_TYPE_RIF:
      bd_info->egress_bd_label = handle_to_id(acl_group);
      break;
    default:
      break;
  }

  bd_info->egress_acl_group_handle = acl_group;
  bd_flags |= SWITCH_BD_ATTR_EGRESS_LABEL;

  status = switch_bd_update(device, rif_info->bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd label set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        rif_info->bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd acl label set on device %d "
      "bd handle %lx bd label %d bp_type %d\n",
      device,
      rif_info->bd_handle,
      bd_info->ingress_bd_label,
      bp_type);

  return status;
}

switch_status_t switch_api_rif_ingress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t acl_group) {
  return switch_rif_ingress_acl_group_label_set(
      device, rif_handle, SWITCH_HANDLE_TYPE_RIF, acl_group, 0);
}

switch_status_t switch_api_rif_ingress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_bd_label_t label) {
  return switch_rif_ingress_acl_group_label_set(
      device, rif_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_rif_egress_acl_group_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_handle_t acl_group) {
  return switch_api_rif_egress_acl_group_label_set(
      device, rif_handle, SWITCH_HANDLE_TYPE_RIF, acl_group, 0);
}

switch_status_t switch_api_rif_egress_acl_label_set_internal(
    switch_device_t device,
    switch_handle_t rif_handle,
    switch_bd_label_t label) {
  return switch_api_rif_egress_acl_group_label_set(
      device, rif_handle, SWITCH_HANDLE_TYPE_NONE, 0, label);
}

switch_status_t switch_api_rif_ingress_acl_group_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *acl_group = bd_info->ingress_acl_group_handle;

  return status;
}

switch_status_t switch_api_rif_ingress_acl_label_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *label = bd_info->ingress_bd_label;

  return status;
}

switch_status_t switch_api_rif_egress_acl_group_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *acl_group = bd_info->egress_acl_group_handle;

  return status;
}

switch_status_t switch_api_rif_egress_acl_label_get_internal(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label) {
  switch_rif_info_t *rif_info = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, rif_info->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = status;
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *label = bd_info->egress_bd_label;

  return status;
}
switch_status_t switch_api_rif_bd_get_internal(const switch_device_t device,
                                               const switch_handle_t rif_handle,
                                               switch_uint32_t *bd) {
  switch_rif_info_t *rif_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_RIF_HANDLE(rif_handle));
  status = switch_rif_get(device, rif_handle, &rif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Error: device: %u, error: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *bd = handle_to_id(rif_info->bd_handle);

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_rif_delete(switch_device_t device,
                                      switch_handle_t rif_handle) {
  SWITCH_MT_WRAP(switch_api_rif_delete_internal(device, rif_handle))
}

switch_status_t switch_api_rif_create(switch_device_t device,
                                      switch_api_rif_info_t *api_rif_info,
                                      switch_handle_t *rif_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_create_internal(device, api_rif_info, rif_handle))
}

switch_status_t switch_api_rif_vrf_handle_set(switch_device_t device,
                                              switch_handle_t rif_handle,
                                              switch_handle_t vrf_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_vrf_handle_set_internal(device, rif_handle, vrf_handle))
}

switch_status_t switch_rif_init(switch_device_t device) {
  SWITCH_MT_WRAP(switch_rif_init_internal(device))
}

switch_status_t switch_api_rif_ipv6_unicast_get(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool *ipv6_unicast) {
  SWITCH_MT_WRAP(switch_api_rif_ipv6_unicast_get_internal(
      device, rif_handle, ipv6_unicast))
}

switch_status_t switch_api_rif_ipv6_urpf_mode_set(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_urpf_mode_t urpf_mode) {
  SWITCH_MT_WRAP(switch_api_rif_ipv6_urpf_mode_set_internal(
      device, intf_handle, urpf_mode))
}

switch_status_t switch_api_rif_ipv6_multicast_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool *ipv6_multicast) {
  SWITCH_MT_WRAP(switch_api_rif_ipv6_multicast_get_internal(
      device, rif_handle, ipv6_multicast))
}

switch_status_t switch_api_rif_ipv4_unicast_set(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool set) {
  SWITCH_MT_WRAP(
      switch_api_rif_ipv4_unicast_set_internal(device, rif_handle, set))
}

switch_status_t switch_rif_free(switch_device_t device) {
  SWITCH_MT_WRAP(switch_rif_free_internal(device))
}

switch_status_t switch_api_rif_dettach_ln(switch_device_t device,
                                          switch_handle_t rif_handle) {
  SWITCH_MT_WRAP(switch_api_rif_dettach_ln_internal(device, rif_handle))
}

switch_status_t switch_api_rif_type_get(switch_device_t device,
                                        switch_handle_t rif_handle,
                                        switch_rif_type_t *type) {
  SWITCH_MT_WRAP(switch_api_rif_type_get_internal(device, rif_handle, type))
}

switch_status_t switch_api_rif_ipv6_multicast_set(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool set) {
  SWITCH_MT_WRAP(
      switch_api_rif_ipv6_multicast_set_internal(device, rif_handle, set))
}

switch_status_t switch_api_rif_attach_intf(switch_device_t device,
                                           switch_handle_t rif_handle,
                                           switch_handle_t intf_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_attach_intf_internal(device, rif_handle, intf_handle))
}

switch_status_t switch_api_rif_ipv4_multicast_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool *ipv4_multicast) {
  SWITCH_MT_WRAP(switch_api_rif_ipv4_multicast_get_internal(
      device, rif_handle, ipv4_multicast))
}

switch_status_t switch_api_rif_ipv4_urpf_mode_set(
    switch_device_t device,
    switch_handle_t intf_handle,
    switch_urpf_mode_t urpf_mode) {
  SWITCH_MT_WRAP(switch_api_rif_ipv4_urpf_mode_set_internal(
      device, intf_handle, urpf_mode))
}

switch_status_t switch_api_rif_ipv4_unicast_get(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool *ipv4_unicast) {
  SWITCH_MT_WRAP(switch_api_rif_ipv4_unicast_get_internal(
      device, rif_handle, ipv4_unicast))
}

switch_status_t switch_api_rif_vrf_handle_get(switch_device_t device,
                                              switch_handle_t rif_handle,
                                              switch_handle_t *vrf_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_vrf_handle_get_internal(device, rif_handle, vrf_handle))
}

switch_status_t switch_api_rif_ipv6_unicast_set(switch_device_t device,
                                                switch_handle_t rif_handle,
                                                bool set) {
  SWITCH_MT_WRAP(
      switch_api_rif_ipv6_unicast_set_internal(device, rif_handle, set))
}

switch_status_t switch_api_rif_intf_handle_get(switch_device_t device,
                                               switch_handle_t rif_handle,
                                               switch_handle_t *intf_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_intf_handle_get_internal(device, rif_handle, intf_handle))
}

switch_status_t switch_api_rif_dettach_intf(switch_device_t device,
                                            switch_handle_t rif_handle) {
  SWITCH_MT_WRAP(switch_api_rif_dettach_intf_internal(device, rif_handle))
}

switch_status_t switch_api_rif_ipv4_multicast_set(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  bool set) {
  SWITCH_MT_WRAP(
      switch_api_rif_ipv4_multicast_set_internal(device, rif_handle, set))
}

switch_status_t switch_api_rif_attach_ln(switch_device_t device,
                                         switch_handle_t rif_handle,
                                         switch_handle_t ln_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_attach_ln_internal(device, rif_handle, ln_handle))
}

switch_status_t switch_api_rif_mtu_set(switch_device_t device,
                                       switch_handle_t rif_handle,
                                       switch_handle_t mtu_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_mtu_set_internal(device, rif_handle, mtu_handle))
}

switch_status_t switch_api_rif_mtu_get(switch_device_t device,
                                       switch_handle_t rif_handle,
                                       switch_handle_t *mtu_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_mtu_get_internal(device, rif_handle, mtu_handle))
}

switch_status_t switch_api_rif_attribute_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    const switch_api_rif_info_t *api_rif_info) {
  SWITCH_MT_WRAP(switch_api_rif_attribute_set_internal(
      device, rif_handle, rif_flags, api_rif_info))
}

switch_status_t switch_api_rif_attribute_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags,
    switch_api_rif_info_t *api_rif_info) {
  SWITCH_MT_WRAP(switch_api_rif_attribute_get_internal(
      device, rif_handle, rif_flags, api_rif_info))
}

switch_status_t switch_api_rif_rmac_handle_get(const switch_device_t device,
                                               const switch_handle_t rif_handle,
                                               switch_handle_t *rmac_handle) {
  SWITCH_MT_WRAP(
      switch_api_rif_rmac_handle_get_internal(device, rif_handle, rmac_handle));
}

switch_status_t switch_api_rif_ingress_acl_group_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_handle_t acl_group_handle) {
  SWITCH_MT_WRAP(switch_api_rif_ingress_acl_group_set_internal(
      device, rif_handle, acl_group_handle));
}

switch_status_t switch_api_rif_ingress_acl_label_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_bd_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_rif_ingress_acl_label_set_internal(device, rif_handle, label));
}

switch_status_t switch_api_rif_ingress_acl_group_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group_handle) {
  SWITCH_MT_WRAP(switch_api_rif_ingress_acl_group_get_internal(
      device, rif_handle, acl_group_handle));
}

switch_status_t switch_api_rif_ingress_acl_label_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label) {
  SWITCH_MT_WRAP(
      switch_api_rif_ingress_acl_label_get_internal(device, rif_handle, label));
}

switch_status_t switch_api_rif_egress_acl_group_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_handle_t acl_group_handle) {
  SWITCH_MT_WRAP(switch_api_rif_egress_acl_group_set_internal(
      device, rif_handle, acl_group_handle));
}

switch_status_t switch_api_rif_egress_acl_label_set(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    const switch_bd_label_t label) {
  SWITCH_MT_WRAP(
      switch_api_rif_egress_acl_label_set_internal(device, rif_handle, label));
}

switch_status_t switch_api_rif_egress_acl_group_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_handle_t *acl_group_handle) {
  SWITCH_MT_WRAP(switch_api_rif_egress_acl_group_get_internal(
      device, rif_handle, acl_group_handle));
}

switch_status_t switch_api_rif_egress_acl_label_get(
    const switch_device_t device,
    const switch_handle_t rif_handle,
    switch_uint16_t *label) {
  SWITCH_MT_WRAP(
      switch_api_rif_egress_acl_label_get_internal(device, rif_handle, label));
}
switch_status_t switch_api_rif_bd_get(const switch_device_t device,
                                      const switch_handle_t rif_handle,
                                      switch_uint32_t *bd) {
  SWITCH_MT_WRAP(switch_api_rif_bd_get_internal(device, rif_handle, bd));
}
