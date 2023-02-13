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

#include "switchapi/switch_ln.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_LOGICAL_NETWORK

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_ln_handle_dump(const switch_device_t device,
                                          const switch_handle_t ln_handle,
                                          const void *cli_ctx) {
  switch_ln_info_t *ln_info = NULL;
  bool detail = TRUE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_LN_HANDLE(ln_handle));
  if (!SWITCH_LN_HANDLE(ln_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ln dump failed on device %d "
        "ln handle 0x%lx: parameters invalid(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_ln_get(device, ln_handle, &ln_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ln dump failed on device %d "
        "ln handle 0x%lx: ln get failed(%s)\n",
        device,
        ln_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\tln handle: 0x%lx\n", ln_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle: 0x%lx\n", ln_info->bd_handle);
  if (SWITCH_INTERFACE_HANDLE(ln_info->l3_intf_handle)) {
    SWITCH_PRINT(cli_ctx, "\t\tintf handle: 0x%lx\n", ln_info->l3_intf_handle);
  }

  if (detail) {
    status = switch_bd_handle_dump(device, ln_info->bd_handle, cli_ctx);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "ln dump failed on device %d "
          "ln handle 0x%lx: bd dump failed(%s)\n",
          device,
          ln_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "ln handle dump on device %d ln handle 0x%lx\n", device, ln_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_ln_dump_all(const switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  return status;
}

#ifdef __cplusplus
}
#endif
