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
#define __TEST_MODULE__ "INTERFACE"

switch_status_t switch_interface_test(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t port_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle2 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle3 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle4 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle1 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle2 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle3 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle4 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle5 = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_interface_info_t api_intf_info = {0};
  switch_api_rif_info_t api_rif_info = {0};

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

  api_port_info.port = 0x3;
  status = switch_api_port_add(device, &api_port_info, &port_handle3);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  api_port_info.port = 0x4;
  status = switch_api_port_add(device, &api_port_info, &port_handle4);
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

  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.handle = port_handle3;
  api_intf_info.vlan = 10;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_PORT_VLAN;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle3);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  /* rmac for l3 interfaces */
  status = switch_api_router_mac_group_create(
      device, SWITCH_RMAC_TYPE_ALL, &rmac_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  switch_mac_addr_t mac;
  mac.mac_addr[0] = 0x00;
  mac.mac_addr[1] = 0x01;
  mac.mac_addr[2] = 0x00;
  mac.mac_addr[3] = 0x00;
  mac.mac_addr[4] = 0x00;
  mac.mac_addr[5] = 0x01;
  status = switch_api_router_mac_add(device, rmac_handle, &mac);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  /* vrf for l3 interfaces */
  status = switch_api_vrf_create(device, 0x200, &vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  api_rif_info.rmac_handle = rmac_handle;
  api_rif_info.vrf_handle = vrf_handle;
  api_rif_info.rif_type = SWITCH_RIF_TYPE_INTF;

  status = switch_api_rif_create(device, &api_rif_info, &rif_handle);

  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.rif_handle = rif_handle;
  api_intf_info.handle = port_handle4;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_ACCESS;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle4);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_interface_delete(device, intf_handle4);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  api_intf_info.rif_handle = rif_handle;
  api_intf_info.handle = port_handle4;
  api_intf_info.vlan = 10;
  api_intf_info.type = SWITCH_INTERFACE_TYPE_PORT_VLAN;
  status = switch_api_interface_create(device, &api_intf_info, &intf_handle5);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_router_mac_delete(device, rmac_handle, &mac);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_router_mac_group_delete(device, rmac_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_interface_delete(device, intf_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_interface_delete(device, intf_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_interface_delete(device, intf_handle3);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_interface_delete(device, intf_handle5);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_rif_delete(device, rif_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_port_delete(device, port_handle1);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_port_delete(device, port_handle2);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_port_delete(device, port_handle3);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_port_delete(device, port_handle4);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_vrf_delete(device, vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
