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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_BD

switch_status_t switch_bd_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_pd_egress_filter_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd default entries add failed on device %d: "
        "egress filter default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_bd_stats_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd default entries add failed on device %d: "
        "bd stats default add failedi(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_bd_flood_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd default entries add failed on device %d: "
        "bd flood default add failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL("bd default entries added on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_bd_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_LOG_DETAIL("bd default entries deleted on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_bd_init(switch_device_t device) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_size_t bd_table_size = 0;
  switch_size_t pv_table_size = 0;
  switch_size_t bd_stats_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  bd_ctx = SWITCH_MALLOC(device, sizeof(switch_bd_context_t), 0x1);
  if (!bd_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_BD, (void *)bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "bd context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(device, SWITCH_TABLE_BD, &bd_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "bd table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_BD, bd_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "bd handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (int i = 1; i <= SWITCH_MAX_VLANS; i++) {
    bd_ctx->vlan_bd_handle[i] = switch_bd_handle_create(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd init failed on device %d: "
          "bd reserve failed(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING, &pv_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "pv mapping table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_BD_MEMBER, pv_table_size);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "bd member handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_new(
      device, SWITCH_XID_SIZE, FALSE, &(bd_ctx->xid_allocator));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "xid allocator init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_table_size_get(
      device, SWITCH_TABLE_INGRESS_BD_STATS, &bd_stats_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "bd stats table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_new(
      device, bd_stats_table_size, FALSE, &(bd_ctx->stats_id_allocator));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd init failed on device %d: "
        "stats id allocator init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG("bd init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  status = switch_bd_free(device);
  SWITCH_ASSERT(status != SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_bd_free(switch_device_t device) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BD, (void **)&bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "bd context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (int i = 1; i <= SWITCH_MAX_VLANS; i++) {
    status = switch_bd_handle_delete(device, bd_ctx->vlan_bd_handle[i]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd free failed on device %d "
          "bd handle %lx: bd handle delete failed(%s)\n",
          device,
          bd_ctx->vlan_bd_handle[i],
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_BD);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "bd handle type free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_BD_MEMBER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "bd member handle type free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, bd_ctx->xid_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "xid allocator free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, bd_ctx->stats_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "stats id allocator free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_LOG_DEBUG("bd free successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_bd_attribute_set(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_uint64_t bd_flags,
                                        switch_bd_info_t *bd_info) {
  switch_bd_info_t *bd_info_tmp = NULL;
  switch_handle_t *tmp_mac_handle = NULL;
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info_tmp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd attribute set failed on device %d "
        "bd handle %lx: bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (bd_flags & SWITCH_BD_ATTR_TYPE) {
    bd_info_tmp->bd_type = bd_info->bd_type;
  }

  if (bd_flags & SWITCH_BD_ATTR_VRF_HANDLE) {
    bd_info_tmp->vrf_handle = bd_info->vrf_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_RMAC_HANDLE) {
    if (SWITCH_RMAC_HANDLE(bd_info->rmac_handle)) {
      bd_info_tmp->rmac_handle = bd_info->rmac_handle;
      status =
          switch_smac_rewrite_index_by_rmac_handle_get(device,
                                                       bd_info->rmac_handle,
                                                       SWITCH_SMAC_TYPE_REWRITE,
                                                       &bd_info->smac_index);
      if (status == SWITCH_STATUS_SUCCESS) {
        bd_info_tmp->smac_index = bd_info->smac_index;
      }

      status = switch_smac_rewrite_index_by_rmac_handle_get(
          device,
          bd_info->rmac_handle,
          SWITCH_SMAC_TYPE_TUNNEL_REWRITE,
          &bd_info->tunnel_smac_index);
      if (status == SWITCH_STATUS_SUCCESS) {
        bd_info_tmp->tunnel_smac_index = bd_info->tunnel_smac_index;
      }
    }
  }

  if (bd_flags & SWITCH_BD_ATTR_STP_HANDLE) {
    bd_info_tmp->stp_handle = bd_info->stp_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_LEARNING) {
    bd_info_tmp->learning = bd_info->learning;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV4_UNICAST) {
    bd_info_tmp->ipv4_unicast = bd_info->ipv4_unicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV6_UNICAST) {
    bd_info_tmp->ipv6_unicast = bd_info->ipv6_unicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV4_MULTICAST) {
    bd_info_tmp->ipv4_multicast = bd_info->ipv4_multicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV6_MULTICAST) {
    bd_info_tmp->ipv6_multicast = bd_info->ipv6_multicast;
  }

  if ((bd_flags & SWITCH_BD_ATTR_UUC_FLOODING_ENABLED) ||
      (bd_flags & SWITCH_BD_ATTR_UMC_FLOODING_ENABLED) ||
      (bd_flags & SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED)) {
    bd_info_tmp->flood_handle = bd_info->flood_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED) {
    bd_info_tmp->mrouters_mc_handle = bd_info->mrouters_mc_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_AGING_INTERVAL) {
    if (bd_info_tmp->aging_interval != bd_info->aging_interval) {
      FOR_EACH_IN_ARRAY(
          mac_handle, bd_info_tmp->mac_array, switch_handle_t, tmp_mac_handle) {
        UNUSED(tmp_mac_handle);
        status = switch_mac_entry_aging_hw_update(
            device, mac_handle, bd_info->aging_interval);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "bd mac aging time set failed on device %d "
              "mac handle 0x%lx\n: hw update failed:(%s)\n",
              device,
              mac_handle,
              switch_error_to_string(status));
        }
      }
      FOR_EACH_IN_ARRAY_END();
    }
    bd_info_tmp->aging_interval = bd_info->aging_interval;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV4_URPF_MODE) {
    bd_info_tmp->ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV6_URPF_MODE) {
    bd_info_tmp->ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
  }

  if (bd_flags & SWITCH_BD_ATTR_IGMP_SNOOPING) {
    bd_info_tmp->igmp_snooping = bd_info->igmp_snooping;
  }

  if (bd_flags & SWITCH_BD_ATTR_MLD_SNOOPING) {
    bd_info_tmp->mld_snooping = bd_info->mld_snooping;
  }

  if (bd_flags & SWITCH_BD_ATTR_MRPF_GROUP) {
    bd_info_tmp->mrpf_group = bd_info->mrpf_group;
  }

  if (bd_flags & SWITCH_BD_ATTR_MTU_HANDLE) {
    bd_info_tmp->mtu_handle = bd_info->mtu_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_NAT_MODE) {
    bd_info_tmp->nat_mode = bd_info->nat_mode;
  }

  if (bd_flags & SWITCH_BD_ATTR_INGRESS_LABEL) {
    bd_info_tmp->ingress_bd_label = bd_info->ingress_bd_label;
  }

  if (bd_flags & SWITCH_BD_ATTR_EGRESS_LABEL) {
    bd_info_tmp->egress_bd_label = bd_info->egress_bd_label;
  }

  bd_info_tmp->bd_flags = bd_flags;

  if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp, SWITCH_BD_INGRESS_PD_ENTRY))) {
    status = switch_pd_bd_table_entry_add(
        device, handle_to_id(bd_handle), bd_info_tmp, &bd_info_tmp->bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: bd table add failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_INGRESS_PD_ENTRY);
  } else {
    status = switch_pd_bd_table_entry_update(
        device, handle_to_id(bd_handle), bd_info_tmp, bd_info_tmp->bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: bd table update failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp, SWITCH_BD_EGRESS_PD_ENTRY))) {
    status = switch_pd_egress_bd_table_entry_add(device,
                                                 handle_to_id(bd_handle),
                                                 bd_info_tmp,
                                                 &bd_info_tmp->egress_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: egress bd table add failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_EGRESS_PD_ENTRY);
  } else {
    status =
        switch_pd_egress_bd_table_entry_update(device,
                                               handle_to_id(bd_handle),
                                               bd_info_tmp,
                                               bd_info_tmp->egress_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: "
          "egress bd table update failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp, SWITCH_BD_EGRESS_OUTER_PD_ENTRY))) {
    status = switch_pd_egress_outer_bd_table_entry_add(
        device,
        handle_to_id(bd_handle),
        bd_info_tmp,
        &bd_info_tmp->egress_outer_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: egress outer_bd table add failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_EGRESS_OUTER_PD_ENTRY);
  } else {
    status = switch_pd_egress_outer_bd_table_entry_update(
        device,
        handle_to_id(bd_handle),
        bd_info_tmp,
        bd_info_tmp->egress_outer_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd attribute set failed on device %d "
          "bd handle %lx: "
          "egress outer_bd table update failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  if (bd_info_tmp->bd_flags & SWITCH_BD_ATTR_UUC_FLOODING_ENABLED) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp, SWITCH_BD_UUC_FLOODING_PD_ENTRY))) {
      status = switch_pd_bd_flood_table_entry_add(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UUC,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          &bd_info_tmp->uuc_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_UUC_FLOODING_PD_ENTRY);
    } else {
      status = switch_pd_bd_flood_table_entry_update(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UUC,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          bd_info_tmp->uuc_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  if (bd_info_tmp->bd_flags & SWITCH_BD_ATTR_UMC_FLOODING_ENABLED) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp, SWITCH_BD_UMC_FLOODING_PD_ENTRY))) {
      status = switch_pd_bd_flood_table_entry_add(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UMC,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          &bd_info_tmp->umc_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_UMC_FLOODING_PD_ENTRY);
    } else {
      status = switch_pd_bd_flood_table_entry_update(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UMC,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          bd_info_tmp->umc_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  if (bd_info_tmp->bd_flags & SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp,
                               SWITCH_BD_BCAST_FLOODING_PD_ENTRY))) {
      status = switch_pd_bd_flood_table_entry_add(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_BCAST,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          &bd_info_tmp->bcast_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_BCAST_FLOODING_PD_ENTRY);
    } else {
      status = switch_pd_bd_flood_table_entry_update(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_BCAST,
          FALSE,
          handle_to_id(bd_info->flood_handle),
          bd_info_tmp->bcast_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  if (bd_info_tmp->bd_flags & SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_info_tmp,
                               SWITCH_BD_MROUTERS_FLOODING_PD_ENTRY))) {
      status = switch_pd_bd_flood_table_entry_add(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UMC,
          TRUE,
          handle_to_id(bd_info->mrouters_mc_handle),
          &bd_info_tmp->mrouters_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
      SWITCH_HW_FLAG_SET(bd_info_tmp, SWITCH_BD_MROUTERS_FLOODING_PD_ENTRY);
    } else {
      status = switch_pd_bd_flood_table_entry_update(
          device,
          handle_to_id(bd_handle),
          SWITCH_BD_FLOOD_UMC,
          TRUE,
          handle_to_id(bd_info->mrouters_mc_handle),
          bd_info_tmp->mrouters_entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd attribute set failed on device %d "
            "bd handle %lx: "
            "bd flooding table add failed(%s)\n",
            device,
            bd_handle,
            switch_error_to_string(status));
        goto cleanup;
      }
    }
  }

  SWITCH_LOG_DEBUG("bd attribute set on device %d bd handle %lx bd flags %lx\n",
                   device,
                   bd_handle,
                   bd_flags);

  return status;

cleanup:
  return status;
}

switch_status_t switch_bd_attribute_get(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_uint64_t bd_flags,
                                        switch_bd_info_t *bd_info) {
  switch_bd_info_t *bd_info_tmp = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  status = switch_bd_get(device, bd_handle, &bd_info_tmp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd attribute get failed on device %d "
        "bd handle %lx: bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (bd_flags & SWITCH_BD_ATTR_VRF_HANDLE) {
    bd_info->vrf_handle = bd_info_tmp->vrf_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_RMAC_HANDLE) {
    bd_info->rmac_handle = bd_info_tmp->rmac_handle;
    bd_info->smac_index = bd_info_tmp->smac_index;
  }

  if (bd_flags & SWITCH_BD_ATTR_STP_HANDLE) {
    bd_info->stp_handle = bd_info_tmp->stp_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_LEARNING) {
    bd_info->learning = bd_info_tmp->learning;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV4_UNICAST) {
    bd_info->ipv4_unicast = bd_info_tmp->ipv4_unicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV6_UNICAST) {
    bd_info->ipv6_unicast = bd_info_tmp->ipv6_unicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV4_MULTICAST) {
    bd_info->ipv4_multicast = bd_info_tmp->ipv4_multicast;
  }

  if (bd_flags & SWITCH_BD_ATTR_IPV6_MULTICAST) {
    bd_info->ipv6_multicast = bd_info_tmp->ipv6_multicast;
  }

  if ((bd_flags & SWITCH_BD_ATTR_UUC_FLOODING_ENABLED) ||
      (bd_flags & SWITCH_BD_ATTR_UMC_FLOODING_ENABLED) ||
      (bd_flags & SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED)) {
    bd_info->flood_handle = bd_info_tmp->flood_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED) {
    bd_info->mrouters_mc_handle = bd_info_tmp->mrouters_mc_handle;
  }

  if (bd_flags & SWITCH_BD_ATTR_AGING_INTERVAL) {
    bd_info->aging_interval = bd_info_tmp->aging_interval;
  }

  SWITCH_LOG_DEBUG("bd attribute get on device %d bd handle %lx bd flags %lx\n",
                   device,
                   bd_handle,
                   bd_flags);

  return status;
}

switch_status_t switch_bd_create(switch_device_t device,
                                 switch_uint64_t bd_flags,
                                 switch_bd_info_t *bd_info,
                                 switch_handle_t *bd_handle) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_bd_info_t *bd_info_tmp = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(bd_info != NULL);
  SWITCH_ASSERT(bd_handle != NULL);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BD, (void **)&bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "bd context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /* vlan bd_handle id equal to VLAN ID */
  if (bd_info->bd_type == SWITCH_BD_TYPE_VLAN) {
    *bd_handle = bd_ctx->vlan_bd_handle[bd_info->vlan];
  } else {
    *bd_handle = switch_bd_handle_create(device);
  }
  if (*bd_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "bd handle create failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, *bd_handle, &bd_info_tmp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "bd get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // init aging to invalid
  bd_info_tmp->aging_interval = SWITCH_AGING_INTERVAL_INVALID;

  status = switch_api_id_allocator_allocate(
      device, bd_ctx->xid_allocator, &bd_info_tmp->xid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "xid id allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_mcast_rid_allocate(device, &bd_info_tmp->rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "rid id allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  bd_info_tmp->mrpf_group = handle_to_id(*bd_handle);

  status = switch_bd_attribute_set(device, *bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd create failed on device %d: "
        "bd attribute set failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  bd_info_tmp->handle = bd_info->handle;

  status = SWITCH_LIST_INIT(&bd_info->members);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("bd created on device %d bd handle %lx bd flags\n",
                   device,
                   *bd_handle,
                   bd_flags);

  return status;
cleanup:
  return status;
}

switch_status_t switch_bd_update(switch_device_t device,
                                 switch_handle_t bd_handle,
                                 switch_uint64_t bd_flags,
                                 switch_bd_info_t *bd_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_bd_attribute_set(device, bd_handle, bd_flags, bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd update failed on device %d: "
        "bd attribute set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("bd updated on device %d bd handle %lx bd flags\n",
                   device,
                   bd_handle,
                   bd_flags);

  return status;
}

switch_status_t switch_bd_delete(switch_device_t device,
                                 switch_handle_t bd_handle) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool handle_delete = true;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BD, (void **)&bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd delete failed on device %d "
        "bd handle %lx: bd context get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "bd delete failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd delete failed on device %d "
        "bd handle %lx: bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (bd_info->bd_type == SWITCH_BD_TYPE_VLAN) handle_delete = false;

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_UUC_FLOODING_PD_ENTRY)) {
    status = switch_pd_bd_flood_table_entry_delete(device, bd_info->uuc_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "bd flood table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_UUC_FLOODING_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_UMC_FLOODING_PD_ENTRY)) {
    status = switch_pd_bd_flood_table_entry_delete(device, bd_info->umc_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "bd flood table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_UMC_FLOODING_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_BCAST_FLOODING_PD_ENTRY)) {
    status =
        switch_pd_bd_flood_table_entry_delete(device, bd_info->bcast_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "bd flood table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_BCAST_FLOODING_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_MROUTERS_FLOODING_PD_ENTRY)) {
    status =
        switch_pd_bd_flood_table_entry_delete(device, bd_info->mrouters_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "bd flood table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_MROUTERS_FLOODING_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_INGRESS_PD_ENTRY)) {
    status = switch_pd_bd_table_entry_delete(device, bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: bd table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_INGRESS_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_EGRESS_PD_ENTRY)) {
    status = switch_pd_egress_bd_table_entry_delete(device,
                                                    bd_info->egress_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "egress bd table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_EGRESS_PD_ENTRY);
  }

  if (SWITCH_HW_FLAG_ISSET(bd_info, SWITCH_BD_EGRESS_OUTER_PD_ENTRY)) {
    status = switch_pd_egress_outer_bd_table_entry_delete(
        device, bd_info->egress_outer_bd_entry);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: "
          "egress outer bd table delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
    SWITCH_HW_FLAG_CLEAR(bd_info, SWITCH_BD_EGRESS_OUTER_PD_ENTRY);
  }

  status = switch_mcast_rid_release(device, bd_info->rid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd delete failed on device %d "
        "bd handle %lx: rid release failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_release(
      device, bd_ctx->xid_allocator, bd_info->xid);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd delete failed on device %d "
        "bd handle %lx: xid release failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  if (handle_delete) {
    status = switch_bd_handle_delete(device, bd_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd delete failed on device %d "
          "bd handle %lx: bd handle delete failed(%s)\n",
          device,
          bd_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
    memset(bd_info, 0, sizeof(switch_bd_info_t));
  }

  SWITCH_LOG_DEBUG(
      "bd handle deleted on device %d bd handle %lx\n", device, bd_handle);

  return status;
}

switch_status_t switch_bd_member_add(switch_device_t device,
                                     switch_handle_t bd_handle,
                                     switch_handle_t *member_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "bd member add failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member add failed on device %d "
        "bd handle %lx: bd get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *member_handle = switch_bd_member_handle_create(device);
  if (*member_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "bd member add failed on device %d "
        "bd handle %lx: "
        "bd member handle create failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_get(device, *member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member add failed on device %d "
        "bd handle %lx: "
        "bd member get failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_BD_MEMBER_INIT(device, bd_member);

  status =
      SWITCH_LIST_INSERT(&(bd_info->members), &(bd_member->node), bd_member);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member add failed on device %d "
        "bd handle %lx: "
        "bd member list insert failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_DEBUG(
      "bd member created on device %d "
      "bd handle %lx member handle %lx\n",
      device,
      bd_handle,
      *member_handle);

  return status;

cleanup:
  return status;
}

switch_status_t switch_bd_member_delete(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_handle_t member_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_bd_member_t *bd_member = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "bd member delete failed on device %d "
        "bd handle %lx member handle %lx: "
        "bd handle invalid(%s)\n",
        device,
        bd_handle,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member delete failed on device %d "
        "bd handle %lx member handle %lx: "
        "bd get failed(%s)\n",
        device,
        bd_handle,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_BD_MEMBER_HANDLE(member_handle));
  if (!SWITCH_BD_MEMBER_HANDLE(member_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "bd member delete failed on device %d "
        "bd handle %lx member handle %lx: "
        "bd member handle invalid(%s)\n",
        device,
        bd_handle,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_get(device, member_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member delete failed on device %d "
        "bd handle %lx member handle %lx: "
        "bd member get failed(%s)\n",
        device,
        bd_handle,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&(bd_info->members), &(bd_member->node));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member delete failed on device %d "
        "bd handle %lx member handle %lx: "
        "bd member list delete failed(%s)\n",
        device,
        bd_handle,
        member_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_handle_delete(device, member_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "bd member deleted on device %d "
      "bd handle %lx member handle %lx\n",
      device,
      bd_handle,
      member_handle);

  return status;
}

switch_status_t switch_bd_member_find(switch_device_t device,
                                      switch_handle_t bd_handle,
                                      switch_handle_t intf_handle,
                                      switch_bd_member_t **member) {
  switch_bd_member_t *bd_member = NULL;
  switch_node_t *node = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  SWITCH_ASSERT(member != NULL);
  if (!SWITCH_BD_HANDLE(bd_handle) || !SWITCH_INTERFACE_HANDLE(intf_handle)) {
    SWITCH_LOG_ERROR(
        "bd member find failed on device %d "
        "bd handle %lx intf handle %lx: "
        "handle invalid(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member find failed on device %d "
        "bd handle %lx intf handle %lx: "
        "bd get failed(%s)\n",
        device,
        bd_handle,
        intf_handle,
        switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(bd_info->members, node) {
    bd_member = (switch_bd_member_t *)node->data;
    if (bd_member->handle == intf_handle) {
      *member = bd_member;
      return SWITCH_STATUS_SUCCESS;
    }
  }
  FOR_EACH_IN_LIST_END();

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  return status;
}

switch_status_t switch_bd_stats_enable(switch_device_t device,
                                       switch_handle_t bd_handle) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;
  switch_id_t bd_stats_id = 0;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BD, (void **)&bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "bd context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (bd_info->stats_enabled) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  bd_info->stats_enabled = TRUE;
  bd_info->bd_stats = SWITCH_MALLOC(device, sizeof(switch_bd_stats_t), 0x1);

  if (!bd_info->bd_stats) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMSET(bd_info->bd_stats, 0x0, sizeof(switch_bd_stats_t));

  status = switch_api_id_allocator_allocate_contiguous(
      device, bd_ctx->stats_id_allocator, SWITCH_BD_STATS_MAX, &bd_stats_id);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    bd_info->bd_stats->stats_id[index] = bd_stats_id++;
  }

  status = switch_pd_bd_table_entry_update(
      device, handle_to_id(bd_handle), bd_info, bd_info->bd_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_egress_bd_stats_table_entry_add(
      device, handle_to_id(bd_handle), bd_info->bd_stats->stats_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats enabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:

  if (bd_info->bd_stats) {
    SWITCH_FREE(device, bd_info->bd_stats);
  }

  bd_info->bd_stats = NULL;

  return status;
}

switch_status_t switch_bd_stats_disable(switch_device_t device,
                                        switch_handle_t bd_handle) {
  switch_bd_context_t *bd_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats disabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_BD, (void **)&bd_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd free failed on device %d: "
        "bd context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats disabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (!bd_info->stats_enabled) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("bd stats disabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (!bd_info->bd_stats) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("bd stats disabled failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    status = switch_api_id_allocator_release(
        device, bd_ctx->stats_id_allocator, bd_info->bd_stats->stats_id[index]);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("bd stats disable failed for device %d: %s",
                       device,
                       switch_error_to_string(status));
    }

    bd_info->bd_stats->stats_id[index] = 0;
  }

  status = switch_pd_bd_table_entry_update(
      device, handle_to_id(bd_handle), bd_info, bd_info->bd_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats disable failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_egress_bd_stats_table_entry_delete(
      device, bd_info->bd_stats->stats_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats disable failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_FREE(device, bd_info->bd_stats);

  bd_info->bd_stats = NULL;
  bd_info->stats_enabled = FALSE;

  return status;

cleanup:

  return status;
}

switch_status_t switch_bd_stats_get(const switch_device_t device,
                                    const switch_handle_t bd_handle,
                                    const switch_uint8_t count,
                                    const switch_bd_counter_id_t *counter_ids,
                                    switch_counter_t *counters) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_bd_counter_id_t counter_id = 0;
  switch_uint8_t index = 0;

  SWITCH_ASSERT(counter_ids != NULL);
  SWITCH_ASSERT(counters != NULL);

  if (!counters || !counter_ids) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (!bd_info->stats_enabled) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(counters, 0x0, sizeof(switch_counter_t) * count);

  status = switch_pd_bd_stats_get(device, bd_info->bd_stats);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < count; index++) {
    counter_id = counter_ids[index];
    switch (counter_id) {
      case SWITCH_BD_STATS_IN_UCAST:
      case SWITCH_BD_STATS_IN_MCAST:
      case SWITCH_BD_STATS_IN_BCAST:
      case SWITCH_BD_STATS_IN_DROP:
      case SWITCH_BD_STATS_OUT_UCAST:
      case SWITCH_BD_STATS_OUT_MCAST:
      case SWITCH_BD_STATS_OUT_BCAST:
      case SWITCH_BD_STATS_OUT_DROP:
        counters[index] = bd_info->bd_stats->counters[counter_id];
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        SWITCH_LOG_ERROR("bd stats get failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
    }
  }

  return status;
}

switch_status_t switch_bd_stats_clear(switch_device_t device,
                                      switch_handle_t bd_handle) {
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("bd stats clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_bd_stats_clear(device, bd_info->bd_stats);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd stats clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_bd_ipv4_unicast_set(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool ipv4_unicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv4 unicast set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;
  bd_info.ipv4_unicast = ipv4_unicast;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv4 unicast set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ipv4 unicast set on device %d "
      "bd handle %lx ipv4 unicast %s\n",
      device,
      bd_handle,
      ipv4_unicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv4_unicast_get(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool *ipv4_unicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv4 unicast get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV4_UNICAST;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv4 unicast get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *ipv4_unicast = bd_info.ipv4_unicast;

  SWITCH_LOG_DETAIL(
      "bd ipv4 unicast get on device %d "
      "bd handle %lx ipv4 unicast %s\n",
      device,
      bd_handle,
      ipv4_unicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv6_unicast_set(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool ipv6_unicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv6 unicast set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;
  bd_info.ipv6_unicast = ipv6_unicast;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv6 unicast set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ipv6 unicast set on device %d "
      "bd handle %lx ipv6 unicast %s\n",
      device,
      bd_handle,
      ipv6_unicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv6_unicast_get(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool *ipv6_unicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv6 unicast get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV6_UNICAST;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv6 unicast get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *ipv6_unicast = bd_info.ipv6_unicast;

  SWITCH_LOG_DETAIL(
      "bd ipv6 unicast get on device %d "
      "bd handle %lx ipv6 unicast %s\n",
      device,
      bd_handle,
      ipv6_unicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv4_multicast_set(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             bool ipv4_multicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv4 multicast set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV4_MULTICAST;
  bd_info.ipv4_multicast = ipv4_multicast;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv4 multicast set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ipv4 multicast set on device %d "
      "bd handle %lx ipv4 multicast %s\n",
      device,
      bd_handle,
      ipv4_multicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv4_multicast_get(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             bool *ipv4_multicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv4 multicast get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV4_MULTICAST;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv4 multicast get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *ipv4_multicast = bd_info.ipv4_multicast;

  SWITCH_LOG_DETAIL(
      "bd ipv4 multicast get on device %d "
      "bd handle %lx ipv4 multicast %s\n",
      device,
      bd_handle,
      ipv4_multicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv6_multicast_set(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             bool ipv6_multicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv6 multicast set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV6_MULTICAST;
  bd_info.ipv6_multicast = ipv6_multicast;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv6 multicast set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd ipv6 multicast set on device %d "
      "bd handle %lx ipv6 multicast %s\n",
      device,
      bd_handle,
      ipv6_multicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_ipv6_multicast_get(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             bool *ipv6_multicast) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd ipv6 multicast get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IPV6_MULTICAST;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd ipv6 multicast get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *ipv6_multicast = bd_info.ipv6_multicast;

  SWITCH_LOG_DETAIL(
      "bd ipv6 multicast get on device %d "
      "bd handle %lx ipv6 multicast %s\n",
      device,
      bd_handle,
      ipv6_multicast ? "true" : "false");

  return status;
}

switch_status_t switch_bd_igmp_snooping_set(switch_device_t device,
                                            switch_handle_t bd_handle,
                                            bool igmp_snooping) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd igmp snooping set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IGMP_SNOOPING;
  bd_info.igmp_snooping = igmp_snooping;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd igmp snooping set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd igmp snooping set on device %d "
      "bd handle %lx igmp snooping %s\n",
      device,
      bd_handle,
      igmp_snooping ? "true" : "false");

  return status;
}

switch_status_t switch_bd_igmp_snooping_get(switch_device_t device,
                                            switch_handle_t bd_handle,
                                            bool *igmp_snooping) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd igmp snooping get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_IGMP_SNOOPING;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd igmp snooping get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *igmp_snooping = bd_info.igmp_snooping;

  SWITCH_LOG_DETAIL(
      "bd igmp snooping get on device %d "
      "bd handle %lx igmp snooping %s\n",
      device,
      bd_handle,
      igmp_snooping ? "true" : "false");

  return status;
}

switch_status_t switch_bd_mld_snooping_set(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool mld_snooping) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mld snooping set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_MLD_SNOOPING;
  bd_info.mld_snooping = mld_snooping;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mld snooping set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd mld snooping set on device %d "
      "bd handle %lx mld snooping %s\n",
      device,
      bd_handle,
      mld_snooping ? "true" : "false");

  return status;
}

switch_status_t switch_bd_mld_snooping_get(switch_device_t device,
                                           switch_handle_t bd_handle,
                                           bool *mld_snooping) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mld snooping get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_MLD_SNOOPING;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mld snooping get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *mld_snooping = bd_info.mld_snooping;

  SWITCH_LOG_DETAIL(
      "bd mld snooping get on device %d "
      "bd handle %lx mld snooping %s\n",
      device,
      bd_handle,
      mld_snooping ? "true" : "false");

  return status;
}

switch_status_t switch_bd_aging_interval_set(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             switch_int32_t aging_interval) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd aging interval set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_AGING_INTERVAL;
  bd_info.aging_interval = aging_interval;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd aging interval set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd aging interval set on device %d "
      "bd handle %lx aging interval %d\n",
      device,
      bd_handle,
      aging_interval);

  return status;
}

switch_status_t switch_bd_aging_interval_get(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             switch_int32_t *aging_interval) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd aging interval get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_AGING_INTERVAL;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd aging interval get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *aging_interval = bd_info.aging_interval;

  SWITCH_LOG_DETAIL(
      "bd aging interval get on device %d "
      "bd handle %lx aging interval %d\n",
      device,
      bd_handle,
      *aging_interval);

  return status;
}

switch_status_t switch_bd_vrf_handle_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t vrf_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd vrf handle set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    SWITCH_LOG_ERROR(
        "bd vrf handle set failed on device %d "
        "bd handle %lx: vrf handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;
  bd_info.vrf_handle = vrf_handle;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd vrf handle set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd vrf handle set on device %d "
      "bd handle %lx vrf handle %lx\n",
      device,
      bd_handle,
      vrf_handle);

  return status;
}

switch_status_t switch_bd_vrf_handle_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t *vrf_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd vrf handle get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_VRF_HANDLE;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd vrf handle get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *vrf_handle = bd_info.vrf_handle;

  SWITCH_LOG_DETAIL(
      "bd vrf handle get on device %d "
      "bd handle %lx vrf handle %lx\n",
      device,
      bd_handle,
      *vrf_handle);

  return status;
}

switch_status_t switch_bd_rmac_handle_set(switch_device_t device,
                                          switch_handle_t bd_handle,
                                          switch_handle_t rmac_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd rmac handle set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_RMAC_HANDLE(rmac_handle));
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    SWITCH_LOG_ERROR(
        "bd rmac handle set failed on device %d "
        "bd handle %lx: rmac handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_RMAC_HANDLE;
  bd_info.rmac_handle = rmac_handle;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd rmac handle set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd rmac handle set on device %d "
      "bd handle %lx rmac handle %lx\n",
      device,
      bd_handle,
      rmac_handle);

  return status;
}

switch_status_t switch_bd_rmac_handle_get(switch_device_t device,
                                          switch_handle_t bd_handle,
                                          switch_handle_t *rmac_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd rmac handle get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_RMAC_HANDLE;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd rmac handle get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *rmac_handle = bd_info.rmac_handle;

  SWITCH_LOG_DETAIL(
      "bd rmac handle get on device %d "
      "bd handle %lx rmac handle %lx\n",
      device,
      bd_handle,
      *rmac_handle);

  return status;
}

switch_status_t switch_bd_learning_set(switch_device_t device,
                                       switch_handle_t bd_handle,
                                       bool learning) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd learning set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_LEARNING;
  bd_info.learning = learning;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd learning set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd learning set on device %d "
      "bd handle %lx learning %s\n",
      device,
      bd_handle,
      learning ? "true" : "false");

  return status;
}

switch_status_t switch_bd_learning_get(switch_device_t device,
                                       switch_handle_t bd_handle,
                                       bool *learning) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd learning get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_LEARNING;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd learning get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *learning = bd_info.learning;

  SWITCH_LOG_DETAIL(
      "bd learning get on device %d "
      "bd handle %lx learning %s\n",
      device,
      bd_handle,
      *learning ? "true" : "false");

  return status;
}

switch_status_t switch_bd_rewrite_smac_index_get(switch_device_t device,
                                                 switch_handle_t bd_handle,
                                                 switch_id_t *smac_index)

{
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR("bd smac index get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bd smac index get failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *smac_index = 0;
  if (bd_info->bd_flags & SWITCH_BD_ATTR_RMAC_HANDLE) {
    *smac_index = bd_info->smac_index;
  }

  return status;
}

switch_status_t switch_bd_stp_handle_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t stp_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd stp handle set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_STP_HANDLE(stp_handle));
  if (!SWITCH_STP_HANDLE(stp_handle)) {
    SWITCH_LOG_ERROR(
        "bd stp handle set failed on device %d "
        "bd handle %lx: stp handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_STP_HANDLE;
  bd_info.stp_handle = stp_handle;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd stp handle set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd stp handle set on device %d "
      "bd handle %lx stp handle %lx\n",
      device,
      bd_handle,
      stp_handle);

  return status;
}

switch_status_t switch_bd_stp_handle_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t *stp_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd stp handle get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_STP_HANDLE;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd stp handle get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *stp_handle = bd_info.stp_handle;

  SWITCH_LOG_DETAIL(
      "bd stp handle get on device %d "
      "bd handle %lx stp handle %lx\n",
      device,
      bd_handle,
      *stp_handle);

  return status;
}

switch_status_t switch_bd_mrpf_group_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_mrpf_group_t mrpf_group) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mrpf group set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_MRPF_GROUP;
  bd_info.mrpf_group = mrpf_group;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mrpf group set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd mrpf group set on device %d "
      "bd handle %lx mrpf group %lx\n",
      device,
      bd_handle,
      mrpf_group);

  return status;
}

switch_status_t switch_bd_mrpf_group_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_mrpf_group_t *mrpf_group) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mrpf group get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_LEARNING;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mrpf group get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *mrpf_group = bd_info.mrpf_group;

  SWITCH_LOG_DETAIL(
      "bd mrpf group get on device %d "
      "bd handle %lx mrpf group %lx\n",
      device,
      bd_handle,
      *mrpf_group);

  return status;
}

switch_status_t switch_bd_handle_get(switch_device_t device,
                                     switch_handle_t handle,
                                     switch_handle_t *bd_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (SWITCH_VLAN_HANDLE(handle)) {
    switch_vlan_info_t *vlan_info = NULL;
    status = switch_vlan_get(device, handle, &vlan_info);
    *bd_handle = status == SWITCH_STATUS_SUCCESS ? vlan_info->bd_handle
                                                 : SWITCH_API_INVALID_HANDLE;
  } else if (SWITCH_LN_HANDLE(handle)) {
    switch_ln_info_t *ln_info = NULL;
    status = switch_ln_get(device, handle, &ln_info);
    *bd_handle = status == SWITCH_STATUS_SUCCESS ? ln_info->bd_handle
                                                 : SWITCH_API_INVALID_HANDLE;
  } else if (SWITCH_RIF_HANDLE(handle)) {
    switch_rif_info_t *rif_info = NULL;
    status = switch_rif_get(device, handle, &rif_info);
    *bd_handle = status == SWITCH_STATUS_SUCCESS ? rif_info->bd_handle
                                                 : SWITCH_API_INVALID_HANDLE;
  } else if (SWITCH_VRF_HANDLE(handle)) {
    switch_vrf_info_t *vrf_info = NULL;
    switch_vrf_get(device, handle, &vrf_info, status);
    *bd_handle = status == SWITCH_STATUS_SUCCESS ? vrf_info->bd_handle
                                                 : SWITCH_API_INVALID_HANDLE;
  } else if (SWITCH_BD_HANDLE(handle)) {
    *bd_handle = handle;
    status = SWITCH_STATUS_SUCCESS;
  } else {
    status = SWITCH_STATUS_INVALID_PARAMETER;
  }

  return status;
}

switch_status_t switch_bd_member_stp_state_set(switch_device_t device,
                                               switch_handle_t network_handle,
                                               switch_status_t intf_handle,
                                               switch_stp_state_t stp_state) {
  switch_bd_member_t *bd_member = NULL;
  switch_stp_state_t prev_stp_state = SWITCH_PORT_STP_STATE_NONE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint64_t flush_type = 0;
  switch_mcast_member_t mcast_member = {0};
  switch_bd_info_t *bd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_NETWORK_HANDLE(network_handle));
  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));
  if (!SWITCH_NETWORK_HANDLE(network_handle) ||
      !SWITCH_INTERFACE_HANDLE(intf_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "bd member stp state set failed on device %d "
        "network handle 0x%lx intf handle 0x%lx "
        "stp state %s: handle invalid:(%s)\n",
        device,
        network_handle,
        intf_handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_handle_get(device, network_handle, &bd_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member stp state set failed on device %d "
        "network handle 0x%lx intf handle 0x%lx "
        "stp state %s: bd handle get failed:(%s)\n",
        device,
        network_handle,
        intf_handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_member_find(device, bd_handle, intf_handle, &bd_member);
  if (status != SWITCH_STATUS_SUCCESS) {
    if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
      return status;
    }
    SWITCH_LOG_ERROR(
        "bd member stp state set failed on device %d "
        "network handle 0x%lx intf handle 0x%lx "
        "stp state %s: bd member get failed:(%s)\n",
        device,
        network_handle,
        intf_handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  status = switch_bd_get(device, bd_member->bd_handle, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd member stp state set failed on device %d "
        "network handle 0x%lx intf handle 0x%lx "
        "stp state %s: bd get failed:(%s)\n",
        device,
        network_handle,
        intf_handle,
        switch_stp_state_to_string(stp_state),
        switch_error_to_string(status));
    return status;
  }

  if (bd_member->stp_state == stp_state) {
    return status;
  }

  prev_stp_state = bd_member->stp_state;
  bd_member->stp_state = stp_state;

  if (bd_member->stp_state == SWITCH_PORT_STP_STATE_BLOCKING ||
      bd_member->stp_state == SWITCH_PORT_STP_STATE_LEARNING) {
    if (SWITCH_HW_FLAG_ISSET(bd_member,
                             SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY)) {
      mcast_member.handle = intf_handle;
      mcast_member.network_handle = network_handle;
      status = switch_api_multicast_member_delete(
          device, bd_info->flood_handle, 0x1, &mcast_member);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd member stp state set failed on device %d "
            "network handle 0x%lx intf handle 0x%lx "
            "stp state %s: mcast member delete failed:(%s)\n",
            device,
            network_handle,
            intf_handle,
            switch_stp_state_to_string(stp_state),
            switch_error_to_string(status));
        return status;
      }
      SWITCH_HW_FLAG_CLEAR(bd_member, SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY);
    }

    flush_type |= SWITCH_MAC_FLUSH_TYPE_NETWORK;
    flush_type |= SWITCH_MAC_FLUSH_TYPE_INTERFACE;
    status = switch_api_mac_table_entry_flush(
        device, flush_type, network_handle, intf_handle, 0x0);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "bd member stp state set failed on device %d "
          "network handle 0x%lx intf handle 0x%lx "
          "stp state %s: mac table flush failed:(%s)\n",
          device,
          network_handle,
          intf_handle,
          switch_stp_state_to_string(stp_state),
          switch_error_to_string(status));
      return status;
    }
  } else if (prev_stp_state == SWITCH_PORT_STP_STATE_BLOCKING ||
             prev_stp_state == SWITCH_PORT_STP_STATE_LEARNING) {
    if (!(SWITCH_HW_FLAG_ISSET(bd_member,
                               SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY))) {
      mcast_member.handle = intf_handle;
      mcast_member.network_handle = network_handle;
      status = switch_api_multicast_member_add(
          device, bd_info->flood_handle, 0x1, &mcast_member);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "bd member stp state set failed on device %d "
            "network handle 0x%lx intf handle 0x%lx "
            "stp state %s: mcast member add failed:(%s)\n",
            device,
            network_handle,
            intf_handle,
            switch_stp_state_to_string(stp_state),
            switch_error_to_string(status));
        return status;
      }
      SWITCH_HW_FLAG_SET(bd_member, SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY);
    }
  }

  SWITCH_LOG_DEBUG(
      "bd member stp state set on device %d "
      "network handle 0x%lx intf handle 0x%lx stp state %s\n",
      device,
      network_handle,
      intf_handle,
      switch_stp_state_to_string(stp_state));
  return status;
}

switch_status_t switch_bd_mrouters_handle_set(switch_device_t device,
                                              switch_handle_t bd_handle,
                                              switch_handle_t mrouters_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mrouters handle set failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED;
  bd_info.mrouters_mc_handle = mrouters_handle;

  status = switch_bd_update(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mrouters handle set failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DETAIL(
      "bd mrouters handle set on device %d "
      "bd handle %lx mrouters handle %lx\n",
      device,
      bd_handle,
      mrouters_handle);

  return status;
}

switch_status_t switch_bd_mrouters_handle_get(
    switch_device_t device,
    switch_handle_t bd_handle,
    switch_handle_t *mrouters_handle) {
  switch_bd_info_t bd_info;
  switch_uint64_t bd_flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  if (!SWITCH_BD_HANDLE(bd_handle)) {
    SWITCH_LOG_ERROR(
        "bd mrouters handle get failed on device %d "
        "bd handle %lx: bd handle invalid(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&bd_info, 0x0, sizeof(bd_info));
  bd_flags |= SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED;

  status = switch_bd_attribute_get(device, bd_handle, bd_flags, &bd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "bd mrouter handle get failed on device %d "
        "bd handle %lx: bd update failed(%s)\n",
        device,
        bd_handle,
        switch_error_to_string(status));
    return status;
  }

  *mrouters_handle = bd_info.mrouters_mc_handle;

  SWITCH_LOG_DETAIL(
      "bd mrouter handle get on device %d "
      "bd handle %lx mroutes handle %lx\n",
      device,
      bd_handle,
      *mrouters_handle);

  return status;
}

#ifdef __cplusplus
}
#endif
