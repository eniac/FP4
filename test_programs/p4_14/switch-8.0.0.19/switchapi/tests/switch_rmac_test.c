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
#define __TEST_MODULE__ "RMAC"

switch_status_t switch_rmac_test(switch_device_t device) {
  SWITCH_START_TEST(__TEST_MODULE__);

  switch_int32_t max_macs = 5;
  switch_int32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_mac_addr_t mac[] = {
      {.mac_addr = {0x00, 0x01, 0x00, 0x00, 0x00, 0x01}},
      {.mac_addr = {0x00, 0x01, 0x00, 0x00, 0x00, 0x02}},
      {.mac_addr = {0x00, 0x01, 0x00, 0x00, 0x00, 0x03}},
      {.mac_addr = {0x00, 0x01, 0x00, 0x00, 0x00, 0x04}},
      {.mac_addr = {0x00, 0x01, 0x00, 0x00, 0x00, 0x05}}};

  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_api_router_mac_group_create(
      device, SWITCH_RMAC_TYPE_ALL, &rmac_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  for (index = 0; index < max_macs; index++) {
    status = switch_api_router_mac_add(device, rmac_handle, &mac[index]);
    TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_api_rmac_handle_dump(device, rmac_handle, NULL);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  for (index = 0; index < max_macs; index++) {
    status = switch_api_router_mac_delete(device, rmac_handle, &mac[index]);
    TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_api_router_mac_group_delete(device, rmac_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
