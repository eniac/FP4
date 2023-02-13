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

#include "switchapi/switch_table.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_RMAC

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_table_sizes_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_uint16_t index = 0;

  switch_device_context_t *device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t num_entries = 0;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("device context get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\t\ttable info\n");
  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    if (device_ctx->table_info[index].valid) {
      SWITCH_PRINT(cli_ctx,
                   "\t\t\ttable name: %s\n",
                   device_ctx->table_info[index].table_name);
      SWITCH_PRINT(cli_ctx,
                   "\t\t\ttable size: %d\n",
                   device_ctx->table_info[index].table_size);
      SWITCH_PRINT(cli_ctx,
                   "\t\t\tdirection %d\n",
                   device_ctx->table_info[index].direction);
      num_entries = 0;
      switch_api_table_entry_count_get(device, index, &num_entries);
      SWITCH_PRINT(cli_ctx, "\t\t\tnum entries: %d\n\n", num_entries);
    }
  }

  SWITCH_LOG_DEBUG("table sizes dump on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_table_sizes_dump(const switch_device_t device,
                                            const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_table_sizes_dump_internal(device, cli_ctx))
}
