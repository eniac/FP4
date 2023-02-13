
#ifndef _switch_types_int_h_
#define _switch_types_int_h_

#include <switchapi/switch_handle.h>
#include <switchapi/switch_base_types.h>
#include "switch_pd_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_PD_STATUS_SUCCESS 0
#define SWITCH_PD_OBJ_NOT_FOUND 6

#define SWITCH_PD_ASSERT SWITCH_ASSERT

typedef switch_uint16_t switch_bd_t;
typedef switch_uint16_t switch_nhop_t;
typedef switch_uint16_t switch_ecmp_t;
typedef switch_uint16_t switch_wcmp_t;
typedef switch_uint16_t switch_mgid_t;
typedef switch_uint16_t switch_tunnel_t;
typedef switch_uint16_t switch_if_label_t;
typedef switch_uint16_t switch_smac_id_t;
typedef switch_uint16_t switch_dmac_id_t;
typedef switch_uint16_t switch_stp_group_t;
typedef switch_uint16_t switch_urpf_group_t;
typedef switch_uint16_t switch_rmac_group_t;
typedef switch_uint16_t switch_meter_id_t;
typedef switch_uint16_t switch_stats_id_t;
typedef switch_uint16_t switch_counter_id_t;
typedef switch_uint16_t switch_vrf_id_t;
typedef switch_uint16_t switch_mirror_t;
typedef switch_int32_t switch_fd_t;
typedef switch_uint16_t switch_lag_t;
typedef switch_uint8_t switch_pipe_t;
typedef switch_int32_t switch_dev_port_t;
typedef switch_uint16_t switch_rpf_group_t;

typedef switch_id_t switch_yid_t;
typedef switch_id_t switch_xid_t;
typedef switch_id_t switch_rid_t;
typedef switch_id_t switch_qos_group_t;

typedef tommy_node switch_node_t;

typedef tommy_hashtable_node switch_hashnode_t;

#define switch_pipe pipe
#define switch_fcntl fcntl
#define switch_fd_read read
#define switch_fd_write write
#define switch_fd_close close
#define switch_fd_send sendto
#define switch_ntohs ntohs
#define switch_htons htons
#define switch_socket socket
#define switch_bind bind
#define switch_ioctl ioctl
#define switch_open open
#define switch_fd_set fd_set
#define switch_select select
#define switch_snprintf snprintf

typedef switch_status_t (*switch_key_func_t)(void *args,
                                             switch_uint8_t *key,
                                             switch_uint32_t *len);

typedef switch_int32_t (*switch_hash_compare_func_t)(const void *key1,
                                                     const void *key2);

typedef switch_int32_t (*switch_list_compare_func_t)(const void *key1,
                                                     const void *key2);

typedef struct switch_array_ {
  void *array;
  switch_size_t num_entries;
} switch_array_t;

typedef struct switch_hashtable_ {
  tommy_hashtable table;
  switch_hash_compare_func_t compare_func;
  switch_key_func_t key_func;
  switch_size_t num_entries;
  switch_size_t size;
  switch_size_t hash_seed;
} switch_hashtable_t;

typedef struct switch_list_ {
  tommy_list list;
  switch_size_t num_entries;
} switch_list_t;

typedef switch_uint16_t switch_pd_table_id_t;
typedef switch_uint16_t switch_pd_action_id_t;

#ifdef SWITCH_PD

typedef p4_pd_entry_hdl_t switch_pd_hdl_t;
typedef p4_pd_grp_hdl_t switch_pd_grp_hdl_t;
typedef p4_pd_mbr_hdl_t switch_pd_mbr_hdl_t;
typedef p4_pd_status_t switch_pd_status_t;
typedef p4_pd_sess_hdl_t switch_pd_sess_hdl_t;
typedef p4_pd_dev_target_t switch_pd_target_t;

#else

typedef switch_uint32_t switch_pd_hdl_t;
typedef switch_uint32_t switch_pd_grp_hdl_t;
typedef switch_uint32_t switch_pd_mbr_hdl_t;
typedef switch_uint32_t switch_pd_status_t;
typedef switch_uint32_t switch_pd_sess_hdl_t;

typedef struct switch_pd_target_s {
  switch_device_t device_id;
  switch_pipe_t dev_pipe_id;
} switch_pd_target_t;

#endif

typedef enum switch_pd_entry_type_s {
  SWITCH_PD_ENTRY_DEFAULT = 0,
  SWITCH_PD_ENTRY_INIT = 1,
  SWITCH_PD_ENTRY_ADD = 2,
  SWITCH_PD_ENTRY_UPDATE = 3,
  SWITCH_PD_ENTRY_DELETE = 4,
  SWITCH_PD_ENTRY_GET = 5
} switch_pd_entry_type_t;

static inline char *switch_pd_entry_type_to_string(
    switch_pd_entry_type_t entry_type) {
  switch (entry_type) {
    case SWITCH_PD_ENTRY_DEFAULT:
      return "DEFAULT";
    case SWITCH_PD_ENTRY_INIT:
      return "INIT";
    case SWITCH_PD_ENTRY_ADD:
      return "ADD";
    case SWITCH_PD_ENTRY_DELETE:
      return "DELETE";
    case SWITCH_PD_ENTRY_GET:
      return "GET";
    default:
      return "UNKNOWN";
  }
}

typedef struct switch_pd_dump_entry_ {
  switch_pd_entry_type_t entry_type;
  switch_uint16_t table_id;
  switch_uint16_t action_id;
  switch_uint8_t *match_spec;
  switch_uint8_t *action_spec;
  switch_uint16_t match_spec_size;
  switch_uint16_t action_spec_size;
  switch_pd_hdl_t pd_hdl;
  switch_pd_grp_hdl_t pd_grp_hdl;
  switch_pd_mbr_hdl_t pd_mbr_hdl;
} switch_pd_dump_entry_t;

/* Tunnels using mgid trees - related definitions */
typedef enum switch_tunnel_mgid_events_ {
  SWITCH_TUNNEL_CREATE = 0x1,
  SWITCH_TUNNEL_DELETE = 0x2,
  SWITCH_ROUTE_ADD = 0x3,
  SWITCH_ROUTE_REMOVE = 0x4,
  SWITCH_NHOP_MGID_TREE_DELETE = 0x5,
  SWITCH_NHOP_MGID_TREE_CREATE = 0x6,
  SWITCH_MGID_ADD = 0x7,
  SWITCH_MGID_REMOVE = 0x8,
} switch_tunnel_mgid_events_t;

typedef switch_status_t (*switch_mgid_state_t)(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

static inline char *switch_macaddress_to_string(const switch_mac_addr_t *mac) {
  static char mac_str[18];
  snprintf(mac_str,
           sizeof(mac_str),
           "%02x:%02x:%02x:%02x:%02x:%02x",
           mac->mac_addr[0],
           mac->mac_addr[1],
           mac->mac_addr[2],
           mac->mac_addr[3],
           mac->mac_addr[4],
           mac->mac_addr[5]);
  return mac_str;
}

static inline char *switch_ipaddress_to_string(
    const switch_ip_addr_t *ip_addr) {
  static char ipv4_str[INET_ADDRSTRLEN + 10];
  static char ipv6_str[INET6_ADDRSTRLEN + 10];
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    uint32_t v4addr = ip_addr->ip.v4addr;
    inet_ntop(AF_INET, &v4addr, ipv4_str, INET_ADDRSTRLEN);
    snprintf(ipv4_str + INET_ADDRSTRLEN, 10, "/%d", ip_addr->prefix_len);
    return ipv4_str;
  } else {
    inet_ntop(AF_INET6, &ip_addr->ip.v6addr, ipv6_str, INET6_ADDRSTRLEN);
    snprintf(ipv6_str + INET6_ADDRSTRLEN, 10, "/%d", ip_addr->prefix_len);
    return ipv6_str;
  }
}

typedef enum switch_hashtable_type_s {
  SWITCH_HASHTABLE_TYPE_MAC = 0x0,
  SWITCH_HASHTABLE_TYPE_ROUTE = 0x1,
  SWITCH_HASHTABLE_TYPE_NHOP = 0x2,
  SWITCH_HASHTABLE_TYPE_NEIGHBOR_DMAC = 0x3,
  SWITCH_HASHTABLE_TYPE_NEIGHBOR_TUNNEL_DMAC = 0x4,
  SWITCH_HASHTABLE_TYPE_HOSTIF = 0x5,
  SWITCH_HASHTABLE_TYPE_SMAC = 0x6,
  SWITCH_HASHTABLE_TYPE_NAT = 0x7,
  SWITCH_HASHTABLE_TYPE_TUNNEL_INGRESS_VNI = 0x8,
  SWITCH_HASHTABLE_TYPE_TUNNEL_EGRESS_VNI = 0x9,
  SWITCH_HASHTABLE_TYPE_TUNNEL_SRC_IP = 0xa,
  SWITCH_HASHTABLE_TYPE_TUNNEL_DST_IP = 0xb,
  SWITCH_HASHTABLE_TYPE_VLAN_PV = 0xc,
  SWITCH_HAHSTABLE_TYPE_MAX

} switch_hashtable_type_t;

static inline char *switch_hashtable_type_to_string(
    switch_hashtable_type_t type) {
  switch (type) {
    case SWITCH_HASHTABLE_TYPE_MAC:
      return "mac";
    case SWITCH_HASHTABLE_TYPE_ROUTE:
      return "route";
    case SWITCH_HASHTABLE_TYPE_NHOP:
      return "nhop";
    case SWITCH_HASHTABLE_TYPE_NEIGHBOR_DMAC:
      return "neighbor dmac";
    case SWITCH_HASHTABLE_TYPE_NEIGHBOR_TUNNEL_DMAC:
      return "neighbor tunnel dmac";
    case SWITCH_HASHTABLE_TYPE_HOSTIF:
      return "hostif";
    case SWITCH_HASHTABLE_TYPE_SMAC:
      return "smac";
    case SWITCH_HASHTABLE_TYPE_NAT:
      return "nat";
    case SWITCH_HASHTABLE_TYPE_TUNNEL_INGRESS_VNI:
      return "tunnel ingress vni";
    case SWITCH_HASHTABLE_TYPE_TUNNEL_EGRESS_VNI:
      return "tunnel egress vni";
    case SWITCH_HASHTABLE_TYPE_TUNNEL_SRC_IP:
      return "tunnel src ip";
    case SWITCH_HASHTABLE_TYPE_TUNNEL_DST_IP:
      return "tunnel dst ip";
    case SWITCH_HASHTABLE_TYPE_VLAN_PV:
      return "port vlan";
    default:
      return "unknown";
  }
}

#ifdef __cplusplus
}
#endif

#endif /* _switch_internal_h_ */
