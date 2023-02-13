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

#include "switchapi/switch_handle.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

static tommy_list sr_info;

switch_status_t switch_sr_endpoint_add(
    switch_device_t device, switch_interface_ip_addr_t *ip_addr_info) {
  switch_sr_endpoint_t *sr_endpoint = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  sr_endpoint = SWITCH_MALLOC(device, sizeof(switch_sr_endpoint_t), 1);
  sr_endpoint->ip_addr_info = *ip_addr_info;
  tommy_list_insert_head(&sr_info, &(sr_endpoint->node), sr_endpoint);

  status = switch_pd_srv6_table_entry_add(
      device, ip_addr_info, &(sr_endpoint->entry_hdl));

  return status;
}

switch_status_t switch_sr_endpoint_delete(
    switch_device_t device, switch_interface_ip_addr_t *ip_addr_info) {
  switch_sr_endpoint_t *sr_endpoint = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  node = tommy_list_head(&sr_info);
  while (node) {
    sr_endpoint = (switch_sr_endpoint_t *)node->data;
    if (sr_endpoint->ip_addr_info.vrf_handle == ip_addr_info->vrf_handle &&
        !memcmp((&(sr_endpoint->ip_addr_info.ip_address)),
                (&(ip_addr_info->ip_address)),
                sizeof(switch_ip_addr_t))) {
      break;
    }
    node = node->next;
  }

  if (!node) {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }

  status = switch_pd_srv6_table_entry_delete(device, sr_endpoint->entry_hdl);

  sr_endpoint = tommy_list_remove_existing(&sr_info, node);
  SWITCH_FREE(device, sr_endpoint);

  return status;
}
