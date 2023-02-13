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

#include "switchapi/switch_mirror.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

switch_status_t switch_mirror_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_mirror_table_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_mirror_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_mirror_init(switch_device_t device) {
  switch_mirror_context_t *mirror_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  mirror_ctx = SWITCH_MALLOC(device, sizeof(switch_mirror_context_t), 0x1);
  if (!mirror_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("mirror init failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_MIRROR, (void *)mirror_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror init failed for device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_MIRROR, SWITCH_MIRROR_SESSIONS_MAX);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_new(device,
                                       SWITCH_MIRROR_SESSIONS_MAX,
                                       FALSE,
                                       &mirror_ctx->session_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  tmp_status = switch_mirror_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_mirror_free(switch_device_t device) {
  switch_mirror_context_t *mirror_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MIRROR, (void **)&mirror_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror free failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_MIRROR);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror free failed for device %d:",
                     device,
                     switch_error_to_string(status));
  }

  status =
      switch_api_id_allocator_destroy(device, mirror_ctx->session_id_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror free failed for device %d:",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_mirror_session_create_internal(
    switch_device_t device,
    switch_api_mirror_info_t *api_mirror_info,
    switch_handle_t *mirror_handle) {
  switch_mirror_context_t *mirror_ctx = NULL;
  switch_mirror_info_t *mirror_info = NULL;
  switch_nhop_info_t *tun_nhop_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t urif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t orif_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t member_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t tunnel_nhop_handle = SWITCH_API_INVALID_HANDLE;
  bool session_in_use = FALSE;
  switch_dev_port_t dev_port = 0;
  switch_api_route_entry_t route_entry = {0};
  switch_api_interface_info_t api_intf_info = {0};
  switch_api_nhop_info_t tmp_api_nhop_info = {0};
  switch_api_tunnel_info_t api_tunnel_info = {0};
  switch_api_rif_info_t api_rif_info = {0};
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MIRROR, (void **)&mirror_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d:  "
        "mirror context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if ((api_mirror_info->session_type != SWITCH_MIRROR_SESSION_TYPE_SIMPLE) &&
      (api_mirror_info->session_type != SWITCH_MIRROR_SESSION_TYPE_COALESCE)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d:  "
        "mirror type invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (api_mirror_info->session_id) {
    session_in_use = switch_api_id_allocator_is_set(
        device, mirror_ctx->session_id_allocator, api_mirror_info->session_id);
    if (session_in_use) {
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "session already exists:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }

    handle = switch_mirror_handle_set(device, api_mirror_info->session_id);
    if (handle == SWITCH_API_INVALID_HANDLE) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "mirror handle set failed:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }
  } else {
    handle = switch_mirror_handle_create(device);
    if (handle == SWITCH_API_INVALID_HANDLE) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "mirror handle set failed:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }
    api_mirror_info->session_id = handle_to_id(handle);
  }

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(handle));

  status = switch_mirror_get(device, handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d session %d: "
        "mirror get failed:(%s)\n",
        device,
        api_mirror_info->session_id,
        switch_error_to_string(status));
    return status;
  }

  if (api_mirror_info->session_type == SWITCH_MIRROR_SESSION_TYPE_COALESCE) {
    if ((api_mirror_info->extract_len > 80) ||
        (api_mirror_info->extract_len & 0x3)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "coalesce extract length invalid:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }

    if ((api_mirror_info->direction != SWITCH_API_DIRECTION_EGRESS) &&
        (api_mirror_info->direction != SWITCH_API_DIRECTION_BOTH)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "direction invalid:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }

    if (!SWITCH_MIRROR_SESSION_ID_COALESCING(api_mirror_info->session_id)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "mirror session id invalid:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(
        &mirror_info->int_coal_pkt_hdr, 0x0, sizeof(switch_coal_pkt_hdr_t));

    // 2 MS bytes are reserved
    mirror_info->int_coal_pkt_hdr.reg_hdr0 = api_mirror_info->session_id;
    mirror_info->int_hdr_len = (sizeof(switch_coal_pkt_hdr_t) + 3) / 4;
  } else {
    if (SWITCH_MIRROR_SESSION_ID_COALESCING(api_mirror_info->session_id)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d session %d: "
          "mirror session id invalid:(%s)\n",
          device,
          api_mirror_info->session_id,
          switch_error_to_string(status));
      return status;
    }
  }

  if ((api_mirror_info->max_pkt_len == 0) ||
      (api_mirror_info->max_pkt_len > 1536)) {
    api_mirror_info->max_pkt_len = 1536;
  }

  mirror_info->mgid_handle = SWITCH_API_INVALID_HANDLE;
  mirror_info->enable = TRUE;

  SWITCH_MEMCPY(&mirror_info->api_mirror_info,
                api_mirror_info,
                sizeof(switch_api_mirror_info_t));

  status = switch_api_id_allocator_set(
      device, mirror_ctx->session_id_allocator, api_mirror_info->session_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d session %d: "
        "mirror session id set failed:(%s)\n",
        device,
        api_mirror_info->session_id,
        switch_error_to_string(status));
    return status;
  }

  port_handle = api_mirror_info->egress_port_handle;
  if (port_handle != SWITCH_API_INVALID_HANDLE) {
    if (!SWITCH_PORT_HANDLE(port_handle)) {
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d egress port handle 0x%lx "
          "port handle invalid:(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
    SWITCH_PORT_DEV_PORT_GET(device, port_handle, dev_port, status);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mirror session create failed on device %d egress port handle 0x%lx "
          "dev port get failed:(%s)\n",
          device,
          port_handle,
          switch_error_to_string(status));
      goto cleanup;
    }
  }

  switch (api_mirror_info->mirror_type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      break;
    case SWITCH_MIRROR_TYPE_REMOTE:
      if (api_mirror_info->rspan_type == SWITCH_MIRROR_RSPAN_TYPE_VLAN_HANDLE) {
        SWITCH_ASSERT(SWITCH_VLAN_HANDLE(api_mirror_info->vlan_handle));
        vlan_handle = api_mirror_info->vlan_handle;
      } else {
        status = switch_api_vlan_id_to_handle_get(
            device, api_mirror_info->vlan_id, &vlan_handle);
        if (status != SWITCH_STATUS_SUCCESS ||
            status != SWITCH_STATUS_ITEM_NOT_FOUND) {
          SWITCH_LOG_ERROR(
              "mirror session create failed on device %d vlan id %d: "
              "vlan id to handle get failed:(%s)\n",
              device,
              api_mirror_info->vlan_id,
              switch_error_to_string(status));
          goto cleanup;
        }
        if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
          status = switch_api_vlan_create(
              device, api_mirror_info->vlan_id, &vlan_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "mirror session create failed on device %d vlan id %d: "
                "vlan id to handle get failed:(%s)\n",
                device,
                api_mirror_info->vlan_id,
                switch_error_to_string(status));
            goto cleanup;
          }
        }
      }

      SWITCH_ASSERT(SWITCH_VLAN_HANDLE(vlan_handle));

      SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(switch_api_interface_info_t));

      api_intf_info.type = SWITCH_INTERFACE_TYPE_TRUNK;
      api_intf_info.handle = port_handle;

      status =
          switch_api_interface_create(device, &api_intf_info, &intf_handle);
      if (intf_handle == SWITCH_API_INVALID_HANDLE) {
        SWITCH_LOG_ERROR(
            "mirror session create failed on device %d vlan id %d: "
            "interface create failed:(%s)\n",
            device,
            api_mirror_info->vlan_id,
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_vlan_member_add(
          device, vlan_handle, intf_handle, &member_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mirror session create failed on device %d vlan id %d: "
            "vlan member add failed:(%s)\n",
            device,
            api_mirror_info->vlan_id,
            switch_error_to_string(status));
        goto cleanup;
      }
      mirror_info->vlan_handle = vlan_handle;
      mirror_info->intf_handle = intf_handle;
      break;

    case SWITCH_MIRROR_TYPE_DTEL_REPORT:
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE:
      if (api_mirror_info->span_mode == SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP ||
          api_mirror_info->span_mode == SWITCH_MIRROR_SPAN_MODE_TUNNEL_PARAMS) {
        if (api_mirror_info->span_mode == SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP) {
          nhop_handle = api_mirror_info->nhop_handle;
          SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
        } else {
          /**
           * create an overlay rif for the tunnel
           */
          SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));
          api_rif_info.rif_type = SWITCH_RIF_TYPE_LOOPBACK;
          api_rif_info.vrf_handle = api_mirror_info->vrf_handle;
          status = switch_api_rif_create(device, &api_rif_info, &orif_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_ASSERT(SWITCH_RIF_HANDLE(orif_handle));

          /**
           * create an underlay rif for the tunnel
           */
          SWITCH_MEMSET(&api_rif_info, 0x0, sizeof(api_rif_info));
          api_rif_info.rif_type = SWITCH_RIF_TYPE_LOOPBACK;
          api_rif_info.vrf_handle = api_mirror_info->vrf_handle;
          status = switch_api_rif_create(device, &api_rif_info, &urif_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_ASSERT(SWITCH_RIF_HANDLE(urif_handle));

          /**
           * create the tunnel object
           */
          SWITCH_MEMSET(&api_tunnel_info, 0x0, sizeof(api_tunnel_info));
          api_tunnel_info.tunnel_type = (api_mirror_info->mirror_type ==
                                         SWITCH_MIRROR_TYPE_ENHANCED_REMOTE)
                                            ? SWITCH_TUNNEL_TYPE_ERSPAN_T3
                                            : SWITCH_TUNNEL_TYPE_DTEL_REPORT;
          api_tunnel_info.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P;
          api_tunnel_info.underlay_rif_handle = urif_handle;
          api_tunnel_info.overlay_rif_handle = orif_handle;
          api_tunnel_info.ip_type = SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4;
          SWITCH_MEMCPY(&api_tunnel_info.src_ip,
                        &api_mirror_info->src_ip,
                        sizeof(switch_ip_addr_t));
          status = switch_api_tunnel_create(
              device, &api_tunnel_info, &tunnel_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));

          /**
           * create the tunnel interface object
           */
          SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
          api_intf_info.type = SWITCH_INTERFACE_TYPE_TUNNEL;
          api_intf_info.handle = tunnel_handle;
          status =
              switch_api_interface_create(device, &api_intf_info, &intf_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(intf_handle));

          /**
           * create the tunnel nexthop object
           */
          SWITCH_MEMSET(&tmp_api_nhop_info, 0x0, sizeof(tmp_api_nhop_info));
          tmp_api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_TUNNEL;
          tmp_api_nhop_info.rewrite_type =
              SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR;
          tmp_api_nhop_info.nhop_tunnel_type = SWITCH_NHOP_TUNNEL_TYPE_VRF;
          tmp_api_nhop_info.vrf_handle = api_mirror_info->vrf_handle;
          tmp_api_nhop_info.tunnel_handle = tunnel_handle;
          SWITCH_MEMCPY(&tmp_api_nhop_info.ip_addr,
                        &api_mirror_info->dst_ip,
                        sizeof(switch_ip_addr_t));
          status =
              switch_api_nhop_create(device, &tmp_api_nhop_info, &nhop_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));

          mirror_info->api_mirror_info.nhop_handle = nhop_handle;
          mirror_info->urif_handle = urif_handle;
          mirror_info->orif_handle = orif_handle;
          mirror_info->tunnel_handle = tunnel_handle;
          mirror_info->intf_handle = intf_handle;
        }

        SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
        status = switch_nhop_get(device, nhop_handle, &nhop_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mirror session create failed on device %d nhop handle 0x%lx: "
              "nhop get failed:(%s)\n",
              device,
              nhop_handle,
              switch_error_to_string(status));
          goto cleanup;
        }

        api_nhop_info = &nhop_info->spath.api_nhop_info;
        SWITCH_ASSERT(api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_TUNNEL);

        status = switch_api_tunnel_mirror_list_add(
            device, nhop_info->tunnel_encap_handle, handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mirror session create failed on device %d nhop handle 0x%lx: "
              "tunnel mirror list add failed:(%s)\n",
              device,
              nhop_handle,
              switch_error_to_string(status));
          goto cleanup;
        }

        status = switch_tunnel_underlay_vrf_handle_get(
            device, api_nhop_info->tunnel_handle, &vrf_handle);
        SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
        SWITCH_ASSERT(SWITCH_VRF_HANDLE(vrf_handle));

        SWITCH_MEMSET(&route_entry, 0x0, sizeof(route_entry));
        route_entry.vrf_handle = vrf_handle;
        SWITCH_MEMCPY(&route_entry.ip_address,
                      &(api_nhop_info->ip_addr),
                      sizeof(switch_ip_addr_t));

        status = switch_api_l3_route_lookup(
            device, &route_entry, &tunnel_nhop_handle);
        if (status == SWITCH_STATUS_SUCCESS) {
          SWITCH_ASSERT(SWITCH_NHOP_HANDLE(tunnel_nhop_handle));
          status = switch_nhop_get(device, tunnel_nhop_handle, &tun_nhop_info);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
          mirror_info->mgid_handle = tun_nhop_info->tunnel_info.mgid_handle;
        }
      } else if (api_mirror_info->span_mode ==
                 SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE) {
        SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
      } else {
        SWITCH_ASSERT(0);
      }
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mirror session create failed on device %d: "
            "mirror type invalid:(%s)\n",
            device,
            switch_error_to_string(status));
        goto cleanup;
      }
      break;
  }

  status =
      switch_pd_mirror_session_update(device, handle, dev_port, mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d nhop handle 0x%lx: "
        "tunnel mirror list add failed:(%s)\n",
        device,
        nhop_handle,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_mirror_table_entry_add(device, handle, mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session create failed on device %d session %d: "
        "mirror table add failed:(%s)\n",
        device,
        api_mirror_info->session_id,
        switch_error_to_string(status));
    return status;
  }

  *mirror_handle = handle;

  return status;

cleanup:
  switch_api_id_allocator_release(
      device, mirror_ctx->session_id_allocator, api_mirror_info->session_id);
  if (SWITCH_MIRROR_HANDLE(*mirror_handle)) {
    switch_mirror_handle_delete(device, *mirror_handle);
  }
  return status;
}

switch_status_t switch_api_mirror_session_update_internal(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const switch_uint64_t flags,
    const switch_api_mirror_info_t *api_mirror_info) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_api_mirror_info_t *tmp_api_mirror_info = NULL;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_dev_port_t dev_port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session update failed on device %d mirror handle 0x%lx: "
        "mirror get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_MIRROR_TYPE(mirror_info) != SWITCH_MIRROR_TYPE_LOCAL) {
    SWITCH_LOG_ERROR(
        "mirror session update failed on device %d mirror handle 0x%lx: "
        "mirror type not local:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  tmp_api_mirror_info = &mirror_info->api_mirror_info;
  if (flags & SWITCH_MIRROR_ATTRIBUTE_SRC_IP) {
    SWITCH_MEMCPY(&tmp_api_mirror_info->src_ip,
                  &api_mirror_info->src_ip,
                  sizeof(switch_ip_addr_t));
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_DST_IP) {
    SWITCH_MEMCPY(&tmp_api_mirror_info->dst_ip,
                  &api_mirror_info->dst_ip,
                  sizeof(switch_ip_addr_t));
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_SRC_MAC) {
    SWITCH_MEMCPY(&tmp_api_mirror_info->src_mac,
                  &api_mirror_info->src_mac,
                  sizeof(switch_mac_addr_t));
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_DST_MAC) {
    SWITCH_MEMCPY(&tmp_api_mirror_info->dst_mac,
                  &api_mirror_info->dst_mac,
                  sizeof(switch_mac_addr_t));
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_TTL) {
    tmp_api_mirror_info->ttl = api_mirror_info->ttl;
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_TOS) {
    tmp_api_mirror_info->tos = api_mirror_info->tos;
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_VLAN_ID) {
    tmp_api_mirror_info->vlan_id = api_mirror_info->vlan_id;
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_VLAN_TPID) {
    tmp_api_mirror_info->vlan_tpid = api_mirror_info->vlan_tpid;
  }

  if (flags & SWITCH_MIRROR_ATTRIBUTE_VLAN_COS) {
    tmp_api_mirror_info->cos = api_mirror_info->cos;
  }

  port_handle = api_mirror_info->egress_port_handle;
  if (port_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
    SWITCH_PORT_DEV_PORT_GET(device, port_handle, dev_port, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_pd_mirror_session_update(
      device, mirror_handle, dev_port, mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session update failed on device %d mirror handle 0x%lx: "
        "pd mirror session update failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (mirror_info->api_mirror_info.span_mode ==
      SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE) {
    status =
        switch_pd_mirror_table_entry_update(device, mirror_handle, mirror_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mirror session update failed on device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  mirror_info->api_mirror_info.egress_port_handle = port_handle;

  return status;
}

switch_status_t switch_api_mirror_session_update_mgid(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const switch_handle_t mgid_handle) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_dev_port_t dev_port = 0;

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  if (!SWITCH_MIRROR_HANDLE(mirror_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("mirror session update mgid failed on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("mirror session update mgid failed on device %d:%s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  mirror_info->mgid_handle = mgid_handle;

  switch (mirror_info->api_mirror_info.session_type) {
    case SWITCH_MIRROR_SESSION_TYPE_SIMPLE:
    case SWITCH_MIRROR_SESSION_TYPE_COALESCE:
      status = switch_pd_mirror_session_update(
          device, mirror_handle, dev_port, mirror_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mirror session update mgid failed on device %d:%s",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      break;

    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mirror session update mgid failed on device %d:%s",
                         device,
                         switch_error_to_string(status));
        return status;
      }
  }

  return status;
}

switch_status_t switch_api_mirror_session_delete_internal(
    const switch_device_t device, const switch_handle_t mirror_handle) {
  switch_mirror_context_t *mirror_ctx = NULL;
  switch_mirror_info_t *mirror_info = NULL;
  switch_api_mirror_info_t *api_mirror_info = NULL;
  switch_nhop_info_t *nhop_info = NULL;
  switch_api_nhop_info_t *api_nhop_info = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_MIRROR, (void **)&mirror_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session delete failed on device %d: "
        "mirror context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session delete failed on device %d mirror handle 0x%lx: "
        "mirror get failed:(%s)\n",
        device,
        mirror_handle,
        switch_error_to_string(status));
    return status;
  }

  api_mirror_info = &mirror_info->api_mirror_info;
  status = switch_pd_mirror_session_delete(device, mirror_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session delete failed on device %d mirror handle 0x%lx: "
        "mirror session delete failed:(%s)\n",
        device,
        mirror_handle,
        switch_error_to_string(status));
    return status;
  }

  switch (api_mirror_info->mirror_type) {
    case SWITCH_MIRROR_TYPE_LOCAL:
      break;
    case SWITCH_MIRROR_TYPE_REMOTE:
      status = switch_api_vlan_member_remove(
          device, mirror_info->vlan_handle, mirror_info->intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mirror session delete failed on device %d mirror handle 0x%lx: "
            "vlan member remove failed:(%s)\n",
            device,
            mirror_handle,
            switch_error_to_string(status));
        return status;
      }

      status = switch_api_interface_delete(device, mirror_info->intf_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "mirror session delete failed on device %d mirror handle 0x%lx: "
            "interface delete failed:(%s)\n",
            device,
            mirror_handle,
            switch_error_to_string(status));
        return status;
      }
      break;
    case SWITCH_MIRROR_TYPE_DTEL_REPORT:
    case SWITCH_MIRROR_TYPE_ENHANCED_REMOTE:
      if (api_mirror_info->span_mode == SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP ||
          api_mirror_info->span_mode == SWITCH_MIRROR_SPAN_MODE_TUNNEL_PARAMS) {
        nhop_handle = api_mirror_info->nhop_handle;
        SWITCH_ASSERT(SWITCH_NHOP_HANDLE(nhop_handle));
        status = switch_nhop_get(device, nhop_handle, &nhop_info);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mirror session delete failed on device %d nhop handle 0x%lx: "
              "nhop get failed:(%s)\n",
              device,
              nhop_handle,
              switch_error_to_string(status));
          return status;
        }

        api_nhop_info = &nhop_info->spath.api_nhop_info;
        SWITCH_ASSERT(api_nhop_info->nhop_type == SWITCH_NHOP_TYPE_TUNNEL);

        status = switch_api_tunnel_mirror_list_remove(
            device, nhop_info->tunnel_encap_handle, mirror_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mirror session delete failed on device %d nhop handle 0x%lx: "
              "tunnel mirror list add failed:(%s)\n",
              device,
              nhop_handle,
              switch_error_to_string(status));
          return status;
        }

        if (api_mirror_info->span_mode ==
            SWITCH_MIRROR_SPAN_MODE_TUNNEL_PARAMS) {
          /**
           * delete the tunnel nhop handle
           */
          status = switch_api_nhop_delete(device, nhop_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

          /**
           * delete the tunnel interface handle
           */
          status =
              switch_api_interface_delete(device, mirror_info->intf_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

          /**
           * delete the tunnel handle
           */
          status = switch_api_tunnel_delete(device, mirror_info->tunnel_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

          /**
           * delete the underlay rif handle
           */
          status = switch_api_rif_delete(device, mirror_info->urif_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

          /**
           * delete the overlay rif handle
           */
          status = switch_api_rif_delete(device, mirror_info->orif_handle);
          SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
        }
      }
      break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      break;
  }

  status = switch_pd_mirror_table_entry_delete(device, mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session delete failed on device %d mirror handle 0x%lx: "
        "mirror entry delete failed:(%s)\n",
        device,
        mirror_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_release(
      device, mirror_ctx->session_id_allocator, api_mirror_info->session_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session delete failed on device %d mirror handle 0x%lx: "
        "mirror entry delete failed:(%s)\n",
        device,
        mirror_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_handle_delete(device, mirror_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_api_mirror_session_type_get_internal(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_type_t *type) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  if (!SWITCH_MIRROR_HANDLE(mirror_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mirror session type get failed: invalid handle on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session type get failed: invalid handle info on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  *type = mirror_info->api_mirror_info.mirror_type;
  return status;
}

switch_status_t switch_api_mirror_session_monitor_port_set_internal(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *api_mirror_info) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_dev_port_t dev_port = 0;
  switch_handle_t intf_handle = 0;
  switch_handle_t vlan_handle = 0;
  switch_handle_t member_handle = 0;
  switch_api_interface_info_t api_intf_info;

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  if (!SWITCH_MIRROR_HANDLE(mirror_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mirror session port set failed: invalid handle on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session port set failed: invalid handle info on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (api_mirror_info->egress_port_handle != SWITCH_API_INVALID_HANDLE) {
    SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(switch_api_interface_info_t));

    if (mirror_info->api_mirror_info.mirror_type == SWITCH_MIRROR_TYPE_REMOTE) {
      status =
          switch_api_interface_by_type_get(device,
                                           api_mirror_info->egress_port_handle,
                                           SWITCH_INTERFACE_TYPE_TRUNK,
                                           &intf_handle);
      if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
        api_intf_info.type = SWITCH_INTERFACE_TYPE_TRUNK;
        api_intf_info.handle = api_mirror_info->egress_port_handle;

        status =
            switch_api_interface_create(device, &api_intf_info, &intf_handle);
        if (intf_handle == SWITCH_API_INVALID_HANDLE) {
          SWITCH_LOG_ERROR(
              "mirror session port set failed: interface handle create failed "
              "on device %d:%s",
              device,
              switch_error_to_string(status));
          return status;
        }
      }

      status = switch_api_vlan_id_to_handle_get(
          device, mirror_info->api_mirror_info.vlan_id, &vlan_handle);
      if (status != SWITCH_STATUS_SUCCESS ||
          status != SWITCH_STATUS_ITEM_NOT_FOUND) {
        SWITCH_LOG_ERROR(
            "mirror session port set failed: invalid vlan on device %d:%s",
            device,
            switch_error_to_string(status));
        return status;
      }
      status = switch_api_vlan_member_add(
          device, vlan_handle, intf_handle, &member_handle);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("mirror session create failed on device %d:%s",
                         device,
                         switch_error_to_string(status));
        return status;
      }
    }
    SWITCH_PORT_DEV_PORT_GET(
        device, api_mirror_info->egress_port_handle, dev_port, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    status = switch_pd_mirror_session_update(
        device, mirror_handle, dev_port, mirror_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mirror session port set failed on device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    mirror_info->api_mirror_info.egress_port_handle =
        api_mirror_info->egress_port_handle;
  } else {
    status = SWITCH_STATUS_INVALID_HANDLE;
  }

  return status;
}

switch_status_t switch_api_mirror_session_monitor_vlan_set_internal(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *api_mirror_info) {
  switch_mirror_info_t *mirror_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_dev_port_t dev_port = 0;
  switch_handle_t vlan_handle;
  switch_handle_t member_handle;
  bool session_update = FALSE;

  SWITCH_ASSERT(SWITCH_MIRROR_HANDLE(mirror_handle));
  if (!SWITCH_MIRROR_HANDLE(mirror_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "mirror session type get failed: invalid handle on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session type get failed: invalid handle info on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  // vlan-id can be modified only for RSPAN.
  if (mirror_info->api_mirror_info.mirror_type != SWITCH_MIRROR_TYPE_REMOTE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "mirror session vlan set failed: invalid mirror type on device %d: %s",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (api_mirror_info->vlan_id) {
    status = switch_api_vlan_id_to_handle_get(
        device, api_mirror_info->vlan_id, &vlan_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
        status = switch_api_vlan_create(
            device, api_mirror_info->vlan_id, &vlan_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "mirror session port set failed: vlan create failed on device "
              "%d:%s",
              device,
              switch_error_to_string(status));

          return status;
        }
      }
    }
    // update the vlan membership of the mirror egress port.
    if (mirror_info->intf_handle == SWITCH_API_INVALID_HANDLE) {
      SWITCH_LOG_ERROR(
          "mirror session vlan set failed: invalid intf handle on device %d: "
          "%s",
          device,
          switch_error_to_string(SWITCH_STATUS_INVALID_HANDLE));
      return SWITCH_STATUS_INVALID_HANDLE;
    }

    status = switch_api_vlan_member_add(
        device, vlan_handle, mirror_info->intf_handle, &member_handle);

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mirror session vlan set failed: vlan member add failed on device "
          "%d:%s",
          device,
          switch_error_to_string(status));
      return status;
    }
    mirror_info->vlan_handle = vlan_handle;
    mirror_info->api_mirror_info.vlan_id = api_mirror_info->vlan_id;
    session_update = TRUE;
  }

  if (api_mirror_info->tos != mirror_info->api_mirror_info.tos) {
    mirror_info->api_mirror_info.tos = api_mirror_info->tos;
    session_update = TRUE;
  }

  if (session_update) {
    status =
        switch_pd_mirror_table_entry_update(device, mirror_handle, mirror_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "mirror session vlan set failed: mirror entry update failed on device"
          "%d:%s",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_mirror_session_update(
        device, mirror_handle, dev_port, mirror_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("mirror session port set failed on device %d:%s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_mirror_session_info_get_internal(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_api_mirror_info_t *api_mirror_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!api_mirror_info) {
    SWITCH_LOG_ERROR("mirror session info get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return SWITCH_STATUS_INVALID_PARAMETER;
  }
  switch_mirror_info_t *mirror_info = NULL;
  status = switch_mirror_get(device, mirror_handle, &mirror_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "mirror session info get failed: invalid handle info on device %d:%s",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(api_mirror_info,
                &mirror_info->api_mirror_info,
                sizeof(switch_api_mirror_info_t));
  return status;
}

switch_status_t switch_api_mirror_session_update(
    const switch_device_t device,
    const switch_handle_t mirror_handle,
    const switch_uint64_t flags,
    const switch_api_mirror_info_t *api_mirror_info) {
  SWITCH_MT_WRAP(switch_api_mirror_session_update_internal(
      device, mirror_handle, flags, api_mirror_info))
}

switch_status_t switch_api_mirror_session_create(
    switch_device_t device,
    switch_api_mirror_info_t *api_mirror_info,
    switch_handle_t *mirror_handle) {
  SWITCH_MT_WRAP(switch_api_mirror_session_create_internal(
      device, api_mirror_info, mirror_handle))
}

switch_status_t switch_api_mirror_session_delete(
    const switch_device_t device, const switch_handle_t mirror_handle) {
  SWITCH_MT_WRAP(
      switch_api_mirror_session_delete_internal(device, mirror_handle))
}

switch_status_t switch_api_mirror_session_type_get(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_type_t *type) {
  SWITCH_MT_WRAP(
      switch_api_mirror_session_type_get_internal(device, mirror_handle, type))
}

switch_status_t switch_api_mirror_session_monitor_port_set(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *api_mirror_info) {
  SWITCH_MT_WRAP(switch_api_mirror_session_monitor_port_set_internal(
      device, mirror_handle, api_mirror_info))
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_mirror_session_monitor_vlan_set(
    switch_device_t device,
    switch_handle_t mirror_handle,
    const switch_api_mirror_info_t *api_mirror_info) {
  SWITCH_MT_WRAP(switch_api_mirror_session_monitor_vlan_set_internal(
      device, mirror_handle, api_mirror_info))
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_mirror_session_info_get(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_api_mirror_info_t *mirror_info) {
  SWITCH_MT_WRAP(switch_api_mirror_session_info_get_internal(
      device, mirror_handle, mirror_info))
  return SWITCH_STATUS_SUCCESS;
}
