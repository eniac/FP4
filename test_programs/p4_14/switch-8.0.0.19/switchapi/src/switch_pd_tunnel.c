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

switch_status_t switch_pd_outer_rmac_table_entry_add(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rmac_group);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_outer_rmac_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_rmac_match_spec_t));
  match_spec.l3_metadata_rmac_group = rmac_group;
  memcpy(match_spec.ethernet_dstAddr, mac, ETH_LEN);

  pd_status = p4_pd_dc_outer_rmac_table_add_with_outer_rmac_hit(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry add failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "outer rmac entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_outer_rmac_table_entry_update(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(rmac_group);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_outer_rmac_table_modify_with_outer_rmac_hit(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry modify failed "
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
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "outer rmac entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_outer_rmac_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  pd_status =
      p4_pd_dc_outer_rmac_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "outer rmac entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "outer rmac entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_src_table_entry_add(
    switch_device_t device,
    switch_vrf_t vrf,
    const switch_ip_addr_t *ip_addr,
    switch_tunnel_type_ingress_t tunnel_type,
    switch_ifindex_t ifindex,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(ifindex);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE

    p4_pd_dc_ipv4_src_vtep_match_spec_t v4_match_spec;
    p4_pd_dc_src_vtep_hit_action_spec_t v4_action_spec;

    SWITCH_MEMSET(&v4_match_spec, 0x0, sizeof(v4_match_spec));
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));

    v4_match_spec.l3_metadata_vrf = vrf;
    v4_match_spec.ipv4_srcAddr = ip_addr->ip.v4addr;
    v4_match_spec.tunnel_metadata_ingress_tunnel_type = tunnel_type;
    v4_action_spec.action_ifindex = ifindex;

    pd_status =
        p4_pd_dc_ipv4_src_vtep_table_add_with_src_vtep_hit(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &v4_match_spec,
                                                           &v4_action_spec,
                                                           entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry add failed "
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
      pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
      pd_entry.match_spec_size = sizeof(v4_match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_ipv6_src_vtep_match_spec_t v6_match_spec;
    p4_pd_dc_src_vtep_hit_action_spec_t v6_action_spec;

    SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));

    v6_match_spec.l3_metadata_vrf = vrf;
    v6_match_spec.tunnel_metadata_ingress_tunnel_type = tunnel_type;

    SWITCH_MEMCPY(&v6_match_spec.ipv6_srcAddr,
                  &ip_addr->ip.v6addr,
                  SWITCH_IPV6_PREFIX_LENGTH);

    v6_action_spec.action_ifindex = ifindex;

    pd_status =
        p4_pd_dc_ipv6_src_vtep_table_add_with_src_vtep_hit(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &v6_match_spec,
                                                           &v6_action_spec,
                                                           entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry add failed "
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
      pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
      pd_entry.match_spec_size = sizeof(v6_match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "src vtep entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "src vtep entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_src_table_entry_update(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_ifindex_t ifindex,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(ifindex);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE
    p4_pd_dc_src_vtep_hit_action_spec_t v4_action_spec;

    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_ifindex = ifindex;

    pd_status = p4_pd_dc_ipv4_src_vtep_table_modify_with_src_vtep_hit(
        switch_cfg_sess_hdl,
        p4_pd_device.device_id,
        entry_hdl,
        &v4_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_src_vtep_hit_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    v6_action_spec.action_ifindex = ifindex;

    pd_status = p4_pd_dc_ipv6_src_vtep_table_modify_with_src_vtep_hit(
        switch_cfg_sess_hdl,
        p4_pd_device.device_id,
        entry_hdl,
        &v6_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "src vtep entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "src vtep entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_src_table_entry_delete(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE
    pd_status = p4_pd_dc_ipv4_src_vtep_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
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
#endif /* P4_IPV4_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    pd_status = p4_pd_dc_ipv6_src_vtep_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "src vtep entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
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
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "src vtep entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "src vtep entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_table_entry_add(
    switch_device_t device,
    switch_vrf_t vrf,
    const switch_ip_addr_t *ip_addr,
    switch_tunnel_type_ingress_t tunnel_type,
    switch_tunnel_term_entry_type_t tunnel_term_type,
    switch_vni_t tunnel_vni,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE
    p4_pd_dc_ipv4_dest_vtep_match_spec_t v4_match_spec;
    SWITCH_MEMSET(&v4_match_spec, 0x0, sizeof(v4_match_spec));
    v4_match_spec.l3_metadata_vrf = vrf;
    v4_match_spec.ipv4_dstAddr = ip_addr->ip.v4addr;
    v4_match_spec.tunnel_metadata_ingress_tunnel_type = tunnel_type;

    if (tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_IPIP ||
        tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_GRE) {
      p4_pd_dc_set_tunnel_vni_and_lookup_flag_action_spec_t v4_action_spec;
      SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
      v4_action_spec.action_tunnel_vni = tunnel_vni;
      v4_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv4_dest_vtep_table_add_with_set_tunnel_vni_and_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &v4_match_spec,
              &v4_action_spec,
              entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry add failed "
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
        pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
        pd_entry.match_spec_size = sizeof(v4_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
        pd_entry.action_spec_size = sizeof(v4_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      p4_pd_dc_set_tunnel_lookup_flag_action_spec_t v4_action_spec;
      SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
      v4_action_spec.action_term_type = tunnel_term_type;
      pd_status = p4_pd_dc_ipv4_dest_vtep_table_add_with_set_tunnel_lookup_flag(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v4_match_spec,
          &v4_action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry add failed "
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
        pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
        pd_entry.match_spec_size = sizeof(v4_match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */

  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_ipv6_dest_vtep_match_spec_t v6_match_spec;
    SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
    v6_match_spec.l3_metadata_vrf = vrf;
    SWITCH_MEMCPY(&v6_match_spec.ipv6_dstAddr,
                  &ip_addr->ip.v6addr,
                  SWITCH_IPV6_PREFIX_LENGTH);
    v6_match_spec.tunnel_metadata_ingress_tunnel_type = tunnel_type;

    if (tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_IPIP ||
        tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_GRE) {
      p4_pd_dc_set_tunnel_vni_and_lookup_flag_action_spec_t v6_action_spec;
      SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
      v6_action_spec.action_tunnel_vni = tunnel_vni;
      v6_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv6_dest_vtep_table_add_with_set_tunnel_vni_and_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &v6_match_spec,
              &v6_action_spec,
              entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry add failed "
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
        pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
        pd_entry.match_spec_size = sizeof(v6_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
        pd_entry.action_spec_size = sizeof(v6_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      p4_pd_dc_set_tunnel_lookup_flag_action_spec_t v6_action_spec;
      SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
      v6_action_spec.action_term_type = tunnel_term_type;
      pd_status = p4_pd_dc_ipv6_dest_vtep_table_add_with_set_tunnel_lookup_flag(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v6_match_spec,
          &v6_action_spec,
          entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry add failed "
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
        pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
        pd_entry.match_spec_size = sizeof(v6_match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dst vtep entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dst vtep entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_table_entry_update(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_tunnel_type_ingress_t tunnel_type,
    switch_vni_t tunnel_vni,
    switch_tunnel_term_entry_type_t tunnel_term_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE
    if (tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_IPIP ||
        tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_GRE) {
      p4_pd_dc_set_tunnel_vni_and_lookup_flag_action_spec_t v4_action_spec;
      SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
      v4_action_spec.action_tunnel_vni = tunnel_vni;
      v4_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv4_dest_vtep_table_modify_with_set_tunnel_vni_and_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &v4_action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry update failed "
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
        pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
        pd_entry.action_spec_size = sizeof(v4_action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      p4_pd_dc_set_tunnel_lookup_flag_action_spec_t v4_action_spec;
      SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
      v4_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv4_dest_vtep_table_modify_with_set_tunnel_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &v4_action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry update failed "
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
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    if (tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_IPIP ||
        tunnel_type == SWITCH_TUNNEL_TYPE_INGRESS_GRE) {
      p4_pd_dc_set_tunnel_vni_and_lookup_flag_action_spec_t v6_action_spec;
      SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
      v6_action_spec.action_tunnel_vni = tunnel_vni;
      v6_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv6_dest_vtep_table_modify_with_set_tunnel_vni_and_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &v6_action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry update failed "
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
        pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
        pd_entry.action_spec_size = sizeof(v6_action_spec);
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      p4_pd_dc_set_tunnel_lookup_flag_action_spec_t v6_action_spec;
      SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
      v6_action_spec.action_term_type = tunnel_term_type;
      pd_status =
          p4_pd_dc_ipv6_dest_vtep_table_modify_with_set_tunnel_lookup_flag(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &v6_action_spec);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "dst vtep entry update failed "
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
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dst vtep entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dst vtep entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_table_entry_delete(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_TUNNEL_DISABLE
    pd_status = p4_pd_dc_ipv4_dest_vtep_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dst vtep entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
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
#endif /* P4_IPV4_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    pd_status = p4_pd_dc_ipv6_dest_vtep_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "dst vtep entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
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
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dst vtep entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dst vtep entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_smac_rewrite_table_entry_add(
    switch_device_t device,
    switch_id_t smac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(smac_index);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_smac_rewrite_match_spec_t match_spec;
  p4_pd_dc_rewrite_tunnel_smac_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_tunnel_smac_rewrite_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_rewrite_tunnel_smac_action_spec_t));

  match_spec.tunnel_metadata_tunnel_smac_index = smac_index;
  SWITCH_MEMCPY(action_spec.action_smac, mac->mac_addr, ETH_LEN);

  pd_status = p4_pd_dc_tunnel_smac_rewrite_table_add_with_rewrite_tunnel_smac(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry add failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel smac rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_smac_rewrite_table_entry_update(
    switch_device_t device,
    switch_id_t smac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(smac_index);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_rewrite_tunnel_smac_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));

  pd_status =
      p4_pd_dc_tunnel_smac_rewrite_table_modify_with_rewrite_tunnel_smac(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry update failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel smac rewrite entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_smac_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)
  pd_status = p4_pd_dc_tunnel_smac_rewrite_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel smac rewrite entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac rewrite entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_dmac_rewrite_table_entry_add(
    switch_device_t device,
    switch_id_t dmac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dmac_index);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_dmac_rewrite_match_spec_t match_spec;
  p4_pd_dc_rewrite_tunnel_dmac_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(
      &match_spec, 0x0, sizeof(p4_pd_dc_tunnel_dmac_rewrite_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_rewrite_tunnel_dmac_action_spec_t));

  match_spec.tunnel_metadata_tunnel_dmac_index = dmac_index;
  SWITCH_MEMCPY(action_spec.action_dmac, mac->mac_addr, ETH_LEN);

  pd_status = p4_pd_dc_tunnel_dmac_rewrite_table_add_with_rewrite_tunnel_dmac(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry add failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dmac rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_dmac_rewrite_table_entry_update(
    switch_device_t device,
    switch_id_t dmac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(dmac_index);
  UNUSED(mac);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_rewrite_tunnel_dmac_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));

  pd_status =
      p4_pd_dc_tunnel_dmac_rewrite_table_modify_with_rewrite_tunnel_dmac(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry update failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dmac rewrite entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_dmac_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE) || \
    defined(P4_DTEL_REPORT_ENABLE)
  pd_status = p4_pd_dc_tunnel_dmac_rewrite_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dmac rewrite entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac rewrite entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_entry_add(
    const switch_device_t device,
    const switch_tunnel_t tunnel_index,
    const switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_rewrite_match_spec_t match_spec;
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_tunnel_rewrite_match_spec_t));
  match_spec.tunnel_metadata_tunnel_index = tunnel_index;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    p4_pd_dc_set_ipv4_tunnel_rewrite_details_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(p4_pd_dc_set_ipv4_tunnel_rewrite_details_action_spec_t));

    action_spec.action_ipv4_sa = ip_addr->ip.v4addr;

    pd_status =
        p4_pd_dc_tunnel_rewrite_table_add_with_set_ipv4_tunnel_rewrite_details(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel rewrite entry add failed "
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
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_set_ipv6_tunnel_rewrite_details_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec,
        0x0,
        sizeof(p4_pd_dc_set_ipv6_tunnel_rewrite_details_action_spec_t));

    SWITCH_MEMCPY(&action_spec.action_ipv6_sa,
                  &ip_addr->ip.v6addr,
                  SWITCH_IPV6_PREFIX_LENGTH);

    pd_status =
        p4_pd_dc_tunnel_rewrite_table_add_with_set_ipv6_tunnel_rewrite_details(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel rewrite entry add failed "
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
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_entry_update(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_set_ipv4_tunnel_rewrite_details_action_spec_t action_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  // action_spec.action_dip_index = dip_index;

  pd_status =
      p4_pd_dc_tunnel_rewrite_table_modify_with_set_ipv4_tunnel_rewrite_details(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry update failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)
  pd_status = p4_pd_dc_tunnel_rewrite_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_tunnel_table_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_vni_t tunnel_vni,
    switch_rid_t ingress_rid,
    switch_tunnel_pd_type_t pd_type,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t entry_hdl[]) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_vni);
  UNUSED(ingress_rid);
  UNUSED(bd_info);
  UNUSED(bd);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_tunnel_match_spec_t match_spec;

  int entry = 0;
  for (entry = 0; entry < 3; entry++) {
    entry_hdl[entry] = SWITCH_PD_INVALID_HANDLE;
  }

  if (pd_type == SWITCH_TUNNEL_PD_TYPE_NON_IP) {
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_match_spec_t));
    match_spec.tunnel_metadata_tunnel_vni = tunnel_vni;
    match_spec.inner_ipv6_valid = FALSE;
    match_spec.inner_ipv4_valid = FALSE;

    p4_pd_dc_terminate_tunnel_inner_non_ip_action_spec_t non_ip_action_spec;
    SWITCH_MEMSET(&non_ip_action_spec,
                  0x0,
                  sizeof(p4_pd_dc_terminate_tunnel_inner_non_ip_action_spec_t));
    non_ip_action_spec.action_bd = bd;
    non_ip_action_spec.action_bd_label = bd_info->ingress_bd_label;
    non_ip_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);

    non_ip_action_spec.action_ingress_rid = ingress_rid;
    non_ip_action_spec.action_exclusion_id = bd_info->xid;

    pd_status = p4_pd_dc_tunnel_table_add_with_terminate_tunnel_inner_non_ip(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &non_ip_action_spec,
        &entry_hdl[0]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&non_ip_action_spec;
      pd_entry.action_spec_size = sizeof(non_ip_action_spec);
      pd_entry.pd_hdl = entry_hdl[0];
      switch_pd_entry_dump(device, &pd_entry);
    }

#ifndef P4_IPV4_TUNNEL_DISABLE
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_match_spec_t));
    match_spec.tunnel_metadata_tunnel_vni = tunnel_vni;
    match_spec.inner_ipv4_valid = TRUE;
    match_spec.inner_ipv6_valid = FALSE;

    p4_pd_dc_terminate_tunnel_inner_ethernet_ipv4_action_spec_t v4_action_spec;
    SWITCH_MEMSET(
        &v4_action_spec,
        0x0,
        sizeof(p4_pd_dc_terminate_tunnel_inner_ethernet_ipv4_action_spec_t));
    v4_action_spec.action_bd = bd;
    v4_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v4_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v4_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v4_action_spec.action_bd_label = bd_info->ingress_bd_label;
    v4_action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
    v4_action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
    v4_action_spec.action_igmp_snooping_enabled = bd_info->igmp_snooping;
#if !defined(P4_URPF_DISABLE)
    v4_action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
#endif /* !P4_URPF_DISABLE */
    v4_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);

    v4_action_spec.action_ingress_rid = ingress_rid;
    v4_action_spec.action_exclusion_id = bd_info->xid;

    pd_status =
        p4_pd_dc_tunnel_table_add_with_terminate_tunnel_inner_ethernet_ipv4(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &v4_action_spec,
            &entry_hdl[1]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl[1];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_match_spec_t));
    match_spec.tunnel_metadata_tunnel_vni = tunnel_vni;
    match_spec.inner_ipv6_valid = TRUE;
    match_spec.inner_ipv4_valid = FALSE;

    p4_pd_dc_terminate_tunnel_inner_ethernet_ipv6_action_spec_t v6_action_spec;
    SWITCH_MEMSET(
        &v6_action_spec,
        0x0,
        sizeof(p4_pd_dc_terminate_tunnel_inner_ethernet_ipv6_action_spec_t));
    v6_action_spec.action_bd = bd;
    v6_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v6_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v6_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v6_action_spec.action_bd_label = bd_info->ingress_bd_label;
    v6_action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
    v6_action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
    v6_action_spec.action_mld_snooping_enabled = bd_info->mld_snooping;
#if !defined(P4_URPF_DISABLE)
    v6_action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */
    v6_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);

    v6_action_spec.action_ingress_rid = ingress_rid;
    v6_action_spec.action_exclusion_id = bd_info->xid;

    pd_status =
        p4_pd_dc_tunnel_table_add_with_terminate_tunnel_inner_ethernet_ipv6(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &match_spec,
            &v6_action_spec,
            &entry_hdl[2]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl[2];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV4_TUNNEL_DISABLE
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_match_spec_t));
    match_spec.tunnel_metadata_tunnel_vni = tunnel_vni;
    match_spec.inner_ipv4_valid = TRUE;
    match_spec.inner_ipv6_valid = FALSE;

    p4_pd_dc_terminate_tunnel_inner_ipv4_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec,
                  0x0,
                  sizeof(p4_pd_dc_terminate_tunnel_inner_ipv4_action_spec_t));
    v4_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v4_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v4_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v4_action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
    v4_action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
#if !defined(P4_URPF_DISABLE)
    v4_action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
#endif /* !P4_URPF_DISABLE */

    pd_status = p4_pd_dc_tunnel_table_add_with_terminate_tunnel_inner_ipv4(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &v4_action_spec,
        &entry_hdl[1]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl[1];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE
    SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_tunnel_match_spec_t));
    match_spec.tunnel_metadata_tunnel_vni = tunnel_vni;
    match_spec.inner_ipv6_valid = TRUE;
    match_spec.inner_ipv4_valid = FALSE;

    p4_pd_dc_terminate_tunnel_inner_ipv6_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec,
                  0x0,
                  sizeof(p4_pd_dc_terminate_tunnel_inner_ipv6_action_spec_t));
    v6_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v6_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v6_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v6_action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
    v6_action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
#if !defined(P4_URPF_DISABLE)
    v6_action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */

    pd_status = p4_pd_dc_tunnel_table_add_with_terminate_tunnel_inner_ipv6(
        switch_cfg_sess_hdl,
        p4_pd_device,
        &match_spec,
        &v6_action_spec,
        &entry_hdl[2]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry add failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl[2];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_vni_t tunnel_vni,
    switch_rid_t ingress_rid,
    switch_tunnel_pd_type_t pd_type,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t entry_hdl[]) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_vni);
  UNUSED(ingress_rid);
  UNUSED(bd_info);
  UNUSED(bd);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (pd_type == SWITCH_TUNNEL_PD_TYPE_NON_IP) {
    p4_pd_dc_terminate_tunnel_inner_non_ip_action_spec_t non_ip_action_spec;
    SWITCH_MEMSET(&non_ip_action_spec, 0x0, sizeof(non_ip_action_spec));
    non_ip_action_spec.action_bd = bd;
    non_ip_action_spec.action_bd_label = bd_info->ingress_bd_label;
    non_ip_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);
    non_ip_action_spec.action_ingress_rid = ingress_rid;
    non_ip_action_spec.action_exclusion_id = bd_info->xid;

    pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_tunnel_inner_non_ip(
        switch_cfg_sess_hdl,
        p4_pd_device.device_id,
        entry_hdl[0],
        &non_ip_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&non_ip_action_spec;
      pd_entry.action_spec_size = sizeof(non_ip_action_spec);
      pd_entry.pd_hdl = entry_hdl[0];
      switch_pd_entry_dump(device, &pd_entry);
    }

#ifndef P4_IPV4_TUNNEL_DISABLE
    p4_pd_dc_terminate_tunnel_inner_ethernet_ipv4_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_bd = bd;
    v4_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v4_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v4_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v4_action_spec.action_bd_label = bd_info->ingress_bd_label;
    v4_action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
    v4_action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
    v4_action_spec.action_igmp_snooping_enabled = bd_info->igmp_snooping;
#if !defined(P4_URPF_DISABLE)
    v4_action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
#endif /* !P4_URPF_DISABLE */
    v4_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);

    v4_action_spec.action_ingress_rid = ingress_rid;
    v4_action_spec.action_exclusion_id = bd_info->xid;

    pd_status =
        p4_pd_dc_tunnel_table_modify_with_terminate_tunnel_inner_ethernet_ipv4(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl[1],
            &v4_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl[1];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE

    p4_pd_dc_terminate_tunnel_inner_ethernet_ipv6_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    v6_action_spec.action_bd = bd;
    v6_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v6_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v6_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v6_action_spec.action_bd_label = bd_info->ingress_bd_label;
    v6_action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
    v6_action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
    v6_action_spec.action_mld_snooping_enabled = bd_info->mld_snooping;
#if !defined(P4_URPF_DISABLE)
    v6_action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */
    v6_action_spec.action_stats_idx = SWITCH_BD_STATS_START_INDEX(bd_info);

    v6_action_spec.action_ingress_rid = ingress_rid;
    v6_action_spec.action_exclusion_id = bd_info->xid;

    pd_status =
        p4_pd_dc_tunnel_table_modify_with_terminate_tunnel_inner_ethernet_ipv6(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl[2],
            &v6_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl[2];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  } else {
#ifndef P4_IPV4_TUNNEL_DISABLE
    p4_pd_dc_terminate_tunnel_inner_ipv4_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v4_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v4_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v4_action_spec.action_ipv4_unicast_enabled = bd_info->ipv4_unicast;
    v4_action_spec.action_ipv4_multicast_enabled = bd_info->ipv4_multicast;
#if !defined(P4_URPF_DISABLE)
    v4_action_spec.action_ipv4_urpf_mode = bd_info->ipv4_urpf_mode;
#endif /* !P4_URPF_DISABLE */

    pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_tunnel_inner_ipv4(
        switch_cfg_sess_hdl,
        p4_pd_device.device_id,
        entry_hdl[1],
        &v4_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl[1];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_terminate_tunnel_inner_ipv6_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    v6_action_spec.action_vrf = handle_to_id(bd_info->vrf_handle);
    v6_action_spec.action_rmac_group = handle_to_id(bd_info->rmac_handle);
    v6_action_spec.action_mrpf_group = handle_to_id(bd_info->mrpf_group);
    v6_action_spec.action_ipv6_unicast_enabled = bd_info->ipv6_unicast;
    v6_action_spec.action_ipv6_multicast_enabled = bd_info->ipv6_multicast;
#if !defined(P4_URPF_DISABLE)
    v6_action_spec.action_ipv6_urpf_mode = bd_info->ipv6_urpf_mode;
#endif /* !P4_URPF_DISABLE */

    pd_status = p4_pd_dc_tunnel_table_modify_with_terminate_tunnel_inner_ipv6(
        switch_cfg_sess_hdl,
        p4_pd_device.device_id,
        entry_hdl[2],
        &v6_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl[2];
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl[]) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  int entry = 0;
  for (entry = 0; entry < 3; entry++) {
    if (!SWITCH_PD_HANDLE_VALID(entry_hdl[entry])) {
      continue;
    }
    pd_status = p4_pd_dc_tunnel_table_delete(
        switch_cfg_sess_hdl, device, entry_hdl[entry]);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel rewrite entry delete failed "
          "on device %d : table %s action %s\n",
          device,
          switch_pd_table_id_to_string(0),
          switch_pd_action_id_to_string(0));
      goto cleanup;
    }

    if (switch_pd_log_level_debug()) {
      switch_pd_dump_entry_t pd_entry;
      SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
      pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
      pd_entry.match_spec_size = 0;
      pd_entry.action_spec_size = 0;
      pd_entry.pd_hdl = entry_hdl[entry];
      switch_pd_entry_dump(device, &pd_entry);
    }
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_egress_vni_table_entry_add(
    switch_device_t device,
    switch_bd_t egress_bd,
    switch_vni_t tunnel_vni,
    switch_tunnel_pd_type_t pd_type,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(egress_bd);
  UNUSED(tunnel_vni);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE
  p4_pd_dc_egress_vni_match_spec_t match_spec;
  p4_pd_dc_set_egress_tunnel_vni_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(p4_pd_dc_egress_vni_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0x0, sizeof(p4_pd_dc_set_egress_tunnel_vni_action_spec_t));

  match_spec.egress_metadata_bd = egress_bd;
  action_spec.action_vnid = tunnel_vni;

  pd_status = p4_pd_dc_egress_vni_table_add_with_set_egress_tunnel_vni(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress vni entry add failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress vni entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress vni entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_egress_vni_table_entry_update(
    switch_device_t device,
    switch_bd_t egress_bd,
    switch_vni_t tunnel_vni,
    switch_tunnel_pd_type_t pd_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(egress_bd);
  UNUSED(tunnel_vni);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dc_set_egress_tunnel_vni_action_spec_t action_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  action_spec.action_vnid = tunnel_vni;

  pd_status = p4_pd_dc_egress_vni_table_modify_with_set_egress_tunnel_vni(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress vni entry update failed "
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress vni entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress vni entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}
switch_status_t switch_pd_egress_vni_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  pd_status =
      p4_pd_dc_egress_vni_table_delete(switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_decap_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_decap_process_inner_match_spec_t i_match_spec;
  p4_pd_dc_tunnel_decap_process_outer_match_spec_t o_match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  /* inner tcp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_inner_match_spec_t));
  i_match_spec.inner_tcp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_inner_table_add_with_decap_inner_tcp(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* inner udp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_inner_match_spec_t));
  i_match_spec.inner_udp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_inner_table_add_with_decap_inner_udp(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* inner icmp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_inner_match_spec_t));
  i_match_spec.inner_icmp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_inner_table_add_with_decap_inner_icmp(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* inner uknown */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_inner_match_spec_t));
  pd_status =
      p4_pd_dc_tunnel_decap_process_inner_table_add_with_decap_inner_unknown(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* vxlan, inner ipv4 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_VXLAN;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_vxlan_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* vxlan, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_VXLAN;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_vxlan_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* vxlan, inner non ip */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_VXLAN;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_vxlan_inner_non_ip(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_GENEVE_DISABLE
  /* geneve, inner ipv4 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GENEVE;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_genv_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* geneve, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GENEVE;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_genv_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* geneve, inner non ip */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GENEVE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_genv_inner_non_ip(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_GENEVE_DISABLE */

#ifndef P4_NVGRE_DISABLE

  /* nvgre, inner ipv4 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_NVGRE;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_nvgre_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* nvgre, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_NVGRE;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_nvgre_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* nvgre, inner non ip */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_NVGRE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_nvgre_inner_non_ip(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#endif /* P4_NVGRE_DISABLE */

  /* gre, inner ipv4 */
  memset(&o_match_spec,
         0,
         sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GRE;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_gre_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* gre, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GRE;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_gre_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* gre, inner non ip */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_GRE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_gre_inner_non_ip(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  /* ipip, inner ipv4 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_IPIP;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_ip_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* ipip, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_IPIP;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_ip_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

#ifdef P4_SRV6_ENABLE
  /* srv6, inner ipv4 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_SRV6;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_sr_inner_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* srv6, inner ipv6 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_SRV6;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_sr_inner_ipv6(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* geneve, inner non ip */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_SRV6;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_sr_inner_non_ip(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_SRV6_ENABLE */

#ifndef P4_MPLS_DISABLE
  /* mpls, inner_ipv4, pop 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_1;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv4_pop1(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, inner_ipv4, pop 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_2;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv4_pop2(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, inner_ipv4, pop 3 */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_3;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv4_pop3(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* mpls, inner_ipv6, pop 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_1;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv6_pop1(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, inner_ipv6, pop 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_2;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv6_pop2(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, inner_ipv6, pop 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L3VPN_NUM_LABELS_3;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ipv6_pop3(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* mpls, ethernet, inner_ipv4, pop 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_1;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv4_pop1(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv4, pop 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_2;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv4_pop2(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv4, pop 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_3;
  o_match_spec.inner_ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv4_pop3(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* mpls, ethernet, inner_ipv6, pop 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_1;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv6_pop1(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv6, pop 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_2;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv6_pop2(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv6, pop 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_3;
  o_match_spec.inner_ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_ipv6_pop3(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* mpls, ethernet, non_ip, pop 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_1;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_non_ip_pop1(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv6, pop 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_2;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_non_ip_pop2(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, inner_ipv6, pop 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_decap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_L2VPN_NUM_LABELS_3;
  pd_status =
      p4_pd_dc_tunnel_decap_process_outer_table_add_with_decap_mpls_inner_ethernet_non_ip_pop3(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_MPLS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel decap init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel decap init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_encap_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_tunnel_encap_process_outer_match_spec_t o_match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  UNUSED(p4_pd_device);
  UNUSED(o_match_spec);

#if !defined(P4_TUNNEL_DISABLE) || \
    (!defined(P4_MIRROR_NEXTHOP_DISABLE) && !defined(P4_DTEL_REPORT_ENABLE))

  p4_pd_dc_tunnel_encap_process_inner_match_spec_t i_match_spec;

  /* ipv4, tcp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv4_valid = TRUE;
  i_match_spec.tcp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv4_tcp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv4, udp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv4_valid = TRUE;
  i_match_spec.udp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv4_udp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv4, icmp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv4_valid = TRUE;
  i_match_spec.icmp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv4_icmp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv4, uknown */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv4_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv4_unknown_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* ipv6, tcp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv6_valid = TRUE;
  i_match_spec.tcp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv6_tcp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv6, udp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv6_valid = TRUE;
  i_match_spec.udp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv6_udp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv6, icmp */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv6_valid = TRUE;
  i_match_spec.icmp_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv6_icmp_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv6, uknown */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  i_match_spec.ipv6_valid = TRUE;
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_ipv6_unknown_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* non ip */
  SWITCH_MEMSET(&i_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_inner_match_spec_t));
  pd_status =
      p4_pd_dc_tunnel_encap_process_inner_table_add_with_inner_non_ip_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &i_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&i_match_spec;
    pd_entry.match_spec_size = sizeof(i_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#endif /* !TUNNEL_DISABLE || (!MIRROR_DISABLE && !DTEL_REPORT_ENABLE) */

#ifndef P4_TUNNEL_DISABLE

  /* default entry */
  pd_status = p4_pd_dc_tunnel_encap_process_outer_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* ipv4 vxlan */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_vxlan_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_vxlan_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* ipv6 vxlan */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_VXLAN;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_vxlan_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_VXLAN;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_vxlan_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_GENEVE_DISABLE
  /* ipv4 geneve */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GENEVE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_genv_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GENEVE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_genv_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE

  /* ipv6 geneve */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GENEVE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_genv_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GENEVE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_genv_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */
#endif /* P4_GENEVE_DISABLE */

#ifndef P4_NVGRE_DISABLE

  /* ipv4 nvgre */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_NVGRE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_nvgre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));

  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_NVGRE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_nvgre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE

  /* ipv6 nvgre */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_NVGRE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_nvgre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_NVGRE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_nvgre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */
#endif /* P4_NVGRE_DISABLE */

  /* ipv4 gre */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GRE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_gre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GRE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_gre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* ipv6 gre */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GRE;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_gre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GRE;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_gre_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#endif /* P4_IPV6_TUNNEL_DISABLE */

  /* ipv4 ip */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_IP;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_ip_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_IP;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_ip_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* ipv6 ip */
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_IP;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_ip_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
  SWITCH_MEMSET(&o_match_spec,
                0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_IP;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv6_ip_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

#endif /* P4_IPV6_TUNNEL_DISABLE */

#ifdef P4_SRV6_ENABLE
  /* srv6 ipv6  */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_SRV6;
  o_match_spec.multicast_metadata_replica = false;
  pd_status = p4_pd_dc_tunnel_encap_process_outer_table_add_with_srv6_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_SRV6;
  o_match_spec.multicast_metadata_replica = true;
  pd_status = p4_pd_dc_tunnel_encap_process_outer_table_add_with_srv6_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_SRV6_ENABLE */

#ifndef P4_MPLS_DISABLE
  /* mpls, ethernet, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ethernet, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ip, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ip, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls, ip, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_MPLS_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#ifdef P4_MPLS_UDP_ENABLE
  /* mpls ipv4 udp*/
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 0;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_push_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 0;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_push_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
  /* mpls ipv4 udp, ethernet, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv4 udp , ethernet, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv4 udp, ethernet, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv4 udp, ip, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv4 udp, ip, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv4 udp, ip, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv4_udp_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  /* mpls ipv6 udp */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 0;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_push_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 0;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_push_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp, ethernet, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp , ethernet, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp, ethernet, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L2VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ethernet_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp, ip, push 1 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 1;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push1_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp, ip, push 2 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 2;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push2_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  /* mpls ipv6 udp, ip, push 3 */
  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = false;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  SWITCH_MEMSET(&o_match_spec,
                0x0,
                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
  o_match_spec.tunnel_metadata_egress_tunnel_type =
      SWITCH_TUNNEL_TYPE_EGRESS_IPV6_MPLS_UDP_L3VPN;
  o_match_spec.tunnel_metadata_egress_header_count = 3;
  o_match_spec.multicast_metadata_replica = true;
  pd_status =
      p4_pd_dc_tunnel_encap_process_outer_table_add_with_mpls_ipv6_udp_ip_push3_rewrite(
          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
    pd_entry.match_spec_size = sizeof(o_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }
#endif /* P4_IPV6_TUNNEL_DISABLE */
#endif /* P4_MPLS_UDP_ENABLE */
#endif /* P4_MPLS_DISABLE */
#endif /* P4_TUNNEL_DISABLE */

#if !defined(P4_MIRROR_NEXTHOP_DISABLE) && !defined(P4_MIRROR_DISABLE)
//  /* ipv4 erspan */
//  SWITCH_MEMSET(&o_match_spec,
//                0x0,
//                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
//  o_match_spec.tunnel_metadata_egress_tunnel_type =
//      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_ERSPAN_T3;
//  o_match_spec.multicast_metadata_replica = false;
//  pd_status =
//      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_erspan_t3_rewrite(
//          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
//  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
//    goto cleanup;
//  }
//
//  if (switch_pd_log_level_debug()) {
//    switch_pd_dump_entry_t pd_entry;
//    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
//    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
//    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
//    pd_entry.match_spec_size = sizeof(o_match_spec);
//    pd_entry.action_spec_size = 0;
//    pd_entry.pd_hdl = entry_hdl;
//    switch_pd_entry_dump(device, &pd_entry);
//  }
//
//  /* ipv4 erspan using mgid trees for tunnels*/
//  SWITCH_MEMSET(&o_match_spec,
//                0x0,
//                sizeof(p4_pd_dc_tunnel_encap_process_outer_match_spec_t));
//  o_match_spec.tunnel_metadata_egress_tunnel_type =
//      SWITCH_TUNNEL_TYPE_EGRESS_IPV4_ERSPAN_T3;
//  o_match_spec.multicast_metadata_replica = true;
//  pd_status =
//      p4_pd_dc_tunnel_encap_process_outer_table_add_with_ipv4_erspan_t3_rewrite(
//          switch_cfg_sess_hdl, p4_pd_device, &o_match_spec, &entry_hdl);
//  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
//    goto cleanup;
//  }
//
//  if (switch_pd_log_level_debug()) {
//    switch_pd_dump_entry_t pd_entry;
//    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
//    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
//    pd_entry.match_spec = (switch_uint8_t *)&o_match_spec;
//    pd_entry.match_spec_size = sizeof(o_match_spec);
//    pd_entry.action_spec_size = 0;
//    pd_entry.pd_hdl = entry_hdl;
//    switch_pd_entry_dump(device, &pd_entry);
//  }

#endif /* P4_MIRROR_NEXTHOP_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel encap init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel encap init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_rewrite_table_fabric_entry_add(
    switch_device_t device,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_type);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite fabric entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite fabric entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_rewrite_table_fabric_entry_update(
    switch_device_t device,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_type);
  UNUSED(tunnel_index);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rewrite fabric entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rewrite fabric entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_rewrite_table_entry_add(
    const switch_device_t device,
    const switch_id_t tunnel_dst_index,
    const switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_dst_index);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    p4_pd_dc_tunnel_dst_rewrite_match_spec_t v4_match_spec;
    p4_pd_dc_rewrite_tunnel_ipv4_dst_action_spec_t v4_action_spec;
    SWITCH_MEMSET(
        &v4_match_spec, 0x0, sizeof(p4_pd_dc_tunnel_dst_rewrite_match_spec_t));
    SWITCH_MEMSET(&v4_action_spec,
                  0x0,
                  sizeof(p4_pd_dc_rewrite_tunnel_ipv4_dst_action_spec_t));
    v4_match_spec.tunnel_metadata_tunnel_dst_index = tunnel_dst_index;
    v4_action_spec.action_ip = ip_addr->ip.v4addr;
    pd_status =
        p4_pd_dc_tunnel_dst_rewrite_table_add_with_rewrite_tunnel_ipv4_dst(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v4_match_spec,
            &v4_action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel dst ipv4 rewrite entry add failed "
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
      pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
      pd_entry.match_spec_size = sizeof(v4_match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_tunnel_dst_rewrite_match_spec_t v6_match_spec;
    p4_pd_dc_rewrite_tunnel_ipv6_dst_action_spec_t v6_action_spec;
    SWITCH_MEMSET(
        &v6_match_spec, 0x0, sizeof(p4_pd_dc_tunnel_dst_rewrite_match_spec_t));
    SWITCH_MEMSET(&v6_action_spec,
                  0,
                  sizeof(p4_pd_dc_rewrite_tunnel_ipv6_dst_action_spec_t));
    v6_match_spec.tunnel_metadata_tunnel_dst_index = tunnel_dst_index;
    SWITCH_MEMCPY(&v6_action_spec.action_ip,
                  &ip_addr->ip.v6addr,
                  SWITCH_IPV6_PREFIX_LENGTH);
    pd_status =
        p4_pd_dc_tunnel_dst_rewrite_table_add_with_rewrite_tunnel_ipv6_dst(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v6_match_spec,
            &v6_action_spec,
            entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel dst ipv6 rewrite entry add failed "
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
      pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
      pd_entry.match_spec_size = sizeof(v6_match_spec);
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = *entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dst ip rewrite entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dst ip rewrite entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_rewrite_table_entry_update(
    const switch_device_t device,
    const switch_id_t tunnel_dst_index,
    const switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_dst_index);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    p4_pd_dc_rewrite_tunnel_ipv4_dst_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_ip = ip_addr->ip.v4addr;
    pd_status =
        p4_pd_dc_tunnel_dst_rewrite_table_modify_with_rewrite_tunnel_ipv4_dst(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl,
            &v4_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel dst ipv4 rewrite entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
      pd_entry.action_spec_size = sizeof(v4_action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
  } else {
#ifndef P4_IPV6_TUNNEL_DISABLE
    p4_pd_dc_rewrite_tunnel_ipv6_dst_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    SWITCH_MEMCPY(&v6_action_spec.action_ip,
                  &ip_addr->ip.v6addr,
                  SWITCH_IPV6_PREFIX_LENGTH);
    pd_status =
        p4_pd_dc_tunnel_dst_rewrite_table_modify_with_rewrite_tunnel_ipv6_dst(
            switch_cfg_sess_hdl,
            p4_pd_device.device_id,
            entry_hdl,
            &v6_action_spec);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "tunnel dst ipv6 rewrite entry update failed "
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
      pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
      pd_entry.action_spec_size = sizeof(v6_action_spec);
      pd_entry.pd_hdl = entry_hdl;
      switch_pd_entry_dump(device, &pd_entry);
    }
#endif /* P4_IPV6_TUNNEL_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dst ip rewrite entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dst ip rewrite entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_ip_dst_rewrite_table_entry_delete(
    const switch_device_t device, const switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  pd_status = p4_pd_dc_tunnel_dst_rewrite_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel dst ip rewrite entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
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

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dst ip rewrite entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dst ip rewrite entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_ingress_fabric_table_entry_add(
    switch_device_t device, switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_fabric_ingress_dst_lkp_match_spec_t match_spec;
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.fabric_header_dstDevice = device;
  pd_status =
      p4_pd_dc_fabric_ingress_dst_lkp_table_add_with_terminate_cpu_packet(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);
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

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress fabric table add failed "
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
        "ingress fabric table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress fabric table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_outer_rmac_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_outer_rmac_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "outer rmac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "outer rmac table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "outer rmac table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_src_vtep_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)

  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_IPV4_TUNNEL_DISABLE
  pd_status = p4_pd_dc_ipv4_src_vtep_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 src vtep table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE

  pd_status = p4_pd_dc_ipv6_src_vtep_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 src vtep table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV6_TUNNEL_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "src vtep table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "src vtep table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_dest_vtep_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_L3_DISABLE)

  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_IPV4_TUNNEL_DISABLE
  pd_status = p4_pd_dc_ipv4_dest_vtep_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 dst vtep table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE

  pd_status = p4_pd_dc_ipv6_dest_vtep_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 dst vtep table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV6_TUNNEL_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE && P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "dst vtep table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "dst vtep table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_smac_rewrite_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_tunnel_smac_rewrite_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel smac table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel smac table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_dmac_rewrite_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) || !defined(P4_MIRROR_NEXTHOP_DISABLE)

  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_tunnel_dmac_rewrite_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE || P4_MIRROR_NEXTHOP_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel dmac table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel dmac table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_rewrite_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  switch_pd_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_tunnel_rewrite_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel rewrite table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel rewrite table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_adjust_lkp_fields_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_PARSING_DISABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_adjust_lkp_fields_set_default_action_non_ip_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "adjust lkp non ip table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  p4_pd_dc_adjust_lkp_fields_match_spec_t match_spec;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv4_valid = true;
  pd_status = p4_pd_dc_adjust_lkp_fields_table_add_with_ipv4_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "adjust lkp ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_IPV6_DISABLE
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv6_valid = true;
  pd_status = p4_pd_dc_adjust_lkp_fields_table_add_with_ipv6_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "adjust lkp ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_PARSING_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "adjust lkp table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "adjust lkp table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(entry_hdl);

#ifdef SWITCH_PD
#ifndef P4_TUNNEL_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  switch_uint16_t priority = 1000;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_tunnel_check_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  // MPLS packets => tunnel lookup eligible
  priority = 100;
  p4_pd_dc_tunnel_check_match_spec_t match_spec;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS;
  match_spec.tunnel_metadata_ingress_tunnel_type_mask = 0xFF;
  match_spec.tunnel_metadata_tunnel_lookup = 0x1;
  match_spec.tunnel_metadata_tunnel_lookup_mask = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit = 0x0;
  match_spec.tunnel_metadata_src_vtep_hit_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type_mask = 0x0;
  pd_status = p4_pd_dc_tunnel_check_table_add_with_tunnel_check_pass(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  // MPLS UDP packets that are tunnel lookup eligible
  priority++;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_ingress_tunnel_type =
      SWITCH_TUNNEL_TYPE_INGRESS_MPLS_UDP;
  match_spec.tunnel_metadata_ingress_tunnel_type_mask = 0xFF;
  match_spec.tunnel_metadata_tunnel_lookup = 0x1;
  match_spec.tunnel_metadata_tunnel_lookup_mask = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit = 0x0;
  match_spec.tunnel_metadata_src_vtep_hit_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type_mask = 0x0;
  pd_status = p4_pd_dc_tunnel_check_table_add_with_tunnel_check_pass(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  // P2P tunnel : Dst vtep hit and src vtep hit ==> tunnel lookup eligible
  priority++;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_ingress_tunnel_type = 0x0;
  match_spec.tunnel_metadata_ingress_tunnel_type_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_lookup = 0x1;
  match_spec.tunnel_metadata_tunnel_lookup_mask = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit_mask = 0x1;
  match_spec.tunnel_metadata_tunnel_term_type = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type_mask = 0x1;
  status = p4_pd_dc_tunnel_check_table_add_with_tunnel_check_pass(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
  // MP2P tunnel : Dst vtep hit  ==> tunnel lookup eligible
  priority++;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_ingress_tunnel_type = 0x0;
  match_spec.tunnel_metadata_ingress_tunnel_type_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_lookup = 0x1;
  match_spec.tunnel_metadata_tunnel_lookup_mask = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit = 0x0;
  match_spec.tunnel_metadata_src_vtep_hit_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type = 0x1;
  match_spec.tunnel_metadata_tunnel_term_type_mask = 0x1;
  status = p4_pd_dc_tunnel_check_table_add_with_tunnel_check_pass(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  // Dst vtep miss (multicast) ==> tunnel lookup eligible
  priority++;
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.tunnel_metadata_ingress_tunnel_type = 0x0;
  match_spec.tunnel_metadata_ingress_tunnel_type_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_lookup = 0x0;
  match_spec.tunnel_metadata_tunnel_lookup_mask = 0x1;
  match_spec.tunnel_metadata_src_vtep_hit = 0x0;
  match_spec.tunnel_metadata_src_vtep_hit_mask = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type = 0x0;
  match_spec.tunnel_metadata_tunnel_term_type_mask = 0x0;
  pd_status = p4_pd_dc_tunnel_check_table_add_with_tunnel_check_pass(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, priority, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_tunnel_set_default_action_tunnel_lookup_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_tunnel_lookup_miss_set_default_action_non_ip_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  p4_pd_dc_tunnel_lookup_miss_match_spec_t match1_spec;
  memset(&match1_spec, 0, sizeof(match1_spec));
  match1_spec.ipv4_valid = true;
  pd_status = p4_pd_dc_tunnel_lookup_miss_table_add_with_ipv4_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &match1_spec, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match1_spec;
    pd_entry.match_spec_size = sizeof(match1_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_IPV6_TUNNEL_DISABLE
  memset(&match1_spec, 0, sizeof(match1_spec));
  match1_spec.ipv6_valid = true;
  pd_status = p4_pd_dc_tunnel_lookup_miss_table_add_with_ipv6_lkp(
      switch_cfg_sess_hdl, p4_pd_device, &match1_spec, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match1_spec;
    pd_entry.match_spec_size = sizeof(match1_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV6_TUNNEL_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel table entry default add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_validate_mpls_packet_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_TUNNEL_DISABLE) && !defined(P4_MPLS_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_validate_mpls_packet_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.mpls_0__valid = 0x1;
#ifdef P4_MPLS_UDP_ENABLE
  match_spec.tunnel_metadata_mpls_in_udp = 0;
#endif /*P4_MPLS_UDP_ENABLE*/
  pd_status = p4_pd_dc_validate_mpls_packet_table_add_with_set_valid_mpls_label(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
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
        "validate mpls table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifdef P4_MPLS_UDP_ENABLE
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.mpls_0__valid = 0x1;
  match_spec.tunnel_metadata_mpls_in_udp = 1;
  pd_status =
      p4_pd_dc_validate_mpls_packet_table_add_with_set_valid_mpls_udp_label(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
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
        "validate mpls table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /*P4_MPLS_UDP_ENABLE*/
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* !defined(P4_TUNNEL_DISABLE) && !defined(P4_MPLS_DISABLE) */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "validate mpls table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "validate mpls table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_fabric_header_table_entry_init(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef FABRIC_ENABLE

  p4_pd_mbr_hdl_t pd_mbr_hdl;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_fabric_lag_action_profile_add_member_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &pd_mbr_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fabric header table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_fabric_lag_set_default_entry(
      switch_cfg_sess_hdl, p4_pd_device, pd_mbr_hdl, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "fabric header table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* FABRIC_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fabric header table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fabric header table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_tunnel_mgid_entry_add(switch_device_t device,
                                                switch_tunnel_t tunnel_index,
                                                switch_mgid_t mc_index,
                                                switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(mc_index);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_TUNNEL_NEXTHOP_ENABLE)
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  p4_pd_dc_tunnel_to_mgid_mapping_match_spec_t match_spec;
  p4_pd_dc_set_tunnel_mgid_action_spec_t action_spec;

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));

  match_spec.tunnel_metadata_tunnel_dst_index = tunnel_index;
  action_spec.action_mc_index = mc_index;

  pd_status = p4_pd_dc_tunnel_to_mgid_mapping_table_add_with_set_tunnel_mgid(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, &action_spec, entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel mgid create failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel mgid create success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel mgid create failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_mgid_entry_update(switch_device_t device,
                                                   switch_tunnel_t tunnel_index,
                                                   switch_mgid_t mc_index,
                                                   switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(tunnel_index);
  UNUSED(mc_index);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_TUNNEL_NEXTHOP_ENABLE)

  p4_pd_dc_set_tunnel_mgid_action_spec_t action_spec;

  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));

  action_spec.action_mc_index = mc_index;

  pd_status = p4_pd_dc_tunnel_to_mgid_mapping_table_modify_with_set_tunnel_mgid(
      switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "tunnel mgid update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel mgid update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel mgid update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_tunnel_mgid_entry_delete(switch_device_t device,
                                                   switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_TUNNEL_NEXTHOP_ENABLE)

  pd_status = p4_pd_dc_tunnel_to_mgid_mapping_table_delete(
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
        "tunnel mgid delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_TUNNEL_NEXTHOP_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "tunnel mgid delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "tunnel mgid delete failed "
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
