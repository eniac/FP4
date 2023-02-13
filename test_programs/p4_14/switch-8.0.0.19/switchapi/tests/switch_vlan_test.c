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
#define __TEST_MODULE__ "VLAN"

switch_status_t switch_vlan_test(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_vlan_t vlan_id = 200;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle2 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle2 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_member1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_member2 = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t api_intf_info;

  SWITCH_START_TEST(__TEST_MODULE__);

  switch_api_port_info_t api_port_info;
  SWITCH_MEMSET(&api_port_info, 0x0, sizeof(api_port_info));
  api_port_info.port_speed = SWITCH_PORT_SPEED_100G;

  api_port_info.port = 0x1;
  status = switch_api_port_add(device, &api_port_info, &port_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  api_port_info.port = 0x2;
  status = switch_api_port_add(device, &api_port_info, &port_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.handle = port_handle1;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_ACCESS;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.handle = port_handle2;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_TRUNK;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_create(device, vlan_id, &vlan_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_member_add(
      device, vlan_handle, intf_handle1, &vlan_member1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_vlan_member_add(
      device, vlan_handle, intf_handle2, &vlan_member2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_handle_dump(device, vlan_handle, NULL);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_member_remove(device, vlan_handle, intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_vlan_member_remove(device, vlan_handle, intf_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vlan_delete(device, vlan_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_interface_delete(device, intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_interface_delete(device, intf_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_port_delete(device, port_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_port_delete(device, port_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
