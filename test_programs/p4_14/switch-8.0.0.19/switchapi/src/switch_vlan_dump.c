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

#include "switchapi/switch_vlan.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_VLAN

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_vlan_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t vlan_handle,
    const void *cli_ctx) {
  switch_vlan_info_t *vlan_info = NULL;
  bool detail = TRUE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "vlan dump failed on device %d "
        "vlan handle %lx: parameters invalid(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_vlan_get(device, vlan_handle, &vlan_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan dump failed on device %d "
        "vlan handle %lx: vlan get failed(%s)\n",
        device,
        vlan_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\tvlan handle: %lx\n", vlan_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\tvlan id: %d\n", vlan_info->vlan_id);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle: %lx\n", vlan_info->bd_handle);

  if (detail) {
    status = switch_bd_handle_dump(device, vlan_info->bd_handle, cli_ctx);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "vlan dump failed on device %d "
          "vlan handle %lx: bd dump failed(%s)\n",
          device,
          vlan_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  SWITCH_PRINT(cli_ctx, "\n\n");

  SWITCH_LOG_DEBUG(
      "vlan handle dump on device %d vlan handle %lx\n", device, vlan_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_vlan_id_dump_internal(const switch_device_t device,
                                                 const switch_vlan_t vlan_id,
                                                 const void *cli_ctx) {
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (vlan_id > SWITCH_MAX_VLANS) {
    status = SWITCH_STATUS_INVALID_VLAN_ID;
    SWITCH_LOG_ERROR(
        "vlan id dump failed on device %d "
        "vlan id %d: vlan id invalid(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_id_to_handle_get(device, vlan_id, &vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan id dump failed on device %d "
        "vlan id %d: vlan id to handle get failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));
  if (!SWITCH_VLAN_HANDLE(vlan_handle)) {
    SWITCH_LOG_ERROR(
        "vlan id dump failed on device %d "
        "vlan id %d: vlan handle invalid(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_vlan_handle_dump_internal(device, vlan_handle, cli_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "vlan id dump failed on device %d "
        "vlan id %d: vlan handle dump failed(%s)\n",
        device,
        vlan_id,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("vlan id dump on device %d vlan id %d\n", device, vlan_id);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_vlan_handle_dump(const switch_device_t device,
                                            const switch_handle_t vlan_handle,
                                            const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_vlan_handle_dump_internal(device, vlan_handle, cli_ctx))
}

switch_status_t switch_api_vlan_id_dump(const switch_device_t device,
                                        const switch_vlan_t vlan_id,
                                        const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_vlan_id_dump_internal(device, vlan_id, cli_ctx))
}
