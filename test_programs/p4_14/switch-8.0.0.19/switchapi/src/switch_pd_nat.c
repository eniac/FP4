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
#include "switch_pd.h"

switch_status_t switch_pd_nat_add_default_entries(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;
  int priority = 100;

  p4_pd_dc_update_l4_checksum_match_spec_t match_spec;

  /* UDP checksum is zero */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.tunnel_metadata_egress_tunnel_type = 0;
  match_spec.tunnel_metadata_egress_tunnel_type_mask = 0xFF;
  match_spec.udp_valid = 1;
  match_spec.udp_valid_mask = 1;
  match_spec.udp_checksum = 0;
  match_spec.udp_checksum_mask = 0xFFFF;
  pd_status = p4_pd_dc_update_l4_checksum_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

  /* UDP checksum */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.tunnel_metadata_egress_tunnel_type = 0;
  match_spec.tunnel_metadata_egress_tunnel_type_mask = 0xFF;
  match_spec.udp_valid = 1;
  match_spec.udp_valid_mask = 1;
  pd_status = p4_pd_dc_update_l4_checksum_table_add_with_update_udp_checksum(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

  /* TCP checksum */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.tunnel_metadata_egress_tunnel_type = 0;
  match_spec.tunnel_metadata_egress_tunnel_type_mask = 0xFF;
  match_spec.tcp_valid = 1;
  match_spec.tcp_valid_mask = 1;
  pd_status = p4_pd_dc_update_l4_checksum_table_add_with_update_tcp_checksum(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

  /* Inner UDP checksum is zero */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.udp_valid = 1;
  match_spec.udp_valid_mask = 1;
  match_spec.inner_udp_checksum = 0;
  match_spec.inner_udp_checksum_mask = 0xFFFF;
  pd_status = p4_pd_dc_update_l4_checksum_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

  /* Inner UDP checksum */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.udp_valid = 1;
  match_spec.udp_valid_mask = 1;
  pd_status =
      p4_pd_dc_update_l4_checksum_table_add_with_update_inner_udp_checksum(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

  /* Inner TCP checksum */
  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_update_checksum = 1;
  match_spec.nat_metadata_update_checksum_mask = 1;
  match_spec.tcp_valid = 1;
  match_spec.tcp_valid_mask = 1;
  pd_status =
      p4_pd_dc_update_l4_checksum_table_add_with_update_inner_udp_checksum(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  priority++;

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nat_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  /* egress tables */
  status = p4_pd_dc_egress_nat_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  status = p4_pd_dc_update_l4_checksum_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  /* add default entries*/
  switch_pd_nat_add_default_entries(device);

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nat_table_entry_add(switch_device_t device,
                                              switch_nat_info_t *nat_info,
                                              switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  switch_api_nat_info_t *api_nat_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  api_nat_info = &nat_info->api_nat_info;
  switch (api_nat_info->nat_rw_type) {
    case SWITCH_NAT_RW_TYPE_SRC: {
      p4_pd_dc_nat_flow_match_spec_t match_spec;
      p4_pd_dc_set_src_nat_rewrite_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = handle_to_id(api_nat_info->vrf_handle);
      match_spec.ipv4_metadata_lkp_ipv4_sa = SWITCH_NAT_SRC_IP(api_nat_info);
      match_spec.l3_metadata_vrf_mask = 0xFFFF;
      match_spec.ipv4_metadata_lkp_ipv4_sa_mask = 0xFFFFFFFF;
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_flow_table_add_with_set_src_nat_rewrite_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_TCP:
    case SWITCH_NAT_RW_TYPE_SRC_UDP: {
      p4_pd_dc_nat_src_match_spec_t match_spec;
      p4_pd_dc_set_src_nat_rewrite_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = handle_to_id(api_nat_info->vrf_handle);
      match_spec.ipv4_metadata_lkp_ipv4_sa = SWITCH_NAT_SRC_IP(api_nat_info);
      match_spec.l3_metadata_lkp_ip_proto = api_nat_info->protocol;
      match_spec.l3_metadata_lkp_l4_sport = api_nat_info->src_port;
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_src_table_add_with_set_src_nat_rewrite_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_DST: {
      p4_pd_dc_nat_flow_match_spec_t match_spec;
      p4_pd_dc_set_dst_nat_nexthop_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = api_nat_info->vrf_handle;
      match_spec.ipv4_metadata_lkp_ipv4_da = SWITCH_NAT_DST_IP(api_nat_info);
      match_spec.l3_metadata_vrf_mask = 0xFFFF;
      match_spec.ipv4_metadata_lkp_ipv4_da_mask = 0xFFFFFFFF;
      action_spec.action_nexthop_index =
          handle_to_id(api_nat_info->nhop_handle);
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_flow_table_add_with_set_dst_nat_nexthop_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_DST_TCP:
    case SWITCH_NAT_RW_TYPE_DST_UDP: {
      p4_pd_dc_nat_dst_match_spec_t match_spec;
      p4_pd_dc_set_dst_nat_nexthop_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = api_nat_info->vrf_handle;
      match_spec.ipv4_metadata_lkp_ipv4_da = SWITCH_NAT_DST_IP(api_nat_info);
      match_spec.l3_metadata_lkp_ip_proto = api_nat_info->protocol;
      match_spec.l3_metadata_lkp_l4_dport = api_nat_info->dst_port;
      action_spec.action_nexthop_index =
          handle_to_id(api_nat_info->nhop_handle);
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_dst_table_add_with_set_dst_nat_nexthop_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_DST: {
      p4_pd_dc_nat_flow_match_spec_t match_spec;
      p4_pd_dc_set_twice_nat_nexthop_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = handle_to_id(api_nat_info->vrf_handle);
      match_spec.ipv4_metadata_lkp_ipv4_sa = SWITCH_NAT_SRC_IP(api_nat_info);
      match_spec.ipv4_metadata_lkp_ipv4_da = SWITCH_NAT_DST_IP(api_nat_info);
      match_spec.l3_metadata_vrf_mask = 0xFFFF;
      match_spec.ipv4_metadata_lkp_ipv4_sa_mask = 0xFFFFFFFF;
      match_spec.ipv4_metadata_lkp_ipv4_da_mask = 0xFFFFFFFF;
      action_spec.action_nexthop_index =
          handle_to_id(api_nat_info->nhop_handle);
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_flow_table_add_with_set_twice_nat_nexthop_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          0,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_DST_TCP:
    case SWITCH_NAT_RW_TYPE_SRC_DST_UDP: {
      p4_pd_dc_nat_twice_match_spec_t match_spec;
      p4_pd_dc_set_twice_nat_nexthop_index_action_spec_t action_spec;

      memset(&match_spec, 0, sizeof(match_spec));
      memset(&action_spec, 0, sizeof(action_spec));

      match_spec.l3_metadata_vrf = handle_to_id(api_nat_info->vrf_handle);
      match_spec.ipv4_metadata_lkp_ipv4_sa = SWITCH_NAT_SRC_IP(api_nat_info);
      match_spec.ipv4_metadata_lkp_ipv4_da = SWITCH_NAT_DST_IP(api_nat_info);
      match_spec.l3_metadata_lkp_ip_proto = api_nat_info->protocol;
      match_spec.l3_metadata_lkp_l4_sport = api_nat_info->src_port;
      match_spec.l3_metadata_lkp_l4_dport = api_nat_info->dst_port;
      action_spec.action_nexthop_index =
          handle_to_id(api_nat_info->nhop_handle);
      action_spec.action_nat_rewrite_index = nat_info->rw_index;

      status = p4_pd_dc_nat_twice_table_add_with_set_twice_nat_nexthop_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nat_table_entry_delete(switch_device_t device,
                                                 switch_nat_info_t *nat_info,
                                                 switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  switch_api_nat_info_t *api_nat_info = NULL;

  api_nat_info = &nat_info->api_nat_info;
  switch (api_nat_info->nat_rw_type) {
    case SWITCH_NAT_RW_TYPE_SRC:
    case SWITCH_NAT_RW_TYPE_DST:
    case SWITCH_NAT_RW_TYPE_SRC_DST:
      status = p4_pd_dc_nat_flow_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
    case SWITCH_NAT_RW_TYPE_SRC_TCP:
    case SWITCH_NAT_RW_TYPE_SRC_UDP:
      status =
          p4_pd_dc_nat_src_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
      break;
    case SWITCH_NAT_RW_TYPE_DST_TCP:
    case SWITCH_NAT_RW_TYPE_DST_UDP:
      status =
          p4_pd_dc_nat_dst_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
      break;
    case SWITCH_NAT_RW_TYPE_SRC_DST_TCP:
    case SWITCH_NAT_RW_TYPE_SRC_DST_UDP:
      status = p4_pd_dc_nat_twice_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nat_rewrite_table_entry_add(
    switch_device_t device,
    switch_nat_info_t *nat_info,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  p4_pd_dc_egress_nat_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;
  switch_api_nat_info_t *api_nat_info = NULL;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  memset(&match_spec, 0, sizeof(match_spec));
  match_spec.nat_metadata_nat_rewrite_index = nat_info->rw_index;

  api_nat_info = &nat_info->api_nat_info;
  switch (api_nat_info->nat_rw_type) {
    case SWITCH_NAT_RW_TYPE_SRC: {
      p4_pd_dc_set_nat_src_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_DST: {
      p4_pd_dc_set_nat_dst_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_dst_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_DST: {
      p4_pd_dc_set_nat_src_dst_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_dst_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_UDP: {
      p4_pd_dc_set_nat_src_udp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      action_spec.action_src_port = api_nat_info->rw_src_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_udp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_DST_UDP: {
      p4_pd_dc_set_nat_dst_udp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      action_spec.action_dst_port = api_nat_info->rw_dst_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_dst_udp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_DST_UDP: {
      p4_pd_dc_set_nat_src_dst_udp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      action_spec.action_src_port = api_nat_info->rw_src_port;
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      action_spec.action_dst_port = api_nat_info->rw_dst_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_dst_udp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_TCP: {
      p4_pd_dc_set_nat_src_tcp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      action_spec.action_src_port = api_nat_info->rw_src_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_tcp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_DST_TCP: {
      p4_pd_dc_set_nat_dst_tcp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      action_spec.action_dst_port = api_nat_info->rw_dst_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_dst_tcp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
    case SWITCH_NAT_RW_TYPE_SRC_DST_TCP: {
      p4_pd_dc_set_nat_src_dst_tcp_rewrite_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(action_spec));
      action_spec.action_src_ip = SWITCH_NAT_RW_SRC_IP(api_nat_info);
      action_spec.action_src_port = api_nat_info->rw_src_port;
      action_spec.action_dst_ip = SWITCH_NAT_RW_DST_IP(api_nat_info);
      action_spec.action_dst_port = api_nat_info->rw_dst_port;
      status = p4_pd_dc_egress_nat_table_add_with_set_nat_src_dst_tcp_rewrite(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);
    } break;
  }
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_nat_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_NAT_DISABLE

  pd_status =
      p4_pd_dc_egress_nat_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_NAT_DISABLE */
#endif /* SWITCH_PD */

  return status;
}
