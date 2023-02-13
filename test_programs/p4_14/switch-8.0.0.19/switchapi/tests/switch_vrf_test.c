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
#define __TEST_MODULE__ "VRF"

switch_status_t switch_vrf_test(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_vrf_t vrf_id = 200;
  switch_vrf_t tmp_vrf_id = 0;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tmp_vrf_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_START_TEST(__TEST_MODULE__);

  status = switch_api_vrf_create(device, 200, &vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  printf("vrf created %lx\n", vrf_handle);

  status = switch_api_vrf_id_to_handle_get(device, vrf_id, &tmp_vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  TEST_ASSERT(vrf_handle == tmp_vrf_handle);
  printf("vrf id %d to handle %lx\n", vrf_id, tmp_vrf_handle);

  status = switch_api_vrf_handle_to_id_get(device, vrf_handle, &tmp_vrf_id);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  TEST_ASSERT(vrf_id == tmp_vrf_id);
  printf("vrf handle %lx to id %d\n", vrf_handle, tmp_vrf_id);

  status = switch_api_vrf_delete(device, vrf_handle);
  printf("vrf deleted %lx\n", vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  vrf_id = 0;
  tmp_vrf_id = 0;
  vrf_handle = SWITCH_API_INVALID_HANDLE;
  tmp_vrf_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_api_vrf_create(device, vrf_id, &vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  printf("vrf created %lx\n", vrf_handle);

  status = switch_api_vrf_handle_to_id_get(device, vrf_handle, &tmp_vrf_id);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  TEST_ASSERT(tmp_vrf_id == 0);
  printf("vrf handle %lx to id %d\n", vrf_handle, tmp_vrf_id);

  status = switch_api_vrf_delete(device, vrf_handle);
  printf("vrf deleted %lx\n", vrf_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
