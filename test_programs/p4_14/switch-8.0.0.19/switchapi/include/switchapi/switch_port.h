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

#ifndef __SWITCH_PORT_H__
#define __SWITCH_PORT_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_id.h"
#include "switch_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup Port Port configuration API
 *  API functions listed to configure the ports. Mostly
 *  related to MAC programming
    The basic configuration on the port dictates the MAC programming.
    The modes can be set to one of 1x100G, 2x50G, 4x25G, 2x40G or 4x10G.
    The ports can be configured with an administrative mode and default behavior can be set.
    The tables that get modified in response to the port APIs are mostly the early stage tables.
    The port can have a default, which generally allows tagging of untagged packets to this default
    domain for forwarding the packets through the device.
 *  @{
 */  // begin of Port

#define SWITCH_MAX_HW_LANES 4

/** port speed */
typedef enum switch_port_speed_s {
  SWITCH_PORT_SPEED_NONE = 0,
  SWITCH_PORT_SPEED_10G = 1,
  SWITCH_PORT_SPEED_25G = 2,
  SWITCH_PORT_SPEED_40G = 3,
  SWITCH_PORT_SPEED_50G = 4,
  SWITCH_PORT_SPEED_100G = 5
} switch_port_speed_t;

typedef enum switch_port_auto_neg_mode_s {
  SWITCH_PORT_AUTO_NEG_MODE_DEFAULT = 0,
  SWITCH_PORT_AUTO_NEG_MODE_ENABLE = 1,
  SWITCH_PORT_AUTO_NEG_MODE_DISABLE = 2
} switch_port_auto_neg_mode_t;

/** port flowcontrol type */
typedef enum switch_flowcontrol_type_s {
  SWITCH_FLOWCONTROL_TYPE_NONE = 0,
  SWITCH_FLOWCONTROL_TYPE_PFC = 1,
  SWITCH_FLOWCONTROL_TYPE_PAUSE = 2
} switch_flowcontrol_type_t;

typedef enum switch_port_event_s {
  SWITCH_PORT_EVENT_ADD = 1,
  SWITCH_PORT_EVENT_DELETE = 2,
  SWITCH_PORT_EVENT_MAX = 3
} switch_port_event_t;

typedef enum switch_port_oper_status_s {
  SWITCH_PORT_OPER_STATUS_NONE = 0,
  SWITCH_PORT_OPER_STATUS_UNKNOWN = 1,
  SWITCH_PORT_OPER_STATUS_UP = 2,
  SWITCH_PORT_OPER_STATUS_DOWN = 3,
  SWITCH_PORT_OPER_STATUS_MAX
} switch_port_oper_status_t;

typedef enum switch_port_attribute_s {
  SWITCH_PORT_ATTR_ADMIN_STATE = (1 << 0),
  SWITCH_PORT_ATTR_SPEED = (1 << 1),
  SWITCH_PORT_ATTR_DEFAULT_TC = (1 << 2),
  SWITCH_PORT_ATTR_INGRESS_QOS_GROUP = (1 << 3),
  SWITCH_PORT_ATTR_EGRESS_QOS_GROUP = (1 << 4),
  SWITCH_PORT_ATTR_TC_QOS_GROUP = (1 << 5),
  SWITCH_PORT_ATTR_TRUST_DSCP = (1 << 6),
  SWITCH_PORT_ATTR_TRUST_PCP = (1 << 7),
  SWITCH_PORT_ATTR_DEFAULT_COLOR = (1 << 8),
  SWITCH_PORT_ATTR_UUC_METER_HANDLE = (1 << 9),
  SWITCH_PORT_ATTR_UMC_METER_HANDLE = (1 << 10),
  SWITCH_PORT_ATTR_BCAST_METER_HANDLE = (1 << 11),
  SWITCH_PORT_ATTR_OPER_STATUS = (1 << 12),
  SWITCH_PORT_ATTR_LANE_LIST = (1 << 13),
  SWITCH_PORT_ATTR_INGRESS_ACL_GROUP = (1 << 14),
  SWITCH_PORT_ATTR_LOOPBACK_MODE = (1 << 15),
  SWITCH_PORT_ATTR_AUTO_NEG_MODE = (1 << 16),
  SWITCH_PORT_ATTR_MTU = (1 << 17),
  SWITCH_PORT_ATTR_NUM_QUEUES = (1 << 18),
  SWITCH_PORT_ATTR_LEARNING_ENABLED = (1 << 19),
  SWITCH_PORT_ATTR_EGRESS_ACL_GROUP = (1 << 20),

} switch_port_attribute_t;

typedef enum switch_port_counter_id_s {
  SWITCH_PORT_STAT_IN_GOOD_OCTETS = 0,
  SWITCH_PORT_STAT_IN_ALL_OCTETS = 1,
  SWITCH_PORT_STAT_IN_GOOD_PKTS = 2,
  SWITCH_PORT_STAT_IN_ALL_PKTS = 3,
  SWITCH_PORT_STAT_IN_VLAN_PKTS = 4,
  SWITCH_PORT_STAT_IN_UCAST_PKTS = 5,
  SWITCH_PORT_STAT_IN_MCAST_PKTS = 6,
  SWITCH_PORT_STAT_IN_BCAST_PKTS = 7,
  SWITCH_PORT_STAT_IN_FCS_ERRORS = 8,
  SWITCH_PORT_STAT_IN_ERROR_PKTS = 9,
  SWITCH_PORT_STAT_IN_CRC_ERRORS = 10,
  SWITCH_PORT_STAT_IN_BUFFER_FULL = 11,
  SWITCH_PORT_STAT_IN_FRAGMENTS = 12,
  SWITCH_PORT_STAT_IN_JABBERS = 13,
  SWITCH_PORT_STAT_OUT_GOOD_OCTETS = 14,
  SWITCH_PORT_STAT_OUT_ALL_OCTETS = 15,
  SWITCH_PORT_STAT_OUT_GOOD_PKTS = 16,
  SWITCH_PORT_STAT_OUT_ALL_PKTS = 17,
  SWITCH_PORT_STAT_OUT_VLAN_PKTS = 18,
  SWITCH_PORT_STAT_OUT_UCAST_PKTS = 19,
  SWITCH_PORT_STAT_OUT_MCAST_PKTS = 20,
  SWITCH_PORT_STAT_OUT_BCAST_PKTS = 21,
  SWITCH_PORT_STAT_OUT_ERROR_PKTS = 22,
  SWITCH_PORT_STAT_IN_PKTS_LT_64 = 23,
  SWITCH_PORT_STAT_IN_PKTS_EQ_64 = 24,
  SWITCH_PORT_STAT_IN_PKTS_65_TO_127 = 25,
  SWITCH_PORT_STAT_IN_PKTS_128_TO_255 = 26,
  SWITCH_PORT_STAT_IN_PKTS_256_TO_511 = 27,
  SWITCH_PORT_STAT_IN_PKTS_512_TO_1023 = 28,
  SWITCH_PORT_STAT_IN_PKTS_1024_TO_1518 = 29,
  SWITCH_PORT_STAT_IN_PKTS_1519_TO_2047 = 30,
  SWITCH_PORT_STAT_IN_PKTS_2048_TO_4095 = 31,
  SWITCH_PORT_STAT_IN_PKTS_4096_TO_8191 = 32,
  SWITCH_PORT_STAT_IN_PKTS_8192_TO_9215 = 33,
  SWITCH_PORT_STAT_IN_PKTS_9216 = 34,
  SWITCH_PORT_STAT_OUT_PKTS_LT_64 = 35,
  SWITCH_PORT_STAT_OUT_PKTS_EQ_64 = 36,
  SWITCH_PORT_STAT_OUT_PKTS_65_TO_127 = 37,
  SWITCH_PORT_STAT_OUT_PKTS_128_TO_255 = 38,
  SWITCH_PORT_STAT_OUT_PKTS_256_TO_511 = 39,
  SWITCH_PORT_STAT_OUT_PKTS_512_TO_1023 = 40,
  SWITCH_PORT_STAT_OUT_PKTS_1024_TO_1518 = 41,
  SWITCH_PORT_STAT_OUT_PKTS_1519_TO_2047 = 42,
  SWITCH_PORT_STAT_OUT_PKTS_2048_TO_4095 = 43,
  SWITCH_PORT_STAT_OUT_PKTS_4096_TO_8191 = 44,
  SWITCH_PORT_STAT_OUT_PKTS_8192_TO_9215 = 45,
  SWITCH_PORT_STAT_OUT_PKTS_9216 = 46,
  SWITCH_PORT_STAT_IN_PFC_0_PKTS = 47,
  SWITCH_PORT_STAT_IN_PFC_1_PKTS = 48,
  SWITCH_PORT_STAT_IN_PFC_2_PKTS = 49,
  SWITCH_PORT_STAT_IN_PFC_3_PKTS = 50,
  SWITCH_PORT_STAT_IN_PFC_4_PKTS = 51,
  SWITCH_PORT_STAT_IN_PFC_5_PKTS = 52,
  SWITCH_PORT_STAT_IN_PFC_6_PKTS = 53,
  SWITCH_PORT_STAT_IN_PFC_7_PKTS = 54,
  SWITCH_PORT_STAT_OUT_PFC_0_PKTS = 55,
  SWITCH_PORT_STAT_OUT_PFC_1_PKTS = 56,
  SWITCH_PORT_STAT_OUT_PFC_2_PKTS = 57,
  SWITCH_PORT_STAT_OUT_PFC_3_PKTS = 58,
  SWITCH_PORT_STAT_OUT_PFC_4_PKTS = 59,
  SWITCH_PORT_STAT_OUT_PFC_5_PKTS = 60,
  SWITCH_PORT_STAT_OUT_PFC_6_PKTS = 61,
  SWITCH_PORT_STAT_OUT_PFC_7_PKTS = 62,
  SWITCH_PORT_STAT_IN_OVER_SIZED_PKTS = 63,
  SWITCH_PORT_STAT_IN_UNDER_SIZED_PKTS = 64,
  SWITCH_PORT_STAT_IN_FRAMES_TOO_LONG = 65,
  SWITCH_PORT_STAT_MAX

} switch_port_counter_id_t;

typedef enum switch_port_breakout_type_s {
  SWITCH_PORT_BREAKOUT_TYPE_LANE_1 = 1,
  SWITCH_PORT_BREAKOUT_TYPE_LANE_2 = 2,
  SWITCH_PORT_BREAKOUT_TYPE_LANE_4 = 3
} switch_port_breakout_type_t;

typedef enum switch_port_loopback_mode_s {
  SWITCH_PORT_LOOPBACK_MODE_NONE = 0,
  SWITCH_PORT_LOOPBACK_MODE_PHY_NEAR = 1,
  SWITCH_PORT_LOOPBACK_MODE_PHY_FAR = 2,
  SWITCH_PORT_LOOPBACK_MODE_MAC_NEAR = 3,
  SWITCH_PORT_LOOPBACK_MODE_MAC_FAR = 4,
  SWITCH_PORT_LOOPBACK_MODE_MAX
} switch_port_loopback_mode_t;

typedef enum switch_port_fec_mode_s {
  SWITCH_PORT_FEC_MODE_NONE = 0,
  SWITCH_PORT_FEC_MODE_FC = 1,
  SWITCH_PORT_FEC_MODE_RS = 2
} switch_port_fec_mode_t;

typedef struct switch_port_lane_list_s {
  switch_uint16_t num_lanes;
  switch_uint16_t lane[SWITCH_MAX_HW_LANES];
} switch_port_lane_list_t;

typedef struct switch_port_attribute_info_s {
  switch_port_oper_status_t oper_status;
  switch_handle_t ingress_qos_group;
  switch_handle_t egress_qos_group;
  switch_handle_t tc_qos_group;
  bool trust_dscp;
  bool trust_pcp;
  switch_color_t default_color;
  switch_port_lane_list_t lane_list;
  switch_port_speed_t port_speed;
  switch_port_breakout_type_t breakout_type;
  switch_s32_list_t supported_breakouts;
  bool admin_state;
  switch_handle_t ingress_acl_group_handle;
  switch_handle_t egress_acl_group_handle;
  switch_port_loopback_mode_t lb_mode;
  switch_port_auto_neg_mode_t an_mode;
  switch_uint8_t num_queues;
  bool learning_enabled;
} switch_port_attribute_info_t;

typedef struct switch_api_port_info_s {
  switch_port_t port;
  switch_port_speed_t port_speed;
  bool initial_admin_state;
  switch_int32_t tx_mtu;
  switch_int32_t rx_mtu;
  switch_port_fec_mode_t fec_mode;
} switch_api_port_info_t;

/**
 * Probe for existing ports - configuration based on current status
 * or default (when called immediately after init with default
 * config
 @param device device to use
 @param max_count maximum number of ports to return
 @param count actual count returned
 @param port_info array of port_info structures per port
 */
switch_status_t switch_api_port_probe(switch_device_t device,
                                      unsigned int max_count,
                                      unsigned int *count,
                                      switch_api_port_info_t *port_info);

/**
 Port Enable  Set- Enabled the port on a device
 @param device device to use
 @param port port on device to set
 @param enable TRUE => port is enabled FALSE => Port is disabled
*/
switch_status_t switch_api_port_enable_set(switch_device_t device,
                                           switch_port_t port,
                                           bool enable);

/**
 Port Enable Get - Get the Port Enabled state
 @param device device to use
 @param port port on device to get information
 @param enable TRUE => port is enabled FALSE => Port is disabled
*/
switch_status_t switch_api_port_enable_get(switch_device_t device,
                                           switch_port_t port,
                                           bool *enable);

/**
 Port Speed Set
 @param device device to use
 @param port port on device to set
 @param speed desired speed of port
*/
switch_status_t switch_api_port_speed_set(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_port_speed_t speed);

/**
Port Speed Get
@param device device to use
@param port port on device to get
@param speed actual speed of port
*/
switch_status_t switch_api_port_speed_get(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_port_speed_t *speed);

/**
 Port Autonegotiation Set
 @param device device to use
 @param port port on device to set
 @param enable Enable Autonegotiation if TRUE else disable
*/
switch_status_t switch_api_port_auto_neg_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t an_mode);
/**
Port Autonegotiation get
@param device device to use
@param port port on device to get
@param enable returns TRUE if Autonegotiation is set else FALSE
*/
switch_status_t switch_api_port_auto_neg_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_auto_neg_mode_t *an_mode);

/** Port Pause message information */
typedef struct switch_port_pause_info_ {
  bool rx;               /**< rx ignore PAUSE FALSE => disable PAUSE */
  bool tx;               /**< tx send PAUSE frames when needed */
  switch_mac_addr_t mac; /**< MAC addr to use when sending pause frames */
  bool symmetric;        /**< Symmetric or Asymmetric mode */
  unsigned int quanta;   /**< time in ms after which to stop sending pause */
} switch_port_pause_info_t;

/**
 Port set admin state. Enable/disable port
 @param device device to use
 @param port_handle port handle of device to get
 @param admin_state Administrative state
*/
switch_status_t switch_api_port_admin_state_set(switch_device_t device,
                                                switch_handle_t port_handle,
                                                bool admin_state);
/**
 Port get admin state
 @param device device to use
 @param port_handle port handle of device to get
 @param admin_state
*/
switch_status_t switch_api_port_admin_state_get(switch_device_t device,
                                                switch_handle_t port_handle,
                                                bool *admin_state);

/**
 Port get operational state
 @param device device to use
 @param port port on device to get
 @param oper_status Operational status
*/
switch_status_t switch_api_port_oper_status_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_oper_status_t *oper_status);

/**
 Port operational state declaration interval
 @param device device to use
 @param port port on device to get
 @param interval microseconds to debounce
*/
switch_status_t switch_api_port_debounce_set(switch_device_t device,
                                             switch_port_t port,
                                             unsigned int interval);

/**
 Port set MAC in loopback
 @param device device to use
 @param port port on device to set
 @param enable loopback enabled if TRUE else FALSE
*/
switch_status_t switch_api_port_loopback_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t lb_mode);

/**
 Port get MAC loopback config
 @param device device to use
 @param port port on device to get
 @param enable TRUE if loopback is enabled else FALSE
*/
switch_status_t switch_api_port_loopback_mode_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_loopback_mode_t *lb_mode);

/**
 Port L2 MTU settings
 @param device device to use
 @param port port on device to set
 @param l2mtu Max frame size on port
*/
switch_status_t switch_api_port_mtu_set(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t rx_mtu,
                                        switch_uint32_t tx_mtu);

/**
 Port L3 MTU settings
 @param device device to use
 @param port port on device to set
 @param l3mtu IP MTU on port
*/
switch_status_t switch_api_port_mtu_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t *rx_mtu,
                                        switch_uint32_t *tx_mtu);

/**
 Port egress rate set
 @param device device to use
 @param port port on device to set
 @param rate rate in kbps
*/
switch_status_t switch_api_port_egress_rate_set(switch_device_t device,
                                                switch_port_t port,
                                                unsigned int rate);

/**
 Set Port configuration
 @param device device to use
 @param api_port_info port information inclduing port number
 (portnumber specified in port_info->port_number)
*/
switch_status_t switch_api_port_set(switch_device_t device,
                                    switch_api_port_info_t *api_port_info);

/**
 Get Port configuration
 @param device device to use
 @param api_port_info port information inclduing port number
 (portnumber specified in port_info->port_number)
*/
switch_status_t switch_api_port_get(switch_device_t device,
                                    switch_api_port_info_t *api_port_info);

/**
 Set meter handle for port
 @param device device to use
 @param port port on device
 @param pkt_type packet type
 @param meter_handle meter handle
 */
switch_status_t switch_api_port_storm_control_set(switch_device_t device,
                                                  switch_handle_t port_handle,
                                                  switch_packet_type_t pkt_type,
                                                  switch_handle_t meter_handle);

/**
 Get meter handle for port
 @param device device to use
 @param port port on device
 @param pkt_type packet type
 @param meter_handle meter handle
 */
switch_status_t switch_api_port_storm_control_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_packet_type_t pkt_type,
    switch_handle_t *meter_handle);
/**
 Meter stats
 @param device device
 @param meter_handle meter handle
 @param count number of counters
 @param counter_ids meter counter ids
 @param counters counter values
 */
switch_status_t switch_api_storm_control_counters_get(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint16_t num_counters,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters);

/**
 Create port PPG
 @param device device
 @param port_handle port handle
 */
switch_status_t switch_api_ppg_create(switch_device_t device,
                                      switch_handle_t port_handle);

switch_status_t switch_api_ppg_delete(switch_device_t device,
                                      switch_handle_t port_handle);

switch_status_t switch_api_port_drop_limit_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               uint32_t num_bytes);

switch_status_t switch_api_port_drop_hysteresis_set(switch_device_t device,
                                                    switch_handle_t port_handle,
                                                    uint32_t num_bytes);

/**
 Set port to cos mapping in ingress
 @param device device
 @param port_handle port handle
 @param ppg_handle priority group handle
 @param cos_bitmap cos bitmap
*/
switch_status_t switch_api_port_cos_mapping(switch_device_t device,
                                            switch_handle_t port_handle,
                                            switch_handle_t ppg_handle,
                                            uint8_t cos_bitmap);

/**
 Set port cos and pfc cos mapping
 @param device device
 @param port_handle port handle
 @param cos_to_icos cos to ingress cos bitmap
*/
switch_status_t switch_api_port_pfc_cos_mapping(switch_device_t device,
                                                switch_handle_t port_handle,
                                                uint8_t *cos_to_icos);

/**
 Enable port shaping
 @param device device
 @param port_handle port handle
 @param shaper_type shaper type in bytes or packets
 @param burst_size burst size
 @param rate rate
*/
switch_status_t switch_api_port_shaping_enable(switch_device_t device,
                                               switch_handle_t port_handle,
                                               switch_shaper_type_t shaper_type,
                                               uint32_t burst_size,
                                               uint32_t rate);

/**
 Disable port shaping
 @param device device
 @param port_handle port handle
*/
switch_status_t switch_api_port_shaping_disable(switch_device_t device,
                                                switch_handle_t port_handle);

/**
 enable dscp trust on port
 @param device device
 @param port_handle port handle
 @param trust_dscp dscp trust
*/
switch_status_t switch_api_port_trust_dscp_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool trust_dscp);

/**
 enable pcp trust on port
 @param device device
 @param port_handle port handle
 @param trust_pcp pcp trust
*/
switch_status_t switch_api_port_trust_pcp_set(switch_device_t device,
                                              switch_handle_t port_handle,
                                              bool trust_pcp);

/**
 enable lossless mode in port priority group
 @param device device
 @param ppg_handle ppg handle
 @param enable enable
*/
switch_status_t switch_api_ppg_lossless_enable(switch_device_t device,
                                               switch_handle_t ppg_handle,
                                               bool enable);

/**
 set guaranteed limit on ppg
 @param device device
 @param ppg_handle ppg handle
 @param num_bytes number of bytes
*/
switch_status_t switch_api_ppg_guaranteed_limit_set(switch_device_t device,
                                                    switch_handle_t ppg_handle,
                                                    uint32_t num_bytes);

/**
 set skid lmit on ppg
 @param device device
 @param ppg_handle ppg handle
 @param num_bytes number of bytes
*/
switch_status_t switch_api_ppg_skid_limit_set(switch_device_t device,
                                              switch_handle_t ppg_handle,
                                              uint32_t num_bytes);

/**
 set hystersis lmit on ppg
 @param device device
 @param ppg_handle ppg handle
 @param num_bytes number of bytes
*/
switch_status_t switch_api_ppg_skid_hysteresis_set(switch_device_t device,
                                                   switch_handle_t ppg_handle,
                                                   uint32_t num_bytes);

/**
 set ingress qos group on port
 @param device device
 @param port_handle port handle
 @param qos_group qos group
*/
switch_status_t switch_api_port_qos_group_ingress_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_group);

/**
 set tc qos group on port
 @param device device
 @param port_handle port handle
 @param qos_group qos group
*/
switch_status_t switch_api_port_qos_group_tc_set(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_handle_t qos_group);

/**
 set egress qos group on port
 @param device device
 @param port_handle port handle
 @param qos_group qos group
*/
switch_status_t switch_api_port_qos_group_egress_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_group);

/**
 set default tc on port
 @param device device
 @param port_handle port handle
 @param tc traffic class
*/
switch_status_t switch_api_port_tc_default_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               uint16_t tc);

/**
 set default color on port
 @param device device
 @param port_handle port handle
 @param color packet color
*/
switch_status_t switch_api_port_color_default_set(switch_device_t device,
                                                  switch_handle_t port_handle,
                                                  switch_color_t color);

/**
 set port flowcontrol mode
 @param device device
 @param port_handle port handle
 @param flow_control flow control type
*/
switch_status_t switch_api_port_flowcontrol_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_flowcontrol_type_t flow_control);

/**
 enable mac learning on port
 @param device device
 @param port_handle port handle
 @param learning_enabled enable mac learning
*/
switch_status_t switch_api_port_learning_enabled_set(
    switch_device_t device, switch_handle_t port_handle, bool learning_enabled);

switch_status_t switch_api_port_add(switch_device_t device,
                                    switch_api_port_info_t *api_port_info,
                                    switch_handle_t *port_handle);

switch_status_t switch_api_port_delete(switch_device_t device,
                                       switch_handle_t port_handle);

typedef void (*switch_port_event_notification_fn)(
    switch_device_t device,
    switch_handle_t handle,
    switch_port_event_t port_event,
    void *app_data);

switch_status_t switch_api_port_event_notification_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_event_notification_fn cb_fn);

switch_status_t switch_api_port_event_notification_deregister(
    switch_device_t device, switch_app_id_t app_id);

typedef void (*switch_port_state_change_notification_fn)(
    switch_device_t device,
    switch_handle_t handle,
    switch_port_oper_status_t oper_status,
    void *app_data);

switch_status_t switch_api_port_state_change_notification_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_port_state_change_notification_fn cb_fn);

switch_status_t switch_api_port_state_change_notification_deregister(
    switch_device_t device, switch_app_id_t app_id);

/**
 Get port handle given port ID
 @param device device to use
 @param port port on device to get
 @param port_handle Port handle
*/
switch_status_t switch_api_port_id_to_handle_get(switch_device_t device,
                                                 switch_port_t port,
                                                 switch_handle_t *port_handle);

/**
 Get port ID given port handle
 @param device device to use
 @param port_handle Port handle
 @param port port on device to get
*/
switch_status_t switch_api_port_handle_to_id_get(switch_device_t device,
                                                 switch_handle_t port_handle,
                                                 switch_port_t *port);

/**
 Get port statistics
 @param device device to use
 @param port_handle Port handle
 @param num_entries Total number of ports to get
 @param counter_ids Counter ID, array should be equal to num_entries
 @param counters Return array of counters matching the counter_ids
*/
switch_status_t switch_api_port_stats_get(switch_device_t device,
                                          switch_handle_t port_handle,
                                          switch_uint16_t num_entries,
                                          switch_port_counter_id_t *counter_ids,
                                          switch_uint64_t *counters);

/**
 Clear port statistics for given counter_ids
 @param device device to use
 @param port_handle Port handle
 @param num_entries Total number of ports to get
 @param counter_ids Counter ID, array should be equal to num_entries
*/
switch_status_t switch_api_port_stats_counter_id_clear(
    const switch_device_t device,
    const switch_handle_t port_handle,
    const switch_uint16_t num_counters,
    const switch_port_counter_id_t *counter_ids);

switch_status_t switch_api_port_stats_clear(switch_device_t device,
                                            switch_handle_t port_handle);

switch_status_t switch_api_port_all_stats_clear(switch_device_t device);

/**
  Set ACL group handle
  The port label is derived from the ACL group handle
  @param device – device
  @param port_handle – port handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_port_ingress_acl_group_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t acl_group);

/**
  Get ACL group handle
  The port label is derived from the ACL group handle
  @param device – device
  @param port_handle – port handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_port_ingress_acl_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group);

/**
  Set custom port label
  This API has to be used to set label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param port_handle – port handle
  @param label – port label
*/
switch_status_t switch_api_port_ingress_acl_label_set(
    switch_device_t device, switch_handle_t port_handle, switch_uint16_t label);

/**
  Get custom port label
  This API can be used to get label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param port_handle – port handle
  @param label – port label
*/
switch_status_t switch_api_port_ingress_acl_label_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t *label);

/**
  Set ACL group handle
  The port label is derived from the ACL group handle
  @param device – device
  @param port_handle – port handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_port_egress_acl_group_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t acl_group);

/**
  Get ACL group handle
  The port label is derived from the ACL group handle
  @param device – device
  @param port_handle – port handle
  @param acl_group - ACL group handle
*/
switch_status_t switch_api_port_egress_acl_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group);

/**
  Set custom port label
  This API has to be used to set label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param port_handle – port handle
  @param label – port label
*/
switch_status_t switch_api_port_egress_acl_label_set(
    switch_device_t device, switch_handle_t port_handle, switch_uint16_t label);

/**
  Get custom port label
  This API can be used to get label when bind type is SWITCH_HANDLE_TYPE_NONE
  @param device – device
  @param port_handle – port handle
  @param label – port label
*/
switch_status_t switch_api_port_egress_acl_label_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t *label);

/**
  Set port bind mode
  @param device – device
  @param port_handle – port handle
  @param bind_mode - Bind mode from switch_port_bind_mode_t
*/
switch_status_t switch_api_port_bind_mode_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t bind_mode);

/**
  Get port bind mode
  @param device – device
  @param port_handle – port handle
  @param bind_mode - Bind mode from switch_port_bind_mode_t
*/
switch_status_t switch_api_port_bind_mode_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_bind_mode_t *bind_mode);

switch_status_t switch_api_port_handle_dump(const switch_device_t device,
                                            const switch_handle_t port_handle,
                                            const void *cli_ctx);

switch_status_t switch_api_port_stats_dump(const switch_device_t device,
                                           const switch_handle_t port_handle,
                                           const void *cli_ctx);

switch_status_t switch_api_port_stats_by_port_number_dump(
    const switch_device_t device,
    const switch_port_t port,
    const void *cli_ctx);

switch_status_t switch_api_port_info_by_port_number_dump(
    const switch_device_t device,
    const switch_port_t port,
    const void *cli_ctx);

/**
  Get total number of queues
  @param device – device
  @param port_handle – port handle
  @param max_queues - Total number of queues on the port
*/
switch_status_t switch_api_port_max_queues_get(switch_device_t device,
                                               switch_handle_t port_handle,
                                               switch_uint32_t *max_queues);

/**
 Port set link prirority flow control
 @param device device to use
 @param port port on device to set
 @param pfc cos bitmap
*/
switch_status_t switch_api_port_pfc_set(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t pfc_map);

/**
 Port get link prirority flow control
 @param device device to use
 @param port port on device to set
 @param pfc cos bitmap
*/
switch_status_t switch_api_port_pfc_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        switch_uint32_t *pfc_map);

/**
 Port get link flow control
 @param device device to use
 @param port port on device to set
 @param rx_pause_en RX flow control
 @param tx_pause_en TX flow control
*/
switch_status_t switch_api_port_link_pause_get(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool *rx_pause_en,
                                               bool *tx_pause_en);
/**
 Port set link flow control
 @param device device to use
 @param port port on device to set
 @param rx_pause and tx_pause
*/
switch_status_t switch_api_port_link_pause_set(switch_device_t device,
                                               switch_handle_t port_handle,
                                               bool rx_pause_en,
                                               bool tx_pause_en);

/**
 Port set FEC mode
 @param device device to use
 @param port_handle port on device to set
 @param FEC mode - NONE/FC/RS
*/
switch_status_t switch_api_port_fec_mode_set(switch_device_t device,
                                             switch_handle_t port_handle,
                                             switch_port_fec_mode_t fec_mode);
/**
 Port get FEC mode
 @param device device to use
 @param port_handle port on device to get
*/
switch_status_t switch_api_port_fec_mode_get(switch_device_t device,
                                             switch_handle_t port_handle,
                                             switch_port_fec_mode_t *fec_mode);

/**
 Port get ingress/egress qos handles
 @param device device to use
 @param port_handle port on device to get
*/
switch_status_t switch_api_port_qos_group_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_qos_handle,
    switch_handle_t *tc_queue_handle,
    switch_handle_t *tc_ppg_handle,
    switch_handle_t *egress_qos_handle);

/**
 Port set ingress mirror
 @param device device to use
 @param port_handle port on device to set
 @param mirror_handle mirror session handle
*/
switch_status_t switch_api_port_ingress_mirror_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle);

/**
 Port get ingress mirror
 @param device device to use
 @param port_handle port on device to get
*/
switch_status_t switch_api_port_ingress_mirror_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle);

/**
 Port set egress mirror
 @param device device to use
 @param port_handle port on device to set
 @param mirror_handle mirror session handle
*/
switch_status_t switch_api_port_egress_mirror_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t mirror_handle);

/**
 Port get egress mirror
 @param device device to use
 @param port_handle port on device to get
*/
switch_status_t switch_api_port_egress_mirror_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *mirror_handle);

switch_status_t switch_api_port_ingress_sflow_handle_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *ingress_sflow_handle);

switch_status_t switch_api_port_egress_sflow_handle_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *egress_sflow_handle);

/**
 Port get max PPGs
 @param device device to use
 @param port_handle port on device to get
*/
switch_status_t switch_api_port_max_ppg_get(switch_device_t device,
                                            switch_handle_t port_handle,
                                            switch_uint8_t *num_ppgs);

/**
 Get port priority groups
 @param device device
 @param port_handle port handle
 @param num_ppgs number of ppgs
 @param ppg_handles list of ppg handles
 */
switch_status_t switch_api_port_ppg_get(switch_device_t device,
                                        switch_handle_t port_handle,
                                        uint8_t *num_ppgs,
                                        switch_handle_t *ppg_handles);

/**
 Port set PFC priority to PPG
 @param device device to use
 @param port_handle port on device to set
 @param qos_map_handle QoS handle for PFC priority to PPG qos-map.
*/
switch_status_t switch_api_port_pfc_priority_to_ppg_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle);

switch_status_t switch_api_port_pfc_priority_to_ppg_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle);

/**
 Port set PFC priority to Queue
 @param device device to use
 @param port_handle port on device to set
 @param qos_map_handle QoS handle for PFC priority to Queue mapping.
*/
switch_status_t switch_api_port_pfc_priority_to_queue_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t qos_map_handle);
switch_status_t switch_api_port_pfc_priority_to_queue_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *qos_map_handle);
switch_status_t switch_api_port_qos_scheduler_group_handles_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *group_handles);
switch_status_t switch_api_port_queue_scheduler_group_handle_count_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint32_t *count);
/**
 Port set Queue scheduler profile
 @param device device to use
 @param port_handle port on device to set
 @param scheduler_handle Queue scheduler profile handle
*/
switch_status_t switch_api_port_scheduler_profile_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t scheduler_handle);
/**
 Port get Queue scheduler profile
 @param device device to use
 @param port_handle port on device to get
 @param scheduler_handle Queue scheduler profile handle
*/
switch_status_t switch_api_port_scheduler_profile_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *scheduler_handle);

/**
 Port get list of lanes
 @param device device to use
 @param port_handle port on device to get
 @param lane_list list of lanes
*/
switch_status_t switch_api_port_lane_list_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_port_lane_list_t *lane_list);

/**
 Port get PPG drop packets
 @param device device to use
 @param ppg_handle port PPG handle
 @param num_packets drop packets
*/
switch_status_t switch_api_ppg_drop_get(switch_device_t device,
                                        switch_handle_t ppg_handle,
                                        uint64_t *num_packets);
/**
 Port get total PPG drop packets
 @param device device to use
 @param port_handle port handle
 @param drop_count total PPG drop packets
*/
switch_status_t switch_api_port_ppg_drop_get(const switch_device_t device,
                                             const switch_handle_t port_handle,
                                             uint64_t *drop_count);

/**
 Port get total Queue drop packets
 @param device device to use
 @param port_handle port handle
 @param drop_count total Queue drop packets
*/
switch_status_t switch_api_port_queue_drop_get(
    const switch_device_t device,
    const switch_handle_t port_handle,
    uint64_t *drop_count);

switch_status_t switch_api_interface_port_stats_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_uint16_t num_entries,
    switch_interface_counter_id_t *counter_id,
    switch_counter_t *counters);

/** @} */  // end of Port
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_PORT_H__ */
