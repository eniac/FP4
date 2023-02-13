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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_BUFFER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_buffer_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_buffer_context_t *buffer_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BUFFER, (void **)&buffer_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "buffer context dump failed on device %d: "
        "buffer context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tBuffer Context:\n");
  SWITCH_PRINT(cli_ctx,
               "\t\t\tingress pool count: %d\n",
               buffer_ctx->ingress_pool_count);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tegress pool count: %d\n", buffer_ctx->egress_pool_count);
  SWITCH_PRINT(cli_ctx, "\t\t\tskid limit: %d\n", buffer_ctx->skid_limit);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tskid hysteresis: %d\n", buffer_ctx->skid_hysteresis);

  SWITCH_PRINT(cli_ctx, "\t\t\tcolor hysteresis: \n");
  for (index = 0; index < SWITCH_COLOR_MAX; index++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\t\t\t%s: %d\n",
                 switch_color_to_string(index),
                 buffer_ctx->color_hysteresis[index]);
  }

  SWITCH_PRINT(cli_ctx, "\n");
  return status;
}

switch_status_t switch_api_buffer_context_dump(const switch_device_t device,
                                               const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_buffer_context_dump_internal(device, cli_ctx));
}

#ifdef __cplusplus
}
#endif
