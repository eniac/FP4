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

#include "switchapi/switch_queue.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_QUEUE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_queue_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t queue_handle,
    const void *cli_ctx) {
  switch_queue_info_t *queue_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
  if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "queue dump failed on device %d "
        "queue handle %lx: parameters invalid(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_queue_get(device, queue_handle, &queue_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "queue dump failed on device %d "
        "queue handle %lx: queue get failed(%s)\n",
        device,
        queue_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n\tqueue handle: 0x%lx\n", queue_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\tport handle: 0x%lx\n", queue_info->port_handle);
  SWITCH_PRINT(cli_ctx,
               "\t\tbuffer profile handle: 0x%lx\n",
               queue_info->buffer_profile_handle);
  SWITCH_PRINT(cli_ctx, "\t\tqueue id: %d\n", queue_info->queue_id);

  SWITCH_LOG_DEBUG("queue handle dump on device %d queue handle 0x%lx\n",
                   device,
                   queue_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_queue_handle_dump(const switch_device_t device,
                                             const switch_handle_t queue_handle,
                                             const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_queue_handle_dump_internal(device, queue_handle, cli_ctx))
}
