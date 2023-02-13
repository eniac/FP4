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

#ifndef _switch_pd_dtel_h_
#define _switch_pd_dtel_h_

#include "switch_internal.h"

//------------------------------------------------------------------------------
// DTel shared pd functions
//------------------------------------------------------------------------------

#define SWITCH_PKT_TYPE_NOT_CLONED 0
#define SWITCH_PKT_TYPE_I2E_CLONED 1
#define SWITCH_PKT_TYPE_E2E_CLONED 3
#define SWITCH_PKT_TYPE_COALESCED 5

#define ERSPAN_FT_D_OTHER_QALERT 0x3800

#define DTEL_REPORT_NEXT_PROTO_ETHERNET 0
#define DTEL_REPORT_NEXT_PROTO_MOD 1
#define DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL 2

switch_uint32_t switch_build_dtel_report_flags(switch_uint8_t version,
                                               switch_uint8_t next_proto,
                                               bool dropped,
                                               bool congested,
                                               bool path_tracking_flow,
                                               switch_uint8_t reserved1,
                                               switch_uint16_t reserved2,
                                               switch_uint8_t hw_id);

switch_status_t switch_pd_dtel_tables_init(switch_device_t device);

switch_status_t switch_pd_dtel_report_sequence_number_set(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t value);

switch_status_t switch_pd_dtel_report_sequence_number_get(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t *values,
    switch_uint8_t *max_num);

switch_status_t switch_pd_dtel_queue_latency_shift_set(switch_device_t device,
                                                       switch_uint8_t shift);

//------------------------------------------------------------------------------
// DTel port conversion functions
//------------------------------------------------------------------------------

switch_status_t switch_pd_dtel_ig_port_convert_set(switch_device_t device,
                                                   switch_port_t in_port,
                                                   switch_port_t out_port);
switch_status_t switch_pd_dtel_eg_port_convert_set(switch_device_t device,
                                                   switch_port_t in_port,
                                                   switch_port_t out_port);

//------------------------------------------------------------------------------
// DTel mirror session pd functions
//------------------------------------------------------------------------------

switch_status_t switch_pd_dtel_mirror_session_add_group(
    switch_device_t device, switch_pd_mbr_hdl_t *pd_grp_hdl);

switch_status_t switch_pd_dtel_mirror_session_add_group_selector(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_grp_hdl,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_mirror_session_delete(switch_device_t device,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_mirror_session_delete_group(
    switch_device_t device, switch_pd_grp_hdl_t pd_grp_hdl);

switch_status_t switch_pd_dtel_mirror_session_add_member(
    switch_device_t device,
    switch_mirror_id_t mirror_id,
    switch_pd_mbr_hdl_t *pd_mbr_hdl,
    switch_pd_grp_hdl_t pd_grp_hdl);

switch_status_t switch_pd_dtel_mirror_session_delete_member(
    switch_device_t device,
    switch_pd_mbr_hdl_t pd_mbr_hdl,
    switch_pd_grp_hdl_t pd_grp_hdl);

//------------------------------------------------------------------------------
// DTel statless and stateful report triggering pd functions
//------------------------------------------------------------------------------

switch_status_t switch_pd_dtel_quantize_latency_set(switch_device_t device,
                                                    switch_uint8_t quant_shift,
                                                    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_bloom_filters_reset(switch_device_t device);

switch_status_t switch_pd_dtel_bloom_filters_range_reset(
    switch_device_t device,
    switch_uint16_t range_number,
    switch_uint16_t total_ranges);

switch_status_t switch_pd_dtel_queue_alert_index_set(switch_device_t device,
                                                     switch_dev_port_t port,
                                                     switch_qid_t queue,
                                                     switch_uint16_t index,
                                                     switch_uint8_t quant_shift,
                                                     switch_pd_hdl_t *entry_hdl,
                                                     bool add);

switch_status_t switch_pd_dtel_queue_alert_index_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_queue_remaining_report_quota_during_breach_get(
    switch_device_t device, switch_uint16_t index, switch_uint16_t *quota);

switch_status_t switch_pd_dtel_set_queue_alert_threshold(
    switch_device_t device,
    switch_uint16_t index,
    switch_uint32_t queue_depth,
    switch_uint32_t queue_latency);

switch_status_t switch_pd_dtel_queue_change_reset(switch_device_t device,
                                                  switch_uint16_t index);

switch_status_t switch_pd_dtel_queue_report_quota_set(switch_device_t device,
                                                      switch_uint16_t index,
                                                      switch_uint16_t quota);

switch_status_t switch_pd_dtel_deflect_on_drop_queue_config_add(
    switch_device_t device,
    switch_dev_port_t port,
    switch_qid_t queue,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_deflect_on_drop_queue_config_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_queue_report_dod_quota_add(
    switch_device_t device,
    switch_dev_port_t port,
    switch_qid_t queue,
    switch_uint16_t index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_queue_report_dod_quota_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

//------------------------------------------------------------------------------
// Postcard pd functions
//------------------------------------------------------------------------------

switch_status_t switch_pd_dtel_postcard_tables_init(switch_device_t device);

switch_status_t switch_pd_dtel_postcard_e2e_enable(switch_device_t device,
                                                   switch_list_t *event_infos);

switch_status_t switch_pd_dtel_postcard_e2e_disable(switch_device_t device);

switch_status_t switch_pd_dtel_postcard_e2e_clear(switch_device_t device);

switch_status_t switch_pd_dtel_postcard_insert_table_add(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_postcard_insert_table_update(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_postcard_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_spec_t *match_fields,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_postcard_watchlist_entry_update(
    switch_device_t device,
    switch_pd_hdl_t entry_hdl,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_pd_dtel_postcard_watchlist_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_postcard_set_sample(switch_device_t device,
                                                   uint16_t index,
                                                   uint8_t percent);

//------------------------------------------------------------------------------
// Mirror on Drop pd functions
//------------------------------------------------------------------------------

switch_status_t switch_pd_mirror_on_drop_tables_init(switch_device_t device);

switch_status_t switch_pd_mirror_on_drop_encap_update(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    dtel_event_info_t *event_infos,
    bool add,
    p4_pd_entry_hdl_t *entry_hdl);

switch_status_t switch_pd_drop_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_spec_t *match_fields,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_drop_watchlist_entry_update(
    switch_device_t device,
    switch_pd_hdl_t entry_hdl,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_pd_drop_watchlist_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

//------------------------------------------------------------------------------
// INT pd functions
//------------------------------------------------------------------------------

#define DTEL_INT_TYPE_INT 0x01
#define DTEL_INT_TYPE_DIGEST_INT 0x03

// INT common

switch_status_t switch_pd_dtel_int_tables_init(switch_device_t device);

switch_status_t switch_pd_dtel_int_switch_id(switch_device_t device,
                                             switch_uint32_t switch_id);

switch_status_t switch_pd_dtel_int_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_spec_t *match_fields,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_watchlist_entry_update(
    switch_device_t device,
    switch_pd_hdl_t entry_hdl,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_pd_dtel_int_watchlist_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_int_enable_int_inst(switch_device_t device);

switch_status_t switch_pd_dtel_int_disable_int_inst(switch_device_t device);

switch_status_t switch_pd_dtel_intl45_diffserv_parser_value_set(
    switch_device_t device,
    switch_uint8_t value,
    switch_uint8_t mask,
    switch_pd_pvs_hdl_t *pvs_hdl);

switch_status_t switch_pd_dtel_intl45_diffserv_parser_value_delete(
    switch_device_t device, switch_pd_pvs_hdl_t pvs_hdl);

switch_status_t switch_pd_dtel_intl45_diffserv_parser_value_modify(
    switch_device_t device,
    switch_uint8_t value,
    switch_uint8_t mask,
    switch_pd_pvs_hdl_t pvs_hdl);

switch_status_t switch_pd_dtel_intl45_icmp_marker_parser_value_set(
    switch_device_t device,
    switch_int8_t index,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t *pvs_hdl);
switch_status_t switch_pd_dtel_intl45_icmp_marker_parser_value_modify(
    switch_device_t device,
    switch_uint8_t index,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t pvs_hdl);
switch_status_t switch_pd_dtel_intl45_icmp_marker_parser_value_delete(
    switch_device_t device, switch_uint8_t index, switch_pd_pvs_hdl_t pvs_hdl);
switch_status_t switch_pd_dtel_intl45_tcp_marker_parser_value_set(
    switch_device_t device,
    switch_int8_t index,
    switch_int16_t port,
    switch_int16_t port_mask,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t *pvs_hdl);
switch_status_t switch_pd_dtel_intl45_tcp_marker_parser_value_modify(
    switch_device_t device,
    switch_uint8_t index,
    switch_int16_t port,
    switch_int16_t port_mask,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t pvs_hdl);
switch_status_t switch_pd_dtel_intl45_tcp_marker_parser_value_delete(
    switch_device_t device, switch_uint8_t index, switch_pd_pvs_hdl_t pvs_hdl);
switch_status_t switch_pd_dtel_intl45_udp_marker_parser_value_set(
    switch_device_t device,
    switch_int8_t index,
    switch_int16_t port,
    switch_int16_t port_mask,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t *pvs_hdl);
switch_status_t switch_pd_dtel_intl45_udp_marker_parser_value_modify(
    switch_device_t device,
    switch_uint8_t index,
    switch_int16_t port,
    switch_int16_t port_mask,
    switch_uint64_t marker,
    switch_pd_pvs_hdl_t pvs_hdl);
switch_status_t switch_pd_dtel_intl45_udp_marker_parser_value_delete(
    switch_device_t device, switch_uint8_t index, switch_pd_pvs_hdl_t pvs_hdl);

switch_status_t switch_pd_dtel_int_ig_port_convert_set(switch_device_t device,
                                                       switch_port_t in_port,
                                                       switch_port_t out_port);
switch_status_t switch_pd_dtel_int_eg_port_convert_set(switch_device_t device,
                                                       switch_port_t in_port,
                                                       switch_port_t out_port);

switch_status_t switch_pd_dtel_int_ig_port_convert_set(switch_device_t device,
                                                       switch_port_t in_port,
                                                       switch_port_t out_port);
switch_status_t switch_pd_dtel_int_eg_port_convert_set(switch_device_t device,
                                                       switch_port_t in_port,
                                                       switch_port_t out_port);

// INT Transit

switch_status_t switch_pd_dtel_int_transit_enable(switch_device_t device);

switch_status_t switch_pd_dtel_int_transit_disable(switch_device_t device);

switch_status_t switch_pd_dtel_int_digest_encode_enable(switch_device_t device);

switch_status_t switch_pd_dtel_int_digest_encode_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_meta_header_update_end_enable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_meta_header_update_end_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_transit_qalert_add(switch_device_t device,
                                                      switch_uint8_t change);

switch_status_t switch_pd_dtel_int_transit_qalert_delete(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_outer_encap_transit_enable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_outer_encap_transit_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_intl45_update_l4_transit_enable(
    switch_device_t device);

switch_status_t switch_pd_dtel_intl45_update_l4_transit_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_transit_report_encap_table_enable_e2e(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    p4_pd_entry_hdl_t *entry_hdl,
    bool add);

// INT EP
switch_status_t switch_pd_dtel_int_insert_table_add_update(
    switch_device_t device,
    switch_uint16_t session_id,
    switch_uint16_t instruction,
    switch_uint8_t max_hop,
    bool add,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_insert_table_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_int_report_encap_table_enable_i2e(
    switch_device_t device,
    switch_uint16_t dest_udp_port,
    p4_pd_entry_hdl_t *entry_hdl,
    bool add);

switch_status_t switch_pd_dtel_int_report_encap_table_add_e2e(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_report_encap_table_modify_e2e(
    switch_device_t device,
    switch_uint32_t switch_id,
    switch_uint16_t dest_udp_port,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_report_encap_table_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_int_outer_encap_table_session_add_update(
    switch_device_t device,
    switch_uint16_t session_id,
    switch_uint16_t instruction,
    switch_dtel_int_info_t *int_info,
    bool add,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_outer_encap_table_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_intl45_update_l4_table_tcp_icmp_enable(
    switch_device_t device,
    switch_pd_hdl_t *entry_hdl,
    switch_dtel_int_info_t *int_info);

switch_status_t switch_pd_dtel_intl45_update_l4_table_udp_session_add_update(
    switch_device_t device,
    switch_uint16_t session_id,
    switch_uint16_t instruction,
    switch_dtel_int_info_t *int_info,
    bool add,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_intl45_update_l4_table_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_int_set_sink_enable(switch_device_t device,
                                                   switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_set_sink_disable(switch_device_t device,
                                                    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_sink_ports_add(switch_device_t device,
                                                  switch_port_t port,
                                                  switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dtel_int_sink_ports_delete(switch_device_t device,
                                                     switch_port_t port,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dtel_int_upstream_report_enable(
    switch_device_t device, switch_list_t *event_infos);

switch_status_t switch_pd_dtel_int_upstream_report_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_sink_local_report_enable(
    switch_device_t device, switch_list_t *event_infos);

switch_status_t switch_pd_dtel_int_sink_local_report_disable(
    switch_device_t device);

switch_status_t switch_pd_dtel_int_terminate_init_update(
    switch_device_t device,
    switch_dtel_int_info_t *int_info,
    switch_pd_hdl_t *entry_hdl,
    bool init);

// internal init functions
switch_pd_status_t switch_pd_dtel_int_ingress_bfilters_init(
    switch_device_t device);

#endif  // _switch_pd_dtel_h_
