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
#ifndef _switch_dtel_h_
#define _switch_dtel_h_

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup DTel DTel API
 *  API functions define and manipulate advanced DTel
 *  @{
 */  // begin of DTel API

//------------------------------------------------------------------------------
// DTel shared data structure and API
//------------------------------------------------------------------------------

/** DTel Types */
typedef enum switch_dtel_type_ {
  SWITCH_DTEL_TYPE_INT,      /**< Inband Network Telemetry (INT) */
  SWITCH_DTEL_TYPE_POSTCARD, /**< Postcard */
  SWITCH_DTEL_TYPE_MOD,      /**< Mirror on Drop */
  SWITCH_DTEL_TYPE_MAX
} switch_dtel_type_t;

/** DTel watchlist match field enum */
typedef enum switch_dtel_watchlist_field_ {
  /** Ethernet type */
  SWITCH_TWL_FIELD_ETHER_TYPE,
  /** IPv4 source address */
  SWITCH_TWL_FIELD_IPV4_SRC,
  /** IPv4 destination address */
  SWITCH_TWL_FIELD_IPV4_DST,
  /** IP protocol */
  SWITCH_TWL_FIELD_IP_PROTO,
  /** IP Diffserv */
  SWITCH_TWL_FIELD_DSCP,
  /** L4 source port */
  SWITCH_TWL_FIELD_L4_PORT_SRC,
  /** L4 source port range start */
  SWITCH_TWL_FIELD_L4_PORT_SRC_START,
  /** L4 source port range end */
  SWITCH_TWL_FIELD_L4_PORT_SRC_END,
  /** L4 destination port */
  SWITCH_TWL_FIELD_L4_PORT_DST,
  /** L4 destination port range start */
  SWITCH_TWL_FIELD_L4_PORT_DST_START,
  /** L4 destination port range end */
  SWITCH_TWL_FIELD_L4_PORT_DST_END,
  /** Tunnel VNI */
  SWITCH_TWL_FIELD_TUNNEL_VNI,
  /** Ethernet type */
  SWITCH_TWL_FIELD_INNER_ETHER_TYPE,
  /** Inner IPv4 Source address */
  SWITCH_TWL_FIELD_INNER_IPV4_SRC,
  /** Inner IPv4 Destination address */
  SWITCH_TWL_FIELD_INNER_IPV4_DST,
  /** Inner IP Protocol */
  SWITCH_TWL_FIELD_INNER_IP_PROTO,
  /** Inner L4 source port */
  SWITCH_TWL_FIELD_INNER_L4_PORT_SRC,
  /** Inner L4 source port range start */
  SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_START,
  /** Inner L4 source port range end */
  SWITCH_TWL_FIELD_INNER_L4_PORT_SRC_END,
  /** Inner L4 destination port */
  SWITCH_TWL_FIELD_INNER_L4_PORT_DST,
  /** Inner L4 destination port range start */
  SWITCH_TWL_FIELD_INNER_L4_PORT_DST_START,
  /** Inner L4 destination port range end */
  SWITCH_TWL_FIELD_INNER_L4_PORT_DST_END,
  SWITCH_TWL_FIELD_MAX
} switch_twl_field_t;

/** DTel watchlist match field list */
typedef union switch_dtel_watchlist_value_ {
  switch_uint16_t ether_type; /**< Ethernet type */
  switch_uint32_t ipv4;       /**< IPv4 address */
  switch_uint8_t ip_proto;    /**< IP protocol */
  switch_uint8_t dscp;        /**< DSCP */
  switch_uint16_t l4_port;    /**< L4 port */
  switch_vni_t tunnel_vni;    /**< tunnel vni */
} switch_twl_value_t;

/** DTel watchlist mask */
typedef uint32_t switch_twl_mask_t;

/** DTel watchlist match field key value pair */
typedef struct switch_dtel_watchlist_key_value_pair_ {
  switch_twl_field_t field; /**< watchlist match field type */
  switch_twl_value_t value; /**< watchlist match field value */
  switch_twl_mask_t mask;   /**< watchlist match field mask */
} switch_twl_key_value_pair_t;

/** DTel watchlist match information */
typedef struct switch_dtel_watchlist_match_info_ {
  switch_uint32_t field_count;         /**< watchlist match field count */
  switch_twl_key_value_pair_t *fields; /**< watchlist match key value pairs */
} switch_twl_match_info_t;

/** DTel watchlist action parameters */
typedef union switch_dtel_watchlist_action_params_ {
  struct {
    switch_uint16_t session_id;         /**< INT session ID */
    bool report_all_packets;            /**< apply suppression at sink switch */
    switch_uint8_t flow_sample_percent; /**< the percent of flows to pick */
  } _int;                               /**< INT struct */
  struct {
    bool report_all_packets;            /**< Apply suppression */
    switch_uint8_t flow_sample_percent; /**< the percent of flows to pick */
  } _postcard;                          /**< Postcard struct */
  struct {
    bool report_queue_tail_drops; /**< enable deflect on drop */
  } _mod;                         /**< Mirror on Drop struct */
} switch_twl_action_params_t;

/** DTel event Types */
typedef enum switch_dtel_event_type_ {
  /** Report triggered by new flow or flow state (e.g., path, latency) change */
  SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE = 0,
  /** Report triggered by every packet of the flow without filtering */
  SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS = 1,
  /** Report triggered by TCP flags */
  SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG = 2,
  /** Report triggered by queue depth or latency threshold breach */
  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH = 3,
  /** Report triggered by queue deflect on drop */
  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP = 4,
  /** Report triggered by packet drop */
  SWITCH_DTEL_EVENT_TYPE_DROP_REPORT = 5,
  SWITCH_DTEL_EVENT_TYPE_MAX
} switch_dtel_event_type_t;

/**
 Add watchlist entries
 @param device - device
 @param type - DTel type
 @param match_info - watchlist match fields
 @param priority - priority for the watchlist entry
 @param watch - apply DTel to matched packets or not
 @param action_params - watchlist action parameters
*/
switch_status_t switch_api_dtel_watchlist_entry_create(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

/**
 Update watchlist entries, match_info is the update key
 @param device - device
 @param type - DTel type
 @param match_info - watchlist match fields
 @param priority - priority for the watchlist entry
 @param watch - apply DTel to matched packets or not
 @param action_params - watchlist action parameters
*/
switch_status_t switch_api_dtel_watchlist_entry_update(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params);

/**
 Delete watchlist entries
 @param device - device
 @param type - DTel type
 @param match_info - watchlist match fields
*/
switch_status_t switch_api_dtel_watchlist_entry_delete(
    switch_device_t device,
    switch_dtel_type_t type,
    switch_twl_match_info_t *match_info);

/**
 Clear all watchlist entries
 @param device - device
 @param type - DTel type
 @param match_info - watchlist match fields
*/
switch_status_t switch_api_dtel_watchlist_clear(switch_device_t device,
                                                switch_dtel_type_t type);

/**
 Set flow state clear cycle
 @param device - device
 @param cycle - clear cycle in seconds
*/
switch_status_t switch_api_dtel_flow_state_clear_cycle(switch_device_t device,
                                                       switch_uint16_t cycle);

/**
 Set latency quantization shift for flow state change detection
 @param device - device
 @param quant_shift - quantization shift
*/
switch_status_t switch_api_dtel_latency_quantization_shift(
    switch_device_t device, switch_uint8_t quant_shift);

/**
 Set queue alerts
 @param device - device
 @param port - egress port
 @param queue - queue id
 @param depth - queue depth threshold
 @param latency - queue latency threshold
 @param quota - # queue report packets
*/
switch_status_t switch_api_dtel_queue_report_create(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth_threshold,
    switch_uint32_t latency_threshold,
    switch_uint16_t report_quota_during_breach,
    bool report_tail_drops);

/**
 Update queue alerts, <port, queue> tuple is the update key
 @param device - device
 @param port - egress port
 @param queue - queue id
 @param depth - queue depth threshold
 @param latency - queue latency threshold
 @param quota - # queue report packets
*/
switch_status_t switch_api_dtel_queue_report_update(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint32_t depth_threshold,
    switch_uint32_t latency_threshold,
    switch_uint16_t report_quota_during_breach,
    bool report_tail_drops);

/**
 Delete queue alerts
 @param device - device
 @param port - egress port
 @param queue - queue id
*/
switch_status_t switch_api_dtel_queue_report_delete(switch_device_t device,
                                                    switch_port_t port,
                                                    switch_int16_t queue);

/**
 Get the remaining quota per port and queue
 @param device - device
 @param port - egress port
 @param queue - queue id
 @param quota - returning quota values
*/
switch_status_t switch_api_dtel_queue_remaining_report_quota_during_breach_get(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t queue,
    switch_uint16_t *quota);

/**
 Add DTel ERSPAN mirror sessions
 @param device - device
 @param mirror_id - mirror session ID
*/
switch_status_t switch_api_dtel_report_session_add(
    switch_device_t device, switch_mirror_id_t mirror_id);

/**
 Delete DTel ERSPAN mirror sessions
 @param device - device
 @param mirror_id - mirror session ID
*/
switch_status_t switch_api_dtel_report_session_delete(
    switch_device_t device, switch_mirror_id_t mirror_id);

/**
 Set DTel switch ID
 @param device - device
 @param switch_id - network-wide unique switch ID
*/
switch_status_t switch_api_dtel_switch_id_set(switch_device_t device,
                                              switch_uint32_t switch_id);

/**
 Set destination UDP port for DTel reports
 @param device - device
 @param dest_udp_port - destination udp port to be used in encap packets
*/
switch_status_t switch_api_dtel_report_udp_dstport_set(
    switch_device_t device, switch_uint16_t dest_udp_port);

/**
 Set sequence number value
 @param device - device
 @param mirror_session_id = mirror session id
 @param value - sequence number value
*/
switch_status_t switch_api_dtel_report_sequence_number_set(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t value);

/**
 Returns the sequence number of that mirror session.
 returns one value per pipe
 @param device - device
 @param mirror_session_id = mirror session id
 @param values - sequence number values
 @param max_num - maximum number to read. will be updated to the number read
*/
switch_status_t switch_api_dtel_report_sequence_number_get(
    switch_device_t device,
    switch_uint16_t mirror_session_id,
    switch_uint32_t *values,
    switch_uint8_t *max_num);

/**
 Get DSCP code for report packets of the specified type
 @param device - device
 @param event_type type of the event that generated the report
 @return dscp 6-bit value for dscp in IPv4 header
*/
switch_status_t switch_api_dtel_event_get_dscp(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t *dscp);

/**
 Set DSCP code for report packets of the specified type
 @param device - device
 @param event_type type of the event that generated the report
 @param dscp 6-bit value for dscp in IPv4 header
*/
switch_status_t switch_api_dtel_event_set_dscp(
    switch_device_t device,
    switch_dtel_event_type_t event_type,
    switch_uint8_t dscp);

//------------------------------------------------------------------------------
// INT API
//------------------------------------------------------------------------------

/**
 Enable INT transit
 @param device - device
*/
switch_status_t switch_api_dtel_int_transit_enable(switch_device_t device);

/**
 Disable INT transit
 @param device - device
*/
switch_status_t switch_api_dtel_int_transit_disable(switch_device_t device);

/**
 Enable INT endpoint
 @param device - device
*/
switch_status_t switch_api_dtel_int_endpoint_enable(switch_device_t device);

/**
 Disable INT endpoint
 @param device - device
*/
switch_status_t switch_api_dtel_int_endpoint_disable(switch_device_t device);

/**
 Create INT sessions
 @param device - device
 @param session_id - INT session ID
 @param instruction - INT instruction bitmap
 @param max_hop - INT max hop
*/
switch_status_t switch_api_dtel_int_session_create(switch_device_t device,
                                                   switch_uint16_t session_id,
                                                   switch_uint16_t instruction,
                                                   switch_uint8_t max_hop);

/**
 Update INT sessions, session_id is the update key
 @param device - device
 @param session_id - INT session ID
 @param instruction - INT instruction bitmap
 @param max_hop - INT max hop
*/
switch_status_t switch_api_dtel_int_session_update(switch_device_t device,
                                                   switch_uint16_t session_id,
                                                   switch_uint16_t instruction,
                                                   switch_uint8_t max_hop);

/**
 Delete INT sessions
 @param device - device
 @param session_id - INT session ID
*/
switch_status_t switch_api_dtel_int_session_delete(switch_device_t device,
                                                   switch_uint16_t session_id);

/**
 Add INT sink downstream ports
 @param device - device
 @param port - egress port to apply INT sink
*/
switch_status_t switch_api_dtel_int_sink_ports_add(switch_device_t device,
                                                   switch_port_t port);

/**
 Delete INT sink downstream ports
 @param device - device
 @param port - egress port to apply INT sink
*/
switch_status_t switch_api_dtel_int_sink_ports_delete(switch_device_t device,
                                                      switch_port_t port);

/**
 Set reserved dscp value for INT over L4
 @param device - device
 @param value - reserved dscp value (6 bits)
 @param mask - reserved dscp value mask (6 bits)
*/
switch_status_t switch_api_dtel_int_dscp_value_set(switch_device_t device,
                                                   switch_uint8_t value,
                                                   switch_uint8_t mask);

/**
 Set marker value for INT over L4
 @param device - device
 @param proto - l4 protocol
 @param marker - reserved marker value (64 bits)
*/
switch_status_t switch_api_dtel_int_marker_set(switch_device_t device,
                                               switch_uint8_t proto,
                                               switch_uint64_t marker);

/**
 Disable marker value for INT over L4
 @param device - device
 @param proto - l4 protocol
*/
switch_status_t switch_api_dtel_int_marker_delete(switch_device_t device,
                                                  switch_uint8_t proto);

/**
 get marker value for INT over L4
 @param device - device
 @param proto - l4 protocol
 @param marker - reserved marker value (64 bits)
*/
switch_status_t switch_api_dtel_int_marker_get(switch_device_t device,
                                               switch_uint8_t proto,
                                               switch_uint64_t *marker);

/**
 Add l4 dst port used besides the marker
 @param device - device
 @param proto - l4 protocol
 @param value - port value
 @param mask - port mask
*/
switch_status_t switch_api_dtel_int_marker_port_add(switch_device_t device,
                                                    switch_uint8_t proto,
                                                    switch_uint16_t value,
                                                    switch_uint16_t mask);

/**
 Del l4 dst port used besides the marker
 @param device - device
 @param proto - l4 protocol
 @param value - port value
 @param mask - port mask
*/
switch_status_t switch_api_dtel_int_marker_port_delete(switch_device_t device,
                                                       switch_uint8_t proto,
                                                       switch_uint16_t value,
                                                       switch_uint16_t mask);

/**
 Cleans l4 dst ports used besides the marker
 @param device - device
 @param proto - l4 protocol
*/
switch_status_t switch_api_dtel_int_marker_port_clear(switch_device_t device,
                                                      switch_uint8_t proto);

//------------------------------------------------------------------------------
// Postcard API
//------------------------------------------------------------------------------

/**
 Enable Postcard
 @param device - device
*/
switch_status_t switch_api_dtel_postcard_enable(switch_device_t device);

/**
 Disable Postcard
 @param device - device
*/
switch_status_t switch_api_dtel_postcard_disable(switch_device_t device);

//------------------------------------------------------------------------------
// Mirror on Drop API
//------------------------------------------------------------------------------

/**
 Enable Mirror on Drop
 @param device - device
*/
switch_status_t switch_api_dtel_drop_report_enable(switch_device_t device);

/**
 Disable Mirror on Drop
 @param device - device
*/
switch_status_t switch_api_dtel_drop_report_disable(switch_device_t device);

/** @} */  // end of DTel API

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // _switch_dtel_h_
