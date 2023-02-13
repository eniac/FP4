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
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_system_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_system_key_value_pair_t *system_acl,
    switch_acl_system_action_t action_type,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(system_acl);
  UNUSED(action_type);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_system_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
  bool copy_only = false;
  switch_qid_t queue_id = 0;
  switch_cos_t icos = 0;
  switch_meter_id_t meter_id = 0;
  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_system_acl_match_spec_t));

  if (opt_action_params) {
    queue_id = opt_action_params->queue_id;
    icos = opt_action_params->ingress_cos;
    meter_id = handle_to_id(opt_action_params->meter_handle);
    stats_index = handle_to_id(opt_action_params->counter_handle);
    UNUSED(stats_index);
  }

  for (i = 0; i < count; i++) {
    switch (system_acl[i].field) {
      case SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = system_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
#ifndef P4_URPF_DISABLE
      case SWITCH_ACL_SYSTEM_FIELD_URPF_CHECK:
        match_spec.l3_metadata_urpf_check_fail =
            system_acl[i].value.urpf_check_fail;
        match_spec.l3_metadata_urpf_check_fail_mask =
            system_acl[i].mask.u.mask & 0xFF;
        break;
#endif /* P4_URPF_DISABLE */
      case SWITCH_ACL_SYSTEM_FIELD_PORT_VLAN_MAPPING_MISS:
        match_spec.l2_metadata_port_vlan_mapping_miss =
            system_acl[i].value.port_vlan_mapping_miss;
        match_spec.l2_metadata_port_vlan_mapping_miss_mask =
            system_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_ACL_DENY:
        match_spec.acl_metadata_acl_deny = system_acl[i].value.acl_deny;
        match_spec.acl_metadata_acl_deny_mask =
            system_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_L3_COPY:
        copy_only = true;
        match_spec.l3_metadata_l3_copy = system_acl[i].value.l3_copy;
        match_spec.l3_metadata_l3_copy_mask = system_acl[i].mask.u.mask & 0xFF;
        break;
#ifndef P4_IPSG_DISABLE
      case SWITCH_ACL_SYSTEM_FIELD_IPSG_CHECK:
        match_spec.security_metadata_ipsg_check_fail =
            system_acl[i].value.ipsg_check;
        match_spec.security_metadata_ipsg_check_fail_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#endif /* P4_IPSG_DISABLE */
#ifndef P4_RACL_DISABLE
      case SWITCH_ACL_SYSTEM_FIELD_RACL_DENY:
        match_spec.acl_metadata_racl_deny = system_acl[i].value.racl_deny;
        match_spec.acl_metadata_racl_deny_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#endif /* P4_RACL_DISABLE */
      case SWITCH_ACL_SYSTEM_FIELD_DROP:
        match_spec.ingress_metadata_drop_flag = system_acl[i].value.drop_flag;
        match_spec.ingress_metadata_drop_flag_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#if defined(P4_QOS_METERING_ENABLE)
      case SWITCH_ACL_SYSTEM_FIELD_METER_DROP:
        match_spec.meter_metadata_meter_drop = system_acl[i].value.meter_drop;
        match_spec.meter_metadata_meter_drop_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#endif /* P4_QOS_METERING_ENABLE */
      case SWITCH_ACL_SYSTEM_FIELD_ROUTED:
        match_spec.l3_metadata_routed = system_acl[i].value.routed;
        match_spec.l3_metadata_routed_mask = system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_LINK_LOCAL:
        match_spec.ipv6_metadata_ipv6_src_is_link_local =
            system_acl[i].value.src_is_link_local;
        match_spec.ipv6_metadata_ipv6_src_is_link_local_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_NEXTHOP_GLEAN:
        match_spec.nexthop_metadata_nexthop_glean =
            system_acl[i].value.nexthop_glean;
        match_spec.nexthop_metadata_nexthop_glean_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#ifndef P4_L3_MULTICAST_DISABLE
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_HIT:
        match_spec.multicast_metadata_mcast_route_hit =
            system_acl[i].value.mcast_route_hit;
        match_spec.multicast_metadata_mcast_route_hit_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_ROUTE_S_G_HIT:
        match_spec.multicast_metadata_mcast_route_s_g_hit =
            system_acl[i].value.mcast_route_s_g_hit;
        match_spec.multicast_metadata_mcast_route_s_g_hit_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_RPF_FAIL:
        match_spec.multicast_metadata_mcast_rpf_fail =
            system_acl[i].value.mcast_rpf_fail;
        match_spec.multicast_metadata_mcast_rpf_fail_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_MCAST_COPY_TO_CPU:
        match_spec.multicast_metadata_mcast_copy_to_cpu =
            system_acl[i].value.mcast_copy_to_cpu;
        match_spec.multicast_metadata_mcast_copy_to_cpu_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
#endif /* P4_L3_MULTICAST_DISABLE */
      case SWITCH_ACL_SYSTEM_FIELD_BD_CHECK:
        match_spec.l3_metadata_same_bd_check = system_acl[i].value.bd_check;
        match_spec.l3_metadata_same_bd_check_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_IF_CHECK:
        match_spec.l2_metadata_same_if_check = system_acl[i].value.if_check;
        match_spec.l2_metadata_same_if_check_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_TUNNEL_IF_CHECK:
        match_spec.tunnel_metadata_tunnel_if_check =
            system_acl[i].value.tunnel_if_check;
        match_spec.tunnel_metadata_tunnel_if_check_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_TTL:
        match_spec.l3_metadata_lkp_ip_ttl = system_acl[i].value.ttl;
        match_spec.l3_metadata_lkp_ip_ttl_mask =
            system_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_EGRESS_IFINDEX:
        match_spec.ingress_metadata_egress_ifindex =
            system_acl[i].value.out_ifindex;
        match_spec.ingress_metadata_egress_ifindex_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_INGRESS_IFINDEX:
        match_spec.ingress_metadata_ifindex =
            system_acl[i].value.ingress_ifindex;
        match_spec.ingress_metadata_ifindex_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_STP_STATE:
        match_spec.l2_metadata_stp_state = system_acl[i].value.stp_state;
        match_spec.l2_metadata_stp_state_mask =
            system_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED:
        match_spec.ipv4_metadata_ipv4_unicast_enabled =
            system_acl[i].value.ipv4_enabled;
        match_spec.ipv4_metadata_ipv4_unicast_enabled_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED:
        match_spec.ipv6_metadata_ipv6_unicast_enabled =
            system_acl[i].value.ipv6_enabled;
        match_spec.ipv6_metadata_ipv6_unicast_enabled_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT:
        match_spec.l3_metadata_rmac_hit = system_acl[i].value.rmac_hit;
        match_spec.l3_metadata_rmac_hit_mask = system_acl[i].mask.u.mask & 0x1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_REASON_CODE:
        copy_only = true;
        match_spec.fabric_metadata_reason_code =
            system_acl[i].value.reason_code;
        match_spec.fabric_metadata_reason_code_mask =
            system_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_MIRROR_ON_DROP:
#ifdef P4_DTEL_DROP_REPORT_ENABLE
        match_spec.dtel_md_mod_watchlist_hit =
            system_acl[i].value.mirror_on_drop;
        match_spec.dtel_md_mod_watchlist_hit_mask =
            system_acl[i].mask.u.mask & 0x1;
#endif
        break;
      case SWITCH_ACL_SYSTEM_FIELD_DROP_CTL:
#ifdef P4_DTEL_DROP_REPORT_ENABLE
        match_spec.ig_intr_md_for_tm_drop_ctl = system_acl[i].value.drop_ctl;
        match_spec.ig_intr_md_for_tm_drop_ctl_mask =
            system_acl[i].mask.u.mask & 0x7;
#endif
        break;
      case SWITCH_ACL_SYSTEM_FIELD_STORM_CONTROL_COLOR:
#ifndef P4_STORM_CONTROL_DISABLE
        if (system_acl[i].value.storm_control_color == SWITCH_COLOR_RED) {
          match_spec.meter_metadata_storm_control_color = 3;
        } else {
          match_spec.meter_metadata_storm_control_color =
              system_acl[i].value.storm_control_color;
        }
        match_spec.meter_metadata_storm_control_color_mask = 0x3;
#endif /* P4_STORM_CONTROL_DISABLE */
        break;
      case SWITCH_ACL_SYSTEM_FIELD_L2_DST_MISS:
        match_spec.l2_metadata_l2_dst_miss = system_acl[i].value.l2_dst_miss;
        match_spec.l2_metadata_l2_dst_miss_mask = 1;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE:
        match_spec.l2_metadata_lkp_pkt_type = system_acl[i].value.packet_type;
        match_spec.l2_metadata_lkp_pkt_type_mask = 0x7;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            system_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = system_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = system_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = system_acl[i].mask.u.mask;
      case SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE:
        match_spec.l2_metadata_arp_opcode = system_acl[i].value.arp_opcode;
        match_spec.l2_metadata_arp_opcode_mask = 0x3;
        break;
      case SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP:
        match_spec.l3_metadata_fib_hit_myip = system_acl[i].value.fib_hit_myip;
        match_spec.l3_metadata_fib_hit_myip_mask =
            system_acl[i].mask.u.mask & 0x1;
        break;
      default:
        break;
    }
  }

  switch (action_type) {
    case SWITCH_ACL_ACTION_NOP:
    case SWITCH_ACL_ACTION_PERMIT:
      pd_status = p4_pd_dc_system_acl_table_add_with_nop(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
      if (copy_only) {
        p4_pd_dc_redirect_to_cpu_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        pd_status = p4_pd_dc_system_acl_table_add_with_redirect_to_cpu(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      } else {
        p4_pd_dc_redirect_to_cpu_with_reason_action_spec_t action_spec;
        memset(&action_spec, 0, sizeof(p4_pd_dc_redirect_to_cpu_action_spec_t));
        action_spec.action_reason_code =
            action_params->cpu_redirect.reason_code;
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        pd_status =
            p4_pd_dc_system_acl_table_add_with_redirect_to_cpu_with_reason(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                priority,
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
      }
      break;
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      if (copy_only) {
        p4_pd_dc_copy_to_cpu_action_spec_t action_spec;
        memset(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        pd_status =
            p4_pd_dc_system_acl_table_add_with_copy_to_cpu(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           priority,
                                                           &action_spec,
                                                           entry_hdl);
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&match_spec;
          pd_entry.match_spec_size = sizeof(match_spec);
          pd_entry.action_spec_size = 0;
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_copy_to_cpu_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        action_spec.action_reason_code =
            action_params->cpu_redirect.reason_code;
        pd_status = p4_pd_dc_system_acl_table_add_with_copy_to_cpu_with_reason(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      }
    } break;
    case SWITCH_ACL_ACTION_DROP:
      if (action_params->drop.reason_code) {
        p4_pd_dc_drop_packet_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_drop_reason = action_params->drop.reason_code;
        pd_status = p4_pd_dc_system_acl_table_add_with_drop_packet_with_reason(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      } else {
        pd_status =
            p4_pd_dc_system_acl_table_add_with_drop_packet(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           priority,
                                                           entry_hdl);
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&match_spec;
          pd_entry.match_spec_size = sizeof(match_spec);
          pd_entry.action_spec_size = 0;
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
      break;
    case SWITCH_ACL_ACTION_MIRROR_AND_DROP:
#ifdef P4_DTEL_DROP_REPORT_ENABLE
      if (action_params->drop.reason_code) {
        p4_pd_dc_mirror_and_drop_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_drop_reason = action_params->drop.reason_code;
        pd_status =
            p4_pd_dc_system_acl_table_add_with_mirror_and_drop_with_reason(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                priority,
                &action_spec,
                entry_hdl);
      } else {
        pd_status = p4_pd_dc_system_acl_table_add_with_mirror_and_drop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
            entry_hdl);
      }
#endif /* P4_DTEL_DROP_REPORT_ENABLE */
      break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "system acl table add failed "
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
        "system acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "system acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_system_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_system_key_value_pair_t *system_acl,
    switch_acl_system_action_t action_type,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(system_acl);
  UNUSED(action_type);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;
  bool copy_only = false;
  switch_qid_t queue_id = 0;
  switch_cos_t icos = 0;
  switch_meter_id_t meter_id = 0;
  switch_stats_id_t stats_index = 0;

  if (opt_action_params) {
    queue_id = opt_action_params->queue_id;
    icos = opt_action_params->ingress_cos;
    meter_id = handle_to_id(opt_action_params->meter_handle);
    stats_index = handle_to_id(opt_action_params->counter_handle);
    UNUSED(meter_id);
    UNUSED(stats_index);
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (action_type) {
    case SWITCH_ACL_ACTION_NOP:
    case SWITCH_ACL_ACTION_PERMIT:
      pd_status = p4_pd_dc_system_acl_table_modify_with_nop(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
      if (copy_only) {
        p4_pd_dc_redirect_to_cpu_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        pd_status = p4_pd_dc_system_acl_table_modify_with_redirect_to_cpu(
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
      } else {
        p4_pd_dc_redirect_to_cpu_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_reason_code =
            action_params->cpu_redirect.reason_code;
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
        pd_status =
            p4_pd_dc_system_acl_table_modify_with_redirect_to_cpu_with_reason(
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
      }
      break;
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      if (copy_only) {
        p4_pd_dc_copy_to_cpu_action_spec_t action_spec;
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        pd_status = p4_pd_dc_system_acl_table_modify_with_copy_to_cpu(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl,
            &action_spec);
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec_size = 0;
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_copy_to_cpu_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_qid = queue_id;
        action_spec.action_icos = icos;
#ifndef P4_COPP_METER_DISABLE
        action_spec.action_meter_id = meter_id;
#endif /* P4_COPP_METER_DISABLE */
        action_spec.action_reason_code =
            action_params->cpu_redirect.reason_code;
        pd_status =
            p4_pd_dc_system_acl_table_modify_with_copy_to_cpu_with_reason(
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
      }
    } break;
    case SWITCH_ACL_ACTION_DROP:
      if (action_params->drop.reason_code) {
        p4_pd_dc_drop_packet_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_drop_reason = action_params->drop.reason_code;
        pd_status =
            p4_pd_dc_system_acl_table_modify_with_drop_packet_with_reason(
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
      } else {
        pd_status = p4_pd_dc_system_acl_table_modify_with_drop_packet(
            switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec_size = 0;
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
      break;
    case SWITCH_ACL_ACTION_MIRROR_AND_DROP:
#ifdef P4_DTEL_DROP_REPORT_ENABLE
      if (action_params->drop.reason_code) {
        p4_pd_dc_mirror_and_drop_with_reason_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_drop_reason = action_params->drop.reason_code;
        pd_status =
            p4_pd_dc_system_acl_table_modify_with_mirror_and_drop_with_reason(
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
      } else {
        pd_status = p4_pd_dc_system_acl_table_modify_with_mirror_and_drop(
            switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec_size = 0;
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
#endif /* P4_DTEL_DROP_REPORT_ENABLE */
      break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "system acl table update failed "
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
        "system acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "system acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_system_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status =
      p4_pd_dc_system_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

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
        "system acl table entry delete failed "
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
        "ssytem acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "system acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv4_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ip_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 acl entry add failed on device %d : %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ip_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (ip_acl[i].field) {
      case SWITCH_ACL_IP_FIELD_IPV4_SRC:
        match_spec.ipv4_metadata_lkp_ipv4_sa = ip_acl[i].value.ipv4_source;
        match_spec.ipv4_metadata_lkp_ipv4_sa_mask =
            ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_IPV4_DEST:
        match_spec.ipv4_metadata_lkp_ipv4_da = ip_acl[i].value.ipv4_dest;
        match_spec.ipv4_metadata_lkp_ipv4_da_mask =
            ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        ;
        break;
      case SWITCH_ACL_IP_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ip_acl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask = ip_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_ICMP_TYPE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        break;
      case SWITCH_ACL_IP_FIELD_ICMP_CODE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        break;
      case SWITCH_ACL_IP_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ip_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ip_acl[i].mask.u.mask & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ip_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ip_acl[i].mask.u.mask & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IP_FIELD_TTL:
        match_spec.l3_metadata_lkp_ip_ttl = ip_acl[i].value.ttl;
        match_spec.l3_metadata_lkp_ip_ttl_mask = ip_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label = ip_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = ip_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = ip_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = ip_acl[i].mask.u.mask;
        break;
#if defined(P4_ETYPE_IN_IP_ACL_KEY_ENABLE)
      case SWITCH_ACL_IP_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = ip_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            ip_acl[i].mask.u.mask & 0xFFFF;
        break;
#endif /* P4_ETYPE_IN_IP_ACL_KEY_ENABLE */
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_RMAC_HIT:
        match_spec.l3_metadata_rmac_hit = ip_acl[i].value.rmac_hit;
        match_spec.l3_metadata_rmac_hit_mask = ip_acl[i].mask.u.mask & 0xFF;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_add_with_acl_deny(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &match_spec,
                                                          priority,
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            priority,
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
        action_spec.action_acl_meter_index = meter_index;
#endif
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        if (opt_action_params) {
          action_spec.action_nat_mode = opt_action_params->nat_mode;
        }
        pd_status = p4_pd_dc_ip_acl_table_add_with_acl_redirect_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      }
    } break;
    case SWITCH_ACL_ACTION_SET_MIRROR: {
#ifndef P4_MIRROR_DISABLE
#ifdef P4_INGRESS_ACL_ACTION_MIRROR_ENABLE
      p4_pd_dc_acl_mirror_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_add_with_acl_mirror(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            priority,
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
#endif /* P4_INGRESS_ACL_ACTION_MIRROR_ENABLE */
#endif /* P4_MIRROR_DISABLE */
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ip_acl_table_add_with_acl_redirect_nexthop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_egress_ipv4_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_EGRESS_ACL_ENABLE
#if !defined(P4_IPV4_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_ip_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  for (i = 0; i < count; i++) {
    switch (ip_acl[i].field) {
      case SWITCH_ACL_IP_FIELD_IPV4_SRC:
        match_spec.ipv4_srcAddr = ip_acl[i].value.ipv4_source;
        match_spec.ipv4_srcAddr_mask = ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_IPV4_DEST:
        match_spec.ipv4_dstAddr = ip_acl[i].value.ipv4_dest;
        match_spec.ipv4_dstAddr_mask = ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_IP_PROTO:
        match_spec.ipv4_protocol = ip_acl[i].value.ip_proto;
        match_spec.ipv4_protocol_mask = ip_acl[i].mask.u.mask & 0xFF;
        break;
#ifdef P4_EGRESS_ACL_RANGE_DISABLE
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        match_spec.l3_metadata_egress_l4_sport = ip_acl[i].value.l4_source_port;
        match_spec.l3_metadata_egress_l4_sport_mask =
            ip_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
        match_spec.l3_metadata_egress_l4_dport = ip_acl[i].value.l4_dest_port;
        match_spec.l3_metadata_egress_l4_dport_mask =
            ip_acl[i].mask.u.mask & 0xFFFF;
        break;
#else
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_egress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_egress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_egress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_egress_dst_port_range_id =
            handle_to_id(ip_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_egress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_egress_dst_port_range_id_mask = 0xFF;
        }
        break;
#endif /* P4_EGRESS_ACL_RANGE_DISABLE */
      case SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            ip_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            ip_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            ip_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask = ip_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ip_acl_table_add_with_egress_acl_deny(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv4 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ip_acl_table_add_with_egress_acl_permit(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv4 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* P4_EGRESS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 egress acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 egress acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE)
  p4_pd_dev_target_t p4_pd_device;

  switch_stats_id_t stats_index = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 acl entry update failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    switch_meter_id_t meter_index = SWITCH_API_INVALID_HANDLE;
    if (opt_action_params->meter_handle != SWITCH_API_INVALID_HANDLE) {
      meter_index = handle_to_id(opt_action_params->meter_handle);
    }
#endif /* P4_ACL_QOS_ENABLE */
    if (opt_action_params->counter_handle != SWITCH_API_INVALID_HANDLE) {
      stats_index = handle_to_id(opt_action_params->counter_handle);
      UNUSED(stats_index);
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
        action_spec.action_acl_meter_index = meter_index;
#endif
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        if (opt_action_params) {
          action_spec.action_nat_mode = opt_action_params->nat_mode;
        }
        pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_redirect_nexthop(
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
      }
    } break;
    case SWITCH_ACL_ACTION_SET_MIRROR: {
#ifndef P4_MIRROR_DISABLE
#ifdef P4_INGRESS_ACL_ACTION_MIRROR_ENABLE
      p4_pd_dc_acl_mirror_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_mirror(
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
#endif /* P4_INGRESS_ACL_ACTION_MIRROR_ENABLE */
#endif /* P4_MIRROR_DISABLE */
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_egress_ipv4_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_EGRESS_ACL_ENABLE
#if !defined(P4_IPV4_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_ip_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  for (i = 0; i < count; i++) {
    switch (ip_acl[i].field) {
      case SWITCH_ACL_IP_FIELD_IPV4_SRC:
        match_spec.ipv4_srcAddr = ip_acl[i].value.ipv4_source;
        match_spec.ipv4_srcAddr_mask = ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_IPV4_DEST:
        match_spec.ipv4_dstAddr = ip_acl[i].value.ipv4_dest;
        match_spec.ipv4_dstAddr_mask = ip_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_IP_PROTO:
        match_spec.ipv4_protocol = ip_acl[i].value.ip_proto;
        match_spec.ipv4_protocol_mask = ip_acl[i].mask.u.mask & 0xFF;
        break;
#ifdef P4_EGRESS_ACL_RANGE_DISABLE
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT:
        match_spec.l3_metadata_egress_l4_sport = ip_acl[i].value.l4_source_port;
        match_spec.l3_metadata_egress_l4_sport_mask =
            ip_acl[i].mask.u.mask & 0xFFFF;
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT:
        match_spec.l3_metadata_egress_l4_dport = ip_acl[i].value.l4_dest_port;
        match_spec.l3_metadata_egress_l4_dport_mask =
            ip_acl[i].mask.u.mask & 0xFFFF;
        break;
#else
      case SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_egress_src_port_range_id =
            handle_to_id(ip_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_egress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_egress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_egress_dst_port_range_id =
            handle_to_id(ip_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_egress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_egress_dst_port_range_id_mask = 0xFF;
        }
        break;
#endif /* P4_EGRESS_ACL_RANGE_DISABLE */
      case SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            ip_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            ip_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_IP_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            ip_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask =
            ip_acl[i].mask.u.mask & 0xFF;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ip_acl_table_modify_with_egress_acl_deny(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv4 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ip_acl_table_modify_with_egress_acl_permit(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv4 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* P4_EGRESS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 egress acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 egress acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE)

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    pd_status =
        p4_pd_dc_ip_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  } else {
#ifdef EGRESS_ACL_ENABLE
    pd_status = p4_pd_dc_egress_ip_acl_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
#endif /* EGRESS_ACL_ENABLE */
  }

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
        "ipv4 acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv6_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv6_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 acl entry add failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv6_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (ipv6_acl[i].field) {
      case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_sa,
                      ipv6_acl[i].value.ipv6_source.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_sa_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_da,
                      ipv6_acl[i].value.ipv6_dest.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_da_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ipv6_acl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_ICMP_TYPE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        break;
      case SWITCH_ACL_IPV6_FIELD_ICMP_CODE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        break;
      case SWITCH_ACL_IPV6_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ipv6_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ipv6_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IPV6_FIELD_TTL:
        break;
      case SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            ipv6_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = ipv6_acl[i].mask.u.mask16;
        break;
      case SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = ipv6_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = ipv6_acl[i].mask.u.mask16;
        break;
#if defined(P4_ETYPE_IN_IP_ACL_KEY_ENABLE)
      case SWITCH_ACL_IPV6_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = ipv6_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            ipv6_acl[i].mask.u.mask16 & 0xFFFF;
        break;
#endif /* P4_ETYPE_IN_IP_ACL_KEY_ENABLE */
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_RMAC_HIT:
        match_spec.l3_metadata_rmac_hit = ipv6_acl[i].value.rmac_hit;
        match_spec.l3_metadata_rmac_hit_mask =
            ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_ipv6_acl_table_add_with_acl_deny(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &match_spec,
                                                            priority,
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status =
          p4_pd_dc_ipv6_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      priority,
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
        action_spec.action_acl_meter_index = meter_index;
#endif
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv6_acl_table_add_with_acl_redirect_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv6_acl_table_add_with_acl_redirect_nexthop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv6_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_ipv6_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)
#ifdef P4_EGRESS_ACL_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_ipv6_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  for (i = 0; i < count; i++) {
    switch (ipv6_acl[i].field) {
      case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        SWITCH_MEMCPY(match_spec.ipv6_srcAddr,
                      ipv6_acl[i].value.ipv6_source.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_srcAddr_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
        SWITCH_MEMCPY(match_spec.ipv6_dstAddr,
                      ipv6_acl[i].value.ipv6_dest.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_dstAddr_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
        match_spec.ipv6_nextHdr = ipv6_acl[i].value.ip_proto;
        match_spec.ipv6_nextHdr_mask =
            ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
#ifdef P4_EGRESS_ACL_RANGE_DISABLE
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT:
        match_spec.l3_metadata_egress_l4_sport =
            ipv6_acl[i].value.l4_source_port;
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT:
        match_spec.l3_metadata_egress_l4_dport = ipv6_acl[i].value.l4_dest_port;
        break;
#else
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_egress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_egress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_egress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_egress_dst_port_range_id =
            handle_to_id(ipv6_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_egress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_egress_dst_port_range_id_mask = 0xFF;
        }
        break;
#endif /* P4_EGRESS_ACL_RANGE_DISABLE */
      case SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            ipv6_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            ipv6_acl[i].mask.u.mask16;
        break;
      case SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            ipv6_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask =
            ipv6_acl[i].mask.u.mask16;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ipv6_acl_table_add_with_egress_acl_deny(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ipv6_acl_table_add_with_egress_acl_permit(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_ACL_ENABLE */
#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress ipv6 acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress ipv6 acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 acl entry update failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
        action_spec.action_acl_meter_index = meter_index;
#endif
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_redirect_nexthop(
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_ipv6_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)
#ifdef P4_EGRESS_ACL_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_ipv6_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  for (i = 0; i < count; i++) {
    switch (ipv6_acl[i].field) {
      case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        SWITCH_MEMCPY(match_spec.ipv6_srcAddr,
                      ipv6_acl[i].value.ipv6_source.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_srcAddr_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IPV6_DEST:
        SWITCH_MEMCPY(match_spec.ipv6_dstAddr,
                      ipv6_acl[i].value.ipv6_dest.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_dstAddr_mask,
                      ipv6_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_FIELD_IP_PROTO:
        match_spec.ipv6_nextHdr = ipv6_acl[i].value.ip_proto;
        match_spec.ipv6_nextHdr_mask =
            ipv6_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
#ifdef P4_EGRESS_ACL_RANGE_DISABLE
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT:
        match_spec.l3_metadata_egress_l4_sport =
            ipv6_acl[i].value.l4_source_port;
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT:
        match_spec.l3_metadata_egress_l4_dport = ipv6_acl[i].value.l4_dest_port;
        break;
#else
      case SWITCH_ACL_IPV6_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_egress_src_port_range_id =
            handle_to_id(ipv6_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_egress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_egress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_egress_dst_port_range_id =
            handle_to_id(ipv6_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_egress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_egress_dst_port_range_id_mask = 0xFF;
        }
        break;
#endif /* P4_EGRESS_ACL_RANGE_DISABLE */
      case SWITCH_ACL_IPV6_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            ipv6_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            ipv6_acl[i].mask.u.mask16;
        break;
      case SWITCH_ACL_IPV6_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            ipv6_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask =
            ipv6_acl[i].mask.u.mask16;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ipv6_acl_table_modify_with_egress_acl_deny(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_ipv6_acl_table_modify_with_egress_acl_permit(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_ACL_ENABLE */
#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress ipv6 acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress ipv6 acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    pd_status =
        p4_pd_dc_ipv6_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  } else {
#ifdef EGRESS_ACL_ENABLE
    pd_status = p4_pd_dc_egress_ipv6_acl_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
#endif /* EGRESS_ACL_ENABLE */
  }

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
        "ipv6 acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv4_racl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_racl_key_value_pair_t *ip_racl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_racl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV4_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv4_racl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 racl entry add failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv4_racl_match_spec_t));

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
    UNUSED(meter_index);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  for (i = 0; i < count; i++) {
    switch (ip_racl[i].field) {
      case SWITCH_ACL_IP_RACL_FIELD_IPV4_SRC:
        match_spec.ipv4_metadata_lkp_ipv4_sa = ip_racl[i].value.ipv4_source;
        match_spec.ipv4_metadata_lkp_ipv4_sa_mask =
            ip_racl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_RACL_FIELD_IPV4_DEST:
        match_spec.ipv4_metadata_lkp_ipv4_da = ip_racl[i].value.ipv4_dest;
        match_spec.ipv4_metadata_lkp_ipv4_da_mask =
            ip_racl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_RACL_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ip_racl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ip_racl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_RACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ip_racl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ip_racl[i].mask.u.mask & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ip_racl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ip_racl[i].mask.u.mask & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IP_RACL_FIELD_TTL:
        match_spec.l3_metadata_lkp_ip_ttl = ip_racl[i].value.ttl;
        match_spec.l3_metadata_lkp_ip_ttl_mask = ip_racl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_RACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_racl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_racl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_RACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_racl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_racl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_RACL_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = ip_racl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = ip_racl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_racl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv4_racl_table_add_with_racl_deny(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      priority,
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv4_racl_table_add_with_racl_permit(switch_cfg_sess_hdl,
                                                        p4_pd_device,
                                                        &match_spec,
                                                        priority,
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv4_racl_table_add_with_racl_redirect_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv4_racl_table_add_with_racl_redirect_nexthop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifndef P4_RACL_REASON_CODE_DISABLE
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#endif /* P4_RACL_REASON_CODE_DISABLE */
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv4_racl_table_add_with_racl_permit(switch_cfg_sess_hdl,
                                                        p4_pd_device,
                                                        &match_spec,
                                                        priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 racl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_RACL_DISABLE && P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 racl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 racl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_racl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_racl_key_value_pair_t *ip_racl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_racl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_RACL_DISABLE) && !defined(P4_IPV4_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 racl entry update failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
    UNUSED(meter_index);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_racl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv4_racl_table_modify_with_racl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv4_racl_table_modify_with_racl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv4_racl_table_modify_with_racl_redirect_nexthop(
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv4_racl_table_modify_with_racl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifndef P4_RACL_REASON_CODE_DISABLE
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#endif /* P4_RACL_REASON_CODE_DISABLE */
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv4_racl_table_modify_with_racl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 racl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_RACL_DISABLE && P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 racl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 racl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_racl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE) && !defined(P4_RACL_DISABLE)

  pd_status =
      p4_pd_dc_ipv4_racl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

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
        "ipv4 racl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE & P4_RACL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 racl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 racl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv6_racl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_racl_key_value_pair_t *ipv6_racl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_racl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && !defined(P4_RACL_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv6_racl_match_spec_t match_spec;
  unsigned int i = 0;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 racl entry add failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv6_racl_match_spec_t));

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
    UNUSED(meter_index);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  for (i = 0; i < count; i++) {
    switch (ipv6_racl[i].field) {
      case SWITCH_ACL_IPV6_RACL_FIELD_IPV6_SRC:
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_IPV6_DEST:
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ipv6_racl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ipv6_racl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_racl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_racl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_racl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_racl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ipv6_racl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ipv6_racl[i].mask.u.mask.u.addr8[0] & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ipv6_racl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ipv6_racl[i].mask.u.mask.u.addr8[0] & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_TTL:
        match_spec.l3_metadata_lkp_ip_ttl = ipv6_racl[i].value.ttl;
        match_spec.l3_metadata_lkp_ip_ttl_mask =
            ipv6_racl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      case SWITCH_ACL_IPV6_RACL_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = ipv6_racl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = ipv6_racl[i].mask.u.mask16;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_racl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv6_racl_table_add_with_racl_deny(switch_cfg_sess_hdl,
                                                      p4_pd_device,
                                                      &match_spec,
                                                      priority,
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv6_racl_table_add_with_racl_permit(switch_cfg_sess_hdl,
                                                        p4_pd_device,
                                                        &match_spec,
                                                        priority,
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv6_racl_table_add_with_racl_redirect_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            priority,
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv6_racl_table_add_with_racl_redirect_nexthop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifndef P4_RACL_REASON_CODE_DISABLE
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#endif /* P4_RACL_REASON_CODE_DISABLE */
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_ipv6_racl_table_add_with_racl_permit(switch_cfg_sess_hdl,
                                                        p4_pd_device,
                                                        &match_spec,
                                                        priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 racl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE & P4_RACL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 racl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 racl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv6_racl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_racl_key_value_pair_t *ipv6_racl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_racl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && !defined(P4_RACL_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 racl entry add failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
    UNUSED(meter_index);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_racl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_racl_table_modify_with_racl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_racl_table_modify_with_racl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv6_racl_table_modify_with_racl_redirect_nexthop(
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_racl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv6_racl_table_modify_with_racl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_racl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifndef P4_RACL_REASON_CODE_DISABLE
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#endif /* P4_RACL_REASON_CODE_DISABLE */
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_racl_table_modify_with_racl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 racl table update ailed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE & P4_RACL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 racl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 racl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}
switch_status_t switch_pd_ipv6_racl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && !defined(P4_RACL_DISABLE)

  pd_status =
      p4_pd_dc_ipv6_racl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
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
        "ipv6 racl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE & P4_RACL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 racl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 racl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_mirror_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE) && defined(P4_MIRROR_ACL_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv4_mirror_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv4_mirror_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (ip_mirror_acl[i].field) {
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_SRC:
        match_spec.ipv4_metadata_lkp_ipv4_sa =
            ip_mirror_acl[i].value.ipv4_source;
        match_spec.ipv4_metadata_lkp_ipv4_sa_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_DEST:
        match_spec.ipv4_metadata_lkp_ipv4_da = ip_mirror_acl[i].value.ipv4_dest;
        match_spec.ipv4_metadata_lkp_ipv4_da_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFFFFFFFF;
        ;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ip_mirror_acl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_mirror_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_mirror_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_mirror_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_mirror_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ip_mirror_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ip_mirror_acl[i].mask.u.mask & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ip_mirror_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_IP_DSCP:
        match_spec.l3_metadata_lkp_dscp = (ip_mirror_acl[i].value.ip_dscp << 2);
        match_spec.l3_metadata_lkp_dscp_mask =
            (ip_mirror_acl[i].mask.u.mask << 2) & 0xFF;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            ip_mirror_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask =
            ip_mirror_acl[i].mask.u.mask;
        break;
#ifdef ROCEV2_MIRROR_ENABLE
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_OPCODE:
        match_spec.l3_metadata_rocev2_opcode =
            ip_mirror_acl[i].value.rocev2_opcode;
        match_spec.l3_metadata_rocev2_opcode_mask =
            ip_mirror_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_ACK_REQ_RSVD:
        match_spec.l3_metadata_rocev2_ack_req_rsvd =
            ip_mirror_acl[i].value.rocev2_ack_req_rsvd;
        match_spec.l3_metadata_rocev2_ack_req_rsvd_mask =
            ip_mirror_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_ROCEV2_DST_QP:
        match_spec.l3_metadata_rocev2_dst_qp =
            ip_mirror_acl[i].value.rocev2_dst_qp;
        match_spec.l3_metadata_rocev2_dst_qp_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFFFFFF;
        break;
#endif /* ROCEV2_MIRROR_ENABLE */
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_SOURCE_PORT:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_mirror_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_mirror_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_L4_DEST_PORT:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_mirror_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_mirror_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
#if defined(P4_ETYPE_IN_IP_ACL_KEY_ENABLE)
      case SWITCH_ACL_IP_MIRROR_ACL_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = ip_mirror_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            ip_mirror_acl[i].mask.u.mask & 0xFFFF;
        break;
#endif /* P4_ETYPE_IN_IP_ACL_KEY_ENABLE */
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_SET_MIRROR: {
      p4_pd_dc_mirror_acl_mirror_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_MIRROR_ACL_STATS_ENABLE
      action_spec.action_acl_stats_index = stats_index;
#endif /* P4_MIRROR_ACL_STATS_ENABLE */
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      pd_status = p4_pd_dc_ipv4_mirror_acl_table_add_with_mirror_acl_mirror(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 mirror acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV4_DISABLE && ACL_MIRROR_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 mirror acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 mirror acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_mirror_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_mirror_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && defined(P4_MIRROR_ACL_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv6_mirror_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv6_mirror_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (ipv6_mirror_acl[i].field) {
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_SRC:
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_sa,
                      ipv6_mirror_acl[i].value.ipv6_source.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_sa_mask,
                      ipv6_mirror_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_DEST:
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_da,
                      ipv6_mirror_acl[i].value.ipv6_dest.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(match_spec.ipv6_metadata_lkp_ipv6_da_mask,
                      ipv6_mirror_acl[i].mask.u.mask.u.addr8,
                      SWITCH_IPV6_PREFIX_LENGTH);
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ipv6_mirror_acl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ipv6_mirror_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_mirror_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_mirror_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_mirror_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_mirror_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ipv6_mirror_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask =
            ipv6_mirror_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags =
            ipv6_mirror_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ipv6_mirror_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IP_DSCP:
        match_spec.l3_metadata_lkp_dscp = ipv6_mirror_acl[i].value.ip_dscp;
        match_spec.l3_metadata_lkp_dscp_mask =
            ipv6_mirror_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            ipv6_mirror_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask =
            ipv6_mirror_acl[i].mask.u.mask16;
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_SOURCE_PORT:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_mirror_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_mirror_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_L4_DEST_PORT:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_mirror_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_mirror_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
#if defined(P4_ETYPE_IN_IP_ACL_KEY_ENABLE)
      case SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = ipv6_mirror_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            ipv6_mirror_acl[i].mask.u.mask16 & 0xFFFF;
        break;
#endif /* P4_ETYPE_IN_IP_ACL_KEY_ENABLE */
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_SET_MIRROR: {
      p4_pd_dc_mirror_acl_mirror_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_MIRROR_ACL_STATS_ENABLE
      action_spec.action_acl_stats_index = stats_index;
#endif /* P4_MIRROR_ACL_STATS_ENABLE */
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      pd_status = p4_pd_dc_ipv6_mirror_acl_table_add_with_mirror_acl_mirror(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 mirror acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_qos_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_qos_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE) && defined(P4_IPV4_QOS_ACL_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv4_qos_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_QOS_METERING_ENABLE */
  //  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv4_qos_acl_match_spec_t));

  if (opt_action_params) {
//    stats_index = handle_to_id(opt_action_params->counter_handle);
#if defined(P4_QOS_METERING_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_QOS_METERING_ENABLE */
  }

  for (i = 0; i < count; i++) {
    switch (ip_qos_acl[i].field) {
      case SWITCH_ACL_IP_QOS_ACL_FIELD_IPV4_SRC:
        match_spec.ipv4_metadata_lkp_ipv4_sa = ip_qos_acl[i].value.ipv4_source;
        match_spec.ipv4_metadata_lkp_ipv4_sa_mask =
            ip_qos_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_IPV4_DEST:
        match_spec.ipv4_metadata_lkp_ipv4_da = ip_qos_acl[i].value.ipv4_dest;
        match_spec.ipv4_metadata_lkp_ipv4_da_mask =
            ip_qos_acl[i].mask.u.mask & 0xFFFFFFFF;
        ;
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_IP_PROTO:
        match_spec.l3_metadata_lkp_ip_proto = ip_qos_acl[i].value.ip_proto;
        match_spec.l3_metadata_lkp_ip_proto_mask =
            ip_qos_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ip_qos_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ip_qos_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ip_qos_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ip_qos_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ip_qos_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask = ip_qos_acl[i].mask.u.mask & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ip_qos_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ip_qos_acl[i].mask.u.mask & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_IP_DSCP:
        match_spec.l3_metadata_lkp_dscp = ip_qos_acl[i].value.ip_dscp;
        match_spec.l3_metadata_lkp_dscp_mask = ip_qos_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IP_QOS_ACL_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            ip_qos_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = ip_qos_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv4_qos_acl_table_add_with_acl_set_qos_fields(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV4_DISABLE && ACL_QOS_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 qos acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_qos_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_qos_acl_key_value_pair_t *ipv6_qos_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_qos_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && defined(P4_IPV6_QOS_ACL_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ipv6_qos_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_QOS_METERING_ENABLE */
  //  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ipv6_qos_acl_match_spec_t));

  if (opt_action_params) {
//    stats_index = handle_to_id(opt_action_params->counter_handle);
#if defined(P4_QOS_METERING_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_QOS_METERING_ENABLE */
  }

  for (i = 0; i < count; i++) {
    switch (ipv6_qos_acl[i].field) {
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_SRC:
        match_spec.ipv6_metadata_lkp_ipv6_sa =
            ipv6_qos_acl[i].value.ipv6_source;
        match_spec.ipv6_metadata_lkp_ipv6_sa_mask =
            ipv6_qos_acl[i].mask.u.mask & 0xFFFFFFFF;
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_DEST:
        match_spec.ipv6_metadata_lkp_ipv6_da = ipv6_qos_acl[i].value.ipv6_dest;
        match_spec.ipv6_metadata_lkp_ipv6_da_mask =
            ipv6_qos_acl[i].mask.u.mask & 0xFFFFFFFF;
        ;
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_PROTO:
        match_spec.l3_metadata_lkp_ipv6_proto =
            ipv6_qos_acl[i].value.ipv6_proto;
        match_spec.l3_metadata_lkp_ipv6_proto_mask =
            ipv6_qos_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_L4_SOURCE_PORT_RANGE:
        match_spec.acl_metadata_ingress_src_port_range_id =
            handle_to_id(ipv6_qos_acl[i].value.sport_range_handle);
        match_spec.acl_metadata_ingress_src_port_range_id_mask = 0;
        if (handle_to_id(ipv6_qos_acl[i].value.sport_range_handle)) {
          match_spec.acl_metadata_ingress_src_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_L4_DEST_PORT_RANGE:
        match_spec.acl_metadata_ingress_dst_port_range_id =
            handle_to_id(ipv6_qos_acl[i].value.dport_range_handle);
        match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0;
        if (handle_to_id(ipv6_qos_acl[i].value.dport_range_handle)) {
          match_spec.acl_metadata_ingress_dst_port_range_id_mask = 0xFF;
        }
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_TCP_FLAGS:
#ifdef P4_TUNNEL_DISABLE
        match_spec.tcp_flags = ipv6_qos_acl[i].value.tcp_flags;
        match_spec.tcp_flags_mask =
            ipv6_qos_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#else
        match_spec.l3_metadata_lkp_tcp_flags = ipv6_qos_acl[i].value.tcp_flags;
        match_spec.l3_metadata_lkp_tcp_flags_mask =
            ipv6_qos_acl[i].mask.u.mask.u.addr8[0] & 0xFF;
#endif /* P4_TUNNEL_DISABLE */
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_IPV6_DSCP:
        match_spec.l3_metadata_lkp_dscp = ipv6_qos_acl[i].value.ipv6_dscp;
        match_spec.l3_metadata_lkp_dscp_mask =
            ipv6_qos_acl[i].mask.u.mask & 0xFF;
        break;
      case SWITCH_ACL_IPV6_QOS_ACL_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            ipv6_qos_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask =
            ipv6_qos_acl[i].mask.u.mask16;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_qos_acl_table_add_with_acl_set_qos_fields(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 qos acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV6_DISABLE && ACL_QOS_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 qos acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 qos acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mac_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if !defined(P4_INGRESS_MAC_ACL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_mac_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 acl entry add failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_mac_acl_match_spec_t));

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  for (i = 0; i < count; i++) {
    switch (mac_acl[i].field) {
      case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = mac_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            mac_acl[i].mask.u.mask16 & 0xFFFF;
        break;
      case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_sa,
                      &mac_acl[i].value.source_mac,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_sa_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_DEST_MAC:
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_da,
                      &mac_acl[i].value.dest_mac,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_da_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            mac_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = mac_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL:
        match_spec.acl_metadata_bd_label = mac_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_bd_label_mask = mac_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_mac_acl_table_add_with_acl_deny(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           priority,
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_mac_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                     p4_pd_device,
                                                     &match_spec,
                                                     priority,
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
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_mac_acl_table_add_with_acl_redirect_nexthop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status =
          p4_pd_dc_mac_acl_table_add_with_acl_permit(switch_cfg_sess_hdl,
                                                     p4_pd_device,
                                                     &match_spec,
                                                     priority,
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
    case SWITCH_ACL_ACTION_REDIRECT:
      break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mac acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mac_qos_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_mac_qos_acl_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if defined(P4_MAC_QOS_ACL_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_mac_qos_acl_match_spec_t match_spec;
  switch_uint32_t i = 0;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_QOS_METERING_ENABLE */
  //  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_mac_qos_acl_match_spec_t));

  if (opt_action_params) {
#if defined(P4_QOS_METERING_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_QOS_METERING_ENABLE */
  }

  for (i = 0; i < count; i++) {
    switch (mac_acl[i].field) {
      case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
        match_spec.l2_metadata_lkp_mac_type = mac_acl[i].value.eth_type;
        match_spec.l2_metadata_lkp_mac_type_mask =
            mac_acl[i].mask.u.mask16 & 0xFFFF;
        break;
      case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_sa,
                      &mac_acl[i].value.source_mac,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_sa_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_DEST_MAC:
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_da,
                      &mac_acl[i].value.dest_mac,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(&match_spec.l2_metadata_lkp_mac_da_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_port_lag_label =
            mac_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = mac_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }
  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_mac_qos_acl_table_add_with_acl_set_qos_fields(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mac qos acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac qos acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac qos acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_mac_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if defined(P4_EGRESS_ACL_ENABLE) && !defined(P4_EGRESS_MAC_ACL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_mac_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_mac_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (mac_acl[i].field) {
      case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
        match_spec.ethernet_etherType = mac_acl[i].value.eth_type;
        match_spec.ethernet_etherType_mask = mac_acl[i].mask.u.mask16 & 0xFFFF;
        break;
      case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
        SWITCH_MEMCPY(match_spec.ethernet_srcAddr,
                      mac_acl[i].value.source_mac.mac_addr,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(match_spec.ethernet_srcAddr_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_DEST_MAC:
        SWITCH_MEMCPY(match_spec.ethernet_dstAddr,
                      mac_acl[i].value.dest_mac.mac_addr,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(match_spec.ethernet_dstAddr_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            mac_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            mac_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            mac_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask = mac_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_mac_acl_table_add_with_egress_acl_deny(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_mac_acl_table_add_with_egress_acl_permit(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
          &action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_ACL_ENABLE && !P4_EGRESS_MAC_ACL_ENABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress mac acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress mac acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_mirror_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
  switch_stats_id_t stats_index = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
    if (opt_action_params->counter_handle != SWITCH_API_INVALID_HANDLE) {
      stats_index = handle_to_id(opt_action_params->counter_handle);
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_MIRROR: {
#ifndef P4_MIRROR_DISABLE
#ifdef P4_INGRESS_ACL_ACTION_MIRROR_ENABLE
      p4_pd_dc_acl_mirror_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      //#ifdef P4_ACL_QOS_ENABLE
      //      action_spec.action_acl_meter_index = meter_index;
      //#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_mirror(
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
#endif /* P4_INGRESS_ACL_ACTION_MIRROR_ENABLE */
#endif /* P4_MIRROR_DISABLE */
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      if (opt_action_params) {
        action_spec.action_nat_mode = opt_action_params->nat_mode;
      }
      pd_status = p4_pd_dc_ip_acl_table_modify_with_acl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_mirror_acl_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 acl entry update failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT: {
      switch_handle_t handle = action_params->redirect.handle;
      if (switch_handle_type_get(handle) == SWITCH_HANDLE_TYPE_NHOP) {
        p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
        action_spec.action_acl_meter_index = meter_index;
#endif
        action_spec.action_acl_stats_index = stats_index;
        action_spec.action_nexthop_index = handle_to_id(handle);
        pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_redirect_nexthop(
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
      }
    } break;
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_acl_table_modify_with_acl_permit(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_qos_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ip_qos_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_IPV4_QOS_ACL_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  switch_stats_id_t stats_index = SWITCH_API_INVALID_HANDLE;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
    if (opt_action_params->counter_handle != SWITCH_API_INVALID_HANDLE) {
      stats_index = handle_to_id(opt_action_params->counter_handle);
    }
#if defined(P4_QOS_METERING_ENABLE)
    if (opt_action_params->meter_handle != SWITCH_API_INVALID_HANDLE) {
      meter_index = handle_to_id(opt_action_params->meter_handle);
    }
#endif /* P4_QOS_METERING_ENABLE */
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv4_qos_acl_table_modify_with_acl_set_qos_fields(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_QOS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 qos acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_qos_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ipv6_qos_acl_key_value_pair_t *ipv6_qos_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ipv6_qos_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_IPV6_QOS_ACL_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  switch_stats_id_t stats_index = SWITCH_API_INVALID_HANDLE;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
    if (opt_action_params->counter_handle != SWITCH_API_INVALID_HANDLE) {
      stats_index = handle_to_id(opt_action_params->counter_handle);
    }
#if defined(P4_QOS_METERING_ENABLE)
    if (opt_action_params->meter_handle != SWITCH_API_INVALID_HANDLE) {
      meter_index = handle_to_id(opt_action_params->meter_handle);
    }
#endif /* P4_QOS_METERING_ENABLE */
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_ipv6_qos_acl_table_modify_with_acl_set_qos_fields(
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
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 qos acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_QOS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 qos acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 qos acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mac_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if !defined(P4_INGRESS_MAC_ACL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_ACL_QOS_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_ACL_QOS_ENABLE */
  switch_stats_id_t stats_index = 0;
  switch_handle_t nhop_handle = 0;

  status = switch_api_hostif_nhop_get(
      device, SWITCH_HOSTIF_REASON_CODE_GLEAN, &nhop_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mac acl entry update failed on device %d\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_ACL_QOS_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_ACL_QOS_ENABLE */
    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_mac_acl_table_modify_with_acl_deny(
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
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_mac_acl_table_modify_with_acl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_acl_redirect_nexthop_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      action_spec.action_nexthop_index = handle_to_id(nhop_handle);
      pd_status = p4_pd_dc_mac_acl_table_modify_with_acl_redirect_nexthop(
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
    case SWITCH_ACL_ACTION_COPY_TO_CPU: {
      p4_pd_dc_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
#ifdef P4_ACL_QOS_ENABLE
      action_spec.action_acl_meter_index = meter_index;
#endif
      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_mac_acl_table_modify_with_acl_permit(
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
    case SWITCH_ACL_ACTION_REDIRECT:
      break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mac acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mac_qos_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_mac_qos_acl_key_value_pair_t *mac_qos_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_qos_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if defined(P4_MAC_QOS_ACL_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
#if defined(P4_QOS_METERING_ENABLE)
  switch_meter_id_t meter_index = 0;
#endif /* P4_QOS_METERING_ENABLE */
  //  switch_stats_id_t stats_index = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (opt_action_params) {
#if defined(P4_QOS_METERING_ENABLE)
    meter_index = handle_to_id(opt_action_params->meter_handle);
#endif /* P4_QOS_METERING_ENABLE */
    //    stats_index = handle_to_id(opt_action_params->counter_handle);
  }

  switch (action) {
    case SWITCH_ACL_ACTION_SET_QOS_FIELDS: {
      p4_pd_dc_acl_set_qos_fields_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_tc = opt_action_params->tc;
      action_spec.action_color = opt_action_params->color;
#if defined(P4_QOS_METERING_ENABLE)
      action_spec.action_acl_meter_index = meter_index;
#endif
      //      action_spec.action_acl_stats_index = stats_index;
      pd_status = p4_pd_dc_mac_qos_acl_table_modify_with_acl_set_qos_fields(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mac qos acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MAC_QOS_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_mac_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(mac_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if defined(P4_EGRESS_ACL_ENABLE) && !defined(P4_EGRESS_MAC_ACL_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_mac_acl_match_spec_t match_spec;
  switch_int32_t i = 0;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_mac_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (mac_acl[i].field) {
      case SWITCH_ACL_MAC_FIELD_ETH_TYPE:
        match_spec.ethernet_etherType = mac_acl[i].value.eth_type;
        match_spec.ethernet_etherType_mask = mac_acl[i].mask.u.mask16 & 0xFFFF;
        break;
      case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
        SWITCH_MEMCPY(match_spec.ethernet_srcAddr,
                      mac_acl[i].value.source_mac.mac_addr,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(match_spec.ethernet_srcAddr_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_DEST_MAC:
        SWITCH_MEMCPY(match_spec.ethernet_dstAddr,
                      mac_acl[i].value.dest_mac.mac_addr,
                      SWITCH_MAC_LENGTH);
        SWITCH_MEMCPY(match_spec.ethernet_dstAddr_mask,
                      &mac_acl[i].mask.u.mask,
                      SWITCH_MAC_LENGTH);
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_port_lag_label =
            mac_acl[i].value.port_lag_label;
        match_spec.acl_metadata_egress_port_lag_label_mask =
            mac_acl[i].mask.u.mask;
        break;
      case SWITCH_ACL_MAC_FIELD_PORT_LAG_LABEL:
        match_spec.acl_metadata_egress_bd_label =
            mac_acl[i].value.vlan_rif_label;
        match_spec.acl_metadata_egress_bd_label_mask = mac_acl[i].mask.u.mask;
        break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_DROP: {
      p4_pd_dc_egress_acl_deny_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_mac_acl_table_modify_with_egress_acl_deny(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_ACL_ACTION_PERMIT: {
      p4_pd_dc_egress_acl_permit_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_acl_copy_reason =
          action_params->cpu_redirect.reason_code;
      pd_status = p4_pd_dc_egress_mac_acl_table_modify_with_egress_acl_permit(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "egress ipv6 acl table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    default:
      break;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_EGRESS_ACL_ENABLE && !P4_EGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress mac acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress mac acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE) && defined(P4_MIRROR_ACL_ENABLE)

  pd_status = p4_pd_dc_ipv4_mirror_acl_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

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
        "ipv4 mirror acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV4_DISABLE && P4_MIRROR_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 mirror acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 mirror acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && defined(P4_MIRROR_ACL_ENABLE)

  pd_status = p4_pd_dc_ipv6_mirror_acl_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

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
        "ipv6 mirror acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV6_DISABLE && P4_MIRROR_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 mirror acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv4_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV4_DISABLE) && defined(P4_QOS_ACL_ENABLE)

  pd_status = p4_pd_dc_ipv4_qos_acl_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

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
        "ipv4 qos acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV4_DISABLE && P4_QOS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 qos acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ipv6_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_IPV6_DISABLE) && defined(P4_QOS_ACL_ENABLE)

  pd_status = p4_pd_dc_ipv6_qos_acl_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

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
        "ipv6 qos acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_IPV6_DISABLE && P4_QOS_ACL_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 qos acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_mac_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if !defined(P4_INGRESS_MAC_ACL_DISABLE)

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    pd_status =
        p4_pd_dc_mac_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  } else {
#if defined(P4_EGRESS_ACL_ENABLE) && !defined(P4_EGRESS_MAC_ACL_DISABLE)
    pd_status = p4_pd_dc_egress_mac_acl_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
#endif /* P4_EGRESS_ACL_ENABLE && !P4_EGRESS_MAC_ACL_ENABLE */
  }

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
        "mac acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mac acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mac acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_mac_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L2_DISABLE)
#if defined(P4_MAC_QOS_ACL_ENABLE)

  pd_status =
      p4_pd_dc_mac_qos_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

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
        "qos mac acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_INGRESS_MAC_ACL_DISABLE */
#endif /* P4_L2_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "qos mac acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "qos mac acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_egress_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_egress_system_key_value_pair_t *egr_acl,
    switch_acl_egress_system_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(egr_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch_uint32_t i = 0;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_egress_system_acl_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_egress_system_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (egr_acl[i].field) {
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT: {
        switch_dev_port_t dev_port = 0;
        SWITCH_PORT_DEV_PORT_GET(
            device, egr_acl[i].value.egr_port, dev_port, status);
        SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
        match_spec.eg_intr_md_egress_port = dev_port;
        match_spec.eg_intr_md_egress_port_mask = 0xFFFF;
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT: {
        match_spec.eg_intr_md_deflection_flag =
            egr_acl[i].value.deflection_flag ? 0x1 : 0;
        match_spec.eg_intr_md_deflection_flag_mask = 0xFF;
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK: {
        match_spec.l3_metadata_l3_mtu_check = egr_acl[i].value.l3_mtu_check;
        match_spec.l3_metadata_l3_mtu_check_mask = 0xFFFF;
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_ACL_DENY: {
        match_spec.acl_metadata_egress_acl_deny = egr_acl[i].value.acl_deny;
        match_spec.acl_metadata_egress_acl_deny_mask = 0xFF;
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_REASON_CODE: {
        match_spec.fabric_metadata_reason_code = egr_acl[i].value.reason_code;
        match_spec.fabric_metadata_reason_code_mask = 0xFFFF;
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_MIRROR_ON_DROP: {
#ifdef P4_DTEL_DROP_REPORT_ENABLE
        match_spec.dtel_md_mod_watchlist_hit = egr_acl[i].value.mirror_on_drop;
        match_spec.dtel_md_mod_watchlist_hit_mask =
            egr_acl[i].mask.u.mask & 0x3;
#endif
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_QUEUE_DOD_ENABLE: {
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
        match_spec.dtel_md_queue_dod_enable = egr_acl[i].value.queue_dod_enable;
        match_spec.dtel_md_queue_dod_enable_mask = egr_acl[i].mask.u.mask & 0x1;
#endif
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_DROP_CTL: {
#ifdef P4_DTEL_DROP_REPORT_ENABLE
        match_spec.eg_intr_md_for_oport_drop_ctl = egr_acl[i].value.drop_ctl;
        match_spec.eg_intr_md_for_oport_drop_ctl_mask =
            egr_acl[i].mask.u.mask & 0x7;
#endif
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_PACKET_COLOR: {
#ifdef P4_COPP_COLOR_DROP_ENABLE
        /*
         * Tofino expects the value of RED to be 3.
         */
        if (egr_acl[i].value.packet_color == SWITCH_COLOR_RED) {
          match_spec.ig_intr_md_for_tm_packet_color = 3;
        } else {
          match_spec.ig_intr_md_for_tm_packet_color =
              egr_acl[i].value.packet_color;
        }
        match_spec.ig_intr_md_for_tm_packet_color_mask = 0x3;
#endif
      } break;
      case SWITCH_ACL_EGRESS_SYSTEM_FIELD_SRC_PORT: {
#ifdef P4_INGRESS_PORT_IN_EGRESS_SYSTEM_ACL_ENABLE
        switch_dev_port_t dev_port = 0;
        SWITCH_PORT_DEV_PORT_GET(
            device, egr_acl[i].value.ing_port, dev_port, status);
        SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
        match_spec.ingress_metadata_ingress_port = dev_port;
        match_spec.ingress_metadata_ingress_port_mask = 0xFFFF;
#endif /* P4_INGRESS_PORT_IN_EGRESS_SYSTEM_ACL_ENABLE */
      } break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_NOP:
      pd_status = p4_pd_dc_egress_system_acl_table_add_with_nop(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
      break;
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_PERMIT: {
      status = p4_pd_dc_egress_system_acl_table_add_with_nop(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP: {
      status = p4_pd_dc_egress_system_acl_table_add_with_drop_packet(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_SET_MIRROR: {
#if !defined(P4_MIRROR_DISABLE)
      p4_pd_dc_egress_mirror_action_spec_t action_spec;
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      pd_status = p4_pd_dc_egress_system_acl_table_add_with_egress_mirror(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
      break;
#endif /* P4_MIRROR_DISABLE */
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_egress_redirect_to_cpu_with_reason_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->cpu_redirect.reason_code;
      pd_status =
          p4_pd_dc_egress_system_acl_table_add_with_egress_redirect_to_cpu_with_reason(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              priority,
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
      break;
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP: {
#ifdef P4_DTEL_DROP_REPORT_ENABLE
      p4_pd_dc_egress_mirror_and_drop_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->drop.reason_code;
      status = p4_pd_dc_egress_system_acl_table_add_with_egress_mirror_and_drop(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          priority,
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
#endif /* P4_DTEL_DROP_REPORT_ENABLE */
      break;
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP_QALERT: {
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
      p4_pd_dc_egress_mirror_and_drop_set_queue_alert_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->drop.reason_code;
      status =
          p4_pd_dc_egress_system_acl_table_add_with_egress_mirror_and_drop_set_queue_alert(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              priority,
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
#endif /* P4_DTEL_QUEUE_REPORT_ENABLE */
      break;
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_INSERT_CPU_TIMESTAMP: {
#ifdef P4_PTP_ENABLE
      pd_status =
          p4_pd_dc_egress_system_acl_table_add_with_egress_insert_cpu_timestamp(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              priority,
              entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
#endif /* P4_PTP_ENABLE */
      break;
    }
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress acl table add failed "
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
        "egress acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_egress_system_key_value_pair_t *egr_acl,
    switch_acl_egress_system_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(egr_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (action) {
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_NOP:
      pd_status = p4_pd_dc_egress_system_acl_table_modify_with_nop(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
      break;
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_SET_MIRROR: {
#if !defined(P4_MIRROR_DISABLE)
      p4_pd_dc_egress_mirror_action_spec_t action_spec;
      action_spec.action_session_id =
          handle_to_id(opt_action_params->mirror_handle);
      pd_status = p4_pd_dc_egress_system_acl_table_modify_with_egress_mirror(
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
      break;
#endif /* P4_MIRROR_DISABLE */
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU: {
      p4_pd_dc_egress_redirect_to_cpu_with_reason_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->cpu_redirect.reason_code;
      pd_status =
          p4_pd_dc_egress_system_acl_table_modify_with_egress_redirect_to_cpu_with_reason(
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
      break;
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP: {
#ifdef P4_DTEL_DROP_REPORT_ENABLE
      p4_pd_dc_egress_mirror_and_drop_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->drop.reason_code;
      pd_status =
          p4_pd_dc_egress_system_acl_table_modify_with_egress_mirror_and_drop(
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
#endif /* P4_DTEL_DROP_REPORT_ENABLE */
      break;
    }
    case SWITCH_ACL_EGRESS_SYSTEM_ACTION_MIRROR_AND_DROP_QALERT: {
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
      p4_pd_dc_egress_mirror_and_drop_set_queue_alert_action_spec_t action_spec;
      action_spec.action_reason_code = action_params->drop.reason_code;
      pd_status =
          p4_pd_dc_egress_system_acl_table_modify_with_egress_mirror_and_drop_set_queue_alert(
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
#endif /* P4_DTEL_QUEUE_REPORT_ENABLE */
      break;
    }
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress acl table update failed "
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
        "egress acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  pd_status = p4_pd_dc_egress_system_acl_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);

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
        "egress acl table entry delete failed "
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
        "egress acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ecn_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ecn_key_value_pair_t *ecn_acl,
    switch_acl_ecn_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ecn_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  switch_uint32_t i = 0;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_ecn_acl_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_ecn_acl_match_spec_t));

  for (i = 0; i < count; i++) {
    switch (ecn_acl[i].field) {
      case SWITCH_ACL_ECN_FIELD_DSCP: {
        match_spec.l3_metadata_lkp_dscp |= (ecn_acl[i].value.dscp << 2);
        match_spec.l3_metadata_lkp_dscp_mask |=
            ((ecn_acl[i].mask.u.mask << 2) & 0xFC);
      } break;
      case SWITCH_ACL_ECN_FIELD_ECN: {
        match_spec.l3_metadata_lkp_dscp |= (ecn_acl[i].value.ecn & 0x3);
        match_spec.l3_metadata_lkp_dscp_mask |= (ecn_acl[i].mask.u.mask & 0x3);
      } break;
      case SWITCH_ACL_ECN_FIELD_PORT_LAG_LABEL: {
        match_spec.acl_metadata_port_lag_label =
            ecn_acl[i].value.port_lag_label;
        match_spec.acl_metadata_port_lag_label_mask = ecn_acl[i].mask.u.mask;
      } break;
      default:
        break;
    }
  }

  switch (action) {
    case SWITCH_ACL_ACTION_TC_AND_COLOR: {
      p4_pd_dc_set_ingress_tc_and_color_for_ecn_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      if (opt_action_params) {
        action_spec.action_tc = opt_action_params->tc;
        action_spec.action_color = opt_action_params->color;
      }
      pd_status =
          p4_pd_dc_ecn_acl_table_add_with_set_ingress_tc_and_color_for_ecn(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              priority,
              &action_spec,
              entry_hdl);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecn acl table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ecn_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_uint32_t count,
    switch_acl_ecn_key_value_pair_t *ecn_acl,
    switch_acl_ecn_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(priority);
  UNUSED(count);
  UNUSED(ecn_acl);
  UNUSED(action);
  UNUSED(action_params);
  UNUSED(opt_action_params);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  switch (action) {
    case SWITCH_ACL_ACTION_TC_AND_COLOR: {
      p4_pd_dc_set_ingress_tc_and_color_for_ecn_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      if (opt_action_params) {
        action_spec.action_tc = opt_action_params->tc;
        action_spec.action_color = opt_action_params->color;
      }
      pd_status =
          p4_pd_dc_ecn_acl_table_modify_with_set_ingress_tc_and_color_for_ecn(
              switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec = (switch_uint8_t *)&action_spec;
        pd_entry.action_spec_size = sizeof(action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } break;
    default:
      break;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecn acl table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ecn_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_WRED_ENABLE

  pd_status =
      p4_pd_dc_ecn_acl_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

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
        "ecn acl table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_WRED_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecn acl table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecn acl table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_acl_table_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_system_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_L2_DISABLE
#if !defined(P4_INGRESS_MAC_ACL_DISABLE)
  pd_status = p4_pd_dc_mac_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_MAC_ACL_ENABLE */

#if defined(P4_MAC_QOS_ACL_ENABLE)
  pd_status = p4_pd_dc_mac_qos_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_MAC_QOS_ACL_ENABLE */
#endif /* P4_L2_DISABLE */

#ifndef P4_IPV4_DISABLE
  pd_status = p4_pd_dc_ip_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if !defined(P4_RACL_DISABLE)
  pd_status = p4_pd_dc_ipv4_racl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_RACL_DISABLE */

#if defined(P4_MIRROR_ACL_ENABLE)
  pd_status = p4_pd_dc_ipv4_mirror_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 mirror acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_MIRROR_ACL_ENABLE */

#if defined(P4_IPV4_QOS_ACL_ENABLE)
  pd_status = p4_pd_dc_ipv4_qos_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 qos acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_QOS_ACL_ENABLE */
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_ipv6_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if !defined(P4_RACL_DISABLE)
  pd_status = p4_pd_dc_ipv6_racl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_RACL_DISABLE */

#if defined(P4_MIRROR_ACL_ENABLE)
  pd_status = p4_pd_dc_ipv6_mirror_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 mirror acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_MIRROR_ACL_ENABLE */

#if defined(P4_IPV6_QOS_ACL_ENABLE)
  pd_status = p4_pd_dc_ipv6_qos_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 qos acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_QOS_ACL_ENABLE */
#endif /* P4_IPV6_DISABLE */

#ifndef P4_STATS_DISABLE
#if !defined(__TARGET_TOFINO__) || defined(BMV2TOFINO)
  pd_status = p4_pd_dc_drop_stats_set_default_action_drop_stats_update(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_acl_stats_set_default_action_acl_stats_update(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_RACL_STATS_ENABLE
  pd_status = p4_pd_dc_racl_stats_set_default_action_racl_stats_update(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_RACL_STATS_ENABLE */

#endif /* __TARGET_TOFINO__ || BMV2TOFINO */
#endif /* P4_STATS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "acl table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "acl table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_egress_acl_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_egress_system_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress acl table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_EGRESS_ACL_ENABLE

#ifndef P4_EGRESS_MAC_ACL_DISABLE
  status = p4_pd_dc_egress_mac_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
#endif /* P4_EGRESS_MAC_ACL_DISABLE */

#ifndef P4_IPV4_DISABLE
  status = p4_pd_dc_egress_ip_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  status = p4_pd_dc_egress_ipv6_acl_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
#endif /* P4_IPV6_DISABLE */

#endif /* P4_EGRESS_ACL_ENABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress acl table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress acl table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_acl_stats_get(switch_device_t device,
                                        switch_counter_id_t acl_stats_index,
                                        switch_counter_t *acl_counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);
  UNUSED(acl_counter);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status =
      p4_pd_dc_counter_read_acl_stats(switch_cfg_sess_hdl,
                                      p4_pd_device,
                                      acl_stats_index,
                                      switch_pd_counter_read_flags(device),
                                      &counter);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Reading acl stats failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    status = switch_pd_status_to_status(pd_status);
    p4_pd_complete_operations(switch_cfg_sess_hdl);
    return status;
  }
  acl_counter->num_packets = counter.packets;
  acl_counter->num_bytes = counter.bytes;

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("acl stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_acl_stats_clear(switch_device_t device,
                                          switch_counter_id_t acl_stats_index) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&counter, 0x0, sizeof(counter));
  pd_status = p4_pd_dc_counter_write_acl_stats(
      switch_cfg_sess_hdl, p4_pd_device, acl_stats_index, counter);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_STATS_DISABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("acl stats clear on device %d\n", device);

  return status;
}

switch_status_t switch_pd_racl_stats_get(switch_device_t device,
                                         switch_counter_id_t acl_stats_index,
                                         switch_counter_t *acl_counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);
  UNUSED(acl_counter);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE) && defined(P4_RACL_STATS_ENABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status =
      p4_pd_dc_counter_read_racl_stats(switch_cfg_sess_hdl,
                                       p4_pd_device,
                                       acl_stats_index,
                                       switch_pd_counter_read_flags(device),
                                       &counter);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Reading racl stats failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    status = switch_pd_status_to_status(pd_status);
    p4_pd_complete_operations(switch_cfg_sess_hdl);
    return status;
  }
  acl_counter->num_packets = counter.packets;
  acl_counter->num_bytes = counter.bytes;

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_STATS_DISABLE && P4_RACL_STATS_ENABLE*/
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("racl stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_racl_stats_clear(
    switch_device_t device, switch_counter_id_t acl_stats_index) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE) && defined(P4_RACL_STATS_ENABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&counter, 0x0, sizeof(counter));
  pd_status = p4_pd_dc_counter_write_racl_stats(
      switch_cfg_sess_hdl, p4_pd_device, acl_stats_index, counter);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_STATS_DISABLE && P4_RACL_STATS_ENABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("racl stats clear on device %d\n", device);

  return status;
}

switch_status_t switch_pd_egress_acl_stats_get(
    switch_device_t device,
    switch_counter_id_t acl_stats_index,
    switch_counter_t *acl_counter) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);
  UNUSED(acl_counter);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE) && defined(P4_EGRESS_ACL_STATS_ENABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_counter_read_egress_acl_stats(
      switch_cfg_sess_hdl,
      p4_pd_device,
      acl_stats_index,
      switch_pd_counter_read_flags(device),
      &counter);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "Reading egress_acl stats failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    status = switch_pd_status_to_status(pd_status);
    p4_pd_complete_operations(switch_cfg_sess_hdl);
    return status;
  }
  acl_counter->num_packets = counter.packets;
  acl_counter->num_bytes = counter.bytes;

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_STATS_DISABLE && P4_EGRESS_ACL_STATS_ENABLE*/
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("egress_acl stats get on device %d\n", device);

  return status;
}

switch_status_t switch_pd_egress_acl_stats_clear(
    switch_device_t device, switch_counter_id_t acl_stats_index) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(acl_stats_index);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_STATS_DISABLE) && defined(P4_EGRESS_ACL_STATS_ENABLE)

  p4_pd_counter_value_t counter;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&counter, 0x0, sizeof(counter));
  pd_status = p4_pd_dc_counter_write_egress_acl_stats(
      switch_cfg_sess_hdl, p4_pd_device, acl_stats_index, counter);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_STATS_DISABLE && P4_EGRESS_ACL_STATS_ENABLE */
#endif /* SWITCH_PD */

  SWITCH_PD_LOG_DEBUG("egress_acl stats clear on device %d\n", device);

  return status;
}

switch_status_t switch_pd_range_entry_add(switch_device_t device,
                                          switch_direction_t direction,
                                          switch_uint16_t range_id,
                                          switch_range_type_t range_type,
                                          switch_range_t *range,
                                          switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch (range_type) {
    case SWITCH_RANGE_TYPE_SRC_PORT: {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_ingress_l4_src_port_match_spec_t match_spec;
        p4_pd_dc_set_ingress_src_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        match_spec.l3_metadata_lkp_l4_sport_start = range->start_value;
        match_spec.l3_metadata_lkp_l4_sport_end = range->end_value;
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_ingress_l4_src_port_table_add_with_set_ingress_src_port_range_id(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                1000,
                &action_spec,
                entry_hdl);
#endif /* P4_INGRES_ACL_RANGE_DISABLE */
      } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_egress_l4_src_port_match_spec_t match_spec;
        p4_pd_dc_set_egress_src_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        match_spec.l3_metadata_egress_l4_sport_start = range->start_value;
        match_spec.l3_metadata_egress_l4_sport_end = range->end_value;
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_egress_l4_src_port_table_add_with_set_egress_src_port_range_id(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                1000,
                &action_spec,
                entry_hdl);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
      }
    } break;

    case SWITCH_RANGE_TYPE_DST_PORT: {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_ingress_l4_dst_port_match_spec_t match_spec;
        p4_pd_dc_set_ingress_dst_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        match_spec.l3_metadata_lkp_l4_dport_start = range->start_value;
        match_spec.l3_metadata_lkp_l4_dport_end = range->end_value;
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_ingress_l4_dst_port_table_add_with_set_ingress_dst_port_range_id(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                1000,
                &action_spec,
                entry_hdl);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */
      } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_egress_l4_dst_port_match_spec_t match_spec;
        p4_pd_dc_set_egress_dst_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        match_spec.l3_metadata_egress_l4_dport_start = range->start_value;
        match_spec.l3_metadata_egress_l4_dport_end = range->end_value;
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_egress_l4_dst_port_table_add_with_set_egress_dst_port_range_id(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                1000,
                &action_spec,
                entry_hdl);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
      }
    } break;

    default:
      return SWITCH_STATUS_NOT_SUPPORTED;
  }

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_range_entry_update(switch_device_t device,
                                             switch_direction_t direction,
                                             switch_uint16_t range_id,
                                             switch_range_type_t range_type,
                                             switch_range_t *range,
                                             switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch (range_type) {
    case SWITCH_RANGE_TYPE_SRC_PORT: {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_set_ingress_src_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_ingress_l4_src_port_table_modify_with_set_ingress_src_port_range_id(
                switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */
      } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_set_egress_src_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_egress_l4_src_port_table_modify_with_set_egress_src_port_range_id(
                switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
      }
    } break;

    case SWITCH_RANGE_TYPE_DST_PORT: {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_set_ingress_dst_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_ingress_l4_dst_port_table_modify_with_set_ingress_dst_port_range_id(
                switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */
      } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
        p4_pd_dc_set_egress_dst_port_range_id_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
        action_spec.action_range_id = range_id;
        pd_status =
            p4_pd_dc_egress_l4_dst_port_table_modify_with_set_egress_dst_port_range_id(
                switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
      }
    } break;

    default:
      return SWITCH_STATUS_NOT_SUPPORTED;
  }

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_range_entry_delete(switch_device_t device,
                                             switch_direction_t direction,
                                             switch_range_type_t range_type,
                                             switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch (range_type) {
    case SWITCH_RANGE_TYPE_SRC_PORT: {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
        pd_status = p4_pd_dc_ingress_l4_src_port_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */
      } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
        pd_status = p4_pd_dc_egress_l4_src_port_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
      }
      break;

      case SWITCH_RANGE_TYPE_DST_PORT: {
        if (direction == SWITCH_API_DIRECTION_INGRESS) {
#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
          pd_status = p4_pd_dc_ingress_l4_dst_port_table_delete(
              switch_cfg_sess_hdl, device, entry_hdl);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */
        } else {
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
          pd_status = p4_pd_dc_egress_l4_dst_port_table_delete(
              switch_cfg_sess_hdl, device, entry_hdl);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
        }
      } break;
      default:
        return SWITCH_STATUS_NOT_SUPPORTED;
    }
  }

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_egress_l4port_fields_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dc_egress_l4port_fields_match_spec_t match_spec;

  status = p4_pd_dc_egress_l4port_fields_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.tcp_valid = 1;
  status =
      p4_pd_dc_egress_l4port_fields_table_add_with_set_egress_tcp_port_fields(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.udp_valid = 1;
  status =
      p4_pd_dc_egress_l4port_fields_table_add_with_set_egress_udp_port_fields(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.icmp_valid = 1;
  status =
      p4_pd_dc_egress_l4port_fields_table_add_with_set_egress_icmp_port_fields(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);

#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_l4port_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#if !defined(P4_INGRESS_ACL_RANGE_DISABLE)
  pd_status = p4_pd_dc_ingress_l4_src_port_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  pd_status = p4_pd_dc_ingress_l4_dst_port_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
#endif /* !P4_INGRESS_ACL_RANGE_DISABLE */

#if !defined(P4_EGRESS_ACL_RANGE_DISABLE)
  pd_status = p4_pd_dc_egress_l4_src_port_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  pd_status = p4_pd_dc_egress_l4_dst_port_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
#endif /* !P4_EGRESS_ACL_RANGE_DISABLE */

#endif /* SWITCH_PD */

  return status;
}

#ifdef __cplusplus
}
#endif
