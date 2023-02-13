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

#ifndef __SWITCH_PORT_INT_H__
#define __SWITCH_PORT_INT_H__

#define SWITCH_YID_MAX 288

#define SWITCH_YID_INVALID 0x1FF

#define SWITCH_CPU_PORT_DEFAULT 64

#define SWITCH_PORT_INVALID -1

#define SWITCH_PORT_STATE_MAX SWITCH_PORT_STATE_DOWN + 1

#define SWITCH_INVALID_PORT_ID 0xFFFF

#define SWITCH_PPG_HANDLE_SIZE 4096

#define SWITCH_MAX_PPG_10G 1
#define SWITCH_MAX_PPG_100G 2
#define SWITCH_MAX_PPG 2

#define SWITCH_INVALID_HW_PORT 0x1FF

#define SWITCH_PORT_EVENT_REGISTRATION_MAX 32

#define SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX 32

#define SWITCH_PORT_RX_MTU_DEFAULT 1600

#define SWITCH_PORT_TX_MTU_DEFAULT 1600

#define ECMP_MEMBER_FAST_DEACTIVATION

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_port_type_ {
  SWITCH_PORT_TYPE_NORMAL = 0,
  SWITCH_PORT_TYPE_FABRIC = 1,
  SWITCH_PORT_TYPE_CPU = 2,
  SWITCH_PORT_TYPE_RECIRC = 3
} switch_port_type_t;

typedef enum switch_port_queues_s {
  SWITCH_PORT_NUM_QUEUES_8 = 8,
  SWITCH_PORT_NUM_QUEUES_16 = 16,
  SWITCH_PORT_NUM_QUEUES_32 = 32,
  SWITCH_PORT_MAX_QUEUES = 32,
} switch_port_queue_t;

#define SWITCH_PORT_NHOP_REF_LIST(info) info->PJLarr_nexthops

#define SWITCH_PORT_MALLOC(_d, _n, _p)                  \
  do {                                                  \
    switch_size_t _p_size = sizeof(switch_port_info_t); \
    _p = SWITCH_MALLOC(_d, _p_size, _n);                \
    if (_p) {                                           \
      SWITCH_MEMSET(_port, 0x0, _p_size);               \
    }                                                   \
  } while (0);

#define SWITCH_PORT_FREE(_d, _p) SWITCH_FREE(_d, _p)

/** port handle wrappers */
#define switch_port_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_PORT, sizeof(switch_port_info_t))

#define switch_port_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_PORT, _handle)

#define switch_port_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_PORT, _handle, (void **)_info)

/** port priority group handle wrappers */
#define switch_ppg_handle_create(_device)                 \
  switch_handle_create(_device,                           \
                       SWITCH_HANDLE_TYPE_PRIORITY_GROUP, \
                       sizeof(switch_port_priority_group_t))

#define switch_ppg_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_PRIORITY_GROUP, _handle)

#define switch_ppg_get(_device, _handle, _info) \
  switch_handle_get(                            \
      _device, SWITCH_HANDLE_TYPE_PRIORITY_GROUP, _handle, (void **)_info)

#define SWITCH_PORT_DEV_PORT_GET(_device, _port_handle, _dev_port, _status) \
  do {                                                                      \
    switch_port_info_t *_port_info = NULL;                                  \
    _status = SWITCH_STATUS_INVALID_HANDLE;                                 \
    _status = switch_port_get(_device, _port_handle, &_port_info);          \
    if (_port_info) {                                                       \
      _dev_port = _port_info->dev_port;                                     \
    }                                                                       \
  } while (0);

static inline char *switch_port_event_to_string(
    switch_port_event_t port_event) {
  switch (port_event) {
    case SWITCH_PORT_EVENT_ADD:
      return "port add";
    case SWITCH_PORT_EVENT_DELETE:
      return "port delete";
    default:
      return "port event unknown";
  }
}

static inline char *switch_port_flowcontrol_type_to_string(
    switch_flowcontrol_type_t fc_type) {
  switch (fc_type) {
    case SWITCH_FLOWCONTROL_TYPE_NONE:
      return "none";
    case SWITCH_FLOWCONTROL_TYPE_PFC:
      return "pfc";
    case SWITCH_FLOWCONTROL_TYPE_PAUSE:
      return "pause";
    default:
      return "none";
  }
}

static inline char *switch_port_counter_id_to_string(
    switch_port_counter_id_t counter_id) {
  switch (counter_id) {
    case SWITCH_PORT_STAT_IN_GOOD_OCTETS:
      return "IN GOOD OCTETS";
    case SWITCH_PORT_STAT_IN_ALL_OCTETS:
      return "IN_ALL OCTETS";
    case SWITCH_PORT_STAT_IN_GOOD_PKTS:
      return "IN GOOD PACKETS";
    case SWITCH_PORT_STAT_IN_ALL_PKTS:
      return "IN ALL  PACKETS";
    case SWITCH_PORT_STAT_IN_VLAN_PKTS:
      return "IN VLAN PACKETS";
    case SWITCH_PORT_STAT_IN_UCAST_PKTS:
      return "IN UCAST PACKETS";
    case SWITCH_PORT_STAT_IN_MCAST_PKTS:
      return "IN MCAST PACKETS";
    case SWITCH_PORT_STAT_IN_BCAST_PKTS:
      return "IN BCAST PACKETS";
    case SWITCH_PORT_STAT_IN_FCS_ERRORS:
      return "IN FCS ERRORS";
    case SWITCH_PORT_STAT_IN_ERROR_PKTS:
      return "IN ERROR PACKETS";
    case SWITCH_PORT_STAT_IN_CRC_ERRORS:
      return "IN CRC ERRORS";
    case SWITCH_PORT_STAT_IN_BUFFER_FULL:
      return "IN BUFFER FULL";
    case SWITCH_PORT_STAT_IN_FRAGMENTS:
      return "IN FRAGMENTS";
    case SWITCH_PORT_STAT_IN_JABBERS:
      return "IN JABBERS";
    case SWITCH_PORT_STAT_IN_OVER_SIZED_PKTS:
      return "IN OVER SIZED PKTS";
    case SWITCH_PORT_STAT_IN_UNDER_SIZED_PKTS:
      return "IN UNDER SIZED PKTS";
    case SWITCH_PORT_STAT_OUT_GOOD_OCTETS:
      return "OUT GOOD OCTETS";
    case SWITCH_PORT_STAT_OUT_ALL_OCTETS:
      return "OUT ALL OCTETS";
    case SWITCH_PORT_STAT_OUT_GOOD_PKTS:
      return "OUT GOOD PACKETS";
    case SWITCH_PORT_STAT_OUT_ALL_PKTS:
      return "OUT ALL PACKETS";
    case SWITCH_PORT_STAT_OUT_VLAN_PKTS:
      return "OUT VLAN PACKETS";
    case SWITCH_PORT_STAT_OUT_UCAST_PKTS:
      return "OUT UCAST PACKETS";
    case SWITCH_PORT_STAT_OUT_MCAST_PKTS:
      return "OUT MCAST PACKETS";
    case SWITCH_PORT_STAT_OUT_BCAST_PKTS:
      return "OUT BCAST PACKETS";
    case SWITCH_PORT_STAT_OUT_ERROR_PKTS:
      return "OUT ERROR PACKETS";
    case SWITCH_PORT_STAT_IN_PKTS_LT_64:
      return "IN PKTS LT 64";
    case SWITCH_PORT_STAT_IN_PKTS_EQ_64:
      return "IN PKTS EQ 64";
    case SWITCH_PORT_STAT_IN_PKTS_65_TO_127:
      return "IN PKTS 65 to 127";
    case SWITCH_PORT_STAT_IN_PKTS_128_TO_255:
      return "IN PKTS 128 to 255";
    case SWITCH_PORT_STAT_IN_PKTS_256_TO_511:
      return "IN PKTS 256 to 511";
    case SWITCH_PORT_STAT_IN_PKTS_512_TO_1023:
      return "IN PKTS 512 to 1023";
    case SWITCH_PORT_STAT_IN_PKTS_1024_TO_1518:
      return "IN PKTS 1024 to 1518";
    case SWITCH_PORT_STAT_IN_PKTS_1519_TO_2047:
      return "IN PKTS 1519 to 2047";
    case SWITCH_PORT_STAT_IN_PKTS_2048_TO_4095:
      return "IN PKTS 2048 to 4095";
    case SWITCH_PORT_STAT_IN_PKTS_4096_TO_8191:
      return "IN PKTS 4096 to 8191";
    case SWITCH_PORT_STAT_IN_PKTS_8192_TO_9215:
      return "IN PKTS 8192 to 9215";
    case SWITCH_PORT_STAT_IN_PKTS_9216:
      return "IN PKTS GT 9216";
    case SWITCH_PORT_STAT_OUT_PKTS_LT_64:
      return "OUT PKTS LT 64";
    case SWITCH_PORT_STAT_OUT_PKTS_EQ_64:
      return "OUT PKTS EQ 64";
    case SWITCH_PORT_STAT_OUT_PKTS_65_TO_127:
      return "OUT PKTS 65 to 127";
    case SWITCH_PORT_STAT_OUT_PKTS_128_TO_255:
      return "OUT PKTS 128 to 255";
    case SWITCH_PORT_STAT_OUT_PKTS_256_TO_511:
      return "OUT PKTS 256 to 511";
    case SWITCH_PORT_STAT_OUT_PKTS_512_TO_1023:
      return "OUT PKTS 512 to 1023";
    case SWITCH_PORT_STAT_OUT_PKTS_1024_TO_1518:
      return "OUT PKTS 1024 to 1518";
    case SWITCH_PORT_STAT_OUT_PKTS_1519_TO_2047:
      return "OUT PKTS 1519 to 2047";
    case SWITCH_PORT_STAT_OUT_PKTS_2048_TO_4095:
      return "OUT PKTS 2048 to 4095";
    case SWITCH_PORT_STAT_OUT_PKTS_4096_TO_8191:
      return "OUT PKTS 4096 to 8191";
    case SWITCH_PORT_STAT_OUT_PKTS_8192_TO_9215:
      return "OUT PKTS 8192 to 9215";
    case SWITCH_PORT_STAT_OUT_PKTS_9216:
      return "OUT PKTS GT 9216";
    case SWITCH_PORT_STAT_IN_PFC_0_PKTS:
      return "IN PFC 0 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_1_PKTS:
      return "IN PFC 1 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_2_PKTS:
      return "IN PFC 2 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_3_PKTS:
      return "IN PFC 3 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_4_PKTS:
      return "IN PFC 4TS";
    case SWITCH_PORT_STAT_IN_PFC_5_PKTS:
      return "IN PFC 5 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_6_PKTS:
      return "IN PFC 6 PKTS";
    case SWITCH_PORT_STAT_IN_PFC_7_PKTS:
      return "IN PFC 7 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_0_PKTS:
      return "OUT PFC 0 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_1_PKTS:
      return "OUT PFC 1 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_2_PKTS:
      return "OUT PFC 2 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_3_PKTS:
      return "OUT PFC 3 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_4_PKTS:
      return "OUT PFC 4 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_5_PKTS:
      return "OUT PFC 5 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_6_PKTS:
      return "OUT PFC 6 PKTS";
    case SWITCH_PORT_STAT_OUT_PFC_7_PKTS:
      return "OUT PFC 7 PKTS";
    case SWITCH_PORT_STAT_IN_FRAMES_TOO_LONG:
      return "FRAMES TOO LONG";
    default:
      return "unkown";
  }
}

static inline char *switch_port_speed_to_string(
    switch_port_speed_t port_speed) {
  switch (port_speed) {
    case SWITCH_PORT_SPEED_10G:
      return "10G";
    case SWITCH_PORT_SPEED_25G:
      return "25G";
    case SWITCH_PORT_SPEED_40G:
      return "40G";
    case SWITCH_PORT_SPEED_50G:
      return "50G";
    case SWITCH_PORT_SPEED_100G:
      return "100G";
    case SWITCH_PORT_SPEED_NONE:
      return "none";
    default:
      return "unknown";
  }
}

static inline char *switch_port_type_to_string(switch_port_type_t port_type) {
  switch (port_type) {
    case SWITCH_PORT_TYPE_NORMAL:
      return "NORMAL";
    case SWITCH_PORT_TYPE_FABRIC:
      return "FABRIC";
    case SWITCH_PORT_TYPE_CPU:
      return "CPU";
    case SWITCH_PORT_TYPE_RECIRC:
      return "RECIRC";
    default:
      return "unknown";
  }
}

static inline char *switch_port_oper_status_to_string(
    switch_port_oper_status_t oper_status) {
  switch (oper_status) {
    case SWITCH_PORT_OPER_STATUS_UNKNOWN:
      return "UNKNOWN";
    case SWITCH_PORT_OPER_STATUS_UP:
      return "UP";
    case SWITCH_PORT_OPER_STATUS_DOWN:
      return "DOWN";
    default:
      return "NONE";
  }
}

static inline char *switch_port_auto_neg_mode_to_string(
    switch_port_auto_neg_mode_t an_mode) {
  switch (an_mode) {
    case SWITCH_PORT_AUTO_NEG_MODE_DEFAULT:
      return "default";
    case SWITCH_PORT_AUTO_NEG_MODE_ENABLE:
      return "enabled";
    case SWITCH_PORT_AUTO_NEG_MODE_DISABLE:
      return "disabled";
    default:
      return "default";
  }
}

static inline char *switch_port_lb_mode_to_string(
    switch_port_loopback_mode_t lb_mode) {
  switch (lb_mode) {
    case SWITCH_PORT_LOOPBACK_MODE_NONE:
      return "none";
    case SWITCH_PORT_LOOPBACK_MODE_PHY_NEAR:
      return "phy near";
    case SWITCH_PORT_LOOPBACK_MODE_PHY_FAR:
      return "phy far";
    case SWITCH_PORT_LOOPBACK_MODE_MAC_NEAR:
      return "mac near";
    case SWITCH_PORT_LOOPBACK_MODE_MAC_FAR:
      return "mac far";
    default:
      return "none";
  }
}

typedef enum switch_port_num_lanes_s {
  SWITCH_PORT_NUM_LANES_1 = 1,
  SWITCH_PORT_NUM_LANES_2 = 2,
  SWITCH_PORT_NUM_LANES_4 = 4
} switch_port_num_lanes_t;

#define SWITCH_PORT_LANE_MAPPING(_fp_num, _port_speed, _lane_list, _status) \
  do {                                                                      \
    _status = SWITCH_STATUS_SUCCESS;                                        \
    _lane_list.num_lanes = 0;                                               \
    switch (_port_speed) {                                                  \
      case SWITCH_PORT_SPEED_10G:                                           \
      case SWITCH_PORT_SPEED_25G:                                           \
        _lane_list.num_lanes = SWITCH_PORT_NUM_LANES_1;                     \
        _lane_list.lane[0] = _fp_num;                                       \
        break;                                                              \
                                                                            \
      case SWITCH_PORT_SPEED_40G:                                           \
      case SWITCH_PORT_SPEED_100G:                                          \
        _lane_list.num_lanes = SWITCH_PORT_NUM_LANES_4;                     \
        _lane_list.lane[0] = _fp_num;                                       \
        _lane_list.lane[1] = _fp_num + 1;                                   \
        _lane_list.lane[2] = _fp_num + 2;                                   \
        _lane_list.lane[3] = _fp_num + 3;                                   \
        break;                                                              \
                                                                            \
      case SWITCH_PORT_SPEED_50G:                                           \
        _lane_list.num_lanes = SWITCH_PORT_NUM_LANES_2;                     \
        _lane_list.lane[0] = _fp_num;                                       \
        _lane_list.lane[1] = _fp_num + 1;                                   \
        break;                                                              \
                                                                            \
      default:                                                              \
        _status = SWITCH_STATUS_INVALID_PARAMETER;                          \
        break;                                                              \
    }                                                                       \
  } while (0);

#define SWITCH_PORT_VALID(_port)                                            \
  ((_port <= SWITCH_MAX_PORTS) || (_port == SWITCH_CPU_PORT_ETH_DEFAULT) || \
   (_port == SWITCH_CPU_PORT_PCIE_DEFAULT))

#define SWITCH_PORT_INTERNAL(_port) (_port == SWITCH_CPU_PORT_PCIE_DEFAULT)

typedef enum switch_port_pd_entry_s {
  SWITCH_PORT_INGRESS_PORT_MAPPING_ENTRY = (1 << 0),
  SWITCH_PORT_INGRESS_PORT_PROPERTIES_ENTRY = (1 << 1),
  SWITCH_PORT_EGRESS_PORT_MAPPING_ENTRY = (1 << 2),
  SWITCH_PORT_LAG_GROUP_ENTRY = (1 << 3),
  SWITCH_PORT_LAG_MEMBER_ENTRY = (1 << 4),
  SWITCH_PORT_SC_UCAST_ENTRY = (1 << 5),
  SWITCH_PORT_SC_MCAST_ENTRY = (1 << 6),
  SWITCH_PORT_SC_BCAST_ENTRY = (1 << 7),
  SWITCH_PORT_ENTRY_MAX
} switch_port_pd_entry_t;

/** port info identified by port handle */
typedef struct switch_port_info_s {
  switch_uint64_t flags;

  switch_uint64_t hw_flags;

  /** ingress port pruning index */
  switch_yid_t yid;

  /** port port_lag_index */
  switch_port_lag_index_t port_lag_index;

  /** list of interfaces created on port */
  switch_array_t intf_array;

  /** self pointer */
  switch_handle_t port_handle;

  /** port type - front panel/fabric/cpu */
  switch_port_type_t port_type;

  /** meter handle array */
  switch_handle_t meter_handle[SWITCH_PACKET_TYPE_MAX];

  /** lag handle - port can be part of only one lag */
  switch_handle_t lag_handle;

  /** ingress port lag label */
  switch_port_lag_label_t ingress_port_lag_label;

  /** egress port lag label */
  switch_port_lag_label_t egress_port_lag_label;

  bool trust_dscp;
  bool trust_pcp;
  switch_qos_group_t ingress_qos_group;
  switch_qos_group_t tc_qos_group;
  switch_qos_group_t egress_qos_group;
  uint16_t tc;
  switch_color_t color;
  switch_uint8_t max_ppg;
  bool learning_enabled;

  /**
   * list of port priority group handles.
   * used for ingress buffer
   */
  switch_handle_t *ppg_handles;

  /** number of queues allocated */
  switch_uint8_t num_queues;

  /** maximum queue per port */
  switch_uint8_t max_queues;

  /**
   * list of queue handles.
   * used for egress buffer
   */
  switch_handle_t *queue_handles;

  /** port number */
  switch_port_t port;

  /** dev port number */
  switch_dev_port_t dev_port;

  /** port speed */
  switch_port_speed_t port_speed;

  /** admin state */
  bool admin_state;

  /** Rx link pause */
  bool rx_pause;

  /** Tx link pause */
  bool tx_pause;

  /** port's PFC Rx and Tx cos bitmap */
  switch_uint32_t pfc_map;

  /** auto negotiation mode */
  switch_port_auto_neg_mode_t an_mode;

  /** operational status */
  switch_port_oper_status_t oper_status;

  /** port loopback mode */
  switch_port_loopback_mode_t lb_mode;

  /** hostif handle */
  switch_handle_t hostif_handle;

  /** port rx mtu - default 1500 */
  switch_uint32_t rx_mtu;

  /** port tx mtu - default 1500 */
  switch_uint32_t tx_mtu;

  /** List of nexthops (handles) pointing to this port */
  Pvoid_t PJLarr_nexthops;

  switch_port_fec_mode_t fec_mode;

  switch_pd_hdl_t ingress_mapping_hw_entry;
  switch_pd_hdl_t egress_mapping_hw_entry;
  switch_pd_hdl_t ingress_prop_hw_entry;
  switch_pd_hdl_t lg_entry;
  switch_pd_mbr_hdl_t mbr_hdl;
  switch_pd_hdl_t meter_pd_hdl[SWITCH_PACKET_TYPE_MAX];

  /** acl ingress group handle */
  switch_handle_t ingress_acl_group_handle;

  /** acl egress group handle */
  switch_handle_t egress_acl_group_handle;

  switch_port_bind_mode_t bind_mode;
  switch_handle_t ingress_sflow_handle;
  switch_handle_t egress_sflow_handle;
  switch_handle_t ingress_sflow_entry_handle;
  switch_handle_t egress_sflow_entry_handle;

  switch_handle_t ingress_qos_handle;
  switch_handle_t tc_queue_handle;
  switch_handle_t tc_ppg_handle;
  switch_handle_t egress_qos_handle;
  switch_handle_t ingress_pfc_ppg_handle;
  switch_handle_t egress_pfc_queue_handle;
  switch_handle_t port_scheduler_group_handle;
  switch_handle_t queue_scheduler_group_handles[SWITCH_MAX_QUEUE];
  switch_handle_t ingress_mirror_handle;
  switch_pd_hdl_t ingress_mirror_hw_entry;
  switch_handle_t egress_mirror_handle;
  switch_pd_hdl_t egress_mirror_hw_entry;
  switch_handle_t scheduler_handle;
  switch_port_lane_list_t lane_list;

} switch_port_info_t;

typedef struct switch_port_state_change_app_info_s {
  /** valid app info */
  bool valid;

  /** application id */
  switch_app_id_t app_id;

  /**
   * App data is set during callback registeration.
   * App data is sent during event notifications
   * to the registered application
   */
  void *app_data;

  /** Callback function for port state change notifications */
  switch_port_state_change_notification_fn cb_fn;

} switch_port_state_change_app_info_t;

typedef struct switch_port_event_app_info_s {
  /** valid app info */
  bool valid;

  /** application id */
  switch_app_id_t app_id;

  /**
   * App data is set during callback registeration.
   * App data is sent during event notifications
   * to the registered application
   */
  void *app_data;

  /** Callback function for port event notifications */
  switch_port_event_notification_fn cb_fn;

} switch_port_event_app_info_t;

/** port device context */
typedef struct switch_port_context_s {
  /** list of port handles created on this device */
  switch_handle_t port_handles[SWITCH_MAX_PORTS];

  /** port pruning index allocator */
  switch_id_allocator_t *yid_allocator;

  /** list of app callbacks for port event notifications */
  switch_port_event_app_info_t
      event_app_list[SWITCH_PORT_EVENT_REGISTRATION_MAX];

  /** list of app callbacks for port state change notifications */
  switch_port_state_change_app_info_t
      sc_app_list[SWITCH_PORT_STATE_CHANGE_REGISTRATION_MAX];

} switch_port_context_t;

typedef struct switch_port_priority_group_s {
  switch_node_t node;
  switch_handle_t ppg_handle;
  switch_handle_t port_handle;
  uint16_t priority;
  bool lossless_enabled;
  switch_handle_t buffer_profile_handle;
  switch_tm_ppg_hdl_t tm_ppg_handle;
  uint8_t ppg_index;
} switch_port_priority_group_t;

switch_status_t switch_port_init(switch_device_t device);

switch_status_t switch_port_free(switch_device_t device);

switch_status_t switch_port_default_entries_add(switch_device_t device);

switch_status_t switch_port_default_entries_delete(switch_device_t device);

switch_status_t switch_port_prune_mask_table_update(
    switch_device_t device, switch_port_info_t *port_info, bool prune);

bool switch_port_is_cpu_port(switch_device_t device,
                             switch_handle_t port_handle);

switch_status_t switch_yid_allocate(switch_device_t device, switch_yid_t *yid);
;

switch_status_t switch_yid_free(switch_device_t device, switch_yid_t yid);

switch_status_t switch_api_port_acl_group_ingress_set(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t acl_group);

switch_status_t switch_api_port_acl_group_ingress_get(
    switch_device_t device,
    switch_handle_t port_handle,
    switch_handle_t *acl_group);
#define SWITCH_ID_FROM_PORT_LAG_INDEX(port_lag_index) \
  (port_lag_index & ((1 << SWITCH_PORT_LAG_INDEX_WIDTH) - 1)

#define SWITCH_COMPUTE_PORT_LAG_INDEX(handle, port_lag_index_type) \
  (handle_to_id(handle) | (port_lag_index_type << SWITCH_PORT_LAG_INDEX_WIDTH))

#define SWITCH_PORT_LAG_INDEX_GET_TYPE(port_lag_index) \
  ((port_lag_index >> SWITCH_PORT_LAG_INDEX_WIDTH))

switch_status_t switch_cpu_port_add(switch_device_t device,
                                    switch_port_t port,
                                    switch_handle_t *port_handle);

switch_status_t switch_port_state_change(switch_device_t device,
                                         switch_handle_t port_handle,
                                         switch_port_oper_status_t oper_status,
                                         void *cookie);

switch_status_t switch_port_dev_port_to_handle_get(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_handle_t *port_handle);

switch_status_t switch_recirc_port_add(switch_device_t device,
                                       switch_port_t port,
                                       switch_handle_t *port_handle);

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_PORT_INT_H__ */
