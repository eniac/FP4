
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

#ifdef SWITCH_PD
p4_pd_mirror_type_e switch_pd_p4_pd_mirror_type(
    switch_mirror_session_type_t type) {
  switch (type) {
    case SWITCH_MIRROR_SESSION_TYPE_SIMPLE:
      return PD_MIRROR_TYPE_NORM;
    case SWITCH_MIRROR_SESSION_TYPE_COALESCE:
      return PD_MIRROR_TYPE_COAL;
    case SWITCH_MIRROR_SESSION_TYPE_TRUNCATE:
    default:
      return PD_MIRROR_TYPE_MAX;
  }
}
#endif /* SWITCH_PD */

#ifdef SWITCH_PD
p4_pd_direction_t switch_pd_p4_pd_direction(switch_direction_t direction) {
  switch (direction) {
    case SWITCH_API_DIRECTION_BOTH:
      return PD_DIR_BOTH;
    case SWITCH_API_DIRECTION_INGRESS:
      return PD_DIR_INGRESS;
    case SWITCH_API_DIRECTION_EGRESS:
      return PD_DIR_EGRESS;
    default:
      return PD_DIR_NONE;
  }
}
#endif /* SWITCH_PD */

switch_status_t switch_pd_mirror_session_update(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_dev_port_t dev_port,
    switch_mirror_info_t *mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
#ifndef BMV2
  p4_pd_mirror_session_info_t mirr_sess_info;
  switch_uint16_t mgid = 0;
  switch_uint32_t level1_hash = 0;
  switch_uint32_t level2_hash = 0;
  switch_uint32_t seed = 0;
#endif /* #ifndef BMV2 */

  UNUSED(device);
  UNUSED(mirror_handle);
  UNUSED(mirror_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;
  switch_api_mirror_info_t *api_mirror_info = NULL;

  api_mirror_info = &mirror_info->api_mirror_info;

#ifndef BMV2
  SWITCH_MEMSET(&mirr_sess_info, 0x0, sizeof(mirr_sess_info));
  mirr_sess_info.type =
      switch_pd_p4_pd_mirror_type(api_mirror_info->session_type);
  mirr_sess_info.dir = switch_pd_p4_pd_direction(api_mirror_info->direction);
  mirr_sess_info.id = handle_to_id(mirror_handle);
  mirr_sess_info.egr_port = dev_port;
  mirr_sess_info.max_pkt_len = api_mirror_info->max_pkt_len;
  mirr_sess_info.cos = api_mirror_info->tos;
  mirr_sess_info.c2c = false;
  mirr_sess_info.extract_len = api_mirror_info->extract_len;
  mirr_sess_info.timeout_usec = api_mirror_info->timeout_usec;
  mirr_sess_info.int_hdr = (uint32_t *)&mirror_info->int_coal_pkt_hdr;
  mirr_sess_info.int_hdr_len = mirror_info->int_hdr_len;
  mirr_sess_info.mcast_grp_a = 0;
  mirr_sess_info.mcast_grp_a_v = false;

  if ((api_mirror_info->mirror_type == SWITCH_MIRROR_TYPE_ENHANCED_REMOTE ||
       api_mirror_info->mirror_type == SWITCH_MIRROR_TYPE_DTEL_REPORT) &&
      api_mirror_info->egress_port_handle == SWITCH_API_INVALID_HANDLE) {
    mgid = handle_to_id(mirror_info->mgid_handle);
    seed = handle_to_id(mirror_handle);
    level1_hash = rand_r(&seed);
    level2_hash = rand_r(&seed);
    mirr_sess_info.mcast_grp_b = mgid;
    mirr_sess_info.mcast_grp_b_v = true;
    mirr_sess_info.level1_mcast_hash = level1_hash;
    mirr_sess_info.level2_mcast_hash = level2_hash;
    mirr_sess_info.egr_port_v = false;
  } else {
    mirr_sess_info.mcast_grp_b = 0;
    mirr_sess_info.mcast_grp_b_v = false;
    mirr_sess_info.level1_mcast_hash = 0;
    mirr_sess_info.level2_mcast_hash = 0;
    mirr_sess_info.egr_port_v = true;
  }

  pd_status = p4_pd_mirror_session_update(
      switch_cfg_sess_hdl, p4_pd_device, &mirr_sess_info, mirror_info->enable);

#else /* #ifndef BMV2 */

  pd_status = p4_pd_mirror_session_update(
      switch_cfg_sess_hdl,
      p4_pd_device,
      switch_pd_p4_pd_mirror_type(api_mirror_info->session_type),
      switch_pd_p4_pd_direction(api_mirror_info->direction),
      handle_to_id(mirror_handle),
      dev_port,
      api_mirror_info->max_pkt_len,
      api_mirror_info->tos,
      false, /*c2c*/
      api_mirror_info->extract_len,
      api_mirror_info->timeout_usec,
      (uint32_t *)&mirror_info->int_coal_pkt_hdr,
      mirror_info->int_hdr_len,
      api_mirror_info->enable);

#endif /* #ifndef BMV2 */

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mirror session create failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror session create add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror session create add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mirror_session_delete(switch_device_t device,
                                                switch_handle_t mirror_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(mirror_handle);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_mirror_session_delete(
      switch_cfg_sess_hdl, p4_pd_device, handle_to_id(mirror_handle));

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mirror session delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror session delete add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror session delete add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mirror_table_entry_add(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_info_t *mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mirror_handle);
  UNUSED(mirror_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MIRROR_DISABLE
  p4_pd_dc_mirror_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;
  switch_api_mirror_info_t *api_mirror_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  match_spec.i2e_metadata_mirror_session_id = handle_to_id(mirror_handle);
  api_mirror_info = &mirror_info->api_mirror_info;

  switch (api_mirror_info->mirror_type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      break;
    case SWITCH_MIRROR_TYPE_REMOTE: {
      p4_pd_dc_set_mirror_bd_action_spec_t action_spec;
      action_spec.action_bd = handle_to_id(mirror_info->vlan_handle);
      action_spec.action_session_id = api_mirror_info->session_id;
      pd_status =
          p4_pd_dc_mirror_table_add_with_set_mirror_bd(switch_cfg_sess_hdl,
                                                       p4_pd_device,
                                                       &match_spec,
                                                       &action_spec,
                                                       &mirror_info->pd_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = mirror_info->pd_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "mirror table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE: {
      if (api_mirror_info->span_mode !=
          SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE) {
#ifndef P4_MIRROR_NEXTHOP_DISABLE
//        SWITCH_ASSERT(SWITCH_NHOP_HANDLE(api_mirror_info->nhop_handle));
//
//        p4_pd_dc_set_mirror_nhop_action_spec_t action_spec;
//        action_spec.action_session_id = api_mirror_info->session_id;
//        action_spec.action_nhop_idx =
//            handle_to_id(api_mirror_info->nhop_handle);
//        pd_status = p4_pd_dc_mirror_table_add_with_set_mirror_nhop(
//            switch_cfg_sess_hdl,
//            p4_pd_device,
//            &match_spec,
//            &action_spec,
//            &mirror_info->pd_hdl);
//        if (switch_pd_log_level_debug()) {
//          switch_pd_dump_entry_t pd_entry;
//          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
//          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
//          pd_entry.match_spec = (switch_uint8_t *)&match_spec;
//          pd_entry.match_spec_size = sizeof(match_spec);
//          pd_entry.action_spec = (switch_uint8_t *)&action_spec;
//          pd_entry.action_spec_size = sizeof(action_spec);
//          pd_entry.pd_hdl = mirror_info->pd_hdl;
//          switch_pd_entry_dump(device, &pd_entry);
//        }
//
//        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
//          SWITCH_PD_LOG_ERROR(
//              "mirror table add failed "
//              "on device %d : table %s action %s\n",
//              device,
//              switch_pd_table_id_to_string(0),
//              switch_pd_action_id_to_string(0));
//          goto cleanup;
//        }
#endif /* !P4_MIRROR_NEXTHOP_DISABLE */
      } else {
#ifdef P4_MIRROR_NEXTHOP_DISABLE
        if (!api_mirror_info->vlan_tag_valid) {
          p4_pd_dc_ipv4_erspan_t3_rewrite_with_eth_hdr_action_spec_t
              action_spec;
          SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.i2e_metadata_mirror_session_id =
              handle_to_id(mirror_handle);
          SWITCH_MEMCPY(action_spec.action_smac,
                        api_mirror_info->src_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          SWITCH_MEMCPY(action_spec.action_dmac,
                        api_mirror_info->dst_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
          action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;
          action_spec.action_ttl = api_mirror_info->ttl;
          action_spec.action_tos = api_mirror_info->tos;

          pd_status =
              p4_pd_dc_mirror_table_add_with_ipv4_erspan_t3_rewrite_with_eth_hdr(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &mirror_info->pd_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = mirror_info->pd_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }

        } else {
          p4_pd_dc_ipv4_erspan_t3_rewrite_with_eth_hdr_and_vlan_tag_action_spec_t
              action_spec;
          SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          match_spec.i2e_metadata_mirror_session_id =
              handle_to_id(mirror_handle);
          SWITCH_MEMCPY(action_spec.action_smac,
                        api_mirror_info->src_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          SWITCH_MEMCPY(action_spec.action_dmac,
                        api_mirror_info->dst_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
          action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;
          action_spec.action_ttl = api_mirror_info->ttl;
          action_spec.action_tos = api_mirror_info->tos;
          action_spec.action_vlan_id = api_mirror_info->vlan_id;
          action_spec.action_vlan_tpid = api_mirror_info->vlan_tpid;
          action_spec.action_cos = api_mirror_info->cos;

          pd_status =
              p4_pd_dc_mirror_table_add_with_ipv4_erspan_t3_rewrite_with_eth_hdr_and_vlan_tag(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &mirror_info->pd_hdl);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&match_spec;
            pd_entry.match_spec_size = sizeof(match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = mirror_info->pd_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        }

        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "mirror table add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
#endif /* P4_MIRROR_NEXTHOP_DISABLE */
      }
    } break;
    case SWITCH_MIRROR_TYPE_DTEL_REPORT: {
#ifdef P4_DTEL_REPORT_ENABLE
      p4_pd_dc_dtel_mirror_encap_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      SWITCH_MEMCPY(action_spec.action_smac,
                    api_mirror_info->src_mac.mac_addr,
                    SWITCH_MAC_LENGTH);
      action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
      action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;

      pd_status = p4_pd_dc_mirror_table_add_with_dtel_mirror_encap(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          &mirror_info->pd_hdl);

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "mirror table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
#endif /* P4_DTEL_REPORT_ENABLE */
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MIRROR_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror table entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mirror_table_entry_update(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_info_t *mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mirror_handle);
  UNUSED(mirror_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MIRROR_DISABLE
  p4_pd_dev_target_t p4_pd_device;
  switch_api_mirror_info_t *api_mirror_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  api_mirror_info = &mirror_info->api_mirror_info;

  switch (api_mirror_info->mirror_type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      break;
    case SWITCH_MIRROR_TYPE_REMOTE: {
      p4_pd_dc_set_mirror_bd_action_spec_t action_spec;
      action_spec.action_bd = handle_to_id(mirror_info->vlan_handle);
      action_spec.action_session_id = api_mirror_info->session_id;

      pd_status = p4_pd_dc_mirror_table_modify_with_set_mirror_bd(
          switch_cfg_sess_hdl,
          p4_pd_device.device_id,
          mirror_info->pd_hdl,
          &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = mirror_info->pd_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "mirror table update failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE: {
      if (api_mirror_info->span_mode !=
          SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE) {
#ifndef P4_MIRROR_NEXTHOP_DISABLE
//        p4_pd_dc_set_mirror_nhop_action_spec_t action_spec;
//        action_spec.action_nhop_idx =
//            handle_to_id(api_mirror_info->nhop_handle);
//        action_spec.action_session_id = api_mirror_info->session_id;
//
//        pd_status = p4_pd_dc_mirror_table_modify_with_set_mirror_nhop(
//            switch_cfg_sess_hdl,
//            p4_pd_device.device_id,
//            mirror_info->pd_hdl,
//            &action_spec);
//        if (switch_pd_log_level_debug()) {
//          switch_pd_dump_entry_t pd_entry;
//          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
//          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
//          pd_entry.match_spec_size = 0;
//          pd_entry.action_spec = (switch_uint8_t *)&action_spec;
//          pd_entry.action_spec_size = sizeof(action_spec);
//          pd_entry.pd_hdl = mirror_info->pd_hdl;
//          switch_pd_entry_dump(device, &pd_entry);
//        }
//
//        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
//          SWITCH_PD_LOG_ERROR(
//              "mirror table update failed "
//              "on device %d : table %s action %s\n",
//              device,
//              switch_pd_table_id_to_string(0),
//              switch_pd_action_id_to_string(0));
//          goto cleanup;
//        }
#endif /* P4_MIRROR_NEXTHOP_DISABLE */
      } else {
#ifdef P4_MIRROR_NEXTHOP_DISABLE
        if (!api_mirror_info->vlan_tag_valid) {
          p4_pd_dc_ipv4_erspan_t3_rewrite_with_eth_hdr_action_spec_t
              action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          SWITCH_MEMCPY(action_spec.action_smac,
                        api_mirror_info->src_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          SWITCH_MEMCPY(action_spec.action_dmac,
                        api_mirror_info->dst_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
          action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;
          action_spec.action_ttl = api_mirror_info->ttl;
          action_spec.action_tos = api_mirror_info->tos;

          pd_status =
              p4_pd_dc_mirror_table_modify_with_ipv4_erspan_t3_rewrite_with_eth_hdr(
                  switch_cfg_sess_hdl,
                  p4_pd_device.device_id,
                  mirror_info->pd_hdl,
                  &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = mirror_info->pd_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }

          if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
            SWITCH_PD_LOG_ERROR(
                "mirror table update failed "
                "on device %d : table %s action %s\n",
                device,
                switch_pd_table_id_to_string(0),
                switch_pd_action_id_to_string(0));
            goto cleanup;
          }
        } else {
          p4_pd_dc_ipv4_erspan_t3_rewrite_with_eth_hdr_and_vlan_tag_action_spec_t
              action_spec;
          SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
          SWITCH_MEMCPY(action_spec.action_smac,
                        api_mirror_info->src_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          SWITCH_MEMCPY(action_spec.action_dmac,
                        api_mirror_info->dst_mac.mac_addr,
                        SWITCH_MAC_LENGTH);
          action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
          action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;
          action_spec.action_ttl = api_mirror_info->ttl;
          action_spec.action_tos = api_mirror_info->tos;
          action_spec.action_vlan_id = api_mirror_info->vlan_id;
          action_spec.action_vlan_tpid = api_mirror_info->vlan_tpid;
          action_spec.action_cos = api_mirror_info->cos;

          pd_status =
              p4_pd_dc_mirror_table_modify_with_ipv4_erspan_t3_rewrite_with_eth_hdr_and_vlan_tag(
                  switch_cfg_sess_hdl,
                  p4_pd_device.device_id,
                  mirror_info->pd_hdl,
                  &action_spec);
          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
            pd_entry.match_spec_size = 0;
            pd_entry.action_spec = (switch_uint8_t *)&action_spec;
            pd_entry.action_spec_size = sizeof(action_spec);
            pd_entry.pd_hdl = mirror_info->pd_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }

          if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
            SWITCH_PD_LOG_ERROR(
                "mirror table update failed "
                "on device %d : table %s action %s\n",
                device,
                switch_pd_table_id_to_string(0),
                switch_pd_action_id_to_string(0));
            goto cleanup;
          }
        }
#endif /* P4_MIRROR_NEXTHOP_DISABLE */
      }
    } break;
    case SWITCH_MIRROR_TYPE_DTEL_REPORT: {
#ifdef P4_DTEL_REPORT_ENABLE
      p4_pd_dc_dtel_mirror_encap_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      SWITCH_MEMCPY(action_spec.action_smac,
                    api_mirror_info->src_mac.mac_addr,
                    SWITCH_MAC_LENGTH);
      action_spec.action_sip = api_mirror_info->src_ip.ip.v4addr;
      action_spec.action_dip = api_mirror_info->dst_ip.ip.v4addr;
      pd_status = p4_pd_dc_mirror_table_modify_with_dtel_mirror_encap(
          switch_cfg_sess_hdl,
          p4_pd_device.device_id,
          mirror_info->pd_hdl,
          &action_spec);

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "mirror table update failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
#endif /* P4_DTEL_REPORT_ENABLE */
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MIRROR_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror table entry update success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mirror_table_entry_delete(
    switch_device_t device, switch_mirror_info_t *mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(mirror_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MIRROR_DISABLE

  switch_api_mirror_info_t *api_mirror_info = NULL;
  api_mirror_info = &mirror_info->api_mirror_info;

  switch (api_mirror_info->mirror_type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      break;
    case SWITCH_MIRROR_TYPE_REMOTE:
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE:
    case SWITCH_MIRROR_TYPE_DTEL_REPORT:
      pd_status = p4_pd_dc_mirror_table_delete(
          switch_cfg_sess_hdl, device, mirror_info->pd_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = mirror_info->pd_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "mirror table entry delete failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
      break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MIRROR_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror table entry delete success "
        "on device %d 0x%lx\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mirror_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MIRROR_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_mirror_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mirror table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MIRROR_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mirror table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mirror table entry default add failed "
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
