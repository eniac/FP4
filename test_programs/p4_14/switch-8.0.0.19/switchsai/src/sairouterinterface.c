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

#include <sairouterinterface.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_interface.h>
#include <switchapi/switch_l3.h>

static sai_api_t api_id = SAI_API_ROUTER_INTERFACE;

/*
static switch_urpf_mode_t sai_to_switch_urpf_mode(uint8_t sai_urpf_mode) {
  switch_urpf_mode_t switch_urpf_mode = SWITCH_URPF_MODE_NONE;
  switch (sai_urpf_mode) {
    case SAI_URPF_MODE_NONE:
      switch_urpf_mode = SWITCH_URPF_MODE_NONE;
      break;
    case SAI_URPF_MODE_STRICT:
      switch_urpf_mode = SWITCH_URPF_MODE_STRICT;
      break;
    case SAI_URPF_MODE_LOOSE:
      switch_urpf_mode = SWITCH_URPF_MODE_LOOSE;
      break;
  }
  return switch_urpf_mode;
}
*/

#define SAI_API_DEFAULT_L3_MTU_SIZE 9400
/*
* Routine Description:
*    Create router interface.
*
* Arguments:
*    [out] rif_id - router interface id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_router_interface(
    _Out_ sai_object_id_t *rif_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_rif_info_t api_rif_info = {0};
  switch_api_interface_info_t intf_api_info = {0};

  const sai_attribute_t *attribute;
  sai_router_interface_type_t sai_intf_type = -1;
  uint32_t index = 0;
  switch_handle_t rmac_handle = 0;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t mtu_handle = SWITCH_API_INVALID_HANDLE;
  switch_mtu_t mtu_size = 0;
  switch_mac_addr_t mac;

  *rif_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  api_rif_info.ipv4_unicast = TRUE;
  api_rif_info.ipv6_unicast = TRUE;
  api_rif_info.ipv4_multicast = FALSE;
  api_rif_info.ipv6_multicast = FALSE;
  api_rif_info.ipv4_urpf_mode = SWITCH_URPF_MODE_NONE;
  api_rif_info.ipv6_urpf_mode = SWITCH_URPF_MODE_NONE;

  attribute =
      get_attr_from_list(SAI_ROUTER_INTERFACE_ATTR_TYPE, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
    return status;
  }
  sai_intf_type = attribute->value.s32;

  attribute = get_attr_from_list(
      SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
    return status;
  }
  api_rif_info.vrf_handle = (switch_handle_t)attribute->value.oid;
  SAI_ASSERT(sai_object_type_query(api_rif_info.vrf_handle) ==
             SAI_OBJECT_TYPE_VIRTUAL_ROUTER);

  switch (sai_intf_type) {
    case SAI_ROUTER_INTERFACE_TYPE_PORT:
      api_rif_info.rif_type = SWITCH_RIF_TYPE_INTF;
      attribute = get_attr_from_list(
          SAI_ROUTER_INTERFACE_ATTR_PORT_ID, attr_list, attr_count);
      if (attribute == NULL) {
        status = SAI_STATUS_INVALID_PARAMETER;
        SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
        return status;
      }
      intf_api_info.type = SWITCH_INTERFACE_TYPE_ACCESS;
      intf_api_info.handle = attribute->value.oid;
      break;

    case SAI_ROUTER_INTERFACE_TYPE_SUB_PORT:
      api_rif_info.rif_type = SWITCH_RIF_TYPE_INTF;
      intf_api_info.type = SWITCH_INTERFACE_TYPE_PORT_VLAN;

      attribute = get_attr_from_list(
          SAI_ROUTER_INTERFACE_ATTR_PORT_ID, attr_list, attr_count);
      if (attribute == NULL) {
        status = SAI_STATUS_INVALID_PARAMETER;
        SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
        return status;
      }
      intf_api_info.handle = attribute->value.oid;

      attribute = get_attr_from_list(
          SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, attr_list, attr_count);
      if (attribute == NULL) {
        status = SAI_STATUS_INVALID_PARAMETER;
        SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
        return status;
      }

      intf_api_info.vlan = attribute->value.u16;
      break;
    case SAI_ROUTER_INTERFACE_TYPE_BRIDGE:
      api_rif_info.rif_type = SWITCH_RIF_TYPE_LN;
      break;
    case SAI_ROUTER_INTERFACE_TYPE_VLAN:
      api_rif_info.rif_type = SWITCH_RIF_TYPE_VLAN;

      attribute = get_attr_from_list(
          SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, attr_list, attr_count);
      if (attribute == NULL) {
        status = SAI_STATUS_INVALID_PARAMETER;
        SAI_LOG_ERROR("missing attribute %s", sai_status_to_string(status));
        return status;
      }
      switch_api_vlan_handle_to_id_get(
          device, attribute->value.oid, &(api_rif_info.vlan));
      break;
    case SAI_ROUTER_INTERFACE_TYPE_LOOPBACK:
      api_rif_info.rif_type = SWITCH_RIF_TYPE_LOOPBACK;
      break;
    default:
      SAI_LOG_ERROR("Unsupported intf type %d\n", sai_intf_type);
      return SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  switch_status =
      switch_api_device_default_rmac_handle_get(device, &rmac_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create router interface: %s",
                  sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_TYPE:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
        switch_status = switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL, &rmac_handle);
        //        SAI_ASSERT(status == SWITCH_STATUS_SUCCESS);
        if (switch_status == SWITCH_STATUS_SUCCESS) {
          memcpy(&mac.mac_addr, &attribute->value.mac, 6);
          switch_status = switch_api_router_mac_add(device, rmac_handle, &mac);
          //        SAI_ASSERT(status == SWITCH_STATUS_SUCCESS);
          api_rif_info.rmac_handle = rmac_handle;
        }
        break;
      case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
        api_rif_info.ipv4_unicast = attribute->value.booldata;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
        api_rif_info.ipv6_unicast = attribute->value.booldata;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
        api_rif_info.ipv4_multicast = attribute->value.booldata;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
        api_rif_info.ipv6_multicast = attribute->value.booldata;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
      case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
      case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_MTU:
        if (attribute->value.u32 == 0) {
          mtu_size = SAI_API_DEFAULT_L3_MTU_SIZE;
        } else {
          mtu_size = attribute->value.u32;
        }
        switch_status =
            switch_api_l3_mtu_size_create(device, mtu_size, &mtu_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get mtu handle: %s",
                        sai_status_to_string(status));
          return status;
        }
        api_rif_info.mtu_handle = mtu_handle;
        break;
      default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
  }

  api_rif_info.rmac_handle = rmac_handle;

  switch_status = (sai_object_id_t)switch_api_rif_create(
      device, &api_rif_info, &rif_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create router interface: %s",
                  sai_status_to_string(status));
    return status;
  }
  /* create interface object implicitly. This is not visible to SAI */
  if (sai_intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT ||
      sai_intf_type == SAI_ROUTER_INTERFACE_TYPE_SUB_PORT) {
    intf_api_info.rif_handle = rif_handle;
    switch_status = (sai_object_id_t)switch_api_interface_create(
        device, &intf_api_info, &intf_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to create router interface: %s",
                    sai_status_to_string(status));
      return status;
    }
  }
  *rif_id = rif_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Remove router interface
*
* Arguments:
*    [in] rif_id - router interface id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_router_interface(_In_ sai_object_id_t rif_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_rif_info_t api_rif_info;
  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tmp_rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(rif_id) == SAI_OBJECT_TYPE_ROUTER_INTERFACE);

  switch_status = switch_api_rif_attribute_get(
      device, rif_id, (switch_uint64_t)UINT64_MAX, &api_rif_info);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove router interface: %s",
                  sai_status_to_string(status));
    return status;
  }

  if (api_rif_info.rif_type == SWITCH_RIF_TYPE_INTF &&
      api_rif_info.intf_handle != SWITCH_API_INVALID_HANDLE) {
    switch_status =
        switch_api_interface_delete(device, api_rif_info.intf_handle);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to remove router interface: %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  switch_status = switch_api_rif_rmac_handle_get(device, rif_id, &rmac_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status == SAI_STATUS_SUCCESS) {
    if (rmac_handle != SWITCH_API_INVALID_HANDLE) {
      switch_api_device_default_rmac_handle_get(device, &tmp_rmac_handle);
      if (tmp_rmac_handle != rmac_handle) {
        switch_status = switch_api_router_mac_group_delete(device, rmac_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to remove router interface: %s",
                        sai_status_to_string(status));
        }
      }
    }
  }

  switch_status = switch_api_rif_delete(device, (switch_handle_t)rif_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove router interface: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Set router interface attribute
*
* Arguments:
*    [in] rif_id - router interface id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_router_interface_attribute(
    _In_ sai_object_id_t rif_id, _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_mtu_t mtu_size = 0, prev_mtu_size = 0;
  switch_handle_t acl_table_id = 0;
  switch_handle_t mtu_handle, prev_mtu_handle;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(rif_id) == SAI_OBJECT_TYPE_ROUTER_INTERFACE);

  switch (attr->id) {
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
      switch_status =
          switch_api_rif_ipv4_unicast_set(device, rif_id, attr->value.booldata);
      break;
    case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
      switch_status =
          switch_api_rif_ipv6_unicast_set(device, rif_id, attr->value.booldata);
      break;
    case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
      switch_status = switch_api_rif_ipv4_multicast_set(
          device, rif_id, attr->value.booldata);
      break;
    case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
      switch_status = switch_api_rif_ipv6_multicast_set(
          device, rif_id, attr->value.booldata);
      break;

    case SAI_ROUTER_INTERFACE_ATTR_MTU:
      mtu_size = attr->value.u32;
      // To update MTU, set the MTU handle for the new size and delete the
      // handle associated with the old MTU size.
      switch_status = switch_api_rif_mtu_get(device, rif_id, &prev_mtu_handle);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to get mtu attribute for router interface: %s",
                      sai_status_to_string(status));
        return status;
      }
      if (prev_mtu_handle != SWITCH_API_INVALID_HANDLE) {
        switch_status =
            switch_api_l3_mtu_get(device, prev_mtu_handle, &prev_mtu_size);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get mtu size for router interface: %s",
                        sai_status_to_string(status));
          return status;
        }
      }

      switch_status =
          switch_api_l3_mtu_size_create(device, mtu_size, &mtu_handle);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to get mtu handle: %s",
                      sai_status_to_string(status));
        return status;
      }

      switch_status = switch_api_rif_mtu_set(device, rif_id, mtu_handle);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to set mtu handle: %s",
                      sai_status_to_string(status));
        return status;
      }

      if (prev_mtu_size) {
        switch_status = switch_api_l3_mtu_size_delete(device, prev_mtu_size);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to delete mtu size: %s",
                        sai_status_to_string(status));
          return status;
        }
      }
      break;

    case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
    case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
      acl_table_id = attr->value.oid;
      if (acl_table_id == SAI_NULL_OBJECT_ID) {
        if (attr->id == SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL) {
          switch_status = switch_api_rif_ingress_acl_group_get(
              device, rif_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to get ingress_handle for rif_id 0x%lx: %s",
                          rif_id,
                          sai_status_to_string(status));
            return status;
          }
        } else {
          switch_status = switch_api_rif_egress_acl_group_get(
              device, rif_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to get egress_handle for rif_id 0x%lx: %s",
                          rif_id,
                          sai_status_to_string(status));
            return status;
          }
        }
        switch_status =
            switch_api_acl_dereference(device, acl_table_id, rif_id);
      } else {
        switch_status = switch_api_acl_reference(device, acl_table_id, rif_id);
      }
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to bind port to acl for rif_id 0x%lx: %s",
                      rif_id,
                      sai_status_to_string(status));
        return status;
      }
      break;
    default:
      SAI_LOG_ERROR("RIF set unsupported attr %d\n", attr->id);
      return SAI_STATUS_INVALID_PARAMETER;
  }

  SAI_LOG_EXIT();

  status = sai_switch_status_to_sai_status(switch_status);
  return status;
}

/*
* Routine Description:
*    Get router interface attribute
*
* Arguments:
*    [in] rif_id - router interface id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_router_interface_attribute(
    _In_ sai_object_id_t rif_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_uint64_t flags = 0;
  uint32_t index;
  switch_status_t switch_status = -1;
  sai_attribute_t *attribute = NULL;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t if_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t api_intf_info = {0};
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t mtu_handle = SWITCH_API_INVALID_HANDLE;
  switch_mtu_t mtu_size = 0;

  SAI_ASSERT(sai_object_type_query(rif_id) == SAI_OBJECT_TYPE_ROUTER_INTERFACE);

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_rif_intf_handle_get(device, rif_id, &if_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove router interface: %s",
                  sai_status_to_string(status));
    return status;
  }

  flags |= SWITCH_INTF_ATTR_VLAN;
  flags |= SWITCH_INTF_ATTR_PORT;

  if (if_handle != SWITCH_API_INVALID_HANDLE) {
    switch_status = switch_api_interface_attribute_get(
        device, if_handle, flags, &api_intf_info);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      return status;
    }
    vlan_handle = api_intf_info.vlan;
    port_handle = api_intf_info.handle;
  }
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID: {
        switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
        switch_status =
            switch_api_rif_vrf_handle_get(device, rif_id, &vrf_handle);
        attribute->value.oid = vrf_handle;
        break;
      }
      case SAI_ROUTER_INTERFACE_ATTR_TYPE:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
        attribute->value.oid = port_handle;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
        attribute->value.oid = vlan_handle;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
        break;
      case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
        switch_status = switch_api_rif_ipv4_unicast_get(
            device, rif_id, &attribute->value.booldata);
        break;
      case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
        switch_status = switch_api_rif_ipv6_unicast_get(
            device, rif_id, &attribute->value.booldata);
        break;
      case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
        switch_status = switch_api_rif_ipv4_multicast_get(
            device, rif_id, &attribute->value.booldata);
        break;
      case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
        switch_status = switch_api_rif_ipv6_multicast_get(
            device, rif_id, &attribute->value.booldata);
        break;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_MTU:
        switch_status = switch_api_rif_mtu_get(device, rif_id, &mtu_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get mtu attribute for router interface: %s",
                        sai_status_to_string(status));
          return status;
        }
        switch_status = switch_api_l3_mtu_get(device, mtu_handle, &mtu_size);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get mtu size for router interface: %s",
                        sai_status_to_string(status));
          return status;
        }

        attribute->value.u32 = mtu_size;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
        switch_status =
            switch_api_rif_ingress_acl_group_get(device, rif_id, &acl_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attribute->value.oid = (acl_handle == SWITCH_API_INVALID_HANDLE)
                                   ? SAI_NULL_OBJECT_ID
                                   : acl_handle;
        break;
      case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
        switch_status =
            switch_api_rif_egress_acl_group_get(device, rif_id, &acl_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attribute->value.oid = (acl_handle == SWITCH_API_INVALID_HANDLE)
                                   ? SAI_NULL_OBJECT_ID
                                   : acl_handle;
        break;
      default:
        SAI_LOG_ERROR("RIF get unsupported attr %d\n", attribute->id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      return status;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Routing interface methods table retrieved with sai_api_query()
*/
sai_router_interface_api_t rif_api = {
    .create_router_interface = sai_create_router_interface,
    .remove_router_interface = sai_remove_router_interface,
    .set_router_interface_attribute = sai_set_router_interface_attribute,
    .get_router_interface_attribute = sai_get_router_interface_attribute,
};

sai_status_t sai_router_interface_initialize(
    sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing router interface");
  sai_api_service->rif_api = rif_api;
  return SAI_STATUS_SUCCESS;
}
