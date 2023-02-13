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

#include <saihostif.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_hostif.h>
#include <switchapi/switch_hash.h>
#include <switchapi/switch_log.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_hostif.h>
#include <switchapi/switch_queue.h>

static sai_api_t api_id = SAI_API_HOSTIF;
static switch_handle_t default_hostif_trap_group_id = 0;
static switch_handle_t default_hostif_trap_group_policer = 0;
static switch_handle_t *cpuQHandles = NULL;

sai_object_id_t sai_hostif_get_default() {
  return (sai_object_id_t)default_hostif_trap_group_id;
}

#define SAI_HOSTIF_TRAP_OBJECT(_reason_code)                           \
  ((_reason_code & 0xFFFF) | (SWITCH_HANDLE_TYPE_HOSTIF_TRAP << 26)) & \
      0xFFFFFFFF

switch_hostif_vlan_action_t sai_hostif_vlan_action_to_switch_vlan_action(
    sai_hostif_vlan_tag_t vlan_action) {
  switch (vlan_action) {
    case SAI_HOSTIF_VLAN_TAG_STRIP:
      return SWITCH_HOSTIF_VLAN_ACTION_REMOVE;
    case SAI_HOSTIF_VLAN_TAG_KEEP:
      return SWITCH_HOSTIF_VLAN_ACTION_ADD;
    case SAI_HOSTIF_VLAN_TAG_ORIGINAL:
    default:
      return SWITCH_HOSTIF_VLAN_ACTION_NONE;
  }
}

/*
* Routine Description:
*    Create host interface
*
* Arguments:
*    [out] hif_id - host interface id
*    [in] switch_id Switch object id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_hostif(_Out_ sai_object_id_t *hif_id,
                               _In_ sai_object_id_t switch_id,
                               _In_ uint32_t attr_count,
                               _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  const sai_attribute_t *attribute;
  uint32_t index = 0;
  switch_uint64_t flags = 0;
  switch_hostif_t hostif;
  switch_handle_t hostif_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_device_info_t api_device_info;
  *hif_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&hostif, 0, sizeof(switch_hostif_t));

  // default get the SRC MAC of switch
  switch_api_device_attribute_get(
      device, SWITCH_DEVICE_ATTR_DEFAULT_MAC, &api_device_info);
  flags |= SWITCH_HOSTIF_ATTR_MAC_ADDRESS;
  memcpy(hostif.mac.mac_addr, api_device_info.mac.mac_addr, 6);

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HOSTIF_ATTR_OBJ_ID:
        hostif.handle = attribute->value.oid;
        flags |= SWITCH_HOSTIF_ATTR_HANDLE;
        break;
      case SAI_HOSTIF_ATTR_TYPE:
        switch (attribute->value.u32) {
          case SAI_HOSTIF_TYPE_NETDEV:
            break;
          case SAI_HOSTIF_TYPE_FD:
          default:
            return SAI_STATUS_NOT_SUPPORTED;
        }
        break;
      case SAI_HOSTIF_ATTR_NAME:
        flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME;
        memcpy(hostif.intf_name, attribute->value.chardata, HOSTIF_NAME_SIZE);
        break;
      case SAI_HOSTIF_ATTR_OPER_STATUS:
        flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS;
        hostif.operstatus = attribute->value.booldata;
        break;
      case SAI_HOSTIF_ATTR_VLAN_TAG:
        flags |= SWITCH_HOSTIF_ATTR_VLAN_ACTION;
        hostif.vlan_action =
            sai_hostif_vlan_action_to_switch_vlan_action(attribute->value.u32);
        break;
      default:
        break;
    }
  }
  // default get the SRC MAC of switch
  switch_api_device_attribute_get(
      device, SWITCH_DEVICE_ATTR_DEFAULT_MAC, &api_device_info);
  flags |= SWITCH_HOSTIF_ATTR_MAC_ADDRESS;
  memcpy(hostif.mac.mac_addr, api_device_info.mac.mac_addr, 6);
  status = switch_api_hostif_create(device, flags, &hostif, &hostif_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create hostif: %s", sai_status_to_string(status));
  }
  *hif_id = hostif_handle;
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

/*
* Routine Description:
*    Remove host interface
*
* Arguments:
*    [in] hif_id - host interface id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_hostif(_In_ sai_object_id_t hif_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(hif_id) == SAI_OBJECT_TYPE_HOSTIF);
  switch_status = switch_api_hostif_delete(device, hif_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove hostif %lx: %s",
                  hif_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();
  return status;
}

/*
* Routine Description:
*    Set host interface attribute
*
* Arguments:
*    [in] hif_id - host interface id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_hostif_attribute(_In_ sai_object_id_t hif_id,
                                      _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(hif_id) == SAI_OBJECT_TYPE_HOSTIF);

  if (attr->id == SAI_HOSTIF_ATTR_OPER_STATUS) {
    switch_api_hostif_oper_state_set(device, hif_id, attr->value.booldata);
  }

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Get host interface attribute
*
* Arguments:
*    [in] hif_id - host interface id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_hostif_attribute(_In_ sai_object_id_t hif_id,
                                      _In_ uint32_t attr_count,
                                      _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(hif_id) == SAI_OBJECT_TYPE_HOSTIF);

  for (unsigned int i = 0; i < attr_count; i++) {
    sai_attribute_t *attr = &attr_list[i];
    if (attr->id == SAI_HOSTIF_ATTR_OPER_STATUS) {
      switch_api_hostif_oper_state_get(device, hif_id, &attr->value.booldata);
    }
  }

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Create host interface trap group
*
* Arguments:
*  [out] hostif_trap_group_id  - host interface trap group id
*  [in] switch_id Switch object id
*  [in] attr_count - number of attributes
*  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_hostif_trap_group(
    _Out_ sai_object_id_t *hostif_trap_group_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  const sai_attribute_t *attribute;
  switch_hostif_group_t hostif_group = {0};
  uint32_t index = 0;
  switch_handle_t hostif_group_handle = SWITCH_API_INVALID_HANDLE;
  *hostif_trap_group_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&hostif_group, 0, sizeof(switch_hostif_group_t));
  hostif_group.queue_handle = cpuQHandles ? cpuQHandles[0] : 0;
  hostif_group.policer_handle = default_hostif_trap_group_policer;
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE:  // Unsupported
        break;
      case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
        if (!cpuQHandles) {
          status = SAI_STATUS_INVALID_PARAMETER;
          SAI_LOG_ERROR("CPU Qhandles Invalid: %s",
                        sai_status_to_string(status));
          return status;
        }
        hostif_group.queue_handle = cpuQHandles[attribute->value.u32];
        break;
      case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
        hostif_group.policer_handle = attribute->value.oid;
        break;
    }
  }

  status = switch_api_hostif_group_create(
      device, &hostif_group, &hostif_group_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create hostif trap group: %s",
                  sai_status_to_string(status));
  }
  *hostif_trap_group_id = hostif_group_handle;
  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove host interface trap group
*
* Arguments:
*  [in] hostif_trap_group_id - host interface trap group id
*
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_hostif_trap_group(_In_ sai_object_id_t
                                              hostif_trap_group_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(hostif_trap_group_id) ==
             SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP);

  status = switch_api_hostif_group_delete(device, hostif_trap_group_id);

  SAI_LOG_EXIT();

  return status;
}

/*
* Routine Description:
*   Set host interface trap group attribute value.
*
* Arguments:
*    [in] hostif_trap_group_id - host interface trap group id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_hostif_trap_group_attribute(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }
  SAI_ASSERT(sai_object_type_query(hostif_trap_group_id) ==
             SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP);

  if (attr->id == SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER) {
    switch_api_hostif_group_meter_set(device,
                                      (switch_handle_t)hostif_trap_group_id,
                                      (switch_handle_t)attr->value.oid);
  }
  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   get host interface trap group attribute value.
*
* Arguments:
*    [in] hostif_trap_group_id - host interface trap group id
*    [in] attr_count - number of attributes
*    [in,out] attr_list - array of attributes
*
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_hostif_trap_group_attribute(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_hostif_group_t hostif_group;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(hostif_trap_group_id) ==
             SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP);

  switch_api_hostif_group_get(device, hostif_trap_group_id, &hostif_group);
  for (unsigned int i = 0; i < attr_count; i++) {
    switch (attr_list[i].id) {
      case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
        attr_list[i].value.u32 = hostif_group.queue_handle;
        break;
      case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
        attr_list[i].value.oid = hostif_group.policer_handle;
        break;
    }
  }
  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

switch_hostif_reason_code_t switch_sai_to_switch_api_reason_code(
    sai_hostif_trap_type_t trap_id) {
  switch_hostif_reason_code_t reason_code = SWITCH_HOSTIF_REASON_CODE_NONE;
  switch (trap_id) {
    case SAI_HOSTIF_TRAP_TYPE_STP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_STP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_LACP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_LACP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_EAPOL:
      reason_code = SWITCH_HOSTIF_REASON_CODE_EAPOL;
      break;
    case SAI_HOSTIF_TRAP_TYPE_LLDP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_LLDP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_PVRST:
      reason_code = SWITCH_HOSTIF_REASON_CODE_PVRST;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_QUERY;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_LEAVE;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V1_REPORT;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V2_REPORT;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V3_REPORT;
      break;
    case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET:
      reason_code = SWITCH_HOSTIF_REASON_CODE_SAMPLEPACKET;
      break;
    case SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST:
      reason_code = SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST;
      break;
    case SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE:
      reason_code = SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE;
      break;
    case SAI_HOSTIF_TRAP_TYPE_DHCP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_DHCP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_OSPF:
      reason_code = SWITCH_HOSTIF_REASON_CODE_OSPF;
      break;
    case SAI_HOSTIF_TRAP_TYPE_PIM:
      reason_code = SWITCH_HOSTIF_REASON_CODE_PIM;
      break;
    case SAI_HOSTIF_TRAP_TYPE_VRRP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_VRRP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_BGP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_BGP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_DHCPV6:
      reason_code = SWITCH_HOSTIF_REASON_CODE_DHCPV6;
      break;
    case SAI_HOSTIF_TRAP_TYPE_OSPFV6:
      reason_code = SWITCH_HOSTIF_REASON_CODE_OSPFV6;
      break;
    case SAI_HOSTIF_TRAP_TYPE_VRRPV6:
      reason_code = SWITCH_HOSTIF_REASON_CODE_VRRPV6;
      break;
    case SAI_HOSTIF_TRAP_TYPE_BGPV6:
      reason_code = SWITCH_HOSTIF_REASON_CODE_BGPV6;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IPV6_NEIGHBOR_DISCOVERY;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_V2;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_REPORT;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE:
      reason_code = SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_DONE;
      break;
    case SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT:
      reason_code = SWITCH_HOSTIF_REASON_CODE_MLD_V2_REPORT;
      break;
    case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR:
      reason_code = SWITCH_HOSTIF_REASON_CODE_L3_MTU_ERROR;
      break;
    case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR:
      reason_code = SWITCH_HOSTIF_REASON_CODE_TTL_ERROR;
      break;
    case SAI_HOSTIF_TRAP_TYPE_SSH:
      reason_code = SWITCH_HOSTIF_REASON_CODE_SSH;
      break;
    case SAI_HOSTIF_TRAP_TYPE_SNMP:
      reason_code = SWITCH_HOSTIF_REASON_CODE_SNMP;
      break;
    case SAI_HOSTIF_TRAP_TYPE_IP2ME:
      reason_code = SWITCH_HOSTIF_REASON_CODE_MYIP;
      break;
    default:
      break;
  }
  return reason_code;
}

switch_acl_action_t switch_sai_action_to_switch_api_action(
    sai_packet_action_t packet_action) {
  switch_acl_action_t acl_action = SWITCH_ACL_ACTION_NOP;
  switch (packet_action) {
    case SAI_PACKET_ACTION_DROP:
      acl_action = SWITCH_ACL_ACTION_DROP;
      break;
    case SAI_PACKET_ACTION_FORWARD:
      acl_action = SWITCH_ACL_ACTION_PERMIT;
      break;
    case SAI_PACKET_ACTION_TRAP:
      acl_action = SWITCH_ACL_ACTION_REDIRECT_TO_CPU;
      break;
    case SAI_PACKET_ACTION_LOG:
      acl_action = SWITCH_ACL_ACTION_COPY_TO_CPU;
      break;
    default:
      break;
  }
  return acl_action;
}

/*
* Routine Description:
*    Create host interface trap
*
* Arguments:
*  [in] hostif_trap_id - host interface trap id
 * [in] switch_id Switch object id
*  [in] attr_count - number of attributes
*  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_hostif_trap(_In_ sai_object_id_t *hostif_trapid,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_hostif_rcode_info_t rcode_api_info;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  const sai_attribute_t *attribute = NULL;
  switch_uint64_t flags = 0;
  uint32_t index = 0;
  switch_handle_t hostif_reason_code_handle = SWITCH_API_INVALID_HANDLE;
  *hostif_trapid = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&rcode_api_info, 0, sizeof(switch_api_hostif_rcode_info_t));
  flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP;
  rcode_api_info.hostif_group_id = sai_hostif_get_default();
  rcode_api_info.priority = sai_acl_priority_to_switch_priority(0);
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE:
        flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE;
        rcode_api_info.reason_code =
            switch_sai_to_switch_api_reason_code(attribute->value.u32);
        break;
      case SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION:
        flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION;
        rcode_api_info.action =
            switch_sai_action_to_switch_api_action(attribute->value.u32);
        break;
      case SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY:
        flags |= SWITCH_HOSTIF_RCODE_ATTR_PRIORITY;
        rcode_api_info.priority =
            sai_acl_priority_to_switch_priority(attribute->value.u32);
        break;
      case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
        rcode_api_info.hostif_group_id = attribute->value.oid;
        break;
      default:
        break;
    }
  }
  switch_status = switch_api_hostif_reason_code_create(
      device, flags, &rcode_api_info, &hostif_reason_code_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS &&
      status != SAI_STATUS_ITEM_ALREADY_EXISTS) {
    SAI_LOG_ERROR("failed to create hostif trap %d: %s",
                  rcode_api_info.reason_code,
                  sai_status_to_string(status));
    return status;
  }

  *hostif_trapid = hostif_reason_code_handle;
  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove host interface trap
*
* Arguments:
*  [in] hostif_trap_id - host interface trap id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_hostif_trap(_In_ sai_object_id_t hostif_trapid) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_hostif_reason_code_delete(device, hostif_trapid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove hostif trap %x: %s",
                  hostif_trapid,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/*
* Routine Description:
*   Set trap attribute value.
*
* Arguments:
*    [in] hostif_trap_id - host interface trap id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_hostif_trap_attribute(_In_ sai_object_id_t hostif_trapid,
                                           _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  switch_api_hostif_rcode_info_t rcode_api_info;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_uint64_t flags = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  if (!attr) {
    status = sai_remove_hostif_trap(hostif_trapid);
    return status;
  }

  memset(&rcode_api_info, 0, sizeof(switch_api_hostif_rcode_info_t));
  rcode_api_info.reason_code =
      switch_sai_to_switch_api_reason_code(hostif_trapid);
  rcode_api_info.priority = sai_acl_priority_to_switch_priority(0);
  switch (attr->id) {
    case SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION:
      flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION;
      rcode_api_info.action =
          switch_sai_action_to_switch_api_action(attr->value.u32);
      break;
    case SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY:
      flags |= SWITCH_HOSTIF_RCODE_ATTR_PRIORITY;
      rcode_api_info.priority =
          sai_acl_priority_to_switch_priority(attr->value.u32);
      break;
    case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
      flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP;
      rcode_api_info.hostif_group_id = attr->value.oid;
      break;
    default:
      break;
  }
  switch_status = switch_api_hostif_reason_code_update(
      device, hostif_trapid, flags, &rcode_api_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to update hostif trap %x: %s",
                  hostif_trapid,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/*
* Routine Description:
*   Get trap attribute value.
*
* Arguments:
*    [in] hostif_trap_id - host interface trap id
*    [in] attr_count - number of attributes
*    [in,out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_hostif_trap_attribute(_In_ sai_object_id_t hostif_trapid,
                                           _In_ uint32_t attr_count,
                                           _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }
  for (unsigned int i = 0; i < attr_count; i++) {
    switch (attr_list[i].id) {
      case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
        attr_list[i].value.oid = default_hostif_trap_group_id;
        break;
    }
  }
  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Set user defined trap attribute value.
*
* Arguments:
*    [in] hostif_user_defined_trap_id - host interface user defined trap id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_hostif_user_defined_trap_attribute(
    _In_ sai_object_id_t hostif_user_defined_trapid,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Get user defined trap attribute value.
*
* Arguments:
*    [in] hostif_user_defined_trap_id - host interface user defined trap id
*    [in] attr_count - number of attributes
*    [in,out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_hostif_user_defined_trap_attribute(
    _In_ sai_object_id_t hostif_user_defined_trapid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

sai_status_t switch_sai_channel_to_switch_api_channel(
    sai_hostif_table_entry_channel_type_t trap_channel,
    switch_hostif_channel_t *hostif_channel) {
  switch (trap_channel) {
    case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_FD:
      *hostif_channel = SWITCH_HOSTIF_CHANNEL_CB;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_CB:
      *hostif_channel = SWITCH_HOSTIF_CHANNEL_CB;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT:
      *hostif_channel = SWITCH_HOSTIF_CHANNEL_NETDEV;
      break;
    default:
      return SAI_STATUS_NOT_SUPPORTED;
      break;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_create_hostif_table_entry(
    _In_ sai_object_id_t *hostif_table_entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  const sai_attribute_t *attribute = NULL;
  uint32_t index = 0;
  sai_object_id_t oid = 0;
  switch_hostif_rx_filter_priority_t priority = 0;
  switch_uint64_t flags = 0;
  switch_hostif_rx_filter_key_t rx_key = {0};
  switch_hostif_rx_filter_action_t rx_action = {0};
  sai_hostif_table_entry_type_t sai_hostif_table_entry_type = 0;
  switch_handle_t hostif_table_entry_handle = SWITCH_API_INVALID_HANDLE;
  *hostif_table_entry_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&rx_key, 0x0, sizeof(rx_key));
  memset(&rx_action, 0x0, sizeof(rx_action));

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE:
        sai_hostif_table_entry_type = attribute->value.u32;
        break;
      case SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID:
        oid = attribute->value.oid;
        break;
      case SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID:
        rx_key.reason_code =
            switch_sai_to_switch_api_reason_code(attribute->value.oid);
        rx_key.reason_code_mask = 0xFFFFFFFF;
        break;
      case SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE:
        break;
      case SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF:
        rx_action.hostif_handle = attribute->value.oid;
        break;
      default:
        break;
    }
  }

  switch (sai_hostif_table_entry_type) {
    case SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT:
      flags |= SWITCH_HOSTIF_RX_FILTER_ATTR_PORT_HANDLE;
      rx_key.port_handle = oid;
      priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_PORT;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG:
      flags |= SWITCH_HOSTIF_RX_FILTER_ATTR_INTF_HANDLE;
      rx_key.intf_handle = oid;
      priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_INTERFACE;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_TYPE_VLAN:
      flags |= SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE;
      priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_VLAN;
      rx_key.handle = oid;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD:
      flags |= SWITCH_HOSTIF_RX_FILTER_ATTR_GLOBAL;
      priority = SWITCH_HOSTIF_RX_FILTER_PRIORITY_MIN;
      break;
    case SAI_HOSTIF_TABLE_ENTRY_TYPE_TRAP_ID:
      flags |= SWITCH_HOSTIF_RX_FILTER_ATTR_REASON_CODE;
      //      status = switch_api_hostif_reason_code_get(device,
      //      (switch_handle_t)oid, &(rx_key.reason_code));
      break;
    default:
      return SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  switch_status = switch_api_hostif_rx_filter_create(
      device, priority, flags, &rx_key, &rx_action, &hostif_table_entry_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create hostif trap %lx: %s",
                  *hostif_table_entry_id,
                  sai_status_to_string(status));
    return status;
  }
  *hostif_table_entry_id = hostif_table_entry_handle;
  SAI_LOG_EXIT();
  return status;
}

sai_status_t sai_remove_hostif_table_entry(_In_ sai_object_id_t
                                               hostif_table_entry_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status =
      switch_api_hostif_rx_filter_delete(device, hostif_table_entry_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove hostif trap %x: %s",
                  hostif_table_entry_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

sai_status_t sai_get_hostif_table_entry_attribute(
    _In_ sai_object_id_t hostif_table_entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  SAI_LOG_EXIT();

  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_set_hostif_table_entry_attribute(
    _In_ sai_object_id_t hostif_table_entry_id,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  SAI_LOG_EXIT();

  return SAI_STATUS_NOT_SUPPORTED;
} /*
 * Routine Description:
 *   hostif receive function
 *
 * Arguments:
 *    [in]  hif_id  - host interface id
 *    [out] buffer - packet buffer
 *    [in,out] buffer_size - [in] allocated buffer size. [out] actual packet
 *size
 *in bytes
 *    [in,out] attr_count - [in] allocated list size. [out] number of attributes
 *    [out] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    SAI_STATUS_BUFFER_OVERFLOW if buffer_size is insufficient,
 *    and buffer_size will be filled with required size. Or
 *    if attr_count is insufficient, and attr_count
 *    will be filled with required count.
 *    Failure status code on error
 */
sai_status_t sai_recv_hostif_packet(_In_ sai_object_id_t hif_id,
                                    _Out_ void *buffer,
                                    _Inout_ sai_size_t *buffer_size,
                                    _Inout_ uint32_t *attr_count,
                                    _Out_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  SAI_ASSERT(sai_object_type_query(hif_id) == SAI_OBJECT_TYPE_HOSTIF);

  SAI_LOG_EXIT();

  return SAI_STATUS_SUCCESS;
}

switch_uint16_t switch_sai_tx_type_to_switch_api_tx_type(
    sai_hostif_tx_type_t tx_type) {
  switch (tx_type) {
    case SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS:
      return SWITCH_BYPASS_ALL;
    case SAI_HOSTIF_TX_TYPE_PIPELINE_LOOKUP:
      return SWITCH_BYPASS_NONE;
    default:
      return SWITCH_BYPASS_NONE;
  }
}

/*
* Routine Description:
*   hostif send function
*
* Arguments:
*    [in] hif_id  - host interface id. only valid for send through FD channel.
*Use SAI_NULL_OBJECT_ID for send through CB channel.
*    [In] buffer - packet buffer
*    [in] buffer size - packet size in bytes
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_send_hostif_packet(_In_ sai_object_id_t hif_id,
                                    _In_ void *buffer,
                                    _In_ sai_size_t buffer_size,
                                    _In_ uint32_t attr_count,
                                    _In_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_hostif_packet_t hostif_packet;
  sai_status_t status = SAI_STATUS_SUCCESS;
  const sai_attribute_t *attribute;
  uint32_t index = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  memset(&hostif_packet, 0, sizeof(switch_hostif_packet_t));
  hostif_packet.pkt = buffer;
  hostif_packet.pkt_size = buffer_size;

  if (!buffer) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return SAI_STATUS_INVALID_PARAMETER;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return SAI_STATUS_INVALID_PARAMETER;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE:
        hostif_packet.bypass_flags =
            switch_sai_tx_type_to_switch_api_tx_type(attribute->value.u32);
        break;
      case SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG:
        hostif_packet.handle = attribute->value.oid;
        // Set is_lag flag if oid is lag
        break;
      default:
        break;
    }
  }

  switch_status = switch_api_hostif_tx_packet(&hostif_packet);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to send hostif packet on %lx: %s",
                  hif_id,
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return status;
}

/*
* Routine Description:
*   hostif receive callback
*
* Arguments:
*    [in] buffer - packet buffer
*    [in] buffer_size - actual packet size in bytes
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*/
void sai_recv_hostif_packet_cb(switch_hostif_packet_t *hostif_packet) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_object_id_t device = 0;
  if (!hostif_packet) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return;
  }

  int max_attr_count = 3;
  int attr_count = 0;
  sai_attribute_t attr_list[max_attr_count];
  sai_attribute_t *attribute;
  attribute = &attr_list[attr_count];
  attribute->id = SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID;
  attribute->value.u32 = hostif_packet->reason_code;
  attr_count++;
  attribute = &attr_list[attr_count];
  attribute->id = SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT;
  attribute->value.oid = hostif_packet->handle;
  attr_count++;
  if (hostif_packet->lag_handle != SWITCH_API_INVALID_HANDLE) {
    attribute = &attr_list[attr_count];
    attribute->id = SAI_HOSTIF_PACKET_ATTR_INGRESS_LAG;
    attribute->value.oid = hostif_packet->lag_handle;
    attr_count++;
  }

  if (sai_switch_notifications.on_packet_event) {
    sai_switch_notifications.on_packet_event(device,
                                             hostif_packet->pkt,
                                             hostif_packet->pkt_size,
                                             attr_count,
                                             attr_list);
  }
  SAI_LOG_EXIT();
  return;
}

// Average packet size as 100Bytes for packet rate policer.
#define SAI_AVERAGE_PACKET_SIZE 100
/*
* hostif methods table retrieved with sai_api_query()
*/
sai_hostif_api_t hostif_api = {
    .create_hostif = sai_create_hostif,
    .remove_hostif = sai_remove_hostif,
    .set_hostif_attribute = sai_set_hostif_attribute,
    .get_hostif_attribute = sai_get_hostif_attribute,
    .create_hostif_trap_group = sai_create_hostif_trap_group,
    .remove_hostif_trap_group = sai_remove_hostif_trap_group,
    .set_hostif_trap_group_attribute = sai_set_hostif_trap_group_attribute,
    .get_hostif_trap_group_attribute = sai_get_hostif_trap_group_attribute,
    .create_hostif_trap = sai_create_hostif_trap,
    .remove_hostif_trap = sai_remove_hostif_trap,
    .set_hostif_trap_attribute = sai_set_hostif_trap_attribute,
    .get_hostif_trap_attribute = sai_get_hostif_trap_attribute,
    .set_hostif_user_defined_trap_attribute =
        sai_set_hostif_user_defined_trap_attribute,
    .get_hostif_user_defined_trap_attribute =
        sai_get_hostif_user_defined_trap_attribute,
    .recv_hostif_packet = sai_recv_hostif_packet,
    .send_hostif_packet = sai_send_hostif_packet,
    .create_hostif_table_entry = sai_create_hostif_table_entry,
    .remove_hostif_table_entry = sai_remove_hostif_table_entry,
    .set_hostif_table_entry_attribute = sai_set_hostif_table_entry_attribute,
    .get_hostif_table_entry_attribute = sai_get_hostif_table_entry_attribute};

sai_status_t sai_hostif_initialize(sai_api_service_t *sai_api_service) {
  switch_hostif_group_t hostif_group;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t cpu_port_handle;
  uint32_t maxCpuQ = 0;
  switch_api_meter_t api_meter_info;
  switch_handle_t policer_handle = 0;

  SAI_LOG_DEBUG("Initializing host interface");
  sai_api_service->hostif_api = hostif_api;
  switch_api_hostif_rx_callback_register(
      device, SWITCH_SAI_APP_ID, &sai_recv_hostif_packet_cb, NULL);

  status = switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "failed to get cpu port handle: %s,"
        " hostif Initialization error",
        sai_status_to_string(status));
    return SAI_STATUS_FAILURE;
  }

  status = switch_api_max_cpu_queues_get(device, &maxCpuQ);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "failed to get max cpu queue: %s,"
        " hostif Initialization error",
        sai_status_to_string(status));
    return SAI_STATUS_FAILURE;
  }

  cpuQHandles =
      (switch_handle_t *)SAI_MALLOC(sizeof(switch_handle_t) * maxCpuQ);
  if (cpuQHandles == NULL) {
    SAI_LOG_ERROR(
        "hostif Initialization error: %s,"
        " cpuQhandle malloc failed");
    return SAI_STATUS_FAILURE;
  }

  memset(cpuQHandles, 0, sizeof(switch_handle_t) * maxCpuQ);
  status =
      switch_api_queues_get(device, cpu_port_handle, &maxCpuQ, cpuQHandles);
  if (status != SWITCH_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "hostif Initialization error: %s,"
        " cpuQhandle get failed");
    SAI_FREE(cpuQHandles);
    cpuQHandles = NULL;
    return SAI_STATUS_FAILURE;
  }

  // create the default trap group
  memset(&hostif_group, 0, sizeof(hostif_group));
  hostif_group.queue_handle = cpuQHandles[0];
  memset(&api_meter_info, 0, sizeof(api_meter_info));
  api_meter_info.meter_mode = SWITCH_METER_MODE_TWO_RATE_THREE_COLOR;
  api_meter_info.color_source = SWITCH_METER_COLOR_SOURCE_BLIND;
  api_meter_info.meter_type = SWITCH_METER_TYPE_PACKETS;
  api_meter_info.cbs = api_meter_info.pbs = 1000;
  api_meter_info.cir = 1000;
  api_meter_info.pir = 5000;
  api_meter_info.action[SWITCH_METER_COUNTER_GREEN] = SAI_PACKET_ACTION_FORWARD;
  api_meter_info.action[SWITCH_METER_COUNTER_YELLOW] =
      SAI_PACKET_ACTION_FORWARD;
  api_meter_info.action[SWITCH_METER_COUNTER_RED] = SAI_PACKET_ACTION_FORWARD;
  switch_api_meter_create(device, &api_meter_info, &policer_handle);
  hostif_group.policer_handle = policer_handle;
  switch_api_hostif_group_create(
      device, &hostif_group, &default_hostif_trap_group_id);

  // create default 100G policer
  memset(&api_meter_info, 0, sizeof(api_meter_info));
  api_meter_info.meter_mode = SWITCH_METER_MODE_TWO_RATE_THREE_COLOR;
  api_meter_info.color_source = SWITCH_METER_COLOR_SOURCE_BLIND;
  api_meter_info.meter_type = SWITCH_METER_TYPE_PACKETS;
  api_meter_info.cbs = api_meter_info.pbs = 1000;
  api_meter_info.cir = 100000000000UL / (8 * SAI_AVERAGE_PACKET_SIZE);
  api_meter_info.pir = 100000000000UL / (8 * SAI_AVERAGE_PACKET_SIZE);
  api_meter_info.action[SWITCH_METER_COUNTER_GREEN] = SAI_PACKET_ACTION_FORWARD;
  api_meter_info.action[SWITCH_METER_COUNTER_YELLOW] =
      SAI_PACKET_ACTION_FORWARD;
  api_meter_info.action[SWITCH_METER_COUNTER_RED] = SAI_PACKET_ACTION_FORWARD;
  switch_api_meter_create(
      device, &api_meter_info, &default_hostif_trap_group_policer);

  return SAI_STATUS_SUCCESS;
}
