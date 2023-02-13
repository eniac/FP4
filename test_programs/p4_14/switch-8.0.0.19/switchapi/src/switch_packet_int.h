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

#ifndef __SWITCH_PACKET_INT_H__
#define __SWITCH_PACKET_INT_H__

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"
#include "switch_device_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef SWITCH_MAX_DEVICE
#define SWITCH_MAX_DEVICE 256
#endif

#define SWITCH_PACKET_MAX_BUFFER_SIZE 10000

#define SWITCH_PACKET_HEADER_OFFSET 2 * SWITCH_MAC_LENGTH

#define SWITCH_FABRIC_HEADER_ETHTYPE 0x9000

#define ETHERTYPE_BF_PKTGEN 0x9001

#define SWITCH_PIPE_STRING "P4"

#define SWITCH_FD_INVALID -1

#define SWITCH_CPU_ETH_INTF_DEFAULT "veth251"

#define SWITCH_CPU_ETH_INTF_DEFAULT_LEN strlen(SWITCH_CPU_ETH_INTF_DEFAULT)

#define SWITCH_PACKET_TX_HASH_TABLE_SIZE 1024

#define SWITCH_PACKET_RX_HASH_TABLE_SIZE 1024

#define SWITCH_PACKET_TX_HASH_KEY_SIZE sizeof(switch_packet_tx_hash_entry_t)

#define SWITCH_PACKET_RX_HASH_KEY_SIZE sizeof(switch_packet_rx_hash_entry_t)

#define SWITCH_ETHERTYPE_QINQ 0x9100

#define SWITCH_ETHERTYPE_DOT1Q 0x8100

#define SWITCH_ETHERTYPE_PTP 0x88F7

#define SWITCH_ETHERTYPE_ARP 0x806

#define SWITCH_PKTDRIVER_RX_FILTER_SIZE 4096

#define SWITCH_PKTDRIVER_TX_FILTER_SIZE 4096

#define SWITCH_ETH_HEADER_SIZE sizeof(switch_ethernet_header_t)

#define SWITCH_VLAN_HEADER_SIZE sizeof(switch_vlan_header_t)

#define SWITCH_PIPE_STRING_LENGTH strlen(SWITCH_PIPE_STRING)

#define SWITCH_PKTDRIVER_TX_EGRESS_QUEUE_DEFAULT 7

#define switch_pktdriver_pipe_dummy_write(_fd)                                \
  do {                                                                        \
    switch_int32_t rc = 0;                                                    \
    rc = switch_fd_write(_fd, SWITCH_PIPE_STRING, SWITCH_PIPE_STRING_LENGTH); \
    SWITCH_ASSERT(rc == SWITCH_PIPE_STRING_LENGTH);                           \
  } while (0);

#define switch_pktdriver_pipe_dummy_read(_fd)                     \
  do {                                                            \
    switch_int32_t rc = 0;                                        \
    switch_int8_t buf[SWITCH_PIPE_STRING_LENGTH];                 \
    rc = switch_fd_read(_fd, buf, SWITCH_PIPE_STRING_LENGTH);     \
    SWITCH_ASSERT(rc == SWITCH_PIPE_STRING_LENGTH);               \
    SWITCH_ASSERT(SWITCH_MEMCMP(buf,                              \
                                SWITCH_PIPE_STRING,               \
                                SWITCH_PIPE_STRING_LENGTH) == 0); \
  } while (0);

#define SWITCH_PACKET_HEADER_NTOH(_pkt_header)                \
  do {                                                        \
    _pkt_header.fabric_header.ether_type =                    \
        switch_ntohs(_pkt_header.fabric_header.ether_type);   \
    _pkt_header.cpu_header.reason_code =                      \
        switch_ntohs(_pkt_header.cpu_header.reason_code);     \
    _pkt_header.cpu_header.ingress_port =                     \
        switch_ntohs(_pkt_header.cpu_header.ingress_port);    \
    _pkt_header.cpu_header.ingress_ifindex =                  \
        switch_ntohs(_pkt_header.cpu_header.ingress_ifindex); \
    _pkt_header.cpu_header.ingress_bd =                       \
        switch_ntohs(_pkt_header.cpu_header.ingress_bd);      \
  } while (0);

#define SWITCH_PACKET_HEADER_HTON(_pkt_header)                     \
  do {                                                             \
    _pkt_header.fabric_header.ether_type =                         \
        switch_htons(_pkt_header.fabric_header.ether_type);        \
    _pkt_header.cpu_header.reason_code =                           \
        switch_htons(_pkt_header.cpu_header.reason_code);          \
    _pkt_header.cpu_header.ingress_port =                          \
        switch_htons(_pkt_header.cpu_header.ingress_port);         \
    _pkt_header.cpu_header.ingress_ifindex =                       \
        switch_htons(_pkt_header.cpu_header.ingress_ifindex);      \
    _pkt_header.cpu_header.ingress_bd =                            \
        switch_htons(_pkt_header.cpu_header.ingress_bd);           \
    _pkt_header.fabric_header.dst_port_or_group =                  \
        switch_htons(_pkt_header.fabric_header.dst_port_or_group); \
  } while (0);

#define switch_pktdriver_rx_filter_handle_create(_device)      \
  switch_handle_create(_device,                                \
                       SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER, \
                       sizeof(switch_pktdriver_rx_filter_info_t))

#define switch_pktdriver_rx_filter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER, _handle)

#define switch_pktdriver_rx_filter_get(_device, _handle, _info) \
  switch_handle_get(_device,                                    \
                    SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER,     \
                    _handle,                                    \
                    (void **)_info)

#define switch_pktdriver_tx_filter_handle_create(_device)      \
  switch_handle_create(_device,                                \
                       SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER, \
                       sizeof(switch_pktdriver_tx_filter_info_t))

#define switch_pktdriver_tx_filter_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER, _handle)

#define switch_pktdriver_tx_filter_get(_device, _handle, _info) \
  switch_handle_get(_device,                                    \
                    SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER,     \
                    _handle,                                    \
                    (void **)_info)

typedef enum switch_fabric_header_type_s {
  SWITCH_FABRIC_HEADER_TYPE_NONE = 0,
  SWITCH_FABRIC_HEADER_TYPE_UNICAST = 1,
  SWITCH_FABRIC_HEADER_TYPE_MULTICAST = 2,
  SWITCH_FABRIC_HEADER_TYPE_MIRROR = 3,
  SWITCH_FABRIC_HEADER_TYPE_CONTROL = 4,
  SWITCH_FABRIC_HEADER_TYPE_CPU = 5
} switch_fabric_header_type_t;

typedef enum switch_pktdriver_packet_type_s {
  SWITCH_PKTDRIVER_PACKET_TYPE_TX_CB = 1,
  SWITCH_PKTDRIVER_PACKET_TYPE_TX_NETDEV = 2,
  SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_ETH = 3,
  SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_PCIE = 4,
  SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_KNET = 5,
} switch_pktdriver_packet_type_t;

static inline char *switch_pktdriver_pkttype_to_string(
    switch_pktdriver_packet_type_t pkt_type) {
  switch (pkt_type) {
    case SWITCH_PKTDRIVER_PACKET_TYPE_TX_CB:
      return "tx callback";
    case SWITCH_PKTDRIVER_PACKET_TYPE_TX_NETDEV:
      return "tx netdev";
    case SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_ETH:
      return "rx ethernet";
    case SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_PCIE:
      return "rx pcie";
    case SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_KNET:
      return "rx knet";
    default:
      return "unknown";
  }
}

typedef enum switch_pktdriver_channel_type_s {
  SWITCH_PKTDRIVER_CHANNEL_TYPE_CB = 1,
  SWITCH_PKTDRIVER_CHANNEL_TYPE_NETDEV = 2,
  SWITCH_PKTDRIVER_CHANNEL_TYPE_FD = 3,
} switch_pktdriver_channel_type_t;

typedef enum switch_pktdriver_rx_filter_attr_s {
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT = (1 << 0),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX = (1 << 1),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD = (1 << 2),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE = (1 << 3),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_ETHER_TYPE = (1 << 4)
} switch_pktdriver_rx_filter_attr_t;

typedef enum switch_pktdriver_tx_filter_attr_s {
  SWITCH_PKTDRIVER_TX_FILTER_ATTR_HOSTIF_FD = (1 << 0),
  SWITCH_PKTDRIVER_TX_FILTER_ATTR_VLAN_ID = (1 << 1)
} switch_pktdriver_tx_filter_attr_t;

typedef enum switch_pktdriver_vlan_action_s {
  SWITCH_PACKET_VLAN_ACTION_NONE = 0x0,
  SWITCH_PACKET_VLAN_ACTION_ADD = 0x1,
  SWITCH_PACKET_VLAN_ACTION_REMOVE = 0x2,
} switch_pktdriver_vlan_action_t;

typedef enum switch_pktdriver_rx_filter_priority_s {
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PORT = 0x1,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_INTERFACE = 0x2,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_VLAN = 0x3,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_LN = 0x4,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_RIF = 0x5,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO1 = 0x10,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO2 = 0x11,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO3 = 0x12,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO4 = 0x13,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MAX = 0x14
} switch_pktdriver_rx_filter_priority_t;

typedef enum switch_pktdriver_tx_filter_priority_s {
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_HOSTIF = 0x1,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO1 = 0x4,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO2 = 0x5,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO3 = 0x6,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO4 = 0x7,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MAX = 0x8
} switch_pktdriver_tx_filter_priority_t;

static inline char *switch_pktdriver_vlan_action_to_string(
    switch_pktdriver_vlan_action_t vlan_action) {
  switch (vlan_action) {
    case SWITCH_PACKET_VLAN_ACTION_NONE:
      return "none";
    case SWITCH_PACKET_VLAN_ACTION_ADD:
      return "add";
    case SWITCH_PACKET_VLAN_ACTION_REMOVE:
      return "remove";
    default:
      return "none";
  }
}

/** ethernet header */
typedef struct PACKED switch_ethernet_header_s {
  /** destination mac */
  uint8_t dst_mac[ETH_LEN];

  /** source mac */
  uint8_t src_mac[ETH_LEN];

  /** ethernet type */
  uint16_t ether_type;

} switch_ethernet_header_t;

/** vlan header */
typedef struct PACKED switch_vlan_header_s {
#if defined(__LITTLE_ENDIAN_BITFIELD)
  /** vlan id */
  uint16_t vid : 12;

  /** format indicator */
  uint16_t dei : 1;

  /** priority */
  uint16_t pcp : 3;
#elif defined(__BIG_ENDIAN_BITFIELD)
  /** priority */
  uint16_t pcp : 3;

  /** format indicator */
  uint16_t dei : 1;

  /** vlan id */
  uint16_t vid : 12;
#else
#error "Please fix <asm/byteorder.h>"
#endif
  /** vlan protocol id */
  uint16_t tpid;

} switch_vlan_header_t;

typedef struct switch_pktdriver_rx_filter_key_s {
  /** port number */
  switch_dev_port_t dev_port;

  /** ifindex */
  switch_ifindex_t ifindex;

  /** bridge domain */
  switch_bd_t bd;

  /** reason code */
  switch_hostif_reason_code_t reason_code;

  /** reason code mask */
  switch_uint32_t reason_code_mask;

} switch_pktdriver_rx_filter_key_t;

typedef struct switch_pktdriver_rx_filter_action_s {
  /** channel type */
  switch_pktdriver_channel_type_t channel_type;

  /** hostif fd */
  switch_fd_t fd;

  /** knet hostif handle - used with kernel packet processing */
  switch_knet_hostif_t knet_hostif_handle;

  /** vlan action */
  switch_pktdriver_vlan_action_t vlan_action;

} switch_pktdriver_rx_filter_action_t;

typedef struct switch_pktdriver_tx_filter_key_s {
  /** netdev fd */
  switch_fd_t hostif_fd;

  /** knet hostif handle - used with kernel packet processing */
  switch_knet_hostif_t knet_hostif_handle;

} switch_pktdriver_tx_filter_key_t;

typedef struct switch_pktdriver_tx_filter_action_s {
  /** tx bypass flags */
  switch_uint16_t bypass_flags;

  /** bd if tx bypass is false */
  switch_bd_t bd;

  /** dev port if tx bypass is true */
  switch_dev_port_t dev_port;

} switch_pktdriver_tx_filter_action_t;

typedef struct switch_pktdriver_rx_filter_info_s {
  /** rx filter key */
  switch_pktdriver_rx_filter_key_t rx_key;

  /** rx filter action */
  switch_pktdriver_rx_filter_action_t rx_action;

  /** rx filter priority */
  switch_pktdriver_rx_filter_priority_t priority;

  /** number of packets */
  switch_uint64_t num_packets;

  /** rx filter flags */
  switch_uint64_t flags;

  /** list node */
  switch_node_t node;

} switch_pktdriver_rx_filter_info_t;

typedef struct switch_pktdriver_tx_filter_info_s {
  /** tx filter key */
  switch_pktdriver_tx_filter_key_t tx_key;

  /** tx filter action */
  switch_pktdriver_tx_filter_action_t tx_action;

  /** tx filter priority */
  switch_pktdriver_tx_filter_priority_t priority;

  /** number of packets */
  switch_uint64_t num_packets;

  /** tx filter flags */
  switch_uint64_t flags;

  /** list node */
  switch_node_t node;

} switch_pktdriver_tx_filter_info_t;

/** fabric header */
typedef struct PACKED switch_fabric_header_s {
  /** fabric header ethertype */
  uint16_t ether_type;

#if defined(__LITTLE_ENDIAN_BITFIELD)
  /** padding */
  uint8_t pad1 : 1;

  /** packet version */
  uint8_t packet_version : 2;

  /** header version */
  uint8_t header_version : 2;

  /** header type - cpu/unicast/multicast */
  uint8_t packet_type : 3;

  /** packet color */
  uint8_t fabric_color : 3;

  /** qos value */
  uint8_t fabric_qos : 5;
#elif defined(__BIG_ENDIAN_BITFIELD)
  /** header type - cpu/unicast/multicast */
  uint8_t packet_type : 3;

  /** header version */
  uint8_t header_version : 2;

  /** packet version */
  uint8_t packet_version : 2;

  /** padding */
  uint8_t pad1 : 1;

  /** qos value */
  uint8_t fabric_qos : 5;

  /** packet color */
  uint8_t fabric_color : 3;
#else
#error "Please fix <asm/byteorder.h>"
#endif

  /** device id */
  uint8_t dst_device;

  /** dev port or mgid */
  uint16_t dst_port_or_group;

} switch_fabric_header_t;

/** cpu header */
typedef struct PACKED switch_cpu_header_s {
#if defined(__LITTLE_ENDIAN_BITFIELD)
  /** reserved */
  uint8_t reserved : 1;

  /** capture departure time */
  uint8_t capture_tstamp_on_tx : 1;

  /** tx bypass */
  uint8_t tx_bypass : 1;

  /** egress queue id */
  uint8_t egress_queue : 5;
#elif defined(__BIG_ENDIAN_BITFIELD)
  /** egress queue id */
  uint8_t egress_queue : 5;

  /** tx bypass */
  uint8_t tx_bypass : 1;

  /** capture departure time */
  uint8_t capture_tstamp_on_tx : 1;

  /** reserved */
  uint8_t reserved : 1;
#else
#error "Please fix <asm/byteorder.h>"
#endif

  /** ingress port */
  uint16_t ingress_port;

  /** ingress ifindex */
  uint16_t ingress_ifindex;

  /** ingress bridge domain */
  uint16_t ingress_bd;

  /**
   * rx - reason code
   * tx - tx bypass flags
   */
  uint16_t reason_code;

} switch_cpu_header_t;

/** cpu timestamp header */
typedef struct PACKED switch_cpu_timestamp_header_s {
  /** Arrival Time */
  switch_uint32_t arrival_time;

} switch_cpu_timestamp_header_t;

typedef struct PACKED switch_packet_header_s {
  /** fabric header */
  switch_fabric_header_t fabric_header;

  /** cpu header */
  switch_cpu_header_t cpu_header;

} switch_packet_header_t;

#define SWITCH_PKTINFO_TX_DEV_PORT(_pkt_info) \
  _pkt_info->pkt_header.fabric_header.dst_port_or_group

#define SWITCH_PKTINFO_RX_DEV_PORT(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.ingress_port

#define SWITCH_PKTINFO_REASON_CODE(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.reason_code

#define SWITCH_PKTINFO_BYPASS_FLAGS(_pkt_info) \
  SWITCH_PKTINFO_REASON_CODE(_pkt_info)

#define SWITCH_PKTINFO_TX_BYPASS(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.tx_bypass

#define SWITCH_PKTINFO_PACKET_TYPE(_pkt_info) \
  _pkt_info->pkt_header.fabric_header.packet_type

#define SWITCH_PKTINFO_ETHER_TYPE(_pkt_info) \
  _pkt_info->pkt_header.fabric_header.ether_type

#define SWITCH_PKTINFO_INGRESS_IFINDEX(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.ingress_ifindex

#define SWITCH_PKTINFO_INGRESS_BD(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.ingress_bd

#define SWITCH_PKTINFO_TX_EGRESS_QUEUE(_pkt_info) \
  _pkt_info->pkt_header.cpu_header.egress_queue

typedef struct switch_packet_info_s {
  /** device id */
  switch_device_t device;

  /** packet type */
  switch_pktdriver_packet_type_t pkt_type;

  /** packet header tx/rx */
  switch_packet_header_t pkt_header;

  /** hostif fd */
  switch_fd_t fd;

  /** packet */
  switch_int8_t *pkt;

  /** packet size */
  switch_int32_t pkt_size;

} switch_packet_info_t;

typedef struct switch_kern_info_s {
  switch_knet_cpuif_t knet_cpuif_id;

  /* bf_knet interface for kernel Tx and RX */
  char cpuif_knetdev_name[IFNAMSIZ];

  struct sockaddr_ll s_addr;

  switch_fd_t sock_fd;
} switch_knet_info_t;

/** packet driver context */
typedef struct switch_pktdriver_context_s {
  /** Is kernel packet processing enabled */
  bool knet_pkt_driver[SWITCH_MAX_DEVICE];

  /* Stores socket information when device uses bf_knet */
  switch_knet_info_t switch_kern_info[SWITCH_MAX_DEVICE];

  /** cpu interface name */
  switch_int8_t intf_name[SWITCH_HOSTIF_NAME_SIZE];

  /** cpu ifindex */
  switch_ifindex_t cpu_ifindex;

  /** cpu fd */
  switch_fd_t cpu_fd;

  /** dummy pipe fd */
  switch_fd_t pipe_fd[2];

  switch_array_t fd_array;

  /** list of tx filters */
  switch_list_t tx_filter;

  /** list of rx filters */
  switch_list_t rx_filter;

  /** total rx packets */
  switch_uint64_t num_rx_packets;

  /** total tx packets */
  switch_uint64_t num_tx_packets;

  /** total tx netdev packets */
  switch_uint64_t num_tx_netdev_packets;

  /** total rx netdev packets */
  switch_uint64_t num_rx_netdev_packets;

  /** total tx callback packets */
  switch_uint64_t num_tx_cb_packets;

  /** total rx callback packets */
  switch_uint64_t num_rx_cb_packets;

  /** rx counters by reason code */
  switch_counter_t rx_rc_counters[SWITCH_HOSTIF_REASON_CODE_MAX];

  /** rx counters by ingress port */
  switch_counter_t rx_port_counters[SWITCH_MAX_PORTS];

  /** rx packet trace enable */
  bool rx_pkt_trace_enable;

  /** tx packet trace enable */
  bool tx_pkt_trace_enable;

  /** vlan to bd mapping table */
  switch_bd_t bd_mapping[SWITCH_MAX_VLANS];

} switch_pktdriver_context_t;

switch_status_t switch_pktdriver_init(void);

switch_status_t switch_pktdriver_free(void);

switch_status_t switch_pktdriver_fd_add(const switch_device_t device,
                                        const switch_fd_t fd);

switch_status_t switch_pktdriver_fd_delete(const switch_device_t device,
                                           const switch_fd_t fd);

switch_status_t switch_pktdriver_rx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle);

switch_status_t switch_pktdriver_rx_filter_delete(
    const switch_device_t device, const switch_handle_t rx_filter_handle);

switch_status_t switch_pktdriver_tx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle);

switch_status_t switch_pktdriver_tx_filter_delete(
    const switch_device_t device, const switch_handle_t tx_filter_handle);

switch_status_t switch_pktdriver_tx(switch_packet_info_t *pkt_info);

switch_status_t switch_pktdriver_rx(switch_packet_info_t *pkt_info);

switch_status_t switch_pktdriver_rx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t filter_handle,
    const void *cli_ctx);

switch_status_t switch_pktdriver_tx_filter_handle_dump(
    const switch_device_t device,
    const switch_handle_t filter_handle,
    const void *cli_ctx);

switch_status_t switch_pktdriver_rx_rc_counters_dump(
    const switch_device_t device, const void *cli_ctx);

switch_status_t switch_pktdriver_rx_port_counters_dump(
    const switch_device_t device, const void *cli_ctx);

switch_status_t switch_pktdriver_rx_total_counters_dump(
    const switch_device_t device, const void *cli_ctx);

switch_status_t switch_pktdriver_rx_tx_debug_enable(
    const switch_device_t device,
    const bool rx,
    const bool enable,
    const void *cli_ctx);

switch_status_t switch_pktdriver_bd_to_vlan_mapping_add(switch_device_t device,
                                                        switch_bd_t bd,
                                                        switch_vlan_t vlan);

switch_status_t switch_pktdriver_bd_to_vlan_mapping_delete(
    switch_device_t device, switch_bd_t bd, switch_vlan_t vlan);

switch_status_t switch_pktdriver_bd_mapping_dump(const switch_device_t device,
                                                 const void *cli_ctx);

bool switch_pktdriver_mode_is_kernel(const switch_device_t device);

switch_status_t switch_pktdriver_knet_device_add(const switch_device_t device);

switch_status_t switch_pktdriver_knet_device_delete(
    const switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_PACKET_INT_H__ */
