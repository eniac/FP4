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

#include "switchapi/switch_mirror.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_MIRROR

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_mirror_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const void *cli_ctx) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_api_mirror_info_t *api_mirror_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror dump failed on device %d "
        "mirror handle 0x%lx: mirror get failed(%s)\n",
        device,
        mirror_handle,
        switch_error_to_string(status));
    return status;
  }

  api_mirror_info = &mirror_info->api_mirror_info;
  SWITCH_PRINT(cli_ctx, "\n\tmirror handle: 0x%lx\n", mirror_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx,
               "\t\t\tmirror type: %s\n",
               switch_mirror_type_to_string(api_mirror_info->mirror_type));
  SWITCH_PRINT(
      cli_ctx,
      "\t\t\tsession type: %s\n",
      switch_mirror_session_type_to_string(api_mirror_info->session_type));
  SWITCH_PRINT(cli_ctx, "\t\t\tsession id : %d\n", api_mirror_info->session_id);
  SWITCH_PRINT(cli_ctx, "\t\t\tdirection: %d\n", api_mirror_info->direction);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tnhop handle:0x%lx\n", api_mirror_info->nhop_handle);
  SWITCH_PRINT(cli_ctx, "\t\t\tvlan id: %d\n", api_mirror_info->vlan_id);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tvlan handle: %d\n", api_mirror_info->vlan_handle);

  SWITCH_LOG_DEBUG("mirror handle dump on device %d mirror handle 0x%lx\n",
                   device,
                   mirror_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_mirror_handle_dump(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_mirror_handle_dump_internal(device, mirror_handle, cli_ctx))
}
