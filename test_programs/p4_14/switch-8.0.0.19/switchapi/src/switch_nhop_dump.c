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

#include "switchapi/switch_nhop.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_NHOP

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_api_nhop_handle_dump_internal(
    const switch_device_t device,
    const switch_handle_t nhop_handle,
    const void *cli_ctx) {
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_node_t *node = NULL;
  switch_spath_info_t *spath_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
  if (!SWITCH_NHOP_HANDLE(nhop_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "nhop dump failed on device %d "
        "nhop handle %lx: parameters invalid(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_nhop_get(device, nhop_handle, &nhop_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop dump failed on device %d "
        "nhop handle %lx: nhop get failed(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "nhop handle dump on device %d nhop handle %lx\n", device, nhop_handle);

  if (SWITCH_NHOP_ID_TYPE_ECMP(nhop_info)) {
    mpath_info = &SWITCH_ECMP_MPATH_INFO(nhop_info);
    SWITCH_PRINT(cli_ctx, "\n");
    SWITCH_PRINT(cli_ctx, "\t\tecmp handle: 0x%lx\n", nhop_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\thw flags: 0x%lx\n", mpath_info->hw_flags);
    SWITCH_PRINT(
        cli_ctx, "\t\t\tgroup entry: 0x%lx\n", mpath_info->pd_group_hdl);
    SWITCH_PRINT(cli_ctx, "\t\t\tmbr entry: 0x%lx\n", mpath_info->mbr_hdl);

    SWITCH_PRINT(
        cli_ctx, "\t\t\tmembers: %d\n", mpath_info->members.num_entries);
    FOR_EACH_IN_LIST(mpath_info->members, node) {
      ecmp_member = (switch_ecmp_member_t *)node->data;
      SWITCH_PRINT(
          cli_ctx, "\t\t\t\tnhop handle: 0x%lx\n", ecmp_member->nhop_handle);
      SWITCH_PRINT(cli_ctx,
                   "\t\t\t\tmember handle: 0x%lx\n",
                   ecmp_member->member_handle);
      SWITCH_PRINT(cli_ctx, "\t\t\t\thw flags: 0x%lx\n", ecmp_member->hw_flags);
      SWITCH_PRINT(cli_ctx, "\t\t\t\tmbr entry: 0x%lx\n", ecmp_member->mbr_hdl);
      SWITCH_PRINT(
          cli_ctx, "\t\t\t\turpf entry: 0x%lx\n", ecmp_member->urpf_pd_hdl);
      SWITCH_PRINT(cli_ctx,
                   "\t\t\t\tactive: %s\n",
                   ecmp_member->active ? "active" : "inactive");
    }
    FOR_EACH_IN_LIST_END();
    SWITCH_PRINT(cli_ctx, "\n");
  } else {
    spath_info = &SWITCH_NHOP_SPATH_INFO(nhop_info);
    api_nhop_info = &spath_info->api_nhop_info;
    SWITCH_PRINT(cli_ctx, "\n");
    SWITCH_PRINT(cli_ctx, "\t\tnhop handle: 0x%lx\n", nhop_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tnhop type: %s\n",
                 switch_nhop_type_to_string(api_nhop_info->nhop_type));
    SWITCH_PRINT(
        cli_ctx,
        "\t\t\trewrite type: %s\n",
        switch_nhop_rewrite_type_to_string(api_nhop_info->rewrite_type));
    SWITCH_PRINT(
        cli_ctx,
        "\t\t\ttunnel type: %s\n",
        switch_nhop_tunnel_type_to_string(api_nhop_info->nhop_tunnel_type));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tvrf handle: 0x%lx\n", api_nhop_info->vrf_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tnetwork handle: 0x%lx\n",
                 api_nhop_info->network_handle);
    SWITCH_PRINT(
        cli_ctx, "\t\t\trif handle: 0x%lx\n", api_nhop_info->rif_handle);
    SWITCH_PRINT(
        cli_ctx, "\t\t\ttunnel handle: 0x%lx\n", api_nhop_info->tunnel_handle);
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmpls handle: 0x%lx\n", api_nhop_info->mpls_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tlabel stack handle: 0x%lx\n",
                 api_nhop_info->label_stack_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\ttunnel vni: %d\n", api_nhop_info->tunnel_vni);
    SWITCH_PRINT(
        cli_ctx, "\t\t\ttunnel dst index: %d\n", spath_info->tunnel_dst_index);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tip address %s\n",
                 switch_ipaddress_to_string(&api_nhop_info->ip_addr));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tneighbor handle: 0x%lx\n", spath_info->neighbor_handle);
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tmgid handle: 0x%lx\n",
                 nhop_info->tunnel_info.mgid_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\tifindex 0x%x\n", spath_info->ifindex);
    SWITCH_PRINT(cli_ctx, "\t\t\tbd handle: 0x%x\n", spath_info->bd_handle);
    SWITCH_PRINT(
        cli_ctx, "\t\t\tecmp reference: %d\n", nhop_info->ecmp_ref_count);

    SWITCH_PRINT(cli_ctx, "\t\t\thw flags: 0x%lx\n", spath_info->hw_flags);
    SWITCH_PRINT(cli_ctx, "\t\t\thw entry: 0x%lx\n", spath_info->hw_entry);
    SWITCH_PRINT(cli_ctx, "\t\t\turpf entry: 0x%lx\n", spath_info->urpf_pd_hdl);
    SWITCH_PRINT(cli_ctx, "\n");
  }

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_nhop_context_dump_internal(
    const switch_device_t device, const void *cli_ctx) {
  switch_nhop_context_t *nhop_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_NHOP, (void **)&nhop_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "nhop context dump failed on device %d: "
        "nhop context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\t\tNexthop Context:\n");
  SWITCH_CLI_HASHTABLE_PRINT(cli_ctx, nhop_ctx->nhop_hashtable, "Nexthop");

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_nhop_handle_dump(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            const void *cli_ctx) {
  SWITCH_MT_WRAP(
      switch_api_nhop_handle_dump_internal(device, nhop_handle, cli_ctx))
}

switch_status_t switch_api_nhop_context_dump(const switch_device_t device,
                                             const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_nhop_context_dump_internal(device, cli_ctx));
}
