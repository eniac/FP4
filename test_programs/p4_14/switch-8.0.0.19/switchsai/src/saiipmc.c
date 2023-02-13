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

#include <saiipmc.h>
#include "saiinternal.h"
#include <switchapi/switch_interface.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_mcast.h>
#include <switchapi/switch_vlan.h>

static sai_api_t api_id = SAI_API_IPMC;

static void sai_ipmc_entry_to_string(_In_ const sai_ipmc_entry_t *ipmc_entry,
                                     _Out_ char *entry_string) {
  int count = 0;
  int len = 0;
  count = snprintf(entry_string,
                   SAI_MAX_ENTRY_STRING_LEN,
                   "route: vrf %" PRIx64 " (",
                   ipmc_entry->vr_id);
  if (count > SAI_MAX_ENTRY_STRING_LEN) {
    return;
  }
  sai_ipaddress_to_string(ipmc_entry->source,
                          SAI_MAX_ENTRY_STRING_LEN - count,
                          entry_string + count,
                          &len);
  count += len;
  if (count > SAI_MAX_ENTRY_STRING_LEN) {
    return;
  }
  count +=
      snprintf(entry_string + count, SAI_MAX_ENTRY_STRING_LEN - count, ",");
  if (count > SAI_MAX_ENTRY_STRING_LEN) {
    return;
  }
  /*
  sai_ipprefix_to_string(ipmc_entry->group,
                         SAI_MAX_ENTRY_STRING_LEN - count,
                         entry_string + count,
                         &len);
                         */
  count += len;
  if (count > SAI_MAX_ENTRY_STRING_LEN) {
    return;
  }
  count +=
      snprintf(entry_string + count, SAI_MAX_ENTRY_STRING_LEN - count, ")");
  return;
}

static void sai_ipmc_entry_parse(_In_ const sai_ipmc_entry_t *ipmc_entry,
                                 _Out_ switch_handle_t *vrf_handle,
                                 _Out_ switch_ip_addr_t *src_addr,
                                 _Out_ switch_ip_addr_t *grp_addr) {
  SAI_ASSERT(sai_object_type_query(ipmc_entry->vr_id) ==
             SAI_OBJECT_TYPE_VIRTUAL_ROUTER);
  *vrf_handle = (switch_handle_t)ipmc_entry->vr_id;

  memset(src_addr, 0, sizeof(switch_ip_addr_t));
  sai_ip_addr_to_switch_ip_addr(&(ipmc_entry->source), src_addr);
  if (((src_addr->type == SWITCH_API_IP_ADDR_V4) &&
       (src_addr->ip.v4addr == 0)) ||
      ((src_addr->type == SWITCH_API_IP_ADDR_V6) &&
       (memcmp(src_addr->ip.v6addr.u.addr8,
               &in6addr_any,
               sizeof(in6addr_any)) == 0))) {
    src_addr->prefix_len = 0;
  }
  // sai_ip_prefix_to_switch_ip_addr(&(ipmc_entry->group), grp_addr);
}

static void sai_ipmc_entry_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _Out_ switch_handle_t *mcast_handle,
    _Out_ switch_handle_t *rpf_handle,
    _Out_ int *action,
    _Out_ int *pri) {
  const sai_attribute_t *attribute;
  uint32_t index = 0;

  *action = SAI_PACKET_ACTION_FORWARD;
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_IPMC_ENTRY_ATTR_PACKET_ACTION:
        *action = attribute->value.s32;
        break;
      case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
        *mcast_handle = (switch_handle_t)attribute->value.oid;
        break;
      case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
        *rpf_handle = (switch_handle_t)attribute->value.oid;
        break;
    }
  }
}

/*
 * Routine Description:
 *    Create IP multicast entry
 *
 * Arguments:
 *    [in] ipmc_entry - IP multicast entry
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_create_ipmc_entry(_In_ const sai_ipmc_entry_t *ipmc_entry,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_ip_addr_t src_addr, grp_addr;
  switch_handle_t vrf_handle = 0;
  int action = -1, pri = -1;
  switch_handle_t mcast_handle = SAI_NULL_OBJECT_ID;
  switch_handle_t rpf_handle = SAI_NULL_OBJECT_ID;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  if (!ipmc_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null ipmc entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  sai_ipmc_entry_parse(ipmc_entry, &vrf_handle, &src_addr, &grp_addr);
  sai_ipmc_entry_attribute_parse(
      attr_count, attr_list, &mcast_handle, &rpf_handle, &action, &pri);

  sai_ipmc_entry_to_string(ipmc_entry, entry_string);
  status = sai_switch_status_to_sai_status(switch_status);

  switch_status = switch_api_multicast_mroute_add(
      device, mcast_handle, rpf_handle, vrf_handle, &src_addr, &grp_addr, 1);
  status = sai_switch_status_to_sai_status(switch_status);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Remove IP multicast entry
 *
 * Arguments:
 *    [in] ipmc_entry - IP multicast entry
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_remove_ipmc_entry(_In_ const sai_ipmc_entry_t *ipmc_entry) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_ip_addr_t src_addr, grp_addr;
  switch_handle_t vrf_handle = 0;

  if (!ipmc_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null ipmc entry: %s", sai_status_to_string(status));
    return status;
  }

  sai_ipmc_entry_parse(ipmc_entry, &vrf_handle, &src_addr, &grp_addr);

  switch_handle_t mcast_handle;
  switch_handle_t rpf_handle;
  switch_status = switch_api_multicast_mroute_tree_get(
      device, vrf_handle, &src_addr, &grp_addr, &mcast_handle, &rpf_handle);
  if (status == SWITCH_STATUS_SUCCESS) {
    switch_status = switch_api_multicast_index_delete(device, mcast_handle);
    assert(switch_status == SWITCH_STATUS_SUCCESS);
  }

  switch_status = switch_api_multicast_mroute_delete(
      device, vrf_handle, &src_addr, &grp_addr);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove ipmc entry: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Set IP multicast entry attribute value
 *
 * Arguments:
 *    [in] IP multicast - IP multicast entry
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_set_ipmc_entry_attribute(
    _In_ const sai_ipmc_entry_t *ipmc_entry, _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];
  switch_handle_t mcast_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rpf_handle = SWITCH_API_INVALID_HANDLE;
  switch_ip_addr_t src_addr, grp_addr;
  switch_handle_t vrf_handle = 0;
  int action = -1, pri = -1;

  if (!ipmc_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null ipmc entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  sai_ipmc_entry_parse(ipmc_entry, &vrf_handle, &src_addr, &grp_addr);
  sai_ipmc_entry_attribute_parse(
      1, attr, &mcast_handle, &rpf_handle, &action, &pri);

  sai_ipmc_entry_to_string(ipmc_entry, entry_string);

  switch (attr->id) {
    case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
      switch_status = switch_api_multicast_mroute_mgid_set(
          device, mcast_handle, vrf_handle, &src_addr, &grp_addr, 1);
      break;
    case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
      switch_status = switch_api_multicast_mroute_rpf_set(
          device, rpf_handle, vrf_handle, &src_addr, &grp_addr, 1);
      break;
    default:
      break;
  }
  status = sai_switch_status_to_sai_status(switch_status);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Get IP multicast entry attribute value
 *
 * Arguments:
 *    [in] ipmc_entry - IP multicast entry
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_get_ipmc_entry_attribute(
    _In_ const sai_ipmc_entry_t *ipmc_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_ip_addr_t src_addr, grp_addr;
  switch_handle_t vrf_handle = 0;
  unsigned int i = 0;
  sai_attribute_t *attr = attr_list;
  switch_handle_t mcast_handle = 0;
  switch_handle_t rpf_handle = 0;

  if (!ipmc_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null ipmc entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  sai_ipmc_entry_parse(ipmc_entry, &vrf_handle, &src_addr, &grp_addr);

  switch_status = switch_api_multicast_mroute_tree_get(
      device, vrf_handle, &src_addr, &grp_addr, &mcast_handle, &rpf_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
        attr->value.oid = (mcast_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : mcast_handle;
        break;
      case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
        attr->value.oid = (rpf_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : rpf_handle;
        break;
      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * IP multicast method table retrieved with sai_api_query()
 */
sai_ipmc_api_t ipmc_api = {
    .create_ipmc_entry = sai_create_ipmc_entry,
    .remove_ipmc_entry = sai_remove_ipmc_entry,
    .set_ipmc_entry_attribute = sai_set_ipmc_entry_attribute,
    .get_ipmc_entry_attribute = sai_get_ipmc_entry_attribute,
};

sai_status_t sai_ipmc_initialize(sai_api_service_t *sai_api_service) {
  // SAI_LOG_DEBUG("Initializing ipmc");
  sai_api_service->ipmc_api = ipmc_api;
  return SAI_STATUS_SUCCESS;
}
