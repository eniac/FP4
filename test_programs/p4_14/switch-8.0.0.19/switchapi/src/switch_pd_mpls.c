

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

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_mpls_table_entry_add(
    switch_device_t device,
    switch_mpls_tunnel_type_ingress_t ingress_tunnel_type,
    switch_mpls_tunnel_subtype_ingress_t mpls_tunnel_type,
    switch_bd_t bd,
    switch_api_mpls_info_t *tunnel_info,
    switch_bd_info_t *bd_info,
    switch_mpls_label_t label,
    switch_ifindex_t egress_ifindex,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ingress_tunnel_type);
  UNUSED(bd);
  UNUSED(tunnel_info);
  UNUSED(label);
  UNUSED(egress_ifindex);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_tunnel_vni = label;
  if (ingress_tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_MPLS_UDP) {
    ingress_tunnel_type = SWITCH_TUNNEL_TYPE_INGRESS_MPLS;
  }
  switch (tunnel_info->mpls_mode) {
    case SWITCH_MPLS_MODE_TERMINATE: {
      switch (tunnel_info->mpls_type) {
        case SWITCH_MPLS_TYPE_EOMPLS: {
          p4_pd_dc_terminate_eompls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.inner_ipv4_valid = TRUE;
          match_spec.inner_ipv6_valid = FALSE;
          match_spec.mpls_0__valid = TRUE;
          action_spec.action_bd = bd;
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_add_with_terminate_eompls(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.inner_ipv4_valid = FALSE;
          match_spec.inner_ipv6_valid = TRUE;
          action_spec.action_bd = bd;
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_add_with_terminate_eompls(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              (entry_hdl + 1));
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_VPLS: {
          p4_pd_dc_terminate_vpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          // TODO: This is a hack. Eompls will work only when the inner packet
          // is IPV4.
          // This is just to avoid programming 3 entries - v4, v6 and non-ip.
          // Ideally, irrespective of inner header, eompls has to terminate
          // but
          // since we are parsing the inner header, either v4 or v6 valid will
          // be set.
          match_spec.inner_ipv4_valid = TRUE;
          match_spec.inner_ipv6_valid = FALSE;
          match_spec.mpls_0__valid = TRUE;
          action_spec.action_bd = bd;
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status =
              p4_pd_dc_tunnel_table_add_with_terminate_vpls(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            &action_spec,
                                                            entry_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_PW: {
          p4_pd_dc_terminate_pw_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.inner_ipv4_valid = FALSE;
          match_spec.inner_ipv6_valid = FALSE;
          match_spec.mpls_0__valid = TRUE;
          action_spec.action_ifindex = egress_ifindex;
          pd_status =
              p4_pd_dc_tunnel_table_add_with_terminate_pw(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &match_spec,
                                                          &action_spec,
                                                          entry_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_IPV4_MPLS: {
#ifndef P4_IPV4_DISABLE
          p4_pd_dc_terminate_ipv4_over_mpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.inner_ipv4_valid = TRUE;
          match_spec.inner_ipv6_valid = FALSE;
          match_spec.mpls_0__valid = TRUE;
          action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_add_with_terminate_ipv4_over_mpls(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
#endif /* P4_IPV4_DISABLE */
        } break;
        case SWITCH_MPLS_TYPE_IPV6_MPLS: {
#ifndef P4_IPV6_DISABLE
          p4_pd_dc_terminate_ipv6_over_mpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.inner_ipv4_valid = FALSE;
          match_spec.inner_ipv6_valid = TRUE;
          match_spec.mpls_0__valid = TRUE;
          action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_add_with_terminate_ipv6_over_mpls(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
#endif /* P4_IPV6_DISABLE */
        } break;
        default:
          status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
      }
    } break;
    case SWITCH_MPLS_MODE_TRANSIT: {
      p4_pd_dc_forward_mpls_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      // TODO: This is a hack. Swap will work only when the inner packet is
      // IPV4.
      // This is just to avoid programming 3 entries - v4, v6 and non-ip.
      match_spec.inner_ipv4_valid = TRUE;
      match_spec.inner_ipv6_valid = FALSE;
      match_spec.mpls_0__valid = TRUE;
      action_spec.action_nexthop_index = handle_to_id(tunnel_info->nhop_handle);
      pd_status =
          p4_pd_dc_tunnel_table_add_with_forward_mpls(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      &action_spec,
                                                      entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mpls table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mpls table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mpls table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mpls_table_entry_update(
    switch_device_t device,
    switch_mpls_tunnel_subtype_ingress_t mpls_tunnel_type,
    switch_bd_t bd,
    switch_api_mpls_info_t *tunnel_info,
    switch_bd_info_t *bd_info,
    switch_mpls_label_t label,
    switch_ifindex_t egress_ifindex,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mpls_tunnel_type);
  UNUSED(bd);
  UNUSED(tunnel_info);
  UNUSED(label);
  UNUSED(egress_ifindex);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE)
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (tunnel_info->mpls_mode) {
    case SWITCH_MPLS_MODE_TERMINATE: {
      switch (tunnel_info->mpls_type) {
        case SWITCH_MPLS_TYPE_EOMPLS: {
          p4_pd_dc_terminate_eompls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          // TODO: This is a hack. Eompls will work only when the inner packet
          // is IPV4.
          // This is just to avoid programming 3 entries - v4, v6 and non-ip.
          // Ideally, irrespective of inner header, eompls has to terminate
          // but
          // since we are parsing the inner header, either v4 or v6 valid will
          // be set.
          action_spec.action_bd = bd;
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_eompls(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_VPLS: {
          p4_pd_dc_terminate_vpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          // TODO: This is a hack. Eompls will work only when the inner packet
          // is IPV4.
          // This is just to avoid programming 3 entries - v4, v6 and non-ip.
          // Ideally, irrespective of inner header, eompls has to terminate
          // but
          // since we are parsing the inner header, either v4 or v6 valid will
          // be set.
          action_spec.action_bd = bd;
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_vpls(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_PW: {
          p4_pd_dc_terminate_pw_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          action_spec.action_ifindex = egress_ifindex;
          pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_pw(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        } break;
        case SWITCH_MPLS_TYPE_IPV4_MPLS: {
#ifndef P4_IPV4_DISABLE
          p4_pd_dc_terminate_ipv4_over_mpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          action_spec.action_vrf = handle_to_id(tunnel_info->vrf_handle);
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status =
              p4_pd_dc_tunnel_table_modify_with_terminate_ipv4_over_mpls(
                  switch_cfg_sess_hdl,
                  p4_pd_device.device_id,
                  entry_hdl,
                  &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
#endif /* P4_IPV4_DISABLE */
        } break;
        case SWITCH_MPLS_TYPE_IPV6_MPLS: {
#ifndef P4_IPV6_DISABLE
          p4_pd_dc_terminate_ipv6_over_mpls_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          action_spec.action_vrf = handle_to_id(tunnel_info->vrf_handle);
          action_spec.action_tunnel_type = mpls_tunnel_type;
          pd_status =
              p4_pd_dc_tunnel_table_modify_with_terminate_ipv6_over_mpls(
                  switch_cfg_sess_hdl,
                  p4_pd_device.device_id,
                  entry_hdl,
                  &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
#endif /* P4_IPV6_DISABLE */
        } break;
        default:
          status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
      }
    } break;
    case SWITCH_MPLS_MODE_TRANSIT: {
      p4_pd_dc_forward_mpls_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      // TODO: This is a hack. Swap will work only when the inner packet is
      // IPV4.
      // This is just to avoid programming 3 entries - v4, v6 and non-ip.
      action_spec.action_nexthop_index = handle_to_id(tunnel_info->nhop_handle);
      pd_status = p4_pd_dc_tunnel_table_modify_with_forward_mpls(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
  }
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mpls table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mpls table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mpls table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mpls_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  pd_status =
      p4_pd_dc_tunnel_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mpls table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mpls table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mpls table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_mpls_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_uint16_t num_labels,
    switch_mpls_t *mpls_header,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(num_labels);
  UNUSED(mpls_header);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_rewrite_match_spec_t));
  match_spec.tunnel_metadata_tunnel_index = tunnel_index;

  switch (num_labels) {
    case 1: {
      p4_pd_dc_set_mpls_rewrite_push1_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_bos = 1;
      pd_status = p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_rewrite_push1(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case 2: {
      p4_pd_dc_set_mpls_rewrite_push2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      action_spec.action_bos = 1;
      pd_status = p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_rewrite_push2(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case 3: {
      p4_pd_dc_set_mpls_rewrite_push3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      action_spec.action_label3 = mpls_header[2].label;
      action_spec.action_ttl3 = mpls_header[2].ttl;
      action_spec.action_exp3 = mpls_header[2].exp;
      action_spec.action_bos = 1;
      pd_status = p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_rewrite_push3(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_mpls_udp_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_id_t smac_index,
    switch_id_t dmac_index,
    switch_id_t sip_index,
    switch_id_t dip_index,
    switch_uint8_t header_count,
    switch_mpls_t *mpls_header,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(smac_index);
  UNUSED(dmac_index);
  UNUSED(header_count);
  UNUSED(mpls_header);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) && \
    defined(P4_MPLS_UDP_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_rewrite_match_spec_t));
  match_spec.tunnel_metadata_tunnel_index = tunnel_index;
  switch_uint8_t bos = 1;

  switch (header_count) {
    case 0: {
      p4_pd_dc_set_mpls_udp_rewrite_push0_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec,
                    0,
                    sizeof(p4_pd_dc_set_mpls_udp_rewrite_push0_action_spec_t));
      action_spec.action_dip_index = dip_index;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_udp_rewrite_push0(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case 1: {
      p4_pd_dc_set_mpls_udp_rewrite_push1_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec,
                    0,
                    sizeof(p4_pd_dc_set_mpls_udp_rewrite_push1_action_spec_t));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_sip_index = sip_index;
      action_spec.action_dip_index = dip_index;
      action_spec.action_bos = bos;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_udp_rewrite_push1(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case 2: {
      p4_pd_dc_set_mpls_udp_rewrite_push2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec,
                    0,
                    sizeof(p4_pd_dc_set_mpls_udp_rewrite_push2_action_spec_t));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      action_spec.action_sip_index = sip_index;
      action_spec.action_dip_index = dip_index;
      action_spec.action_bos = bos;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_udp_rewrite_push2(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case 3: {
      p4_pd_dc_set_mpls_udp_rewrite_push3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec,
                    0,
                    sizeof(p4_pd_dc_set_mpls_udp_rewrite_push3_action_spec_t));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      action_spec.action_label3 = mpls_header[2].label;
      action_spec.action_ttl3 = mpls_header[2].ttl;
      action_spec.action_exp3 = mpls_header[2].exp;
      action_spec.action_sip_index = sip_index;
      action_spec.action_dip_index = dip_index;
      action_spec.action_bos = bos;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_add_with_set_mpls_udp_rewrite_push3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    default:
      status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_mpls_entry_update(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_uint16_t num_labels,
    switch_mpls_t *mpls_header,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(num_labels);
  UNUSED(mpls_header);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (num_labels) {
    case 1: {
      p4_pd_dc_set_mpls_rewrite_push1_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_modify_with_set_mpls_rewrite_push1(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case 2: {
      p4_pd_dc_set_mpls_rewrite_push2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_modify_with_set_mpls_rewrite_push2(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case 3: {
      p4_pd_dc_set_mpls_rewrite_push3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_label1 = mpls_header[0].label;
      action_spec.action_ttl1 = mpls_header[0].ttl;
      action_spec.action_exp1 = mpls_header[0].exp;
      action_spec.action_label2 = mpls_header[1].label;
      action_spec.action_ttl2 = mpls_header[1].ttl;
      action_spec.action_exp2 = mpls_header[1].exp;
      action_spec.action_label3 = mpls_header[2].label;
      action_spec.action_ttl3 = mpls_header[2].ttl;
      action_spec.action_exp3 = mpls_header[2].exp;
      pd_status =
          p4_pd_dc_tunnel_rewrite_table_modify_with_set_mpls_rewrite_push3(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      status = SWITCH_STATUS_INVALID_ENCAP_TYPE;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_MPLS_DISABLE) && !defined(P4_TUNNEL_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_rewrite_table_mpls_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_tunnel_t tunnel_index,
    switch_neighbor_tunnel_type_t neigh_type,
    switch_mac_addr_t dmac,
    switch_mpls_label_t label,
    switch_uint8_t header_count,
    switch_id_t tunnel_dmac_index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(tunnel_index);
  UNUSED(neigh_type);
  UNUSED(dmac);
  UNUSED(label);
  UNUSED(header_count);
  UNUSED(tunnel_dmac_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MPLS_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_rewrite_match_spec_t));
  match_spec.l3_metadata_nexthop_index = nhop_index;
  switch (neigh_type) {
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN: {
      p4_pd_dc_set_mpls_push_rewrite_l2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      pd_status = p4_pd_dc_rewrite_table_add_with_set_mpls_push_rewrite_l2(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status = p4_pd_dc_rewrite_table_add_with_set_mpls_push_rewrite_l3(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status = p4_pd_dc_rewrite_table_add_with_set_mpls_swap_push_rewrite_l3(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_L3VPN: {
      header_count = 0;
      p4_pd_dc_set_mpls_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status = p4_pd_dc_rewrite_table_add_with_set_mpls_swap_push_rewrite_l3(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
#ifdef P4_MPLS_UDP_ENABLE
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L2VPN: {
      p4_pd_dc_set_mpls_ipv4_udp_push_rewrite_l2_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(p4_pd_dc_set_mpls_ipv4_udp_push_rewrite_l2_action_spec_t));
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv4_udp_push_rewrite_l2(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L2VPN: {
      p4_pd_dc_set_mpls_ipv6_udp_push_rewrite_l2_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(p4_pd_dc_set_mpls_ipv6_udp_push_rewrite_l2_action_spec_t));
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv6_udp_push_rewrite_l2(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_ipv4_udp_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(p4_pd_dc_set_mpls_ipv4_udp_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv4_udp_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_ipv6_udp_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(p4_pd_dc_set_mpls_ipv6_udp_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv6_udp_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_ipv4_udp_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(
              p4_pd_dc_set_mpls_ipv4_udp_swap_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv4_udp_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_ipv6_udp_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(
              p4_pd_dc_set_mpls_ipv6_udp_swap_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv6_udp_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV4_UDP_SWAP_L3VPN: {
      header_count = 0;
      p4_pd_dc_set_mpls_ipv4_udp_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(
              p4_pd_dc_set_mpls_ipv4_udp_swap_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv4_udp_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_IPV6_UDP_SWAP_L3VPN: {
      header_count = 0;
      p4_pd_dc_set_mpls_ipv6_udp_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(
          &action_spec,
          0,
          sizeof(
              p4_pd_dc_set_mpls_ipv6_udp_swap_push_rewrite_l3_action_spec_t));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_add_with_set_mpls_ipv6_udp_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              &action_spec,
              entry_hdl);
    } break;
#endif /*MPLS_UDP_ENABLE*/
    default:
      header_count = 0;
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rewrite table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MPLS_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_rewrite_table_mpls_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_tunnel_t tunnel_index,
    switch_neighbor_tunnel_type_t neigh_type,
    switch_mac_addr_t dmac,
    switch_mpls_label_t label,
    switch_uint8_t header_count,
    switch_id_t tunnel_dmac_index,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(bd);
  UNUSED(nhop_index);
  UNUSED(tunnel_index);
  UNUSED(neigh_type);
  UNUSED(dmac);
  UNUSED(label);
  UNUSED(header_count);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MPLS_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (neigh_type) {
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN: {
      p4_pd_dc_set_mpls_push_rewrite_l2_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      pd_status = p4_pd_dc_rewrite_table_modify_with_set_mpls_push_rewrite_l2(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status = p4_pd_dc_rewrite_table_modify_with_set_mpls_push_rewrite_l3(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_PUSH_L3VPN: {
      p4_pd_dc_set_mpls_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_modify_with_set_mpls_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_SWAP_L3VPN: {
      header_count = 0;
      p4_pd_dc_set_mpls_swap_push_rewrite_l3_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_bd = bd;
      action_spec.action_label = label;
      action_spec.action_tunnel_index = tunnel_index;
      action_spec.action_header_count = header_count;
      action_spec.action_dmac_idx = tunnel_dmac_index;
      SWITCH_MEMCPY(action_spec.action_dmac, &dmac, ETH_LEN);
      pd_status =
          p4_pd_dc_rewrite_table_modify_with_set_mpls_swap_push_rewrite_l3(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      header_count = 0;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rewrite table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MPLS_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

#ifdef __cplusplus
}
#endif
