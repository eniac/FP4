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

#include "switchapi/switch.h"

#include "switch_test.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __TEST_MODULE__
#define __TEST_MODULE__ "L2"

switch_status_t switch_l2_test(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;
  switch_uint16_t num_macs = 16;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_member1 = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t api_intf_info;
  switch_api_mac_entry_t api_mac_entry;

  SWITCH_START_TEST(__TEST_MODULE__);

  switch_api_port_info_t api_port_info;
  SWITCH_MEMSET(&api_port_info, 0x0, sizeof(api_port_info));
  api_port_info.port_speed = SWITCH_PORT_SPEED_100G;

  api_port_info.port = 0x1;
  status = switch_api_port_add(device, &api_port_info, &port_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  switch_mac_addr_t mac;

  mac.mac_addr[0] = 0x00;
  mac.mac_addr[1] = 0x00;
  mac.mac_addr[2] = 0x01;
  mac.mac_addr[3] = 0x00;
  mac.mac_addr[4] = 0x00;
  mac.mac_addr[5] = 0x01;

  status = switch_api_vlan_create(device, 10, &vlan_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  api_intf_info.type = SWITCH_INTERFACE_TYPE_ACCESS;
  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.handle = port_handle1;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_ACCESS;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_member_add(
      device, vlan_handle, intf_handle1, &vlan_member1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&api_mac_entry, 0x0, sizeof(api_mac_entry));
  for (index = 0; index < num_macs; index++) {
    mac.mac_addr[5] = index;
    api_mac_entry.network_handle = vlan_handle;
    api_mac_entry.handle = intf_handle1;
    SWITCH_MEMCPY(&api_mac_entry.mac, &mac, sizeof(switch_mac_addr_t));
    status = switch_api_mac_table_entry_add(device, &api_mac_entry);
    TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
    status = switch_api_mac_entry_dump(device, &api_mac_entry, NULL);
    TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  SWITCH_MEMSET(&api_mac_entry, 0x0, sizeof(api_mac_entry));
  for (index = 0; index < num_macs; index++) {
    mac.mac_addr[5] = index;
    api_mac_entry.network_handle = vlan_handle;
    api_mac_entry.handle = intf_handle1;
    SWITCH_MEMCPY(&api_mac_entry.mac, &mac, sizeof(switch_mac_addr_t));
    status = switch_api_mac_table_entry_delete(device, &api_mac_entry);
    TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_api_vlan_member_remove(device, vlan_handle, intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_interface_delete(device, intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_port_delete(device, port_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_delete(device, vlan_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
