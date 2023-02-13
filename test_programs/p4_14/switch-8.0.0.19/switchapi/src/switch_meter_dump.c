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

#include "switchapi/switch_meter.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_METER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_meter_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const void *cli_ctx) {
  switch_meter_info_t *meter_info = NULL;
  switch_api_meter_t *api_meter = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "meter dump failed on device %d: "
        "parameters invalid(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "meter dump failed on device %d: "
        "meter get failed(%s)",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  api_meter = &meter_info->api_meter_info;

  SWITCH_PRINT(cli_ctx, "\tmeter handle: 0x%lx\n", meter_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\tmode: %s\n",
               switch_meter_mode_to_string(api_meter->meter_mode));
  SWITCH_PRINT(cli_ctx,
               "\t\tcolor source: %s\n",
               switch_meter_color_source_to_string(api_meter->color_source));
  SWITCH_PRINT(cli_ctx,
               "\t\tmeter mode: %s\n",
               switch_meter_type_to_string(api_meter->meter_type));
  SWITCH_PRINT(cli_ctx,
               "\t\tmeter target type: %s\n",
               switch_meter_target_type_to_string(meter_info->meter_type));
  SWITCH_PRINT(cli_ctx, "\t\tcbs: %ld\n", api_meter->cbs);
  SWITCH_PRINT(cli_ctx, "\t\tpbs: %ld\n", api_meter->pbs);
  SWITCH_PRINT(cli_ctx, "\t\tcir: %ld\n", api_meter->cir);
  SWITCH_PRINT(cli_ctx, "\t\tpir: %ld\n", api_meter->pir);
  SWITCH_PRINT(
      cli_ctx,
      "\t\tgreen action: %s\n",
      switch_packet_action_to_string(api_meter->action[SWITCH_COLOR_GREEN]));
  SWITCH_PRINT(
      cli_ctx,
      "\t\tyellow action: %s\n",
      switch_packet_action_to_string(api_meter->action[SWITCH_COLOR_YELLOW]));
  SWITCH_PRINT(
      cli_ctx,
      "\t\tred action: %s\n",
      switch_packet_action_to_string(api_meter->action[SWITCH_COLOR_RED]));
  SWITCH_PRINT(
      cli_ctx, "\t\tmeter index pd hdl: 0x%lx\n", meter_info->meter_idx_pd_hdl);
  SWITCH_PRINT(cli_ctx,
               "\t\tmeter green action pd hdl: 0x%lx\n",
               meter_info->action_pd_hdl[SWITCH_COLOR_GREEN]);
  SWITCH_PRINT(cli_ctx,
               "\t\tmeter yellow action pd hdl: 0x%lx\n",
               meter_info->action_pd_hdl[SWITCH_COLOR_YELLOW]);
  SWITCH_PRINT(cli_ctx,
               "\t\tmeter red action pd hdl: 0x%lx\n",
               meter_info->action_pd_hdl[SWITCH_COLOR_RED]);

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG("meter handle dump on device %d meter handle 0x%lx\n",
                   device,
                   meter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_meter_dump_all(const switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_meter_handle_dump(const switch_device_t device,
                                             const switch_handle_t meter_handle,
                                             const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_meter_handle_dump_internal(device, meter_handle, cli_ctx))
}
