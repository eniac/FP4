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
#ifndef _switch_dtel_int_h_
#define _switch_dtel_int_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define INT_SESSION_MAX_NUM 64
#define INT_SESSION_RP_HANDLE_NUM 2
#define INT_L4_HANDLE_NUM 2
#define INTL45_MARKER_DEFAULT_VALUE 0xAAAAAAAABBBBBBBB
#define INTL45_DSCP_DISABLE_MASK 0
#define INT_DEFAULT_SESSION_INSTRUCTION 0xDC00
#define INT_E2E_INSTRUCTION 0xD400
#define INT_DEFAULT_SESSION_MAX_HOP 8
#define INT_RPI_HANDLE_NUM 4  // 4=pipes
#define DTEL_FLOW_STATE_TRACK_NO_RESET_CYCLE 0

#ifdef INT_EP_ENABLE
#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
#define INT_RP_HANDLE_NUM 16  // 4 * 4 pipes
#else
#define INT_RP_HANDLE_NUM 4  // 1 * 4 pipes
#endif                       // STATELESS
#endif                       // INT_EP
#ifdef INT_TRANSIT_ENABLE
#define INT_RP_HANDLE_NUM 8  // 2 * 4 pipes
#endif                       // INT_TRANSIT
#ifndef INT_RP_HANDLE_NUM
#define INT_RP_HANDLE_NUM 4
#endif

#define POSTCARD_INSERT_HANDLE_NUM 12  // 3 * 4pipes default entry not included
#define MIRROR_ON_DROP_ENCAP_ENTRIES_NUM \
  64  // MIRROR_ON_DROP_ENCAP_TABLE_SIZE * 4pipes

// any non-zero value is OK
#define DTEL_QUEUE_REPORT_DEFAULT_QUOTA 1

#define SWITCH_DTEL_IP_PROTO_ICMP 1
#define SWITCH_DTEL_IP_PROTO_TCP 6
#define SWITCH_DTEL_IP_PROTO_UDP 17

#define SWITCH_DTEL_MARKER_SET_ICMP_INDEX 0
#define SWITCH_DTEL_MARKER_SET_TCP_INDEX 1
#define SWITCH_DTEL_MARKER_SET_UDP_INDEX 2
// disable latency change detection by default
// 30 means zero out latency, hence no change detection
#define DTEL_DEFAULT_LATENCY_QUANTIZATION_SHIFT 18

/** DTel watchlist match fields */
typedef struct switch_dtel_watchlist_match_spec_ {
  switch_uint16_t ether_type;
  switch_uint16_t ether_type_mask;
  switch_uint32_t ipv4_src;
  switch_uint32_t ipv4_src_mask;
  switch_uint32_t ipv4_dst;
  switch_uint32_t ipv4_dst_mask;
  switch_uint8_t ip_proto;
  switch_uint8_t ip_proto_mask;
  switch_uint8_t dscp;
  switch_uint8_t dscp_mask;
  switch_uint16_t l4_port_src_start;
  switch_uint16_t l4_port_src_end;
  switch_uint16_t l4_port_dst_start;
  switch_uint16_t l4_port_dst_end;
  switch_vni_t tunnel_vni;
  switch_vni_t tunnel_vni_mask;
  switch_uint16_t inner_ether_type;
  switch_uint16_t inner_ether_type_mask;
  switch_uint32_t inner_ipv4_src;
  switch_uint32_t inner_ipv4_src_mask;
  switch_uint32_t inner_ipv4_dst;
  switch_uint32_t inner_ipv4_dst_mask;
  switch_uint8_t inner_ip_proto;
  switch_uint8_t inner_ip_proto_mask;
  switch_uint16_t inner_l4_port_src_start;
  switch_uint16_t inner_l4_port_src_end;
  switch_uint16_t inner_l4_port_dst_start;
  switch_uint16_t inner_l4_port_dst_end;
} switch_twl_match_spec_t;

typedef struct switch_dtel_watchlist_entry_ {
  /** match fields */
  switch_twl_match_spec_t match;
  /** hash node */
  switch_hashnode_t node;
  /** rule priority */
  switch_uint16_t priority;
  /** watched or not watched */
  bool watch;
  /** INT session ID */
  switch_uint16_t int_session_id;
  /** pd handle */
  switch_pd_hdl_t pd_hdl;
} switch_twl_entry_t;

typedef struct switch_dtel_mirror_session_entry_ {
  /** mirror session id */
  switch_mirror_id_t mirror_id;
  /** hash node */
  switch_hashnode_t node;
  /** mirror pd handle */
  switch_pd_mbr_hdl_t pd_mbr_hdl;
  switch_device_t device;
} switch_dtel_mirror_session_entry_t;

typedef struct switch_dtel_mirror_info_ {
  /** mirror session hash table */
  switch_hashtable_t sessions;
  /** default mirror session group */
  switch_pd_grp_hdl_t default_session_grp;
  /** default mirror session pd handle */
  switch_pd_hdl_t default_session_hdl;
} switch_dtel_mirror_info_t;

typedef struct switch_dtel_int_session_entry_ {
  /** INT session id */
  switch_uint16_t session_id;
  /** hash node */
  switch_hashnode_t node;
  /** instruction */
  switch_uint16_t instruction;
  /** pd handle for int_insert table */
  switch_pd_hdl_t ins_hdl;
  /** pd handle for int_outer_encap table */
  switch_pd_hdl_t en_hdl;
  /** pd handle for intl45_update_l4 table*/
  switch_pd_hdl_t l4_hdl;
  /** ref count for watchlist references to this session */
  switch_uint16_t ref_count;
} switch_dtel_int_session_entry_t;

typedef struct switch_dtel_int_marker_port_entry_ {
  /** port value */
  switch_uint16_t value;
  /** port mask */
  switch_uint16_t mask;
  /** protocol **/
  switch_uint8_t proto;
  /** hash node */
  switch_hashnode_t node;
  /** pvs handle */
  switch_pd_pvs_hdl_t pvs_hdl;
} switch_dtel_int_marker_port_entry_t;

/** DTel INT info */
typedef struct switch_dtel_int_info_ {
  /** if endpoint/transit is enabled */
  bool enabled;
  /** INT watchlist */
  switch_hashtable_t watchlist;
  /** INT config sessions */
  switch_hashtable_t sessions;
  /** pd handle for the priority zero not_watch rule that disables INT */
  switch_pd_hdl_t off_hdl;
  /** pd handle for int_set_sink if int_header is valid */
  switch_pd_hdl_t set_sink_hdl;
  /** pd handle for int_terminate table */
  switch_pd_hdl_t term_hdl;
  /** pd handles for int_report_encap table */
  switch_pd_hdl_t rpi_hdl[INT_RPI_HANDLE_NUM];
  switch_pd_hdl_t rp_hdl[INT_RP_HANDLE_NUM];
  /** pd handles for intl45_update_l4 table*/
  switch_pd_hdl_t l4_hdl[INT_L4_HANDLE_NUM];
  /** pd handle for intl45 diffserv parser value set */
  switch_pd_pvs_hdl_t int_l45_dscp_pvs_hdl;
  /** value for intl45 diffserv parser value set */
  switch_uint8_t l45_diffserv_value;
  /** mask for intl45 diffserv parser value set, 0 not programmed*/
  switch_uint8_t l45_diffserv_mask;
  /** value for intl45 marker parser value set */
  switch_uint64_t l45_marker_udp_value;
  switch_uint64_t l45_marker_tcp_value;
  switch_uint64_t l45_marker_icmp_value;
  /** INT L45 marker ports for UDP */
  switch_hashtable_t l45_marker_udp_ports;
  /** INT L45 marker ports for TCP */
  switch_hashtable_t l45_marker_tcp_ports;
  /** INT L45 marker pvs handles for port agnostic pvs */
  switch_pd_pvs_hdl_t l45_marker_icmp_pvs_hdls[4];
  switch_pd_pvs_hdl_t l45_marker_tcp_pvs_hdls[3];
  switch_pd_pvs_hdl_t l45_marker_udp_pvs_hdls[3];
  /** INT sink port pd handles */
  switch_pd_hdl_t sink_port_hdl[SWITCH_MAX_PORTS];
} switch_dtel_int_info_t;

/** DTel Postcard info */
typedef struct switch_dtel_postcard_info_ {
  /** Postcard watchlist */
  switch_hashtable_t watchlist;
  /** pd handle for the priority zero not_watch rule that disables Postcard */
  switch_pd_hdl_t off_hdl;
  /** pd handles for postcard_insert table */
  switch_pd_hdl_t pi_hdl[POSTCARD_INSERT_HANDLE_NUM];
} switch_dtel_postcard_info_t;

/** DTel Mirror on Drop info */
typedef struct switch_mirror_on_drop_info_ {
  /** Mirror on Drop watchlist */
  switch_hashtable_t watchlist;
  /** pd handle for the priority zero not_watch rule that disables MoD */
  switch_pd_hdl_t off_hdl;
  /** pd handle for mirror on drop encap table */
  switch_pd_hdl_t me_hdl[MIRROR_ON_DROP_ENCAP_ENTRIES_NUM];
  /** deflect on drop destination queue initialized */
  bool dod_init;
} switch_mirror_on_drop_info_t;

typedef struct switch_queue_alert_index_entry_ {
  /** egress port */
  switch_dev_port_t port;
  /** queue id */
  switch_qid_t queue;
  /** hash node */
  switch_hashnode_t node;
  /** register array index */
  switch_uint16_t index;
  /** pd handle for dtel_queue_alert table */
  switch_pd_hdl_t qalert_pd_hdl;
  /** pd handle for deflect_on_drop_queue_config table */
  switch_pd_hdl_t qdod_pd_hdl;
  /** pd handle for dtel_queue_report_quota_dod table */
  switch_pd_hdl_t qdod_quota_pd_hdl;
} switch_queue_alert_index_entry_t;

typedef struct switch_queue_alert_info_ {
  /** hash table for queue alert info */
  switch_hashtable_t index_map;
  /** stack of available register array index */
  int index_stack[DTEL_QUEUE_TABLE_SIZE];
  /** pointer to the top of stack */
  int top;
} switch_queue_alert_info_t;

typedef struct dtel_event_info_ {
  /** list node */
  switch_node_t node;
  switch_dtel_event_type_t type;
  switch_uint8_t dscp;
} dtel_event_info_t;

/** DTel context */
typedef struct switch_dtel_context_ {
  /** Erspan mirror info */
  switch_dtel_mirror_info_t _mirror;
  /** INT info */
  switch_dtel_int_info_t _int;
  /** Postcard info */
  switch_dtel_postcard_info_t _postcard;
  /** Mirror on Drop info */
  switch_mirror_on_drop_info_t _mod;
  /** Queue alert info */
  switch_queue_alert_info_t _queue_alert;
  /** reset cycle */
  switch_uint16_t flowstate_reset_cycle;
  /** reset timer */
  bf_sys_timer_t flowstate_reset_timer;
  /** DSC of events */
  dtel_event_info_t event_infos[SWITCH_DTEL_EVENT_TYPE_MAX];
  switch_list_t event_infos_sorted_list;
  /** switch ID */
  switch_uint32_t switch_id;
  /** Dest UDP port number used in encapsulated report */
  switch_uint16_t dest_udp_port;
  /** latency quantization shift */
  switch_uint8_t quantization_shift;
} switch_dtel_context_t;

switch_status_t switch_twl_match_spec_print(
    switch_twl_match_spec_t *match_spec);

switch_status_t switch_dtel_init(switch_device_t device);

switch_status_t switch_dtel_free(switch_device_t device);

switch_status_t switch_dtel_default_entries_add(switch_device_t device);

switch_status_t switch_dtel_default_entries_delete(switch_device_t device);

switch_status_t switch_twl_key_init(void *args,
                                    switch_uint8_t *key,
                                    switch_uint32_t *len);

switch_status_t switch_twl_key_compare(const void *key1, const void *key2);

switch_status_t switch_twl_convert_match_spec(
    switch_uint32_t field_count,
    switch_twl_key_value_pair_t *fields,
    switch_twl_match_spec_t *match_spec);

//------------------------------------------------------------------------------
// INT internal
//------------------------------------------------------------------------------

switch_status_t switch_dtel_int_init(switch_device_t device);

switch_status_t switch_dtel_int_default_entries_add(switch_device_t device);

switch_status_t switch_dtel_int_default_entries_delete(switch_device_t device);

switch_status_t switch_int_session_key_init(void *args,
                                            switch_uint8_t *key,
                                            switch_uint32_t *len);

switch_status_t switch_int_session_key_compare(const void *key1,
                                               const void *key2);

switch_status_t switch_dtel_int_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_int_watchlist_entry_update(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_int_watchlist_entry_delete(
    switch_device_t device, switch_twl_match_info_t *match_info);

switch_status_t switch_dtel_int_watchlist_clear(switch_device_t device);

switch_status_t switch_int_marker_port_key_init(void *args,
                                                switch_uint8_t *key,
                                                switch_uint32_t *len);

switch_status_t switch_int_marker_port_key_compare(const void *key1,
                                                   const void *key2);

//------------------------------------------------------------------------------
// Postcard internal
//------------------------------------------------------------------------------

switch_status_t switch_dtel_postcard_init(switch_device_t device);

switch_status_t switch_dtel_postcard_default_entries_add(
    switch_device_t device);

switch_status_t switch_dtel_postcard_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_postcard_watchlist_entry_update(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_postcard_watchlist_entry_delete(
    switch_device_t device, switch_twl_match_info_t *match_info);

switch_status_t switch_dtel_postcard_watchlist_clear(switch_device_t device);

switch_status_t switch_dtel_postcard_switch_id(switch_device_t device,
                                               uint32_t switch_id);

switch_status_t switch_dtel_postcard_dest_udp_port(
    switch_device_t device, switch_uint16_t dest_udp_port);

//------------------------------------------------------------------------------
// Mirror on Drop internal
//------------------------------------------------------------------------------

switch_status_t switch_mirror_on_drop_init(switch_device_t device);

switch_status_t switch_mirror_on_drop_default_entries_add(
    switch_device_t device);

switch_status_t switch_dtel_drop_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_drop_watchlist_entry_update(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

switch_status_t switch_dtel_drop_watchlist_entry_delete(
    switch_device_t device, switch_twl_match_info_t *match_info);

switch_status_t switch_dtel_drop_watchlist_clear(switch_device_t device);

switch_status_t switch_dtel_int_switch_id(switch_device_t device,
                                          switch_uint32_t switch_id);

switch_status_t switch_dtel_int_dest_udp_port(switch_device_t device,
                                              switch_uint16_t dest_udp_port);
switch_status_t switch_mirror_on_drop_enable_dod(switch_device_t device);

switch_status_t switch_dtel_int_marker_disable(switch_device_t device);
switch_status_t switch_dtel_int_marker_enable(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif
