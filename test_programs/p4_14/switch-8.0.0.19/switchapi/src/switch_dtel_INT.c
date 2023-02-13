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

#include "switchapi/switch_dtel.h"
#include "switch_internal.h"
#include "switch_pd_dtel.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_dtel_int_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_INT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT init failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  dtel_ctx->_int.enabled = false;

#ifdef P4_INT_EP_ENABLE
  dtel_ctx->_int.watchlist.size = DTEL_FLOW_WATCHLIST_TABLE_SIZE * 2;
  dtel_ctx->_int.watchlist.compare_func = switch_twl_key_compare;
  dtel_ctx->_int.watchlist.key_func = switch_twl_key_init;
  dtel_ctx->_int.watchlist.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&dtel_ctx->_int.watchlist);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Cannot init INT Watchlist for device %d: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_int.sessions.size = INT_SESSION_MAX_NUM * 2;
  dtel_ctx->_int.sessions.compare_func = switch_int_session_key_compare;
  dtel_ctx->_int.sessions.key_func = switch_int_session_key_init;
  dtel_ctx->_int.sessions.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&dtel_ctx->_int.sessions);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Cannot init INT sessions for device %d: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_int.off_hdl = SWITCH_PD_INVALID_HANDLE;
  for (int i = 0; i < INT_RPI_HANDLE_NUM; i++) {
    dtel_ctx->_int.rpi_hdl[i] = SWITCH_PD_INVALID_HANDLE;
  }
  for (int i = 0; i < INT_RP_HANDLE_NUM; i++) {
    dtel_ctx->_int.rp_hdl[i] = SWITCH_PD_INVALID_HANDLE;
  }
  for (int i = 0; i < INT_L4_HANDLE_NUM; i++) {
    dtel_ctx->_int.l4_hdl[i] = SWITCH_PD_INVALID_HANDLE;
  }
  dtel_ctx->_int.set_sink_hdl = SWITCH_PD_INVALID_HANDLE;
  dtel_ctx->_int.term_hdl = SWITCH_PD_INVALID_HANDLE;

#endif /* P4_INT_EP_ENABLE */

#ifdef P4_INT_L45_MARKER_ENABLE
  dtel_ctx->_int.l45_marker_udp_ports.size = INT_L45_MARKER_MAX_L4_PORTS * 2;
  dtel_ctx->_int.l45_marker_udp_ports.compare_func =
      switch_int_marker_port_key_compare;
  dtel_ctx->_int.l45_marker_udp_ports.key_func =
      switch_int_marker_port_key_init;
  dtel_ctx->_int.l45_marker_udp_ports.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&dtel_ctx->_int.l45_marker_udp_ports);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Cannot init INT marker-UDP port table for device %d: %s \n",
        device,
        switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_int.l45_marker_tcp_ports.size = INT_L45_MARKER_MAX_L4_PORTS * 2;
  dtel_ctx->_int.l45_marker_tcp_ports.compare_func =
      switch_int_marker_port_key_compare;
  dtel_ctx->_int.l45_marker_tcp_ports.key_func =
      switch_int_marker_port_key_init;
  dtel_ctx->_int.l45_marker_tcp_ports.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&dtel_ctx->_int.l45_marker_tcp_ports);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Cannot init INT marker-TCP port table for device %d: %s \n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (int i = 0; i < 3; i++) {
    dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
  }
  for (int i = 0; i < 3; i++) {
    dtel_ctx->_int.l45_marker_udp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
  }
  for (int i = 0; i < 4; i++) {
    dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
  }

  // INT source needs to use a value
  dtel_ctx->_int.l45_marker_udp_value = INTL45_MARKER_DEFAULT_VALUE;
  dtel_ctx->_int.l45_marker_tcp_value = INTL45_MARKER_DEFAULT_VALUE;
  dtel_ctx->_int.l45_marker_icmp_value = INTL45_MARKER_DEFAULT_VALUE;
#endif /* P4_INT_L45_MARKER_ENABLE */

#ifdef P4_INT_L45_DSCP_ENABLE
  dtel_ctx->_int.l45_diffserv_value = 0;
  dtel_ctx->_int.l45_diffserv_mask = INTL45_DSCP_DISABLE_MASK;
  dtel_ctx->_int.int_l45_dscp_pvs_hdl = SWITCH_PD_INVALID_HANDLE;
#endif /* P4_INT_L45_DSCP_ENABLE */

#endif /* P4_INT_ENABLE */
  return status;
}

switch_status_t switch_dtel_int_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_INT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT default entries add failed for device %d: %s, "
        "cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_tables_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT table pd init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_dtel_int_endpoint_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT endpoint disable by default failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_dtel_int_transit_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disable by default failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_transit_qalert_add(
      device,
      dtel_ctx->event_infos[SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH]
          .dscp);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int_transit_qalert failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_switch_id(device, dtel_ctx->switch_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT set switch_id to 0 failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_report_encap_table_enable_i2e(
      device, dtel_ctx->dest_udp_port, dtel_ctx->_int.rpi_hdl, true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP int_report_encap enable i2e failed "
        "on device %d : %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_dtel_int_report_encap_table_add_e2e(device,
                                                    dtel_ctx->switch_id,
                                                    dtel_ctx->dest_udp_port,
                                                    dtel_ctx->_int.rp_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP report encap add e2e failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_transit_report_encap_table_enable_e2e(
      device,
      dtel_ctx->switch_id,
      dtel_ctx->dest_udp_port,
      dtel_ctx->_int.rp_hdl,
      true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT Transit report encap enable e2e with switch_id 0 failed "
        "for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_upstream_report_enable(
      device, &dtel_ctx->event_infos_sorted_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP enable int_upstream_report failed "
        "for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_sink_local_report_enable(
      device, &dtel_ctx->event_infos_sorted_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP enable int_sink_local_report failed "
        "for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

#ifdef P4_INT_L45_MARKER_ENABLE
  // this is guarded by sink so no harm to populate by default
  status = switch_pd_dtel_int_terminate_init_update(
      device, &dtel_ctx->_int, &dtel_ctx->_int.term_hdl, true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT terminate init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // Add ROCE2 for UDP
  status = switch_api_dtel_int_marker_set(
      device, SWITCH_DTEL_IP_PROTO_UDP, INTL45_MARKER_DEFAULT_VALUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("UDP marker init failed failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_api_dtel_int_marker_port_add(
      device, SWITCH_DTEL_IP_PROTO_UDP, 4791, 0xffff);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("UDP ROCE_V2 port add failed failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

#endif  // P4_INT_L45_MARKER_ENABLE

#endif  // P4_INT_ENABLE
  return status;
}

switch_status_t switch_dtel_int_switch_id(switch_device_t device,
                                          switch_uint32_t switch_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(switch_id);

#ifdef P4_INT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT switch id failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_switch_id(device, switch_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT set switch_id failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  // INT_RP_HANDLE_NUM is defined based on max pipes (4) pipes
  // only max_pipes must be checked
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  int used_rp_hdls = INT_RP_HANDLE_NUM / 4 * max_pipes;
  for (int i = 0; i < used_rp_hdls; i++) {
    if (dtel_ctx->_int.rp_hdl[i] == SWITCH_PD_INVALID_HANDLE) {
      SWITCH_LOG_ERROR(
          "INT EP int_report_encap modify session failed for device %d: %s, "
          "item not found\n",
          device,
          switch_error_to_string(status));
      return SWITCH_STATUS_ITEM_NOT_FOUND;
    }
  }

  status = switch_pd_dtel_int_report_encap_table_modify_e2e(
      device, switch_id, dtel_ctx->dest_udp_port, dtel_ctx->_int.rp_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP report encap modify e2e failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_transit_report_encap_table_enable_e2e(
      device, switch_id, dtel_ctx->dest_udp_port, dtel_ctx->_int.rp_hdl, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT Transit report encap enable e2e failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
#endif  // P4_INT_ENABLE

  return status;
}

switch_status_t switch_dtel_int_dest_udp_port(switch_device_t device,
                                              switch_uint16_t dest_udp_port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(dest_udp_port);

#ifdef P4_INT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT dest udp port set failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // INT_RP_HANDLE_NUM is defined based on max pipes (4) pipes
  // only max_pipes must be checked
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  int used_rp_hdls = INT_RP_HANDLE_NUM / 4 * max_pipes;
  for (int i = 0; i < used_rp_hdls; i++) {
    if (dtel_ctx->_int.rp_hdl[i] == SWITCH_PD_INVALID_HANDLE) {
      SWITCH_LOG_ERROR(
          "INT EP int_report_encap modify session failed for device %d: %s, "
          "item not found\n",
          device,
          switch_error_to_string(status));
      return SWITCH_STATUS_ITEM_NOT_FOUND;
    }
  }

  status = switch_pd_dtel_int_report_encap_table_modify_e2e(
      device, dtel_ctx->switch_id, dest_udp_port, dtel_ctx->_int.rp_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP report encap modify e2e failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_report_encap_table_enable_i2e(
      device, dest_udp_port, dtel_ctx->_int.rpi_hdl, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP report encap modify i2e failed "
        "for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_transit_report_encap_table_enable_e2e(
      device, dtel_ctx->switch_id, dest_udp_port, dtel_ctx->_int.rp_hdl, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT Transit report encap enable e2e failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
#endif  // P4_INT_ENABLE

  return status;
}

switch_status_t switch_dtel_int_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_INT_EP_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT default entries delete failed for device %d: %s, cannot "
        "get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (int i = 0; i < INT_RP_HANDLE_NUM; i++) {
    if (dtel_ctx->_int.rp_hdl[i] != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_int_report_encap_table_delete(
          device, dtel_ctx->_int.rp_hdl[i]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "INT EP int_report_encap delete session failed for device %d: %s\n",
            device,
            switch_error_to_string(status));
        goto cleanup;
      }
      dtel_ctx->_int.rp_hdl[i] = SWITCH_PD_INVALID_HANDLE;
    }
  }

cleanup:
#endif  // P4_INT_EP_ENABLE
  return status;
}

//------------------------------------------------------------------------------
// INT Transit enable/disable
//------------------------------------------------------------------------------

switch_status_t switch_api_dtel_int_transit_enable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_INT_TRANSIT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT enable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
#ifdef P4_INT_L45_DSCP_ENABLE
  if (dtel_ctx->_int.l45_diffserv_mask == INTL45_DSCP_DISABLE_MASK) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel INT enable failed for device %d: %s, "
        "no DSCP for INT over L4 configured\n",
        device,
        switch_error_to_string(status));
    return status;
  }
#endif /* P4_INT_L45_DSCP_ENABLE */

  if (dtel_ctx->_int.enabled) {
    return status;
  }
  dtel_ctx->_int.enabled = true;

  status = switch_pd_dtel_int_transit_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int transit failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_digest_encode_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int_digest_encode failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_meta_header_update_end_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int_meta_header_update_end failed for device %d: "
        "%s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_enable_int_inst(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int_instruction failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_outer_encap_transit_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling int_outer_encap failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_intl45_update_l4_transit_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit enabling intl45_update_l4 failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

#ifdef P4_INT_L45_DSCP_ENABLE
  if (dtel_ctx->_int.int_l45_dscp_pvs_hdl == SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_diffserv_parser_value_set(
        device,
        dtel_ctx->_int.l45_diffserv_value,
        dtel_ctx->_int.l45_diffserv_mask,
        &dtel_ctx->_int.int_l45_dscp_pvs_hdl);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP enabling l45 diffserv failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
#endif /* P4_INT_L45_DSCP_ENABLE */

  status = switch_dtel_int_marker_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 marker enable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

cleanup:
#endif  // P4_INT_TRANSIT_ENABLE

  return status;
}

switch_status_t switch_api_dtel_int_transit_disable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);

#ifdef P4_INT_TRANSIT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT disable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // disable is used at init for initialization so even though enabled=false run
  // this

  status = switch_pd_dtel_int_transit_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling int transit failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_digest_encode_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling int_digest_encode failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_meta_header_update_end_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling int_meta_header_update_end failed for device "
        "%d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_disable_int_inst(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling int_instructions failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_outer_encap_transit_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling int_encap_outer failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_intl45_update_l4_transit_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT transit disabling intl45_update_l4 failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  if (dtel_ctx->_int.int_l45_dscp_pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_diffserv_parser_value_delete(
        device, dtel_ctx->_int.int_l45_dscp_pvs_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT L45 diffserv parser value delete failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    dtel_ctx->_int.int_l45_dscp_pvs_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_dtel_int_marker_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 marker disable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  dtel_ctx->_int.enabled = false;

cleanup:
#endif  // P4_INT_TRANSIT_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// INT Endpoint enable/disable
//------------------------------------------------------------------------------

switch_status_t switch_api_dtel_int_endpoint_enable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);

#ifdef P4_INT_EP_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT enable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (dtel_ctx->_int.enabled) {
    return status;
  }

#ifdef P4_INT_L45_DSCP_ENABLE
  if (dtel_ctx->_int.l45_diffserv_mask == INTL45_DSCP_DISABLE_MASK) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel INT enable failed for device %d: %s, "
        "no DSCP for INT over L4 configured\n",
        device,
        switch_error_to_string(status));
    return status;
  }
#endif /* P4_INT_L45_DSCP_ENABLE */

  dtel_ctx->_int.enabled = true;

  status = switch_pd_dtel_int_enable_int_inst(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP enabling int_instruction failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status =
      switch_pd_dtel_int_set_sink_enable(device, &dtel_ctx->_int.set_sink_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT terminate sink enable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_intl45_update_l4_table_tcp_icmp_enable(
      device, dtel_ctx->_int.l4_hdl, &dtel_ctx->_int);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP enabling l45_update_l4 failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  if (dtel_ctx->_int.off_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_int_watchlist_entry_delete(device,
                                                       dtel_ctx->_int.off_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT enable failed for device %d: %s \n",
                       device,
                       switch_error_to_string(status));
    }
    dtel_ctx->_int.off_hdl = SWITCH_PD_INVALID_HANDLE;
  }

#ifdef P4_INT_L45_DSCP_ENABLE
  if (dtel_ctx->_int.int_l45_dscp_pvs_hdl == SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_diffserv_parser_value_set(
        device,
        dtel_ctx->_int.l45_diffserv_value,
        dtel_ctx->_int.l45_diffserv_mask,
        &dtel_ctx->_int.int_l45_dscp_pvs_hdl);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP enabling l45 diffserv failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }
#endif /* P4_INT_L45_DSCP_ENABLE */

  status = switch_dtel_int_marker_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 marker enable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

cleanup:
#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_api_dtel_int_endpoint_disable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);

#ifdef P4_INT_EP_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT disable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  // disable used at init for initialization so run this even if enabled=false

  for (int i = 0; i < INT_L4_HANDLE_NUM; i++) {
    if (dtel_ctx->_int.l4_hdl[i] != SWITCH_PD_INVALID_HANDLE) {
      status = switch_pd_dtel_intl45_update_l4_table_delete(
          device, dtel_ctx->_int.l4_hdl[i]);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "INT EP intl45_update_l4 delete session failed for device %d: %s\n",
            device,
            switch_error_to_string(status));
      }
      dtel_ctx->_int.l4_hdl[i] = SWITCH_PD_INVALID_HANDLE;
    }
  }

  status = switch_pd_dtel_int_disable_int_inst(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP disabling int_instructions failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status =
      switch_pd_dtel_int_set_sink_disable(device, &dtel_ctx->_int.set_sink_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT terminate sink disable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_dtel_flow_state_clear_cycle(device, 0);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT set clearing cycle failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  // to disabled source by adding notwatch entry with priority 0 in
  // int_watchlist
  if (dtel_ctx->_int.off_hdl == SWITCH_PD_INVALID_HANDLE) {
    switch_twl_match_spec_t twl_match;
    SWITCH_MEMSET(&twl_match, 0x0, sizeof(switch_twl_match_spec_t));
    twl_match.l4_port_src_start = 0;
    twl_match.l4_port_src_end = 0xFFFF;
    twl_match.l4_port_dst_start = 0;
    twl_match.l4_port_dst_end = 0xFFFF;
    twl_match.inner_l4_port_src_start = 0;
    twl_match.inner_l4_port_src_end = 0xFFFF;
    twl_match.inner_l4_port_dst_start = 0;
    twl_match.inner_l4_port_dst_end = 0xFFFF;
    status = switch_pd_dtel_int_watchlist_entry_create(
        device, &twl_match, 0, false, NULL, &dtel_ctx->_int.off_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT EP disable failed for device %d: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  if (dtel_ctx->_int.int_l45_dscp_pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_diffserv_parser_value_delete(
        device, dtel_ctx->_int.int_l45_dscp_pvs_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT L45 diffserv parser value delete failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    dtel_ctx->_int.int_l45_dscp_pvs_hdl = SWITCH_PD_INVALID_HANDLE;
  }

  status = switch_dtel_int_marker_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 marker disable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  dtel_ctx->_int.enabled = false;

cleanup:
#endif  // P4_INT_EP_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// INT sessions create/update/delete
//------------------------------------------------------------------------------

switch_status_t switch_int_session_key_init(void *args,
                                            switch_uint8_t *key,
                                            switch_uint32_t *len) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("INT session key init invalid parameter: %s\n",
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMCPY(key, args, sizeof(switch_uint16_t));
  *len = sizeof(switch_uint16_t);
  return status;
}

switch_status_t switch_int_session_key_compare(const void *key1,
                                               const void *key2) {
  return SWITCH_MEMCMP(key1, key2, sizeof(switch_uint16_t));
}

switch_status_t switch_int_marker_port_key_init(void *args,
                                                switch_uint8_t *key,
                                                switch_uint32_t *len) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("INT marker port key init invalid parameter: %s\n",
                     switch_error_to_string(status));
    return status;
  }
  // port + mask + proto
  SWITCH_MEMCPY(
      key, args, 2 * sizeof(switch_uint16_t) + sizeof(switch_uint8_t));
  *len = 2 * sizeof(switch_uint16_t) + sizeof(switch_uint8_t);
  return status;
}

switch_status_t switch_int_marker_port_key_compare(const void *key1,
                                                   const void *key2) {
  // port + mask + proto
  return SWITCH_MEMCMP(
      key1, key2, 2 * sizeof(switch_uint16_t) + sizeof(switch_uint8_t));
}

switch_status_t switch_api_dtel_int_session_create_internal(
    switch_device_t device,
    switch_uint16_t session_id,
    switch_uint16_t instruction,
    switch_uint8_t max_hop) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(session_id);
  UNUSED(instruction);
  UNUSED(max_hop);

#ifdef P4_INT_EP_ENABLE

  // get context
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT session add failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

#ifdef P4_INT_L45_DSCP_ENABLE
  if (dtel_ctx->_int.l45_diffserv_mask == INTL45_DSCP_DISABLE_MASK) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "DTel INT session add failed for device %d: %s, "
        "no DSCP for INT over L4 configured\n",
        device,
        switch_error_to_string(status));
    return status;
  }
#endif /* P4_INT_L45_DSCP_ENABLE */

  // check capacity
  int count = SWITCH_HASHTABLE_COUNT(&dtel_ctx->_int.sessions);
  if (count > INT_SESSION_MAX_NUM - 2) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR("INT sessions full for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // search if entry exists
  switch_dtel_int_session_entry_t *int_session = NULL;
  status = SWITCH_HASHTABLE_SEARCH(
      &dtel_ctx->_int.sessions, (void *)(&session_id), (void **)&int_session);

  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    if (session_id == 0) {
      SWITCH_LOG_ERROR("INT session add failed for device %d, ",
                       "default INT session 0 is already set\n",
                       device);
    } else {
      SWITCH_LOG_ERROR("INT session %d already exists for device %d: %s\n",
                       session_id,
                       device,
                       switch_error_to_string(status));
    }
    return status;
  }

  int_session =
      SWITCH_MALLOC(device, sizeof(switch_dtel_int_session_entry_t), 1);
  if (!int_session) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("INT session memory allocation failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMSET(int_session, 0, sizeof(switch_dtel_int_session_entry_t));
  int_session->session_id = session_id;
  int_session->instruction = instruction;
  int_session->ins_hdl = SWITCH_PD_INVALID_HANDLE;
  int_session->en_hdl = SWITCH_PD_INVALID_HANDLE;
  int_session->l4_hdl = SWITCH_PD_INVALID_HANDLE;
  int_session->ref_count = 0;

  status = switch_pd_dtel_int_insert_table_add_update(
      device, session_id, instruction, max_hop, true, &int_session->ins_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP config session add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_outer_encap_table_session_add_update(
      device,
      session_id,
      instruction,
      &dtel_ctx->_int,
      true,
      &int_session->en_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP outer encap add session failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

#ifdef INT_OVER_L4_ENABLE
  status = switch_pd_dtel_intl45_update_l4_table_udp_session_add_update(
      device,
      session_id,
      instruction,
      &dtel_ctx->_int,
      true,
      &int_session->l4_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP intl45_update_l4 add session failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
#endif

  status = SWITCH_HASHTABLE_INSERT(&dtel_ctx->_int.sessions,
                                   &int_session->node,
                                   (void *)(&int_session->session_id),
                                   (void *)(int_session));

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT session hash table insert failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

cleanup:
#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_api_dtel_int_session_update_internal(
    switch_device_t device,
    switch_uint16_t session_id,
    switch_uint16_t instruction,
    switch_uint8_t max_hop) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(session_id);
  UNUSED(instruction);
  UNUSED(max_hop);

#ifdef P4_INT_EP_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT session update failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  switch_dtel_int_session_entry_t *int_session = NULL;
  status = SWITCH_HASHTABLE_SEARCH(
      &dtel_ctx->_int.sessions, (void *)(&session_id), (void **)&int_session);

  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "INT session update failed for device %d: %s"
        " INT session %d does not exist\n",
        device,
        switch_error_to_string(status),
        session_id);
    return status;
  }

  status = switch_pd_dtel_int_insert_table_add_update(
      device, session_id, instruction, max_hop, false, &int_session->ins_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP config session update failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_pd_dtel_int_outer_encap_table_session_add_update(
      device,
      session_id,
      instruction,
      &dtel_ctx->_int,
      false,
      &int_session->en_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP outer encap update session failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

#ifdef INT_OVER_L4_ENABLE
  status = switch_pd_dtel_intl45_update_l4_table_udp_session_add_update(
      device,
      session_id,
      instruction,
      &dtel_ctx->_int,
      false,
      &int_session->l4_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT EP intl45_update_l4 update session failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
#endif

  int_session->instruction = instruction;

cleanup:
#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_api_dtel_int_session_delete_internal(
    switch_device_t device, switch_uint16_t session_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(session_id);

#ifdef P4_INT_EP_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT session delete failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  switch_dtel_int_session_entry_t *int_session = NULL;
  status = SWITCH_HASHTABLE_SEARCH(
      &dtel_ctx->_int.sessions, (void *)(&session_id), (void **)&int_session);

  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "INT session delete failed for device %d: %s"
        " session %d hashtable lookup failed\n",
        device,
        switch_error_to_string(status),
        session_id);
    return status;
  }

  if (int_session->ref_count != 0) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "INT session delete failed for device %d: %s"
        " %d watchlist(s) still referring to session %d\n",
        device,
        switch_error_to_string(status),
        int_session->ref_count,
        session_id);
    return status;
  }

  if (int_session->ins_hdl != SWITCH_PD_INVALID_HANDLE) {
    status =
        switch_pd_dtel_int_insert_table_delete(device, int_session->ins_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT EP config session delete failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
    }
  }

  if (int_session->en_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_int_outer_encap_table_delete(device,
                                                         int_session->en_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT EP outer encap delete session failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
    }
  }

  if (int_session->l4_hdl != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_update_l4_table_delete(device,
                                                          int_session->l4_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT EP intl45_update_l4 delete session failed for device %d: %s\n",
          device,
          switch_error_to_string(status));
    }
  }

  status = SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_int.sessions,
                                        &int_session->node);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("INT sessions delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, int_session);

#endif  // P4_INT_EP_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// INT sink ports
//------------------------------------------------------------------------------

switch_status_t switch_api_dtel_int_sink_ports_add_internal(
    switch_device_t device, switch_port_t port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(port);

#ifdef P4_INT_EP_ENABLE

  switch_handle_t port_handle;
  status = switch_api_port_id_to_handle_get(device, port, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP sink port add failed for device %d: %s\n",
                     " cannot get port handle",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_port_info_t *port_info = NULL;
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel port info get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT sink port add failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_sink_ports_add(
      device, port_info->dev_port, &(dtel_ctx->_int.sink_port_hdl[port]));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP sink port add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_api_dtel_int_sink_ports_delete_internal(
    switch_device_t device, switch_port_t port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(status);
  UNUSED(port);

#ifdef P4_INT_EP_ENABLE
  switch_handle_t port_handle;
  status = switch_api_port_id_to_handle_get(device, port, &port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP sink port add failed for device %d: %s\n",
                     " cannot get port handle",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_port_info_t *port_info = NULL;
  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DTel port info get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT sink port delete failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_sink_ports_delete(
      device, port_info->dev_port, dtel_ctx->_int.sink_port_hdl[port]);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT EP sink port delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
#endif  // P4_INT_EP_ENABLE

  return status;
}

//------------------------------------------------------------------------------
// INT Watchlist internal add/update/delete/clear
//------------------------------------------------------------------------------

switch_status_t switch_dtel_int_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);
  UNUSED(priority);
  UNUSED(watch);
  UNUSED(action_params);

#ifdef P4_INT_EP_ENABLE
  if (watch && action_params->_int.flow_sample_percent > 100) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("INT Watchlist add failed for device %d, ",
                     "percent must be <= 100\n",
                     device);
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist add failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  int count = SWITCH_HASHTABLE_COUNT(&dtel_ctx->_int.watchlist);
  if (count > DTEL_FLOW_WATCHLIST_TABLE_SIZE - 2) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR("INT watchlist full for device %d\n", device);
    return status;
  }

  // search if watchlist entry already exists
  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel INT add searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "INT watchlist add failed for device %d: %s, item already exists\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // search if int session entry exists
  switch_dtel_int_session_entry_t *int_session = NULL;
  if (watch) {
    switch_uint16_t session_id = action_params->_int.session_id;
    status = SWITCH_HASHTABLE_SEARCH(
        &dtel_ctx->_int.sessions, (void *)(&session_id), (void **)&int_session);

    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist add failed for device %d: %s, "
          "INT session %d does not exist\n",
          device,
          switch_error_to_string(status),
          session_id);
      return status;
    }
    if (int_session->ref_count == 65535) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist add failed for device %d: %s, "
          "INT session %d ref count limit exceeded\n",
          device,
          switch_error_to_string(status),
          session_id);
      return status;
    }
  }

  // create new entry
  twl_entry = SWITCH_MALLOC(device, sizeof(switch_twl_entry_t), 1);
  if (!twl_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("INT watchlist memory allocation failed for device %d\n",
                     device);
    return status;
  }
  SWITCH_MEMSET(twl_entry, 0, sizeof(switch_twl_entry_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_entry->match);
  SWITCH_PD_LOG_DEBUG("DTel INT adding for: ");
  switch_twl_match_spec_print(&twl_match_spec);
  twl_entry->priority = priority;
  twl_entry->watch = watch;
  if (watch) {
    twl_entry->int_session_id = action_params->_int.session_id;
  } else {
    twl_entry->int_session_id = 0;
  }
  twl_entry->pd_hdl = 0;

  // add new entry to h/w
  status = switch_pd_dtel_int_watchlist_entry_create(device,
                                                     &twl_entry->match,
                                                     priority,
                                                     watch,
                                                     action_params,
                                                     &twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT watchlist add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // add new entry to hashtable
  status = SWITCH_HASHTABLE_INSERT(&dtel_ctx->_int.watchlist,
                                   &twl_entry->node,
                                   (void *)(&twl_entry->match),
                                   (void *)(twl_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT watchlist hashtable insert failed for device %d\n",
                     device);
    return status;
  }

  if (watch) {
    int_session->ref_count++;
  }

#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_dtel_int_watchlist_entry_update(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);
  UNUSED(priority);
  UNUSED(watch);
  UNUSED(action_params);

#ifdef P4_INT_EP_ENABLE

  if (watch && action_params->_int.flow_sample_percent > 100) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "INT Watchlist add failed for device %d, "
        "percent must be <= 100\n",
        device);
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist update failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // search if entry exists
  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel INT update searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist update failed for device %d: %s, item not found\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // search if int session entry exists
  switch_dtel_int_session_entry_t *int_session = NULL;
  switch_uint16_t session_id = 0;
  if (watch) {
    session_id = action_params->_int.session_id;
    status = SWITCH_HASHTABLE_SEARCH(
        &dtel_ctx->_int.sessions, (void *)(&session_id), (void **)&int_session);

    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, "
          "INT session %d does not exist\n",
          device,
          switch_error_to_string(status),
          session_id);
      return status;
    }
    // if new int session ref count is to be incremented, check max exceeded
    if ((twl_entry->watch == false ||
         twl_entry->int_session_id != session_id) &&
        int_session->ref_count == 65535) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, "
          "INT session %d ref count limit exceeded\n",
          device,
          switch_error_to_string(status),
          session_id);
      return status;
    }
  }

  if (twl_entry->priority != priority) {
    // different priority, have to delete and add again
    status =
        switch_pd_dtel_int_watchlist_entry_delete(device, twl_entry->pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, delete failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_dtel_int_watchlist_entry_create(device,
                                                       &twl_entry->match,
                                                       priority,
                                                       watch,
                                                       action_params,
                                                       &twl_entry->pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, add failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    twl_entry->priority = priority;
  } else {  // same priority, update by pd handle
    status = switch_pd_dtel_int_watchlist_entry_update(
        device, twl_entry->pd_hdl, watch, action_params);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, update failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  // increment new session ref count if necessary
  if (watch == true &&
      (twl_entry->watch == false || twl_entry->int_session_id != session_id)) {
    int_session->ref_count++;
  }
  // decrement old int session ref count if necessary
  if (twl_entry->watch == true &&
      (watch == false || twl_entry->int_session_id != session_id)) {
    switch_dtel_int_session_entry_t *old_int_session = NULL;
    status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.sessions,
                                     (void *)(&twl_entry->int_session_id),
                                     (void **)&old_int_session);
    if (status != SWITCH_STATUS_SUCCESS || old_int_session->ref_count == 0) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist update failed for device %d: %s, "
          "old INT session %d does not exist or ref_count == 0\n",
          device,
          switch_error_to_string(status),
          twl_entry->int_session_id);
    } else {
      old_int_session->ref_count--;
    }
  }

  twl_entry->watch = watch;
  if (watch) {
    twl_entry->int_session_id = action_params->_int.session_id;
  } else {
    twl_entry->int_session_id = 0;
  }

#endif  // P4_INT_EP_ENABLE

  return status;
}

switch_status_t switch_dtel_int_watchlist_entry_delete(
    switch_device_t device, switch_twl_match_info_t *match_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);

#ifdef P4_INT_EP_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist delete failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel INT delete searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist delete failed for device %d: %s, item not found\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_watchlist_entry_delete(device, twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT watchlist delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // decrement int session ref count if necessary
  if (twl_entry->watch == true) {
    switch_dtel_int_session_entry_t *int_session = NULL;
    status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.sessions,
                                     (void *)(&twl_entry->int_session_id),
                                     (void **)&int_session);
    if (status != SWITCH_STATUS_SUCCESS || int_session->ref_count == 0) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist delete failed for device %d: %s, "
          "INT session %d does not exist or ref_count == 0\n",
          device,
          switch_error_to_string(status),
          twl_entry->int_session_id);
    } else {
      int_session->ref_count--;
    }
  }

  status =
      SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_int.watchlist, &twl_entry->node);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "INT watchlist hashtable delete failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, twl_entry);

#endif  // P4_INT_EP_ENABLE

  return status;
}

#ifdef P4_INT_EP_ENABLE
static void dtel_int_watchlist_entry_delete_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_twl_entry_t *twl_entry = (switch_twl_entry_t *)data;
  switch_status_t status =
      switch_pd_dtel_int_watchlist_entry_delete(*device, twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT watchlist delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT watchlist delete failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  // decrement int session ref count if necessary
  if (twl_entry->watch == true) {
    switch_dtel_int_session_entry_t *int_session = NULL;
    status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_int.sessions,
                                     (void *)(&twl_entry->int_session_id),
                                     (void **)&int_session);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist delete failed for device %d: %s, "
          "INT session %d does not exist\n",
          device,
          switch_error_to_string(status),
          twl_entry->int_session_id);
    } else if (int_session->ref_count == 0) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "INT watchlist delete failed for device %d: %s, "
          "INT session %d ref_count == 0\n",
          device,
          switch_error_to_string(status),
          twl_entry->int_session_id);
    } else {
      int_session->ref_count--;
    }
  }

  status =
      SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_int.watchlist, &twl_entry->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT hashtable delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  SWITCH_FREE(*device, twl_entry);
}
#endif /* P4_INT_EP_ENABLE */

switch_status_t switch_dtel_int_watchlist_clear(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

#ifdef P4_INT_EP_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT watchlist clear failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.watchlist,
                                   &dtel_int_watchlist_entry_delete_foreach,
                                   &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT watchlist clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

#endif  // P4_INT_EP_ENABLE

  return status;
}

#ifdef P4_INT_L45_DSCP_ENABLE
static void dtel_int_sessions_update_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_session_entry_t *int_session =
      (switch_dtel_int_session_entry_t *)data;

  switch_dtel_context_t *dtel_ctx = NULL;
  switch_status_t status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT outer encap update session failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_pd_dtel_int_outer_encap_table_session_add_update(
      *device,
      int_session->session_id,
      int_session->instruction,
      &dtel_ctx->_int,
      false,
      &int_session->en_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT outer encap update session failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
  }
}
#endif /* P4_INT_L45_DSCP_ENABLE */

switch_status_t switch_api_dtel_int_dscp_value_set_internal(
    switch_device_t device, switch_uint8_t value, switch_uint8_t mask) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(value);
  UNUSED(mask);

#ifdef P4_INT_L45_DSCP_ENABLE

  if (mask > 0x3F) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "INT diffserve value set failed for device %d: %s"
        " Mask must be smaller or equal to 0x3F \n",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (!(mask == 0x3F || mask == 0x1 || mask == 0x2 || mask == 0x4 ||
        mask == 0x8 || mask == 0x10 || mask == 0x20)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "INT diffserve value set failed for device %d: %s"
        " Mask must either cover all bits (0x3f) or only one bit \n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT diffserve value set  failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  value = value & mask;

  dtel_ctx->_int.l45_diffserv_value = value;
  dtel_ctx->_int.l45_diffserv_mask = mask;

  if (dtel_ctx->_int.int_l45_dscp_pvs_hdl == SWITCH_PD_INVALID_HANDLE) {
    if (dtel_ctx->_int.enabled) {
      // if not enabled don't program pvs
      status = switch_pd_dtel_intl45_diffserv_parser_value_set(
          device,
          dtel_ctx->_int.l45_diffserv_value,
          dtel_ctx->_int.l45_diffserv_mask,
          &dtel_ctx->_int.int_l45_dscp_pvs_hdl);
    }
  } else {
    status = switch_pd_dtel_intl45_diffserv_parser_value_modify(
        device,
        dtel_ctx->_int.l45_diffserv_value,
        dtel_ctx->_int.l45_diffserv_mask,
        dtel_ctx->_int.int_l45_dscp_pvs_hdl);
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 parser value set failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_dtel_int_terminate_init_update(
      device,
      &dtel_ctx->_int,
      &dtel_ctx->_int.term_hdl,
      dtel_ctx->_int.term_hdl == SWITCH_PD_INVALID_HANDLE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT terminate modify failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(
      &dtel_ctx->_int.sessions, &dtel_int_sessions_update_foreach, &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT L45 parser value set failed in updating sessions for device %d: "
        "%s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

#endif /* P4_INT_L45_DSCP_ENABLE */

  return status;
}

#ifdef P4_INT_L45_MARKER_ENABLE
static void dtel_int_marker_modify_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_marker_port_entry_t *marker_entry =
      (switch_dtel_int_marker_port_entry_t *)data;

  switch_dtel_context_t *dtel_ctx = NULL;
  switch_status_t status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT L45 marker update failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  switch (marker_entry->proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_modify(
            *device,
            0,
            marker_entry->value,
            marker_entry->mask,
            dtel_ctx->_int.l45_marker_tcp_value,
            marker_entry->pvs_hdl);
      }
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_udp_marker_parser_value_modify(
            *device,
            0,
            marker_entry->value,
            marker_entry->mask,
            dtel_ctx->_int.l45_marker_udp_value,
            marker_entry->pvs_hdl);
      }
      break;
    default:
      SWITCH_LOG_ERROR(
          "INT L45 marker update failed for device %d:"
          " invalid L4 protocol %d",
          device,
          marker_entry->proto);
      return;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT L45 marker update failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
}

static void dtel_intl45_sessions_update_marker_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_session_entry_t *int_session =
      (switch_dtel_int_session_entry_t *)data;

  switch_dtel_context_t *dtel_ctx = NULL;
  switch_status_t status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT l45 update marker failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_pd_dtel_intl45_update_l4_table_udp_session_add_update(
      *device,
      int_session->session_id,
      int_session->instruction,
      &dtel_ctx->_int,
      false,
      &int_session->l4_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT L45 update l4 update marker failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
  }
}

#endif /* P4_INT_L45_MARKER_ENABLE */

switch_status_t switch_api_dtel_int_marker_set_internal(
    switch_device_t device, switch_uint8_t proto, switch_uint64_t marker) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);
  UNUSED(marker);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker set failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      dtel_ctx->_int.l45_marker_tcp_value = marker;
      status =
          SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_tcp_ports,
                                       &dtel_int_marker_modify_foreach,
                                       &device);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("INT L45 marker-TCP set failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      for (int i = 0; i < 3; i++) {
        if (dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i] ==
            SWITCH_PD_INVALID_HANDLE) {
          status = switch_pd_dtel_intl45_tcp_marker_parser_value_set(
              device,
              i + 1,
              0,
              0,
              marker,
              dtel_ctx->_int.l45_marker_tcp_pvs_hdls + i);
        } else {
          status = switch_pd_dtel_intl45_tcp_marker_parser_value_modify(
              device,
              i + 1,
              0,
              0,
              marker,
              dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i]);
        }
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT L45 marker-TCP set failed for device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }
      }
      if (dtel_ctx->_int.l4_hdl[0] != SWITCH_PD_INVALID_HANDLE) {
        // only if entry is there (ep is enabled)
        status = switch_pd_dtel_intl45_update_l4_table_tcp_icmp_enable(
            device, dtel_ctx->_int.l4_hdl, &dtel_ctx->_int);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT L45 marker-ICMP set failed for device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }
      }
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      dtel_ctx->_int.l45_marker_udp_value = marker;
      status =
          SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_udp_ports,
                                       &dtel_int_marker_modify_foreach,
                                       &device);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("INT L45 marker-UDP set failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      for (int i = 0; i < 3; i++) {
        if (dtel_ctx->_int.l45_marker_udp_pvs_hdls[i] ==
            SWITCH_PD_INVALID_HANDLE) {
          status = switch_pd_dtel_intl45_udp_marker_parser_value_set(
              device,
              i + 1,
              0,
              0,
              marker,
              dtel_ctx->_int.l45_marker_udp_pvs_hdls + i);
        } else {
          status = switch_pd_dtel_intl45_udp_marker_parser_value_modify(
              device,
              i + 1,
              0,
              0,
              marker,
              dtel_ctx->_int.l45_marker_udp_pvs_hdls[i]);
        }
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT L45 marker-UDP set failed for device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }
      }
      status = SWITCH_HASHTABLE_FOREACH_ARG(
          &dtel_ctx->_int.sessions,
          &dtel_intl45_sessions_update_marker_foreach,
          &device);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("INT L45 marker-UDP set failed for device %d: %s\n",
                         device,
                         switch_error_to_string(status));
        return status;
      }
      break;
    case SWITCH_DTEL_IP_PROTO_ICMP:
      dtel_ctx->_int.l45_marker_icmp_value = marker;
      // ICMP will not have port in the first parser state
      for (int i = 0; i < 4; i++) {
        if (dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i] ==
            SWITCH_PD_INVALID_HANDLE) {
          if (i == 0 && !dtel_ctx->_int.enabled) {
            // Entry in frist state acts as enable,
            // if int is not enabled don't program it
            continue;
          }
          status = switch_pd_dtel_intl45_icmp_marker_parser_value_set(
              device, i, marker, dtel_ctx->_int.l45_marker_icmp_pvs_hdls + i);
        } else {
          status = switch_pd_dtel_intl45_icmp_marker_parser_value_modify(
              device, i, marker, dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i]);
        }
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT L45 marker-ICMP set failed for device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }
      }
      if (dtel_ctx->_int.l4_hdl[0] != SWITCH_PD_INVALID_HANDLE) {
        // only if entry is there (ep is enabled)
        status = switch_pd_dtel_intl45_update_l4_table_tcp_icmp_enable(
            device, dtel_ctx->_int.l4_hdl, &dtel_ctx->_int);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT L45 marker-ICMP set failed for device %d: %s\n",
                           device,
                           switch_error_to_string(status));
          return status;
        }
      }
      break;
  }

#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

#ifdef P4_INT_L45_MARKER_ENABLE
static void dtel_int_marker_port_disable_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_marker_port_entry_t *marker_entry =
      (switch_dtel_int_marker_port_entry_t *)data;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port delete failed for device %d: %s,"
        " cannot get context\n",
        *device,
        switch_error_to_string(status));
    return;
  }

  switch (marker_entry->proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_delete(
            *device, 0, marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port delete failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
        marker_entry->pvs_hdl = SWITCH_PD_INVALID_HANDLE;
      }
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_udp_marker_parser_value_delete(
            *device, 0, marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port delete failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
        marker_entry->pvs_hdl = SWITCH_PD_INVALID_HANDLE;
      }
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port hashtable delete failed for device %d: %s\n",
        *device,
        switch_error_to_string(status));
    return;
  }
}

static void dtel_int_marker_port_enable_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_marker_port_entry_t *marker_entry =
      (switch_dtel_int_marker_port_entry_t *)data;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port delete failed for device %d: %s,"
        " cannot get context\n",
        *device,
        switch_error_to_string(status));
    return;
  }

  switch (marker_entry->proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      if (marker_entry->pvs_hdl == SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_set(
            *device,
            0,
            marker_entry->value,
            marker_entry->mask,
            dtel_ctx->_int.l45_marker_tcp_value,
            &marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port add failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
      }
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      if (marker_entry->pvs_hdl == SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_udp_marker_parser_value_set(
            *device,
            0,
            marker_entry->value,
            marker_entry->mask,
            dtel_ctx->_int.l45_marker_udp_value,
            &marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port add failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
      }
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port hashtable enable failed for device %d: %s\n",
        *device,
        switch_error_to_string(status));
    return;
  }
}
#endif  // P4_INT_L45_MARKER_ENABLE

// Just remove the entries from the first state of marker pvs to disable that
switch_status_t switch_dtel_int_marker_disable(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker disable failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_udp_ports,
                                        &dtel_int_marker_port_disable_foreach,
                                        &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker-UDP port clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_tcp_ports,
                                        &dtel_int_marker_port_disable_foreach,
                                        &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker-TCP port clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_int.l45_marker_icmp_pvs_hdls[0] != SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_icmp_marker_parser_value_delete(
        device, 0, dtel_ctx->_int.l45_marker_icmp_pvs_hdls[0]);
    dtel_ctx->_int.l45_marker_icmp_pvs_hdls[0] = SWITCH_PD_INVALID_HANDLE;
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT L45 marker-ICMP disable failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }
#endif  // P4_INT_L45_MARKER_ENABLE

  return status;
}

switch_status_t switch_dtel_int_marker_enable(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker enable failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_udp_ports,
                                        &dtel_int_marker_port_enable_foreach,
                                        &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker-UDP port enable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(&dtel_ctx->_int.l45_marker_tcp_ports,
                                        &dtel_int_marker_port_enable_foreach,
                                        &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker-TCP port enable failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_int.l45_marker_icmp_pvs_hdls[0] == SWITCH_PD_INVALID_HANDLE) {
    status = switch_pd_dtel_intl45_icmp_marker_parser_value_set(
        device,
        0,
        dtel_ctx->_int.l45_marker_icmp_value,
        dtel_ctx->_int.l45_marker_icmp_pvs_hdls);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT L45 marker-ICMP enable failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }
#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

switch_status_t switch_api_dtel_int_marker_delete_internal(
    switch_device_t device, switch_uint8_t proto) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker disable failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (proto == SWITCH_DTEL_IP_PROTO_TCP || proto == SWITCH_DTEL_IP_PROTO_UDP) {
    status = switch_api_dtel_int_marker_port_clear(device, proto);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT L45 marker disable failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      for (int i = 0; i < 3; i++) {
        if (dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i] !=
            SWITCH_PD_INVALID_HANDLE) {
          status = switch_pd_dtel_intl45_tcp_marker_parser_value_delete(
              device, i + 1, dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i]);
          dtel_ctx->_int.l45_marker_tcp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "INT L45 marker-TCP disable failed for device %d: %s\n",
                device,
                switch_error_to_string(status));
            return status;
          }
        }
      }
      // INT source still needs to use a value
      dtel_ctx->_int.l45_marker_tcp_value = INTL45_MARKER_DEFAULT_VALUE;
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      for (int i = 0; i < 3; i++) {
        if (dtel_ctx->_int.l45_marker_udp_pvs_hdls[i] !=
            SWITCH_PD_INVALID_HANDLE) {
          status = switch_pd_dtel_intl45_udp_marker_parser_value_delete(
              device, i + 1, dtel_ctx->_int.l45_marker_udp_pvs_hdls[i]);
          dtel_ctx->_int.l45_marker_udp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "INT L45 marker-UDP disable failed for device %d: %s\n",
                device,
                switch_error_to_string(status));
            return status;
          }
        }
      }
      // INT source still needs to use a value
      dtel_ctx->_int.l45_marker_udp_value = INTL45_MARKER_DEFAULT_VALUE;
      break;
    case SWITCH_DTEL_IP_PROTO_ICMP:
      // ICMP will not have port in the first parser state so 4 states
      for (int i = 0; i < 4; i++) {
        if (dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i] !=
            SWITCH_PD_INVALID_HANDLE) {
          status = switch_pd_dtel_intl45_icmp_marker_parser_value_delete(
              device, i, dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i]);
          dtel_ctx->_int.l45_marker_icmp_pvs_hdls[i] = SWITCH_PD_INVALID_HANDLE;
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "INT L45 marker-ICMP disable failed for device %d: %s\n",
                device,
                switch_error_to_string(status));
            return status;
          }
        }
      }
      // INT source still needs to use a value
      dtel_ctx->_int.l45_marker_icmp_value = INTL45_MARKER_DEFAULT_VALUE;
      break;
  }

#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

switch_status_t switch_api_dtel_int_marker_get_internal(
    switch_device_t device, switch_uint8_t proto, switch_uint64_t *marker) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);
  UNUSED(marker);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker get failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      *marker = dtel_ctx->_int.l45_marker_tcp_value;
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      *marker = dtel_ctx->_int.l45_marker_udp_value;
      break;
    case SWITCH_DTEL_IP_PROTO_ICMP:
      *marker = dtel_ctx->_int.l45_marker_icmp_value;
      break;
    default:
      SWITCH_LOG_ERROR(
          "INT marker get failed for device %d, unsupported protocol %d\n",
          device,
          proto);
      status = SWITCH_STATUS_INVALID_PARAMETER;
      return status;
  }
#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

switch_status_t switch_api_dtel_int_marker_port_add_internal(
    switch_device_t device,
    switch_uint8_t proto,
    switch_uint16_t value,
    switch_uint16_t mask) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);
  UNUSED(value);
  UNUSED(mask);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker port add failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch_hashtable_t *hashtable = NULL;
  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      hashtable = &dtel_ctx->_int.l45_marker_tcp_ports;
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      hashtable = &dtel_ctx->_int.l45_marker_udp_ports;
      break;
    default:
      SWITCH_LOG_ERROR(
          "INT marker port add failed for device %d, unsupported protocol %d\n",
          device,
          proto);
      status = SWITCH_STATUS_INVALID_PARAMETER;
      return status;
  }

  // check capacity
  int count = SWITCH_HASHTABLE_COUNT(hashtable);
  if (count > INT_L45_MARKER_MAX_L4_PORTS - 1) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR(
        "INT marker port parser for protocol %d is full for device %d: %s\n",
        proto,
        device,
        switch_error_to_string(status));
    return status;
  }

  // search if entry exists
  switch_dtel_int_marker_port_entry_t marker_entry_search;
  SWITCH_MEMSET(
      &marker_entry_search, 0x0, sizeof(switch_dtel_int_marker_port_entry_t));
  marker_entry_search.value = value;
  marker_entry_search.mask = mask;
  marker_entry_search.proto = proto;

  switch_dtel_int_marker_port_entry_t *marker_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(
      hashtable, (void *)(&marker_entry_search), (void **)&marker_entry);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "INT marker add failed for device %d: %s, item already exists\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  marker_entry =
      SWITCH_MALLOC(device, sizeof(switch_dtel_int_marker_port_entry_t), 1);
  if (!marker_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "INT marker entry memory allocation failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_MEMSET(marker_entry, 0, sizeof(switch_dtel_int_marker_port_entry_t));
  marker_entry->value = value;
  marker_entry->mask = mask;
  marker_entry->proto = proto;
  marker_entry->pvs_hdl = SWITCH_PD_INVALID_HANDLE;

  if (dtel_ctx->_int.enabled) {
    switch (proto) {
      case SWITCH_DTEL_IP_PROTO_TCP:
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_set(
            device,
            0,
            value,
            mask,
            dtel_ctx->_int.l45_marker_tcp_value,
            &marker_entry->pvs_hdl);
        break;
      case SWITCH_DTEL_IP_PROTO_UDP:
        status = switch_pd_dtel_intl45_udp_marker_parser_value_set(
            device,
            0,
            value,
            mask,
            dtel_ctx->_int.l45_marker_udp_value,
            &marker_entry->pvs_hdl);
        break;
    }
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT marker add failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  // add new entry to hashtable
  status = SWITCH_HASHTABLE_INSERT(hashtable,
                                   &marker_entry->node,
                                   (void *)(marker_entry),
                                   (void *)(marker_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker hashtable insert failed for device %d\n",
                     device);
    return status;
  }
#endif /* P4_INT_L45_MARKER_ENABLE */
  return status;
}

switch_status_t switch_api_dtel_int_marker_port_delete_internal(
    switch_device_t device,
    switch_uint8_t proto,
    switch_uint16_t value,
    switch_uint16_t mask) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);
  UNUSED(value);
  UNUSED(mask);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker port delete failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch_hashtable_t *hashtable = NULL;
  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      hashtable = &dtel_ctx->_int.l45_marker_tcp_ports;
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      hashtable = &dtel_ctx->_int.l45_marker_udp_ports;
      break;
    default:
      SWITCH_LOG_ERROR(
          "INT marker delete failed for device %d, unsupported protocol %d\n",
          device,
          proto);
      status = SWITCH_STATUS_INVALID_PARAMETER;
      return status;
  }

  switch_dtel_int_marker_port_entry_t marker_entry_search;
  switch_dtel_int_marker_port_entry_t *marker_entry;
  SWITCH_MEMSET(
      &marker_entry_search, 0x0, sizeof(switch_dtel_int_marker_port_entry_t));
  marker_entry_search.value = value;
  marker_entry_search.mask = mask;
  marker_entry_search.proto = proto;

  status = SWITCH_HASHTABLE_SEARCH(
      hashtable, (void *)(&marker_entry_search), (void **)&marker_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker delete failed for device %d: %s, item not found\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
    switch (proto) {
      case SWITCH_DTEL_IP_PROTO_TCP:
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_delete(
            device, 0, marker_entry->pvs_hdl);
        break;
      case SWITCH_DTEL_IP_PROTO_UDP:
        status = switch_pd_dtel_intl45_udp_marker_parser_value_delete(
            device, 0, marker_entry->pvs_hdl);
        break;
    }
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("INT marker port delete failed for device %d: %s\n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_HASHTABLE_DELETE_NODE(hashtable, &marker_entry->node);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR("INT marker hashtable delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, marker_entry);
#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

#ifdef P4_INT_L45_MARKER_ENABLE
static void dtel_int_marker_port_delete_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_dtel_int_marker_port_entry_t *marker_entry =
      (switch_dtel_int_marker_port_entry_t *)data;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port delete failed for device %d: %s,"
        " cannot get context\n",
        *device,
        switch_error_to_string(status));
    return;
  }

  switch (marker_entry->proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_tcp_marker_parser_value_delete(
            *device, 0, marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port delete failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
      }
      status = SWITCH_HASHTABLE_DELETE_NODE(
          &dtel_ctx->_int.l45_marker_tcp_ports, &marker_entry->node);
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      if (marker_entry->pvs_hdl != SWITCH_PD_INVALID_HANDLE) {
        status = switch_pd_dtel_intl45_udp_marker_parser_value_delete(
            *device, 0, marker_entry->pvs_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("INT marker port delete failed for device %d: %s\n",
                           *device,
                           switch_error_to_string(status));
          return;
        }
      }
      status = SWITCH_HASHTABLE_DELETE_NODE(
          &dtel_ctx->_int.l45_marker_udp_ports, &marker_entry->node);
      break;
  }
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "INT marker port hashtable delete failed for device %d: %s\n",
        *device,
        switch_error_to_string(status));
    return;
  }
  SWITCH_FREE(*device, marker_entry);
}
#endif /* P4_INT_L45_MARKER_ENABLE */

switch_status_t switch_api_dtel_int_marker_port_clear_internal(
    switch_device_t device, switch_uint8_t proto) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(proto);

#ifdef P4_INT_L45_MARKER_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel INT marker port clean failed for device %d: %s, cannot get "
        "context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  switch_hashtable_t *hashtable = NULL;
  switch (proto) {
    case SWITCH_DTEL_IP_PROTO_TCP:
      hashtable = &dtel_ctx->_int.l45_marker_tcp_ports;
      break;
    case SWITCH_DTEL_IP_PROTO_UDP:
      hashtable = &dtel_ctx->_int.l45_marker_udp_ports;
      break;
    default:
      SWITCH_LOG_ERROR(
          "INT marker delete failed for device %d, unsupported protocol %d\n",
          device,
          proto);
      status = SWITCH_STATUS_INVALID_PARAMETER;
      return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(
      hashtable, &dtel_int_marker_port_delete_foreach, &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("INT marker port clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
#endif /* P4_INT_L45_MARKER_ENABLE */

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_dtel_int_transit_enable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_int_transit_enable_internal(device))
}

switch_status_t switch_api_dtel_int_transit_disable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_int_transit_disable_internal(device))
}

switch_status_t switch_api_dtel_int_endpoint_enable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_int_endpoint_enable_internal(device))
}

switch_status_t switch_api_dtel_int_endpoint_disable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_int_endpoint_disable_internal(device))
}

switch_status_t switch_api_dtel_int_session_create(switch_device_t device,
                                                   switch_uint16_t session_id,
                                                   switch_uint16_t instruction,
                                                   switch_uint8_t max_hop) {
  SWITCH_MT_WRAP(switch_api_dtel_int_session_create_internal(
      device, session_id, instruction, max_hop))
}

switch_status_t switch_api_dtel_int_session_update(switch_device_t device,
                                                   switch_uint16_t session_id,
                                                   switch_uint16_t instruction,
                                                   switch_uint8_t max_hop) {
  SWITCH_MT_WRAP(switch_api_dtel_int_session_update_internal(
      device, session_id, instruction, max_hop))
}

switch_status_t switch_api_dtel_int_session_delete(switch_device_t device,
                                                   switch_uint16_t session_id) {
  SWITCH_MT_WRAP(
      switch_api_dtel_int_session_delete_internal(device, session_id))
}

switch_status_t switch_api_dtel_int_sink_ports_add(switch_device_t device,
                                                   switch_port_t port) {
  SWITCH_MT_WRAP(switch_api_dtel_int_sink_ports_add_internal(device, port))
}

switch_status_t switch_api_dtel_int_sink_ports_delete(switch_device_t device,
                                                      switch_port_t port) {
  SWITCH_MT_WRAP(switch_api_dtel_int_sink_ports_delete_internal(device, port))
}

switch_status_t switch_api_dtel_int_dscp_value_set(switch_device_t device,
                                                   switch_uint8_t value,
                                                   switch_uint8_t mask) {
  SWITCH_MT_WRAP(
      switch_api_dtel_int_dscp_value_set_internal(device, value, mask))
}

switch_status_t switch_api_dtel_int_marker_set(switch_device_t device,
                                               switch_uint8_t proto,
                                               switch_uint64_t marker) {
  SWITCH_MT_WRAP(switch_api_dtel_int_marker_set_internal(device, proto, marker))
}

switch_status_t switch_api_dtel_int_marker_delete(switch_device_t device,
                                                  switch_uint8_t proto) {
  SWITCH_MT_WRAP(switch_api_dtel_int_marker_delete_internal(device, proto))
}

switch_status_t switch_api_dtel_int_marker_get(switch_device_t device,
                                               switch_uint8_t proto,
                                               switch_uint64_t *marker) {
  SWITCH_MT_WRAP(switch_api_dtel_int_marker_get_internal(device, proto, marker))
}

switch_status_t switch_api_dtel_int_marker_port_add(switch_device_t device,
                                                    switch_uint8_t proto,
                                                    switch_uint16_t value,
                                                    switch_uint16_t mask) {
  SWITCH_MT_WRAP(
      switch_api_dtel_int_marker_port_add_internal(device, proto, value, mask))
}

switch_status_t switch_api_dtel_int_marker_port_delete(switch_device_t device,
                                                       switch_uint8_t proto,
                                                       switch_uint16_t value,
                                                       switch_uint16_t mask) {
  SWITCH_MT_WRAP(switch_api_dtel_int_marker_port_delete_internal(
      device, proto, value, mask))
}

switch_status_t switch_api_dtel_int_marker_port_clear(switch_device_t device,
                                                      switch_uint8_t proto) {
  SWITCH_MT_WRAP(switch_api_dtel_int_marker_port_clear_internal(device, proto))
}
