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

#include "switchapi/switch_bd.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_BD

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_bd_handle_dump(const switch_device_t device,
                                      const switch_handle_t bd_handle,
                                      const void *cli_ctx) {
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "bd dump failed on device %d "
        "bd handle %lx: parameters invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd dump failed on device %d "
        "bd handle %lx: bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\n");
  SWITCH_PRINT(cli_ctx, "\tbd handle: %lx\n", bd_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(
      cli_ctx, "\t\tbd type: %s\n", switch_bd_type_to_string(bd_info->bd_type));
  SWITCH_PRINT(cli_ctx, "\t\tvrf handle: %lx\n", bd_info->vrf_handle);
  SWITCH_PRINT(cli_ctx, "\t\trmac handle: %lx\n", bd_info->rmac_handle);
  SWITCH_PRINT(cli_ctx, "\t\tstp handle: %lx\n", bd_info->stp_handle);
  SWITCH_PRINT(cli_ctx, "\t\tflood handle: %lx\n", bd_info->flood_handle);
  SWITCH_PRINT(
      cli_ctx, "\t\tmrouters mc handle: %lx\n", bd_info->mrouters_mc_handle);
  SWITCH_PRINT(cli_ctx, "\t\tipv4 unicast: %d\n", bd_info->ipv4_unicast);
  SWITCH_PRINT(cli_ctx, "\t\tipv6 unicast: %d\n", bd_info->ipv6_unicast);
  SWITCH_PRINT(cli_ctx, "\t\tipv4 multicast: %d\n", bd_info->ipv4_multicast);
  SWITCH_PRINT(cli_ctx, "\t\tipv6 multicast: %d\n", bd_info->ipv6_multicast);
  SWITCH_PRINT(cli_ctx, "\t\tigmp snooping: %d\n", bd_info->igmp_snooping);
  SWITCH_PRINT(cli_ctx, "\t\tmld snooping: %d\n", bd_info->mld_snooping);
  SWITCH_PRINT(cli_ctx, "\t\taging interval: %d\n", bd_info->aging_interval);
  SWITCH_PRINT(cli_ctx, "\t\trid: %d\n", bd_info->rid);
  SWITCH_PRINT(cli_ctx, "\t\txid: %d\n", bd_info->xid);
  SWITCH_PRINT(cli_ctx, "\t\tsmac id: %d\n\n", bd_info->smac_index);

  SWITCH_PRINT(cli_ctx, "\t\tbd members: %d\n", bd_info->members.num_entries);
  FOR_EACH_IN_LIST(bd_info->members, node) {
    bd_member = (switch_bd_member_t *)node->data;
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmember handle: 0x%lx\n", bd_member->member_handle);
    SWITCH_PRINT(cli_ctx, "\t\t\trid: %d\n", bd_member->rid);
    SWITCH_PRINT(cli_ctx, "\t\t\thandle: 0x%lx\n", bd_member->handle);
    SWITCH_PRINT(cli_ctx, "\t\t\tpv entry 0x%lx\n", bd_member->pv_tagged_entry);
    SWITCH_PRINT(cli_ctx, "\t\t\txlate entry 0x%lx\n", bd_member->xlate_entry);
  }
  FOR_EACH_IN_LIST_END();

  SWITCH_PRINT(cli_ctx, "\n\tpd handles:\n");
  SWITCH_PRINT(cli_ctx, "\t\tingress bd entry: %lx\n", bd_info->bd_entry);
  SWITCH_PRINT(cli_ctx, "\t\tegress bd entry: %lx\n", bd_info->egress_bd_entry);
  SWITCH_PRINT(cli_ctx,
               "\t\tegress outer bd entry: %lx\n",
               bd_info->egress_outer_bd_entry);
  SWITCH_PRINT(cli_ctx, "\t\tuuc entry: %lx\n", bd_info->uuc_entry);
  SWITCH_PRINT(cli_ctx, "\t\tumc entry: %lx\n", bd_info->uuc_entry);
  SWITCH_PRINT(cli_ctx, "\t\tbcast entry: %lx\n", bd_info->bcast_entry);
  SWITCH_PRINT(cli_ctx, "\t\tmrouters entry: %lx\n", bd_info->mrouters_entry);
  SWITCH_PRINT(cli_ctx, "\t\tcpu entry: %lx\n", bd_info->cpu_entry);
  SWITCH_PRINT(cli_ctx, "\n");

  SWITCH_LOG_DEBUG(
      "bd handle dump on device %d bd handle %lx\n", device, bd_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_bd_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t member_handle,
    const void *cli_ctx) {
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_BD_MEMBER_HANDLE(member_handle));
  if (!SWITCH_BD_HANDLE(member_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "bd member dump failed on device %d "
        "member handle %lx: parameters invalid(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_get(device, member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member dump failed on device %d "
        "bd handle %lx: bd member get failed(%s)\n",
        device,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_PRINT(cli_ctx, "\tbd member handle: %lx\n", member_handle);
  SWITCH_PRINT(cli_ctx, "\t\tdevice: %d\n", device);
  SWITCH_PRINT(cli_ctx, "\t\thandle: %d\n", bd_member->handle);
  SWITCH_PRINT(cli_ctx, "\t\tbd handle: %d\n", bd_member->bd_handle);
  SWITCH_PRINT(cli_ctx, "\t\trid: %d\n", bd_member->rid);
  SWITCH_PRINT(cli_ctx, "\t\touter vlan: %d\n", bd_member->outer_vlan);
  SWITCH_PRINT(cli_ctx, "\t\tinner vlan: %d\n", bd_member->inner_vlan);
  SWITCH_PRINT(cli_ctx, "\t\touter hw vlan: %d\n", bd_member->pv_hw_outer_vlan);
  SWITCH_PRINT(cli_ctx, "\t\tinner hw vlan: %d\n", bd_member->pv_hw_inner_vlan);
  SWITCH_PRINT(cli_ctx,
               "\t\tstp state: %s\n",
               switch_stp_state_to_string(bd_member->stp_state));

  SWITCH_PRINT(cli_ctx, "\n");

  SWITCH_PRINT(cli_ctx, "\n\tpd handles:\n");
  SWITCH_PRINT(
      cli_ctx, "\t\tpv untagged entry: %lx\n", bd_member->pv_untagged_entry);
  SWITCH_PRINT(
      cli_ctx, "\t\tpv tagged entry: %lx\n", bd_member->pv_tagged_entry);
  SWITCH_PRINT(
      cli_ctx, "\t\tpv ifindex entry: %lx\n", bd_member->pv_ifindex_entry);
  SWITCH_PRINT(cli_ctx, "\t\txlate entry: %lx\n", bd_member->xlate_entry);
  SWITCH_PRINT(cli_ctx,
               "\t\ttunnel hw entry: %lx %lx %lx\n",
               bd_member->tunnel_hw_entry[0],
               bd_member->tunnel_hw_entry[1],
               bd_member->tunnel_hw_entry[2]);
  SWITCH_PRINT(cli_ctx,
               "\t\ttunne egress vni entry: %lx\n",
               bd_member->egress_bd_hw_entry);
  SWITCH_PRINT(cli_ctx,
               "\t\tegress outer bd entry: %lx\n",
               bd_member->egress_outer_bd_hw_entry);
  SWITCH_LOG_DEBUG("bd member handle dump on device %d member handle %lx\n",
                   device,
                   member_handle);

  SWITCH_LOG_EXIT();

  return status;
}

#ifdef __cplusplus
}
#endif
