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
#define __MODULE__ SWITCH_API_TYPE_RPF

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_rpf_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t rpf_group_handle,
    const void *cli_ctx) {
  switch_rpf_info_t *rpf_info = NULL;
  switch_rpf_entry_t *rpf_entry = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_RPF_GROUP_HANDLE(rpf_group_handle));
  if (!SWITCH_RPF_GROUP_HANDLE(rpf_group_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "rpf group dump failed on device %d "
        "rpf group handle %lx: parameters invalid(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rpf_group_get(device, rpf_group_handle, &rpf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "rpf dump failed on device %d "
        "rpf handle %lx: rpf group get failed(%s)\n",
        device,
        rpf_group_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\trpf group handle: %lx\n", rpf_group_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\trpf group: %d\n", rpf_info->rpf_group);
  SWITCH_PRINT(cli_ctx, "\t\tpim mode: %d\n", rpf_info->pim_mode);
  SWITCH_PRINT(cli_ctx,
               "\t\trpf type: %d\n\n",
               switch_rpf_type_to_string(rpf_info->pim_mode));

  SWITCH_PRINT(cli_ctx, "\t\tmembers:\n");
  FOR_EACH_IN_LIST(rpf_info->rpf_list, node) {
    rpf_entry = node->data;
    SWITCH_PRINT(cli_ctx, "\t\t\trif handle: %lx\n", rpf_entry->rif_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\touter pd hdl: %lx\n", rpf_entry->outer_pd_hdl);
    SWITCH_PRINT(cli_ctx, "\t\t\tinner pd hdl: %lx\n", rpf_entry->inner_pd_hdl);
    SWITCH_PRINT(cli_ctx, "\t\t\thw flags: %lx\n", rpf_entry->hw_flags);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_LOG_DEBUG("rpf group handle dump on device %d rpf group handle %lx\n",
                   device,
                   rpf_group_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_rpf_dump_all(const switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_rpf_handle_dump(
    const switch_device_t device,
    const switch_handle_t rpf_group_handle,
    const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_rpf_handle_dump_internal(device, rpf_group_handle, cli_ctx))
}
