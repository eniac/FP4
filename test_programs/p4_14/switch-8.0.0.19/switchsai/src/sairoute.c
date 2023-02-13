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

#include <sairoute.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_interface.h>
#include <switchapi/switch_l3.h>
#include <switchapi/switch_hostif.h>

static sai_api_t api_id = SAI_API_ROUTE;

static void sai_route_entry_to_string(_In_ const sai_route_entry_t *route_entry,
                                      _Out_ char *entry_string) {
  int count = 0;
  int len = 0;
  count = snprintf(entry_string,
                   SAI_MAX_ENTRY_STRING_LEN,
                   "route: vrf %" PRIx64 " ",
                   route_entry->vr_id);
  sai_ipprefix_to_string(route_entry->destination,
                         SAI_MAX_ENTRY_STRING_LEN - count,
                         entry_string + count,
                         &len);
  return;
}

static void sai_route_entry_parse(_In_ const sai_route_entry_t *route_entry,
                                  _Out_ switch_handle_t *vrf_handle,
                                  _Out_ switch_ip_addr_t *ip_addr) {
  const sai_ip_prefix_t *sai_ip_prefix;

  SAI_ASSERT(sai_object_type_query(route_entry->vr_id) ==
             SAI_OBJECT_TYPE_VIRTUAL_ROUTER);
  *vrf_handle = (switch_handle_t)route_entry->vr_id;

  memset(ip_addr, 0, sizeof(switch_ip_addr_t));
  sai_ip_prefix = &route_entry->destination;
  sai_ip_prefix_to_switch_ip_addr(sai_ip_prefix, ip_addr);
}

static void sai_route_entry_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    switch_handle_t *nhop_handle,
    int *action,
    int *pri) {
  const sai_attribute_t *attribute;
  uint32_t index = 0;

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
        *nhop_handle = (switch_handle_t)attribute->value.oid;
        break;
      case SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY:
        *pri = attribute->value.u8;
        break;
      case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
        *action = attribute->value.s32;
        break;
    }
  }
}

bool sai_route_entry_host_route(const sai_ip_prefix_t *sai_ip_addr,
                                switch_handle_t nexthop_handle) {
  uint32_t prefix_len = 0;
  switch_handle_t cpu_port_handle = 0;
  switch_api_device_cpu_port_handle_get(device, &cpu_port_handle);

  if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    sai_ipv4_prefix_length(ntohl(sai_ip_addr->mask.ip4), &prefix_len);
    if (prefix_len == 32 && (nexthop_handle == cpu_port_handle)) {
      return TRUE;
    }
  } else if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    sai_ipv6_prefix_length(sai_ip_addr->mask.ip6, &prefix_len);
    if (prefix_len == 128 && (nexthop_handle == cpu_port_handle)) {
      return TRUE;
    }
  }
  return FALSE;
}

sai_status_t sai_route_entry_update(const sai_route_entry_t *route_entry,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list,
                                    bool add) {
  switch_ip_addr_t ip_addr;
  switch_handle_t nhop_handle = 0;
  switch_handle_t vrf_handle = 0;
  switch_api_route_entry_t api_route_entry;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];
  int action = -1, pri = -1;
  if (!route_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null unicast entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  sai_route_entry_parse(route_entry, &vrf_handle, &ip_addr);
  sai_route_entry_attribute_parse(
      attr_count, attr_list, &nhop_handle, &action, &pri);
  sai_route_entry_to_string(route_entry, entry_string);

  if (!nhop_handle && action != -1) {
    switch (action) {
      case SAI_PACKET_ACTION_DROP:
        switch_status = switch_api_hostif_nhop_get(
            device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &nhop_handle);
        break;
      case SAI_PACKET_ACTION_FORWARD:
        break;
      case SAI_PACKET_ACTION_TRAP:
        switch_status = switch_api_hostif_nhop_get(
            device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
        break;
      default:
        break;
    }
  }

  if (sai_object_type_query(nhop_handle) == SAI_OBJECT_TYPE_ROUTER_INTERFACE) {
    switch_status = switch_api_hostif_nhop_get(
        device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  }

  if (nhop_handle) {
    memset(&api_route_entry, 0, sizeof(switch_api_route_entry_t));
    api_route_entry.vrf_handle = vrf_handle;
    api_route_entry.nhop_handle = nhop_handle;
    api_route_entry.neighbor_installed = FALSE;
    memcpy(&api_route_entry.ip_address, &ip_addr, sizeof(switch_ip_addr_t));
    if (add) {
      if (sai_route_entry_host_route(&route_entry->destination, nhop_handle)) {
        api_route_entry.route_type = SWITCH_ROUTE_TYPE_MYIP;
        switch_status = switch_api_hostif_nhop_get(
            device, SWITCH_HOSTIF_REASON_CODE_MYIP, &nhop_handle);
        api_route_entry.nhop_handle = nhop_handle;
      }
      switch_status = switch_api_l3_route_add(device, &api_route_entry);
    } else {
      switch_status = switch_api_l3_route_update(device, &api_route_entry);
    }
    status = sai_switch_status_to_sai_status(switch_status);
    status = SAI_STATUS_SUCCESS;
  }
  return status;
}
/*
* Routine Description:
*    Create Route
*
* Arguments:
*    [in] route_entry - route entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP prefix/mask expected in Network Byte Order.
*
*/
sai_status_t sai_create_route_entry(_In_ const sai_route_entry_t *route_entry,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  sai_route_entry_to_string(route_entry, entry_string);

  status = sai_route_entry_update(route_entry, attr_count, attr_list, TRUE);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Route entry create failed for route entry %s: %s",
                  entry_string,
                  sai_status_to_string(status));
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Remove Route
*
* Arguments:
*    [in] route_entry - route entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP prefix/mask expected in Network Byte Order.
*/
sai_status_t sai_remove_route_entry(_In_ const sai_route_entry_t *route_entry) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_ip_addr_t ip_addr;
  switch_api_route_entry_t api_route_entry;
  switch_handle_t vrf_handle = 0;
  switch_handle_t nhop_handle = 0;

  if (!route_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null unicast entry: %s", sai_status_to_string(status));
    return status;
  }

  sai_route_entry_parse(route_entry, &vrf_handle, &ip_addr);
  memset(&api_route_entry, 0, sizeof(switch_api_route_entry_t));
  api_route_entry.vrf_handle = vrf_handle;
  memcpy(&api_route_entry.ip_address, &ip_addr, sizeof(switch_ip_addr_t));
  api_route_entry.neighbor_installed = FALSE;

  switch_status =
      switch_api_l3_route_lookup(device, &api_route_entry, &nhop_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  SAI_ASSERT(status == SAI_STATUS_SUCCESS);
  api_route_entry.nhop_handle = nhop_handle;

  switch_status = switch_api_l3_route_delete(device, &api_route_entry);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove route entry: %s",
                  sai_status_to_string(status));
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Set route attribute value
*
* Arguments:
*    [in] route_entry - route entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_route_entry_attribute(
    _In_ const sai_route_entry_t *route_entry,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  sai_route_entry_to_string(route_entry, entry_string);

  status = sai_route_entry_update(route_entry, 1, attr, FALSE);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Route entry update failed for route entry %s: %s",
                  entry_string,
                  sai_status_to_string(status));
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Get route attribute value
*
* Arguments:
*    [in] route_entry - route entry
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_route_entry_attribute(
    _In_ const sai_route_entry_t *route_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  unsigned int i = 0;
  switch_ip_addr_t ip_addr;
  switch_handle_t nhop_handle = 0;
  switch_handle_t vrf_handle = 0;
  switch_api_route_entry_t api_route_entry;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t glean_handle = 0;
  switch_handle_t drop_handle = 0;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];
  sai_attribute_t *attr = attr_list;

  if (!route_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null unicast entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  sai_route_entry_parse(route_entry, &vrf_handle, &ip_addr);
  memset(&api_route_entry, 0, sizeof(switch_api_route_entry_t));
  api_route_entry.vrf_handle = vrf_handle;
  api_route_entry.neighbor_installed = FALSE;
  memcpy(&api_route_entry.ip_address, &ip_addr, sizeof(switch_ip_addr_t));
  sai_route_entry_to_string(route_entry, entry_string);

  switch_status =
      switch_api_l3_route_lookup(device, &api_route_entry, &nhop_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get nexthop for route entry %s : %s",
                  entry_string,
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &glean_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get glean nexthop entry : %s",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_NULL_DROP, &drop_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get drop nexthop entry : %s",
                  sai_status_to_string(status));
    return status;
  }

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
        attr->value.oid = (nhop_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : nhop_handle;
        break;
      case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
        if (nhop_handle == drop_handle) {
          attr->value.oid = SAI_PACKET_ACTION_DROP;
        } else if (nhop_handle == glean_handle) {
          attr->value.oid = SAI_PACKET_ACTION_TRAP;
        } else if (nhop_handle != SWITCH_API_INVALID_HANDLE) {
          attr->value.oid = SAI_PACKET_ACTION_FORWARD;
        }
        break;
      default:
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Router entry methods table retrieved with sai_api_query()
*/
sai_route_api_t route_api = {
    .create_route_entry = sai_create_route_entry,
    .remove_route_entry = sai_remove_route_entry,
    .set_route_entry_attribute = sai_set_route_entry_attribute,
    .get_route_entry_attribute = sai_get_route_entry_attribute,
};

sai_status_t sai_route_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing route");
  sai_api_service->route_api = route_api;
  return SAI_STATUS_SUCCESS;
}
