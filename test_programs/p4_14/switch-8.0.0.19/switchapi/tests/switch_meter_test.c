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
#define __TEST_MODULE__ "METER"

switch_status_t switch_meter_test(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_api_meter_t meter_info;
  switch_api_meter_t tmp_meter_info;
  switch_uint64_t flags = 0;
  switch_handle_t meter_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_START_TEST(__TEST_MODULE__);

  SWITCH_MEMSET(&meter_info, 0x0, sizeof(meter_info));

  meter_info.meter_mode = SWITCH_METER_MODE_TWO_RATE_THREE_COLOR;
  meter_info.color_source = SWITCH_METER_COLOR_SOURCE_BLIND;
  meter_info.meter_type = SWITCH_METER_TYPE_PACKETS;
  meter_info.cbs = 1000;
  meter_info.pbs = 2000;
  meter_info.cir = 500;
  meter_info.pir = 600;
  meter_info.action[SWITCH_COLOR_GREEN] = SWITCH_ACL_ACTION_PERMIT;
  meter_info.action[SWITCH_COLOR_YELLOW] = SWITCH_ACL_ACTION_PERMIT;
  meter_info.action[SWITCH_COLOR_YELLOW] = SWITCH_ACL_ACTION_DROP;

  status = switch_api_meter_create(device, &meter_info, &meter_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);
  printf("meter handle %lx\n", meter_handle);

  SWITCH_MEMSET(&tmp_meter_info, 0x0, sizeof(tmp_meter_info));
  status = switch_api_meter_get(device, meter_handle, &tmp_meter_info);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  printf(
      "meter created on device %d meter handle %lx "
      "mode %d type %d source %d "
      "cbs %" PRId64 " pbs %" PRId64 " cir %" PRId64 " pir %" PRId64
      " "
      "action green(%d) yellow(%d) red(%d)\n",
      device,
      meter_handle,
      tmp_meter_info.meter_mode,
      tmp_meter_info.meter_type,
      tmp_meter_info.color_source,
      tmp_meter_info.cbs,
      tmp_meter_info.pbs,
      tmp_meter_info.cir,
      tmp_meter_info.pir,
      tmp_meter_info.action[SWITCH_COLOR_GREEN],
      tmp_meter_info.action[SWITCH_COLOR_YELLOW],
      tmp_meter_info.action[SWITCH_COLOR_RED]);

  TEST_ASSERT(meter_info.cbs == tmp_meter_info.cbs);
  TEST_ASSERT(meter_info.pbs == tmp_meter_info.pbs);
  TEST_ASSERT(meter_info.cir == tmp_meter_info.cir);
  TEST_ASSERT(meter_info.pir == tmp_meter_info.pir);

  status = switch_api_meter_handle_dump(device, meter_handle, NULL);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&meter_info, 0x0, sizeof(meter_info));
  flags |= SWITCH_METER_ATTR_CBS;
  meter_info.cbs = 2000;
  flags |= SWITCH_METER_ATTR_PBS;
  meter_info.pbs = 3000;
  flags |= SWITCH_METER_ATTR_CIR;
  meter_info.cir = 700;
  flags |= SWITCH_METER_ATTR_PIR;
  meter_info.pir = 800;
  status = switch_api_meter_update(device, meter_handle, flags, &meter_info);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_meter_handle_dump(device, meter_handle, NULL);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&tmp_meter_info, 0x0, sizeof(tmp_meter_info));
  status = switch_api_meter_get(device, meter_handle, &tmp_meter_info);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  printf(
      "meter updated on device %d meter handle %lx "
      "mode %d type %d source %d "
      "cbs %" PRId64 " pbs %" PRId64 " cir %" PRId64 " pir %" PRId64
      " "
      "action green(%d) yellow(%d) red(%d)\n",
      device,
      meter_handle,
      tmp_meter_info.meter_mode,
      tmp_meter_info.meter_type,
      tmp_meter_info.color_source,
      tmp_meter_info.cbs,
      tmp_meter_info.pbs,
      tmp_meter_info.cir,
      tmp_meter_info.pir,
      tmp_meter_info.action[SWITCH_COLOR_GREEN],
      tmp_meter_info.action[SWITCH_COLOR_YELLOW],
      tmp_meter_info.action[SWITCH_COLOR_RED]);

  TEST_ASSERT(meter_info.cbs == tmp_meter_info.cbs);
  TEST_ASSERT(meter_info.pbs == tmp_meter_info.pbs);
  TEST_ASSERT(meter_info.cir == tmp_meter_info.cir);
  TEST_ASSERT(meter_info.pir == tmp_meter_info.pir);

  status = switch_api_meter_delete(device, meter_handle);
  TEST_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(&tmp_meter_info, 0x0, sizeof(tmp_meter_info));
  status = switch_api_meter_get(device, meter_handle, &tmp_meter_info);
  TEST_ASSERT(status != SWITCH_STATUS_SUCCESS);

  SWITCH_END_TEST(__TEST_MODULE__);

  return status;
}

#ifdef __cplusplus
}
#endif
