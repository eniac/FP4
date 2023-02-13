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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_mcast_table_entry_add(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_mode_t mc_mode,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_mrpf_group_t mrpf_group) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mgid_index);
  UNUSED(mc_mode);
  UNUSED(group_info);
  UNUSED(core_entry);
  UNUSED(vrf_entry);
  UNUSED(mrpf_group);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE
  p4_pd_dev_target_t p4_pd_device;
  switch_mcast_group_key_t *group_key;

  UNUSED(p4_pd_device);

  group_key = &(group_info->group_key);
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (core_entry) {
#ifndef P4_TUNNEL_MULTICAST_DISABLE
    if (group_key->sg_entry) {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        if (vrf_entry) {
          p4_pd_dc_outer_ipv4_multicast_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_route_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
          match_spec.multicast_metadata_ipv4_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_VRF;
          match_spec.multicast_metadata_ipv4_mcast_key =
              handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
          match_spec.ipv4_srcAddr = SWITCH_MCAST_GROUP_IPV4_SRC_IP(group_key);
          match_spec.ipv4_dstAddr = SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_table_add_with_outer_multicast_route_s_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->outer_hw_entry));
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_ipv4_multicast_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_bridge_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.multicast_metadata_ipv4_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_BD;
          match_spec.multicast_metadata_ipv4_mcast_key =
              handle_to_id(group_key->handle);
          match_spec.ipv4_srcAddr = SWITCH_MCAST_GROUP_IPV4_SRC_IP(group_key);
          match_spec.ipv4_dstAddr = SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_table_add_with_outer_multicast_bridge_s_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->outer_hw_entry));
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        if (vrf_entry) {
          p4_pd_dc_outer_ipv6_multicast_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_route_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
          match_spec.multicast_metadata_ipv6_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_VRF;
          match_spec.multicast_metadata_ipv6_mcast_key =
              handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
          SWITCH_MEMCPY(&(match_spec.ipv6_srcAddr),
                        &(SWITCH_MCAST_GROUP_IPV6_SRC_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);
          SWITCH_MEMCPY(&(match_spec.ipv6_dstAddr),
                        &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_outer_ipv6_multicast_table_add_with_outer_multicast_route_s_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->outer_hw_entry));
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_ipv6_multicast_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_bridge_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.multicast_metadata_ipv6_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_BD;
          match_spec.multicast_metadata_ipv6_mcast_key =
              handle_to_id(group_key->handle);
          SWITCH_MEMCPY(&(match_spec.ipv6_srcAddr),
                        &(SWITCH_MCAST_GROUP_IPV6_SRC_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);
          SWITCH_MEMCPY(&(match_spec.ipv6_dstAddr),
                        &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv6_multicast_table_add_with_outer_multicast_bridge_s_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->outer_hw_entry));
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    } else {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        if (vrf_entry) {
          if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
            p4_pd_dc_outer_ipv4_multicast_star_g_match_spec_t match_spec;
            p4_pd_dc_outer_multicast_route_sm_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
            match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
            match_spec.multicast_metadata_ipv4_mcast_key_type =
                SWITCH_MCAST_KEY_TYPE_VRF;
            match_spec.multicast_metadata_ipv4_mcast_key =
                handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
            match_spec.ipv4_dstAddr = SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
            match_spec.ipv4_dstAddr_mask = 0xffffffff;

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            status =
                p4_pd_dc_outer_ipv4_multicast_star_g_table_add_with_outer_multicast_route_sm_star_g_hit(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    &match_spec,
                    1000,
                    &action_spec,
                    &(group_info->outer_hw_entry));
          } else {
            p4_pd_dc_outer_ipv4_multicast_star_g_match_spec_t match_spec;
            p4_pd_dc_outer_multicast_route_bidir_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
            match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
            match_spec.multicast_metadata_ipv4_mcast_key_type =
                SWITCH_MCAST_KEY_TYPE_VRF;
            match_spec.multicast_metadata_ipv4_mcast_key =
                handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
            match_spec.ipv4_dstAddr = SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
            match_spec.ipv4_dstAddr_mask = 0xffffffff;

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv4_multicast_star_g_table_add_with_outer_multicast_route_bidir_star_g_hit(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    &match_spec,
                    1000,
                    &action_spec,
                    &(group_info->outer_hw_entry));
          }
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_ipv4_multicast_star_g_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_bridge_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.multicast_metadata_ipv4_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_BD;
          match_spec.multicast_metadata_ipv4_mcast_key =
              handle_to_id(group_key->handle);
          match_spec.ipv4_dstAddr = SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
          match_spec.ipv4_dstAddr_mask = 0xffffffff;

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_star_g_table_add_with_outer_multicast_bridge_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  1000,
                  &action_spec,
                  &(group_info->outer_hw_entry));
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        if (vrf_entry) {
          if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
            p4_pd_dc_outer_ipv6_multicast_star_g_match_spec_t match_spec;
            p4_pd_dc_outer_multicast_route_sm_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
            match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
            match_spec.multicast_metadata_ipv6_mcast_key_type =
                SWITCH_MCAST_KEY_TYPE_VRF;
            match_spec.multicast_metadata_ipv6_mcast_key =
                handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
            SWITCH_MEMCPY(&(match_spec.ipv6_dstAddr),
                          &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                          SWITCH_IPV6_PREFIX_LENGTH);
            SWITCH_MEMSET(&(match_spec.ipv6_dstAddr_mask),
                          0xFF,
                          SWITCH_IPV6_PREFIX_LENGTH);

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv6_multicast_star_g_table_add_with_outer_multicast_route_sm_star_g_hit(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    &match_spec,
                    1000,
                    &action_spec,
                    &(group_info->outer_hw_entry));
          } else {
            p4_pd_dc_outer_ipv6_multicast_star_g_match_spec_t match_spec;
            p4_pd_dc_outer_multicast_route_bidir_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef P4_OUTER_MULTICAST_BRIDGE_DISABLE
            match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
#else
            match_spec.multicast_metadata_ipv6_mcast_key_type =
                SWITCH_MCAST_KEY_TYPE_VRF;
            match_spec.multicast_metadata_ipv6_mcast_key =
                handle_to_id(group_key->handle);
#endif /* P4_OUTER_MULTICAST_BRIDGE_DISABLE */
            SWITCH_MEMCPY(&(match_spec.ipv6_dstAddr),
                          &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                          SWITCH_IPV6_PREFIX_LENGTH);
            SWITCH_MEMSET(&(match_spec.ipv6_dstAddr_mask),
                          0xFF,
                          SWITCH_IPV6_PREFIX_LENGTH);

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv6_multicast_star_g_table_add_with_outer_multicast_route_bidir_star_g_hit(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    &match_spec,
                    1000,
                    &action_spec,
                    &(group_info->outer_hw_entry));
          }
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_ipv6_multicast_star_g_match_spec_t match_spec;
          p4_pd_dc_outer_multicast_bridge_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.multicast_metadata_ipv6_mcast_key_type =
              SWITCH_MCAST_KEY_TYPE_BD;
          match_spec.multicast_metadata_ipv6_mcast_key =
              handle_to_id(group_key->handle);
          SWITCH_MEMCPY(&(match_spec.ipv6_dstAddr),
                        &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);
          SWITCH_MEMSET(
              &(match_spec.ipv6_dstAddr_mask), 0xFF, SWITCH_IPV6_PREFIX_LENGTH);

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          status =
              p4_pd_dc_outer_ipv6_multicast_star_g_table_add_with_outer_multicast_bridge_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  1000,
                  &action_spec,
                  &(group_info->outer_hw_entry));
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    }
#endif /* P4_TUNNEL_MULTICAST_DISABLE */
  }

  if (group_key->sg_entry) {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        p4_pd_dc_ipv4_multicast_route_match_spec_t match_spec;
        p4_pd_dc_multicast_route_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
        match_spec.ipv4_metadata_lkp_ipv4_sa =
            SWITCH_MCAST_GROUP_IPV4_SRC_IP(group_key);
        match_spec.ipv4_metadata_lkp_ipv4_da =
            SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
        action_spec.action_mc_index = mgid_index;
        action_spec.action_mcast_rpf_group = mrpf_group;

        pd_status =
            p4_pd_dc_ipv4_multicast_route_table_add_with_multicast_route_s_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_ipv4_multicast_bridge_match_spec_t match_spec;
        p4_pd_dc_multicast_bridge_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.ingress_metadata_bd = handle_to_id(group_key->handle);
        match_spec.ipv4_metadata_lkp_ipv4_sa =
            SWITCH_MCAST_GROUP_IPV4_SRC_IP(group_key);
        match_spec.ipv4_metadata_lkp_ipv4_da =
            SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv4_multicast_bridge_table_add_with_multicast_bridge_s_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        p4_pd_dc_ipv6_multicast_route_match_spec_t match_spec;
        p4_pd_dc_multicast_route_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
        SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_sa),
                      &(SWITCH_MCAST_GROUP_IPV6_SRC_IP(group_key)),
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_da),
                      &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                      SWITCH_IPV6_PREFIX_LENGTH);

        action_spec.action_mc_index = mgid_index;
        action_spec.action_mcast_rpf_group = mrpf_group;

        status =
            p4_pd_dc_ipv6_multicast_route_table_add_with_multicast_route_s_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_ipv6_multicast_bridge_match_spec_t match_spec;
        p4_pd_dc_multicast_bridge_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.ingress_metadata_bd = handle_to_id(group_key->handle);
        SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_sa),
                      &(SWITCH_MCAST_GROUP_IPV6_SRC_IP(group_key)),
                      SWITCH_IPV6_PREFIX_LENGTH);
        SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_da),
                      &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                      SWITCH_IPV6_PREFIX_LENGTH);

        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv6_multicast_bridge_table_add_with_multicast_bridge_s_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  } else {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
          p4_pd_dc_ipv4_multicast_route_star_g_match_spec_t match_spec;
          p4_pd_dc_multicast_route_sm_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
          match_spec.ipv4_metadata_lkp_ipv4_da =
              SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv4_multicast_route_star_g_table_add_with_multicast_route_sm_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->inner_hw_entry));
        } else {
          p4_pd_dc_ipv4_multicast_route_star_g_match_spec_t match_spec;
          p4_pd_dc_multicast_route_bidir_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
          match_spec.ipv4_metadata_lkp_ipv4_da =
              SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv4_multicast_route_star_g_table_add_with_multicast_route_bidir_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->inner_hw_entry));
        }
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_ipv4_multicast_bridge_star_g_match_spec_t match_spec;
        p4_pd_dc_multicast_bridge_star_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.ingress_metadata_bd = handle_to_id(group_key->handle);
        match_spec.ipv4_metadata_lkp_ipv4_da =
            SWITCH_MCAST_GROUP_IPV4_GRP_IP(group_key);
        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv4_multicast_bridge_star_g_table_add_with_multicast_bridge_star_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
          p4_pd_dc_ipv6_multicast_route_star_g_match_spec_t match_spec;
          p4_pd_dc_multicast_route_sm_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
          SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_da),
                        &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);

          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv6_multicast_route_star_g_table_add_with_multicast_route_sm_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->inner_hw_entry));
        } else {
          p4_pd_dc_ipv6_multicast_route_star_g_match_spec_t match_spec;
          p4_pd_dc_multicast_route_bidir_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          match_spec.l3_metadata_vrf = handle_to_id(group_key->handle);
          SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_da),
                        &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                        SWITCH_IPV6_PREFIX_LENGTH);

          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv6_multicast_route_star_g_table_add_with_multicast_route_bidir_star_g_hit(
                  switch_cfg_sess_hdl,
                  p4_pd_device,
                  &match_spec,
                  &action_spec,
                  &(group_info->inner_hw_entry));
        }
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_ipv6_multicast_bridge_star_g_match_spec_t match_spec;
        p4_pd_dc_multicast_bridge_star_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        match_spec.ingress_metadata_bd = handle_to_id(group_key->handle);
        SWITCH_MEMCPY(&(match_spec.ipv6_metadata_lkp_ipv6_da),
                      &(SWITCH_MCAST_GROUP_IPV6_GRP_IP(group_key)),
                      SWITCH_IPV6_PREFIX_LENGTH);

        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv6_multicast_bridge_star_g_table_add_with_multicast_bridge_star_g_hit(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &match_spec,
                &action_spec,
                &(group_info->inner_hw_entry));
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  }

#endif /* P4_MULTICAST_DISABLE */

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_table_entry_delete(
    switch_device_t device,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry)

{
  SWITCH_FAST_RECONFIG(device)
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(group_info);
  UNUSED(core_entry);
  UNUSED(vrf_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  switch_mcast_group_key_t *group_key;
  group_key = &(group_info->group_key);

  if (core_entry) {
#ifndef P4_TUNNEL_MULTICAST_DISABLE
    if (group_key->sg_entry) {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        status = p4_pd_dc_outer_ipv4_multicast_table_delete(
            switch_cfg_sess_hdl, device, group_info->outer_hw_entry);
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        status = p4_pd_dc_outer_ipv6_multicast_table_delete(
            switch_cfg_sess_hdl, device, group_info->outer_hw_entry);
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    } else {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        status = p4_pd_dc_outer_ipv4_multicast_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->outer_hw_entry);
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        status = p4_pd_dc_outer_ipv6_multicast_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->outer_hw_entry);
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    }
#endif /* P4_TUNNEL_MULTICAST_DISABLE */
  }

  if (group_key->sg_entry) {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_ipv4_multicast_route_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        status = p4_pd_dc_ipv4_multicast_bridge_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_ipv6_multicast_route_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        status = p4_pd_dc_ipv6_multicast_bridge_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  } else {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_ipv4_multicast_route_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        status = p4_pd_dc_ipv4_multicast_bridge_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_ipv6_multicast_route_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        status = p4_pd_dc_ipv6_multicast_bridge_star_g_table_delete(
            switch_cfg_sess_hdl, device, group_info->inner_hw_entry);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  }

#endif /* P4_MULTICAST_DISABLE */

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_table_entry_update(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_mode_t mc_mode,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_mrpf_group_t mrpf_group) {
  SWITCH_FAST_RECONFIG(device)
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mgid_index);
  UNUSED(mc_mode);
  UNUSED(group_info);
  UNUSED(core_entry);
  UNUSED(vrf_entry);
  UNUSED(mrpf_group);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE
  p4_pd_dev_target_t p4_pd_device;
  switch_mcast_group_key_t *group_key;

  UNUSED(p4_pd_device);

  group_key = &(group_info->group_key);
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (core_entry) {
#ifndef P4_TUNNEL_MULTICAST_DISABLE
    if (group_key->sg_entry) {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        if (vrf_entry) {
          p4_pd_dc_outer_multicast_route_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_table_modify_with_outer_multicast_route_s_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_multicast_bridge_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_table_modify_with_outer_multicast_bridge_s_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        if (vrf_entry) {
          p4_pd_dc_outer_multicast_route_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_outer_ipv6_multicast_table_modify_with_outer_multicast_route_s_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_multicast_bridge_s_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv6_multicast_table_modify_with_outer_multicast_bridge_s_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    } else {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
        if (vrf_entry) {
          if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
            p4_pd_dc_outer_multicast_route_sm_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            status =
                p4_pd_dc_outer_ipv4_multicast_star_g_table_modify_with_outer_multicast_route_sm_star_g_hit(
                    switch_cfg_sess_hdl,
                    device,
                    group_info->outer_hw_entry,
                    &action_spec);
          } else {
            p4_pd_dc_outer_multicast_route_bidir_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv4_multicast_star_g_table_modify_with_outer_multicast_route_bidir_star_g_hit(
                    switch_cfg_sess_hdl,
                    device,
                    group_info->outer_hw_entry,
                    &action_spec);
          }
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_multicast_bridge_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          pd_status =
              p4_pd_dc_outer_ipv4_multicast_star_g_table_modify_with_outer_multicast_bridge_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
        if (vrf_entry) {
          if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
            p4_pd_dc_outer_multicast_route_sm_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv6_multicast_star_g_table_modify_with_outer_multicast_route_sm_star_g_hit(
                    switch_cfg_sess_hdl,
                    device,
                    group_info->outer_hw_entry,
                    &action_spec);
          } else {
            p4_pd_dc_outer_multicast_route_bidir_star_g_hit_action_spec_t
                action_spec;
            SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
            action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */
            action_spec.action_mcast_rpf_group = mrpf_group;

            pd_status =
                p4_pd_dc_outer_ipv6_multicast_star_g_table_modify_with_outer_multicast_route_bidir_star_g_hit(
                    switch_cfg_sess_hdl,
                    device,
                    group_info->outer_hw_entry,
                    &action_spec);
          }
        } else {
#ifndef P4_OUTER_MULTICAST_BRIDGE_DISABLE
          p4_pd_dc_outer_multicast_bridge_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

#ifdef OUTER_MULTICAST_TREE_ENABLED
          action_spec.action_mc_index = mgid_index;
#endif /* OUTER_MULTICAST_TREE_ENABLED */

          status =
              p4_pd_dc_outer_ipv6_multicast_star_g_table_modify_with_outer_multicast_bridge_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->outer_hw_entry,
                  &action_spec);
#endif /* !P4_OUTER_MULTICAST_BRIDGE_DISABLE */
        }
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
      }
    }
#endif /* P4_TUNNEL_MULTICAST_DISABLE */
  }

  if (group_key->sg_entry) {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        p4_pd_dc_multicast_route_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_mc_index = mgid_index;
        action_spec.action_mcast_rpf_group = mrpf_group;

        pd_status =
            p4_pd_dc_ipv4_multicast_route_table_modify_with_multicast_route_s_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_multicast_bridge_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv4_multicast_bridge_table_modify_with_multicast_bridge_s_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        p4_pd_dc_multicast_route_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

        action_spec.action_mc_index = mgid_index;
        action_spec.action_mcast_rpf_group = mrpf_group;

        status =
            p4_pd_dc_ipv6_multicast_route_table_modify_with_multicast_route_s_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_multicast_bridge_s_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv6_multicast_bridge_table_modify_with_multicast_bridge_s_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  } else {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
          p4_pd_dc_multicast_route_sm_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv4_multicast_route_star_g_table_modify_with_multicast_route_sm_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->inner_hw_entry,
                  &action_spec);
        } else {
          p4_pd_dc_multicast_route_bidir_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv4_multicast_route_star_g_table_modify_with_multicast_route_bidir_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->inner_hw_entry,
                  &action_spec);
        }
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_multicast_bridge_star_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv4_multicast_bridge_star_g_table_modify_with_multicast_bridge_star_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        if (mc_mode == SWITCH_API_MCAST_IPMC_PIM_SM) {
          p4_pd_dc_multicast_route_sm_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv6_multicast_route_star_g_table_modify_with_multicast_route_sm_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->inner_hw_entry,
                  &action_spec);
        } else {
          p4_pd_dc_multicast_route_bidir_star_g_hit_action_spec_t action_spec;
          SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

          action_spec.action_mc_index = mgid_index;
          action_spec.action_mcast_rpf_group = mrpf_group;

          pd_status =
              p4_pd_dc_ipv6_multicast_route_star_g_table_modify_with_multicast_route_bidir_star_g_hit(
                  switch_cfg_sess_hdl,
                  device,
                  group_info->inner_hw_entry,
                  &action_spec);
        }
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
        p4_pd_dc_multicast_bridge_star_g_hit_action_spec_t action_spec;
        SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));

        action_spec.action_mc_index = mgid_index;

        pd_status =
            p4_pd_dc_ipv6_multicast_bridge_star_g_table_modify_with_multicast_bridge_star_g_hit(
                switch_cfg_sess_hdl,
                device,
                group_info->inner_hw_entry,
                &action_spec);
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  }

#endif /* P4_MULTICAST_DISABLE */

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_table_entry_stats_get(
    switch_device_t device,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_counter_t *counter)

{
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(group_info);
  UNUSED(core_entry);
  UNUSED(vrf_entry);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  p4_pd_counter_value_t p4_pd_counter;
  SWITCH_MEMSET(&p4_pd_counter, 0x0, sizeof(p4_pd_counter_value_t));

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  switch_mcast_group_key_t *group_key;
  group_key = &(group_info->group_key);

  if (core_entry) {
#ifndef P4_TUNNEL_MULTICAST_DISABLE
    if (group_key->sg_entry) {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
/* outer_ipv4_mulitcast_s_g_stats not defined */
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_DISABLE
/* outer_ipv6_mulitcast_s_g_stats not defined */
#endif /* P4_IPV6_DISABLE */
      }
    } else {
      if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
/* outer_ipv6_mulitcast_star_g_stats not defined */
#endif /* P4_IPV4_DISABLE */
      } else {
#ifndef P4_IPV6_DISABLE
/* outer_ipv6_mulitcast_star_g_stats not defined */
#endif /* P4_IPV6_DISABLE */
      }
    }
#endif /* P4_TUNNEL_MULTICAST_DISABLE */
  }

  if (group_key->sg_entry) {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_counter_read_ipv4_multicast_route_s_g_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            group_info->inner_hw_entry,
            switch_pd_counter_read_flags(device),
            &p4_pd_counter);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
/* ipv4_multicast_bridge_s_g_stats not defined */
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_counter_read_ipv6_multicast_route_s_g_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            group_info->inner_hw_entry,
            switch_pd_counter_read_flags(device),
            &p4_pd_counter);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
/* ipv6_multicast_bridge_s_g_stats not defined */
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  } else {
    if (SWITCH_MCAST_GROUP_IP_TYPE(group_key) == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_counter_read_ipv4_multicast_route_star_g_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            group_info->inner_hw_entry,
            switch_pd_counter_read_flags(device),
            &p4_pd_counter);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
/* ipv4_multicast_bridge_star_g_stats not defined */
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV4_DISABLE */
    } else {
#ifndef P4_IPV6_DISABLE
      if (vrf_entry) {
#ifndef P4_L3_MULTICAST_DISABLE
        status = p4_pd_dc_counter_read_ipv6_multicast_route_star_g_stats(
            switch_cfg_sess_hdl,
            p4_pd_device,
            group_info->inner_hw_entry,
            switch_pd_counter_read_flags(device),
            &p4_pd_counter);
#endif /* P4_L3_MULTICAST_DISABLE */
      } else {
#ifndef P4_L2_MULTICAST_DISABLE
/* ipv6_multicast_bridge_star_g_stats not defined */
#endif /* P4_L2_MULTICAST_DISABLE */
      }
#endif /* P4_IPV6_DISABLE */
    }
  }

  counter->num_packets = p4_pd_counter.packets;
  counter->num_bytes = p4_pd_counter.bytes;

#endif /* P4_MULTICAST_DISABLE */

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_multicast_rpf_entry_add(
    switch_device_t device,
    switch_rpf_type_t rpf_type,
    switch_mrpf_group_t mrpf_group,
    switch_bd_t bd,
    switch_pd_hdl_t *pd_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rpf_type);
  UNUSED(mrpf_group);
  UNUSED(bd);
  UNUSED(pd_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_multicast_rpf_entry_delete(switch_device_t device,
                                                     switch_rpf_type_t rpf_type,
                                                     switch_pd_hdl_t pd_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(rpf_type);
  UNUSED(pd_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_egress_ifindex_table_entry_add(
    switch_device_t device,
    switch_rid_t rid,
    switch_ifindex_t ifindex,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rid);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)
  p4_pd_dc_mcast_egress_ifindex_match_spec_t match_spec = {0};
  p4_pd_dc_set_egress_ifindex_from_rid_action_spec_t action_spec = {0};
  p4_pd_dev_target_t p4_pd_device = {0};

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  match_spec.eg_intr_md_egress_rid = rid;
  action_spec.action_egress_ifindex = ifindex;

  pd_status =
      p4_pd_dc_mcast_egress_ifindex_table_add_with_set_egress_ifindex_from_rid(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rid table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

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

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
cleanup:

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_rid_table_entry_add(
    switch_device_t device,
    switch_rid_type_t rid_type,
    switch_rid_t rid,
    switch_bd_t bd,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_id_t dmac_index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rid);
  UNUSED(bd);
  UNUSED(tunnel_type);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  p4_pd_dc_rid_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.eg_intr_md_egress_rid = rid;

  if (rid_type == SWITCH_RID_TYPE_UNICAST) {
#if defined(P4_TUNNEL_NEXTHOP_ENABLE) || defined(P4_DTEL_REPORT_ENABLE)
    p4_pd_dc_unicast_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_outer_bd = bd;
    action_spec.action_dmac_idx = dmac_index;

    pd_status = p4_pd_dc_rid_table_add_with_unicast_replica_from_rid(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &action_spec,
        entry_hdl);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#endif /* P4_TUNNEL_NEXTHOP_ENABLE */
  } else if (rid_type == SWITCH_RID_TYPE_OUTER_REPLICA) {
#if !defined(P4_MULTICAST_DISABLE)
    p4_pd_dc_outer_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_bd = bd;
    action_spec.action_dmac_idx = dmac_index;
    action_spec.action_tunnel_type = tunnel_type;
    action_spec.action_tunnel_index = tunnel_index;

    pd_status =
        p4_pd_dc_rid_table_add_with_outer_replica_from_rid(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           &action_spec,
                                                           entry_hdl);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#endif /* !P4_MULTICAST_DISABLE */
  } else {
#if !defined(P4_MULTICAST_DISABLE)
    p4_pd_dc_inner_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_bd = bd;

    pd_status =
        p4_pd_dc_rid_table_add_with_inner_replica_from_rid(switch_cfg_sess_hdl,
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

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table add failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
#endif /* !P4_MULTICAST_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_rid_table_entry_update(
    switch_device_t device,
    switch_rid_type_t rid_type,
    switch_rid_t rid,
    switch_bd_t bd,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_id_t dmac_index,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rid);
  UNUSED(bd);
  UNUSED(tunnel_type);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (rid_type == SWITCH_RID_TYPE_UNICAST) {
#if defined(P4_TUNNEL_NEXTHOP_ENABLE) || defined(P4_DTEL_REPORT_ENABLE)
    p4_pd_dc_unicast_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_outer_bd = bd;
    action_spec.action_dmac_idx = dmac_index;

    pd_status = p4_pd_dc_rid_table_modify_with_unicast_replica_from_rid(
        switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#endif /* P4_TUNNEL_NEXTHOP_ENABLE */
  } else if (rid_type == SWITCH_RID_TYPE_OUTER_REPLICA) {
#if !defined(P4_MULTICAST_DISABLE)
    p4_pd_dc_outer_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_bd = bd;
    action_spec.action_tunnel_type = tunnel_type;
    action_spec.action_tunnel_index = tunnel_index;

    pd_status = p4_pd_dc_rid_table_modify_with_outer_replica_from_rid(
        switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

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
#endif /* !P4_MULTICAST_DISABLE */

  } else {
#if !defined(P4_MULTICAST_DISABLE)
    p4_pd_dc_inner_replica_from_rid_action_spec_t action_spec;
    SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
    action_spec.action_bd = bd;

    pd_status = p4_pd_dc_rid_table_modify_with_inner_replica_from_rid(
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

    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "rid table update failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }
#endif /* !P4_MULTICAST_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mcast_egress_ifindex_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_dc_mcast_egress_ifindex_table_delete(
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
        "rid table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_MULTICAST_DISABLE || P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_rid_table_entry_delete(switch_device_t device,
                                                 switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)
  pd_status = p4_pd_dc_rid_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

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
        "rid table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_MULTICAST_DISABLE || P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_mcast_mgrp_tree_create(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_info_t *mcast_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_mc_mgrp_create(switch_cfg_mc_sess_hdl,
                                   p4_pd_device.device_id,
                                   mgid_index,
                                   &mcast_info->mgrp_hdl);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_mgrp_tree_delete(
    switch_device_t device, switch_mcast_info_t *mcast_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(mcast_info);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_mgrp_destroy(
      switch_cfg_mc_sess_hdl, device, mcast_info->mgrp_hdl);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_entry_add(switch_device_t device,
                                          switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)
  pd_status = p4_pd_mc_node_create(switch_cfg_mc_sess_hdl,
                                   device,
                                   SWITCH_MCAST_NODE_RID(node),
                                   SWITCH_MCAST_NODE_INFO_PORT_MAP(node),
                                   SWITCH_MCAST_NODE_INFO_LAG_MAP(node),
                                   &(SWITCH_MCAST_NODE_INFO_HW_ENTRY(node)));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_entry_update(switch_device_t device,
                                             switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_node_update(switch_cfg_mc_sess_hdl,
                                   device,
                                   SWITCH_MCAST_NODE_INFO_HW_ENTRY(node),
                                   SWITCH_MCAST_NODE_INFO_PORT_MAP(node),
                                   SWITCH_MCAST_NODE_INFO_LAG_MAP(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_entry_delete(switch_device_t device,
                                             switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_node_destroy(
      switch_cfg_mc_sess_hdl, device, SWITCH_MCAST_NODE_INFO_HW_ENTRY(node));
  SWITCH_MCAST_NODE_INFO_HW_ENTRY(node) = 0;

  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_mgid_table_entry_add(
    switch_device_t device, mc_mgrp_hdl_t mgid_hdl, switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mgid_hdl);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  bool xid_valid = FALSE;
  pd_status = p4_pd_mc_associate_node(switch_cfg_mc_sess_hdl,
                                      device,
                                      mgid_hdl,
                                      SWITCH_MCAST_NODE_INFO_HW_ENTRY(node),
                                      node->xid,
                                      xid_valid);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_mgid_table_entry_delete(
    switch_device_t device, mc_mgrp_hdl_t mgid_hdl, switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(mgid_hdl);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_dissociate_node(switch_cfg_mc_sess_hdl,
                                       device,
                                       mgid_hdl,
                                       SWITCH_MCAST_NODE_INFO_HW_ENTRY(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_ecmp_group_create(switch_device_t device,
                                                  switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_ecmp_create(
      switch_cfg_mc_sess_hdl, device, &(SWITCH_MCAST_ECMP_INFO_HW_ENTRY(node)));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_ecmp_group_delete(switch_device_t device,
                                                  switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_ecmp_destroy(
      switch_cfg_mc_sess_hdl, device, SWITCH_MCAST_ECMP_INFO_HW_ENTRY(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_lag_mcast_port_map_update(
    switch_device_t device,
    switch_lag_t lag_index,
    switch_mc_port_map_t port_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(lag_index);
  UNUSED(port_map);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_set_lag_membership(
      switch_cfg_mc_sess_hdl, device, lag_index, port_map);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_ecmp_entry_add(switch_device_t device,
                                               switch_mcast_node_t *ecmp_node,
                                               switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ecmp_node);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_ecmp_mbr_add(switch_cfg_mc_sess_hdl,
                                    device,
                                    SWITCH_MCAST_ECMP_INFO_HW_ENTRY(ecmp_node),
                                    SWITCH_MCAST_NODE_INFO_HW_ENTRY(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_ecmp_entry_remove(
    switch_device_t device,
    switch_mcast_node_t *ecmp_node,
    switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ecmp_node);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_ecmp_mbr_rem(switch_cfg_mc_sess_hdl,
                                    device,
                                    SWITCH_MCAST_ECMP_INFO_HW_ENTRY(ecmp_node),
                                    SWITCH_MCAST_NODE_INFO_HW_ENTRY(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_mgid_table_ecmp_entry_add(
    switch_device_t device,
    switch_pd_hdl_t mgrp_hdl,
    switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mgrp_hdl);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  bool xid_valid = FALSE;
  pd_status = p4_pd_mc_associate_ecmp(switch_cfg_mc_sess_hdl,
                                      device,
                                      mgrp_hdl,
                                      SWITCH_MCAST_ECMP_INFO_HW_ENTRY(node),
                                      node->xid,
                                      xid_valid);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mcast_mgid_table_ecmp_entry_remove(
    switch_device_t device,
    switch_pd_hdl_t mgrp_hdl,
    switch_mcast_node_t *node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(mgrp_hdl);
  UNUSED(node);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_dissociate_ecmp(switch_cfg_mc_sess_hdl,
                                       device,
                                       mgrp_hdl,
                                       SWITCH_MCAST_ECMP_INFO_HW_ENTRY(node));
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_prune_mask_table_update(
    switch_device_t device, switch_yid_t yid, switch_mc_port_map_t port_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(yid);
  UNUSED(port_map);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  pd_status = p4_pd_mc_update_port_prune_table(
      switch_cfg_mc_sess_hdl, device, yid, port_map);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ip_mcast_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  switch_pd_hdl_t entry_hdl;

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  UNUSED(entry_hdl);
  UNUSED(p4_pd_device);

#ifndef P4_IPV4_DISABLE
#ifndef P4_TUNNEL_MULTICAST_DISABLE
  pd_status = p4_pd_dc_outer_ipv4_multicast_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_outer_ipv4_multicast_star_g_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_TUNNEL_MULTICAST_DISABLE */
#ifndef P4_L2_MULTICAST_DISABLE
  pd_status = p4_pd_dc_ipv4_multicast_bridge_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  pd_status = p4_pd_dc_ipv4_multicast_bridge_star_g_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_L2_MULTICAST_DISABLE */
#ifndef P4_L3_MULTICAST_DISABLE
  pd_status = p4_pd_dc_ipv4_multicast_route_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  pd_status =
      p4_pd_dc_ipv4_multicast_route_star_g_set_default_action_multicast_route_star_g_miss(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_L3_MULTICAST_DISABLE */
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
#ifndef P4_IPV6_TUNNEL_MULTICAST_DISABLE
  pd_status = p4_pd_dc_outer_ipv6_multicast_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  pd_status = p4_pd_dc_outer_ipv6_multicast_star_g_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_TUNNEL_MULTICAST_DISABLE */
#ifndef P4_L2_MULTICAST_DISABLE
  pd_status = p4_pd_dc_ipv6_multicast_bridge_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  pd_status = p4_pd_dc_ipv6_multicast_bridge_star_g_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_L2_MULTICAST_DISABLE */
#ifndef P4_L3_MULTICAST_DISABLE
  pd_status = p4_pd_dc_ipv6_multicast_route_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  pd_status =
      p4_pd_dc_ipv6_multicast_route_star_g_set_default_action_multicast_route_star_g_miss(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mcast table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_L3_MULTICAST_DISABLE */
#endif /* P4_IPV6_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_rid_table_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_MULTICAST_DISABLE) || defined(P4_TUNNEL_NEXTHOP_ENABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_rid_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "rid table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !P4_MULTICAST_DISABLE || P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_replica_type_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_MULTICAST_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_replica_type_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "replica table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "replica table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "replica table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_replica_type_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_MULTICAST_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_replica_type_match_spec_t match_spec;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));

  match_spec.multicast_metadata_replica = 0x1;
  match_spec.egress_metadata_same_bd_check = 0;
  match_spec.egress_metadata_same_bd_check_mask = 0xFFFF;

  pd_status = p4_pd_dc_replica_type_table_add_with_set_replica_copy_bridged(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 100, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "replica type table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "replica type table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "replica type table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_mcast_global_rid_set(switch_device_t device,
                                               switch_rid_t rid) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rid);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_MULTICAST_DISABLE

  pd_status = p4_pd_mc_set_global_rid(switch_cfg_mc_sess_hdl, device, rid);
  p4_pd_mc_complete_operations(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);

#endif /* P4_MULTICAST_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

#ifdef __cplusplus
}
#endif
