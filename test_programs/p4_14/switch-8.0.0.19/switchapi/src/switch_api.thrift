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
/*
        switcht API thrift file
*/

namespace py switch_api namespace cpp switch_api

    /*
    enum switcht_status_t {
        SWITCHT_API_STATUS_SUCCESS = 0,
        SWITCHT_API_STATUS_PARAM_INVALID,
        SWITCHT_API_STATUS_INVALID_OPERATION,
        SWITCHT_API_STATUS_NOT_SUPPORTED,
        SWITCHT_API_STATUS_DUPLICATE,
        SWITCHT_API_STATUS_UNKNOWN_ERROR
    }

    enum switcht_handle_type_t {
        SWITCHT_HANDLE_TYPE_NONE,
        SWITCHT_HANDLE_TYPE_PORT,
        SWITCHT_HANDLE_TYPE_LAG,
        SWITCHT_HANDLE_TYPE_INTERFACE,
        SWITCHT_HANDLE_TYPE_VRF,
        SWITCHT_HANDLE_TYPE_BD,
        SWITCHT_HANDLE_TYPE_TUNNEL,
        SWITCHT_HANDLE_TYPE_NHOP,
        SWITCHT_HANDLE_TYPE_ECMP,
        SWITCHT_HANDLE_TYPE_ARP,
        SWITCHT_HANDLE_TYPE_MY_MAC,
        SWITCH_HANDLE_TYPE_WRED,
        SWITCHT_HANDLE_TYPE_MAX=15
    }

    enum switcht_direction_t {
        SWITCHT_API_DIRECTION_BOTH,
        SWITCHT_API_DIRECTION_INGRESS,
        SWITCHT_API_DIRECTION_EGRESS
    }

    enum switcht_sflow_collector_type_t {
        SWITCHT_API_SFLOW_COLLECTOR_TYPE_CPU = 0;
        SWITCHT_API_SFLOW_COLLECTOR_TYPE_REMOTE;
    }

    enum switcht_hostif_channel_t {
      SWITCHT_HOSTIF_CHANNEL_CB,
      SWITCHT_HOSTIF_CHANNEL_FD,
      SWITCHT_HOSTIF_CHANNEL_NETDEV
    }
    */

    typedef i32 switcht_status_t typedef i32 switcht_direction_t typedef i32
        switcht_interface_type_t typedef i32 switcht_rif_type_t typedef i32
            switcht_handle_type_t

    typedef byte switcht_device_t typedef byte switcht_pipe_t typedef i32
        switcht_vrf_id_t typedef i64 switcht_handle_t typedef i64
            switcht_uint64_t typedef string switcht_mac_addr_t typedef i32
                switcht_port_t typedef i16 switcht_vlan_t typedef i32
                    switcht_ifindex_t

    typedef i32 switcht_stp_mode_t typedef i32 switcht_stp_state_t typedef i32
        switcht_intf_attr_t

    typedef i16 switcht_nat_mode_t typedef i16 switcht_nat_rw_type_t typedef i16
        switcht_mcast_mode_t

    typedef i16 switcht_rpf_type_t

    typedef i32 switcht_urpf_group_t

    typedef i32 switcht_sflow_collector_type_t typedef i32
        switcht_sflow_sample_mode_t

    typedef byte switcht_packet_type_t typedef byte
        switcht_tunnel_type_t typedef byte switcht_tunnel_entry_type_t

struct switcht_api_port_info_t {
  1 : i32 port;
  2 : i16 port_speed;
  3 : bool initial_admin_state;
  4 : i32 tx_mtu;
  5 : i32 rx_mtu;
  6 : i16 fec_mode;
}

struct switcht_api_vlan_info_t {
  1 : bool learning_enabled;
  2 : bool igmp_snooping_enabled;
  3 : bool mld_snooping_enabled;
  4 : i32 aging_interval;
  5 : switcht_handle_t stp_handle;
  6 : i16 mrpf_group;
}

struct switcht_table_t {
  1: bool valid;
  2: i32 table_size;
  3: i32 num_entries;
  4: switcht_direction_t direction;
  5: string table_name;
}

struct switcht_ip_addr_t {
  1 : byte addr_type;
  2 : string ipaddr;
  3 : i32 prefix_length;
}

struct switcht_flow_t {
  1 : switcht_ip_addr_t src_ip;
  2 : switcht_ip_addr_t dst_ip;
  3 : bool is_local_flow;
}

struct switcht_api_mac_entry_t {
  1: switcht_handle_t network_handle;
  2: switcht_mac_addr_t mac_addr;
  3: byte entry_type;
  4: switcht_handle_t handle;
  5: switcht_ip_addr_t tunnel_ip;
}

typedef list<switcht_flow_t> switcht_flow_list_t

    struct switcht_port_vlan_t {
  2 : switcht_handle_t port_lag_handle;
  3 : i16 vlan_id; /**< VLAN id on port */
}

struct switcht_mcast_member_t {
  1 : switcht_handle_t network_handle;
  2 : switcht_handle_t intf_handle;
}

struct switcht_mroute_tree_t {
  1 : switcht_handle_t mgid_handle;
  2 : switcht_handle_t rpf_handle;
}

typedef i32 switcht_protocol_t

    struct switcht_udp_t {
  1 : i16 src_port;
  2 : i16 dst_port;
}

struct switcht_tcp_t {
  1 : i16 src_port;
  2 : i16 dst_port;
}

union switcht_udp_tcp_t {
  1 : switcht_udp_t udp;
  2 : switcht_tcp_t tcp;
}

struct switcht_interface_flags {
  1 : bool core_intf;
  2 : bool flood_enabled;
  3 : bool learn_enabled;
}

struct switcht_interface_info_t {
  1 : switcht_interface_type_t type;
  2 : switcht_handle_t handle;
  3 : switcht_handle_t rif_handle;
  4 : switcht_vlan_t vlan;
  5 : switcht_handle_t native_vlan_handle;
  6 : bool flood_enabled;
}

struct switcht_rif_info_t {
  1 : switcht_handle_t vrf_handle;
  2 : switcht_handle_t rmac_handle;
  4 : i16 v4_urpf_mode;
  5 : i16 v6_urpf_mode;
  6 : bool v4_unicast_enabled;
  7 : bool v6_unicast_enabled;
  8 : bool v4_multicast_enabled;
  9 : bool v6_multicast_enabled;
  10 : switcht_handle_t handle;
  11 : switcht_nat_mode_t nat_mode;
  12 : switcht_handle_t intf_handle;
  13 : switcht_vlan_t vlan;
  14 : switcht_handle_t ln_handle;
  15 : switcht_rif_type_t rif_type;
}

enum switcht_neighbor_type_t {
  SWITCHT_API_NEIGHBOR_L3_UNICAST,
  SWITCHT_API_NEIGHBOR_MPLS_L2VPN,
  SWITCHT_API_NEIGHBOR_MPLS_L3VPN
}

enum switcht_neighbor_rw_type_t {
  SWITCH_API_NEIGHBOR_RW_TYPE_L2,
  SWITCH_API_NEIGHBOR_RW_TYPE_L3
}

struct switcht_api_neighbor_info_t {
  1: byte neighbor_type;
  2: byte neighbor_tunnel_type;
  3: byte rw_type;
  4: switcht_handle_t nhop_handle;
  5: switcht_handle_t rif_handle;
  6: switcht_ip_addr_t ip_addr;
  7: switcht_mac_addr_t mac_addr;
}

struct switcht_nat_info_t {
  1 : switcht_nat_rw_type_t nat_rw_type;
  2 : switcht_ip_addr_t src_ip;
  3 : switcht_ip_addr_t dst_ip;
  4 : i32 src_port;
  5 : i32 dst_port;
  6 : i16 protocol;
  7 : switcht_handle_t vrf_handle;
  9 : switcht_handle_t nhop_handle;
  10 : switcht_ip_addr_t rw_src_ip;
  11 : switcht_ip_addr_t rw_dst_ip;
  12 : i32 rw_src_port;
  13 : i32 rw_dst_port;
}

struct switcht_vxlan_id_t {
  1 : i32 vnid;
}

struct switcht_geneve_id_t {
  1 : i32 vni;
}

struct switcht_nvgre_id_t {
  1 : i32 tnid;
}

struct switcht_ln_flags {
}

union switcht_bridge_type {
  1 : switcht_vlan_t vlan_id;
  2 : switcht_vxlan_id_t vxlan_info;
  3 : switcht_geneve_id_t geneve_info;
  4 : switcht_nvgre_id_t nvgre_info;
  5 : i32 tunnel_vni;
}

struct switcht_encap_info_t {
  1 : i32 encap_type;
  2 : switcht_bridge_type u;
}

struct switcht_mpls_t {
  1 : i32 label;
  2 : byte exp;
  3 : byte ttl;
}

struct switcht_srv6_segment_t {
  1 : string sid;
}

struct switcht_api_tunnel_info_t {
  1: byte tunnel_type;
  2: byte entry_type;
  3: byte direction;
  4: switcht_ip_addr_t src_ip;
  5: byte ttl;
  6: i32 gre_key;
  7: switcht_handle_t decap_mapper_handle;
  8: switcht_handle_t encap_mapper_handle;
  9: switcht_handle_t underlay_rif_handle;
  10: switcht_handle_t overlay_rif_handle;
  11: i16 erspan_span_id;
  12: byte ip_type;
}

struct switcht_api_tunnel_term_info_t {
  1: switcht_handle_t tunnel_handle;
  2: switcht_handle_t vrf_handle;
  3: byte tunnel_type;
  4: byte term_entry_type;
  5: switcht_ip_addr_t src_ip;
  6: switcht_ip_addr_t dst_ip;
}

struct switcht_api_tunnel_mapper_entry_t {
  1: byte tunnel_map_type;
  2: switcht_handle_t tunnel_mapper_handle;
  3: i32 tunnel_vni;
  4: switcht_handle_t ln_handle;
  5: switcht_handle_t vlan_handle;
  6: switcht_handle_t vrf_handle;
}

struct switcht_api_tunnel_mapper_t {
  1: byte tunnel_map_type;
}

struct switcht_mpls_label_stack_t {
  1: list<switcht_mpls_t> label_list;
  2: bool bos;
}

struct switcht_api_mpls_info_t {
  1: byte tunnel_type;
  2: byte mpls_type;
  3: byte mpls_mode;
  4: switcht_handle_t vrf_handle;
  5: switcht_handle_t network_handle;
  6: i32 swap_label;
  7: i32 pop_label;
  8: byte pop_count;
  9: switcht_handle_t intf_handle;
  10: switcht_handle_t nhop_handle;
  11: switcht_mac_addr_t mac_addr;
}

struct switcht_logical_network_t {
  1 : i32 type;
  2 : switcht_handle_t vrf_handle;
  3 : switcht_handle_t rmac_handle;
  4 : i32 age_interval;
  5 : bool flood_enabled;
  6 : bool learn_enabled;
  7 : bool core_bd;
  8 : bool ipv4_unicast_enabled;
  9 : bool ipv6_unicast_enabled;
  10 : bool ipv4_multicast_enabled;
  11 : bool ipv6_multicast_enabled;
}

union switcht_acl_value_t {
  1 : string value_str;
  2 : i64 value_num;
}

struct switcht_acl_key_value_pair_t {
  1 : i32 field;
  2 : switcht_acl_value_t value;
  3 : switcht_acl_value_t mask;
}

typedef i32 switcht_acl_type_t
typedef switcht_acl_key_value_pair_t switcht_acl_system_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ip_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_mirror_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_qos_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_mac_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ipv6_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ipracl_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ipv6racl_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_egress_system_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_egress_ip_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_egress_ipv6_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_sflow_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ipv6_mirror_key_value_pair_t
typedef switcht_acl_key_value_pair_t switcht_acl_ip_mirror_key_value_pair_t

struct switcht_vlan_port_t {
  1: switcht_handle_t handle;
  2: i16 tagging_mode;
}

struct switcht_nhop_key_t {
  1: switcht_handle_t handle;
  2: switcht_ip_addr_t ip_addr;
}

struct switcht_api_nhop_info_t {
  1: byte nhop_type;
  2: byte nhop_tunnel_type;
  3: switcht_handle_t vrf_handle;
  4: switcht_handle_t network_handle;
  5: switcht_handle_t rif_handle;
  6: switcht_handle_t tunnel_handle;
  7: switcht_handle_t intf_handle;
  8: switcht_handle_t label_stack_handle;
  9: switcht_ip_addr_t ip_addr;
  10: byte rewrite_type;
  11: i32 tunnel_vni;
  12: switcht_mac_addr_t mac_addr;
  13: switcht_handle_t mpls_handle;
}

struct switcht_hostif_group_t {
  1 : switcht_handle_t queue_handle;
  2 : switcht_handle_t policer_handle;
}

struct switcht_counter_t {
  1 : i64 num_packets;
  2 : i64 num_bytes;
}

typedef i32 switcht_acl_action_t
typedef i32 switcht_hostif_reason_code_t
typedef byte switcht_hostif_channel_t

struct switcht_hostif_rcode_info_t {
  1 : switcht_hostif_reason_code_t reason_code;
  2 : switcht_acl_action_t action;
  3 : i32 priority;
  4 : switcht_handle_t hostif_group_id;
}

struct switcht_hostif_t {
  1: switcht_hostif_channel_t type;
  2: string intf_name;
  3: switcht_handle_t handle;
  4: switcht_mac_addr_t mac;
  5: switcht_ip_addr_t v4addr;
  6: switcht_ip_addr_t v6addr;
  7: bool operstatus;
  8: byte vlan_action;
  9: bool admin_state;
}

struct switcht_hostif_table_entry_t {
  1 : switcht_hostif_reason_code_t rcode;
  2 : switcht_handle_t interface_handle;
  3 : switcht_hostif_channel_t type;
  4 : switcht_handle_t hostif_handle;

}

struct switcht_acl_action_cpu_redirect {
  1 : i32 reason_code;
}

struct switcht_acl_action_redirect {
  1 : switcht_handle_t handle;
}

struct switcht_acl_action_drop {
  1 : i32 reason_code;
}

union switcht_acl_action_params_t {
  1 : switcht_acl_action_cpu_redirect cpu_redirect;
  2 : switcht_acl_action_redirect redirect;
  3 : switcht_acl_action_drop drop;
}

struct switcht_acl_opt_action_params_t {
  1 : switcht_handle_t mirror_handle;
  2 : switcht_handle_t meter_handle;
  3 : switcht_handle_t counter_handle;
  4 : switcht_nat_mode_t nat_mode;
}

struct switcht_acl_action_spec_t {
  1 : switcht_acl_action_t action;
  2 : switcht_acl_action_params_t action_params;
  3 : switcht_acl_opt_action_params_t opt_action_params;
}

typedef i32 switcht_mirror_id_t

struct switcht_mirror_info_t {
  1 : switcht_mirror_id_t session_id;
  2 : switcht_direction_t direction;
  3 : switcht_handle_t egress_port_handle;
  4 : i32 mirror_type;
  5 : byte cos;
  6 : i32 max_pkt_len;
  7 : i32 ttl;
  8 : switcht_handle_t nhop_handle;
  9 : i32 session_type;
  10 : switcht_vlan_t vlan_id;
  11 : i32 extract_len;
  12 : i32 timeout_usec;
  13 : byte span_mode;
  14 : switcht_ip_addr_t src_ip;
  15 : switcht_ip_addr_t dst_ip;
  16 : switcht_mac_addr_t src_mac;
  17 : switcht_mac_addr_t dst_mac;
  18 : byte tos;
  19 : bool vlan_tag_valid;
  20 : i16 vlan_tpid;
  21 : switcht_handle_t vrf_handle;
}

struct switcht_sflow_info_t {
  1 : i32 timeout_usec;
  2 : i32 sample_rate;
  3 : i32 extract_len;
  4 : switcht_sflow_collector_type_t collector_type;
  5 : switcht_handle_t egress_port_hdl;
  6 : switcht_sflow_sample_mode_t sample_mode;
}

struct switcht_tunnel_mapper_t {
  1 : switcht_handle_t ln_handle;
  2 : i32 tunnel_vni;
}

struct switcht_bfd_session_info_t {
  1 : i32 my_disc;
  2 : i32 your_disc;
  3 : byte detect_mult;               /* used for rx timeout */
  4 : i32 desired_tx_interval;        /* usec - goes in pkt*/
  5 : i32 min_rx_interval;            /* usec - goes in pkt*/
  6 : i32 tx_interval;                /* usec - negotiated val */
  7 : i32 rx_interval;                /* usec - negotiated val */
  8 : i32 remote_desired_tx_interval; /* usec - goes in pkt*/
  9 : i32 remote_min_rx_interval;     /* usec - goes in pkt*/
  10 : switcht_ip_addr_t sip;         // v4 or v6 addr
  11 : switcht_ip_addr_t dip;
  12 : i16 sport;
  13 : i16 dport;  // 1hop, multihop bfd session
  14 : switcht_handle_t vrf_hdl;
  15 : switcht_handle_t rmac_hdl;
  16 : switcht_mac_addr_t rmac;
}

typedef i64 switcht_cbs_t
typedef i64 switcht_pbs_t
typedef i64 switcht_cir_t
typedef i64 switcht_pir_t
typedef byte switcht_meter_mode_t
typedef byte switcht_meter_color_source_t
typedef byte switcht_meter_type_t
typedef byte switcht_meter_counter_t

struct switcht_wred_info_t {
  1: bool enable;
  2: bool ecn_mark;
  3: i32 min_threshold;
  4: i32 max_threshold;
  5: double max_probability;
  6: double time_constant;
}

struct switcht_wred_profile_info_t {
  1 : i32 min_threshold_yellow;
  2 : i32 max_threshold_yellow;
  3 : bool enable_yellow;
  4 : i32 probability_yellow;
  5 : bool ecn_mark_yellow;
  6 : i32 min_threshold_green;
  7 : i32 max_threshold_green;
  8 : bool enable_green;
  9 : i32 probability_green;
  10 : bool ecn_mark_green;
  11 : i32 min_threshold_red;
  12 : i32 max_threshold_red;
  13 : bool enable_red;
  14 : i32 probability_red;
  15 : bool ecn_mark_red;
}

struct switcht_meter_info_t {
  1 : switcht_meter_mode_t meter_mode;
  2 : switcht_meter_color_source_t color_source;
  3 : switcht_meter_type_t meter_type;
  4 : switcht_cbs_t cbs;
  5 : switcht_pbs_t pbs;
  6 : switcht_cir_t cir;
  7 : switcht_pir_t pir;
  8 : switcht_acl_action_t green_action;
  9 : switcht_acl_action_t yellow_action;
  10 : switcht_acl_action_t red_action;
}

struct switcht_buffer_profile_t {
  1 : byte threshold_mode;
  2 : i32 threshold;
  3 : switcht_handle_t pool_handle;
  4 : i32 buffer_size;
  5 : i32 xoff_threshold;
  6 : i32 xon_threshold;
}

struct switcht_buffer_pool_t {
  1 : switcht_direction_t dir;
  2 : i32 pool_size;
  3 : i32 threshold;
  4 : i32 xoff_size;
  5 : i32 shared_size;
}

typedef i16 switcht_qos_map_type_t typedef byte switcht_color_t

    struct switcht_qos_map_t {
  1 : byte dscp;
  2 : byte tos;
  3 : byte pcp;
  4 : i16 tc;
  5 : switcht_color_t color;
  6 : byte icos;
  7 : byte qid;
}

typedef byte switcht_scheduler_type_t typedef byte switcht_shaper_type_t

struct switcht_scheduler_info_t {
  1 : switcht_scheduler_type_t scheduler_type;
  2 : switcht_shaper_type_t shaper_type;
  3 : i32 priority;
  4 : i32 rem_bw_priority;
  5 : i32 weight;
  6 : i32 min_burst_size;
  7 : i32 min_rate;
  8 : i32 max_burst_size;
  9 : i32 max_rate;
}

struct switcht_scheduler_group_info_t {
  1 : i16 group_type;
  2 : switcht_handle_t port_handle;
  3 : switcht_handle_t scheduler_handle;
  4 : switcht_handle_t queue_handle;
}

typedef byte switcht_range_type_t

struct switcht_range_t {
  1 : i32 start_value;
  2 : i32 end_value;
}

struct switcht_hostif_rx_filter_key_t {
  1 : switcht_handle_t port_handle;
  2 : switcht_handle_t intf_handle;
  3 : switcht_handle_t handle;
  4 : i32 reason_code;
  5 : i32 reason_code_mask;
}

struct switcht_hostif_rx_filter_action_t {
  1 : switcht_handle_t hostif_handle;
  2 : switcht_vlan_t vlan_id;
  3 : i32 vlan_action;
}

struct switcht_hostif_tx_filter_key_t {
  1 : switcht_handle_t hostif_handle;
  2 : switcht_vlan_t vlan_id;
}

struct switcht_hostif_tx_filter_action_t {
  1 : i32 bypass_flags;
  2 : switcht_handle_t handle;
}

struct switcht_route_entry_t {
  1 : switcht_handle_t vrf_handle;
  2 : switcht_handle_t rif_handle;
  3 : switcht_ip_addr_t ip_addr;
  4 : switcht_handle_t nhop_handle;
}

union switcht_twl_value_t {
  1 : string value_str;
  2 : i64 value_num;
}

struct switcht_twl_key_value_pair_t {
  1 : i32 field;
  2 : switcht_twl_value_t value;
  3 : switcht_twl_value_t mask;
}

struct switcht_twl_int_params_t {
  1 : i16 session_id;
  2 : bool report_all_packets;
  3 : byte flow_sample_percent;
}

struct switcht_twl_postcard_params_t {
  1 : bool report_all_packets;
  2 : byte flow_sample_percent;
}

struct switcht_twl_mod_params_t {
  1 : bool report_queue_tail_drops;
}

typedef i32 switcht_dtel_event_type_t

struct switch_hash_ipv6_input_fields_res_t {
  1 : switcht_status_t status;
  2 : i32 fields;
}

struct switch_hash_ipv4_input_fields_res_t {
  1 : switcht_status_t status;
  2 : i32 fields;
}

struct switch_hash_non_ip_input_fields_res_t {
  1 : switcht_status_t status;
  2 : i32 fields;
}

struct switch_hash_input_fields_attribute_res_t {
  1: switcht_status_t status;
  2: i32 attr_flags;
}

struct switch_hash_ipv6_algo_res_t {
  1 : switcht_status_t status;
  2 : i32 algorithm;
}

struct switch_hash_ipv4_algo_res_t {
  1 : switcht_status_t status;
  2 : i32 algorithm;
}

struct switch_hash_non_ip_algo_res_t {
  1 : switcht_status_t status;
  2 : i32 algorithm;
}

struct switch_hash_ipv6_seed_res_t {
  1 : switcht_status_t status;
  2 : i64 seed;
}

struct switch_hash_ipv4_seed_res_t {
  1 : switcht_status_t status;
  2 : i64 seed;
}

struct switch_hash_non_ip_seed_res_t {
  1 : switcht_status_t status;
  2 : i64 seed;
}

struct switcht_api_device_info_t {
  1 : switcht_vrf_id_t default_vrf;
  2 : switcht_handle_t vrf_handle;
  3 : switcht_vlan_t default_vlan;
  4 : switcht_handle_t vlan_handle;
  5 : switcht_mac_addr_t mac;
  6 : switcht_handle_t rmac_handle;
  7 : i16 max_lag_groups;
  8 : i16 max_lag_members;
  9 : i16 max_ecmp_groups;
  10 : i16 max_ecmp_members;
  11 : i16 lag_hash_algorithm;
  12 : i32 lag_hash_flags;
  13 : i16 ecmp_hash_algorithm;
  14 : i32 ecmp_hash_flags;
  15 : i16 default_log_level;
  16 : bool install_dmac;
  17 : i16 max_vrf;
  18 : i32 max_ports;
  19 : list<switcht_handle_t> port_list;
  20 : i32 eth_cpu_port;
  21 : i32 pcie_cpu_port;
  22 : i32 refresh_interval;
  23 : i32 aging_interval;
  24 : i32 num_ports;
  25 : i32 num_active_ports;
  26 : i32 max_port_mtu;
}

exception InvalidSwitchOperation {
  1 : i32 code
}

service switch_api_rpc {
    /* init */
    switcht_status_t switch_api_init(1:switcht_device_t device);

    /* table details */
    switcht_table_t switch_api_table_get(1:switcht_device_t device, 2:i16 table_id);
    i16 switch_api_table_size_get(1:switcht_device_t device, 2:i16 table_id);
    list<switcht_table_t> switch_api_table_all_get(1:switcht_device_t device);

    /* drop stats */
    list<i64> switch_api_drop_stats_get(1:switcht_device_t device);

    /* Batch APIs */

    switcht_status_t switch_api_batch_begin();
    switcht_status_t switch_api_batch_end(1: bool hw_synchronous);

    /* Device */
    switcht_handle_t switch_api_device_cpu_port_handle_get(
                             1: switcht_device_t device);
    switcht_port_t switch_api_device_cpu_port_get(
                             1: switcht_device_t device);
    switcht_status_t switch_api_device_mac_aging_interval_set(
                             1: switcht_device_t device,
                             2: i32 aging_time);
    i32 switch_api_device_mac_aging_interval_get(
                             1: switcht_device_t device);
    switcht_api_device_info_t switch_api_device_attribute_get(
                             1: switcht_device_t device,
                             2: i64 flags);
    switcht_handle_t switch_api_device_default_rmac_handle_get(
                             1: switcht_device_t device);
    switcht_handle_t switch_api_device_default_vrf_handle_get(
                             1: switcht_device_t device);
    switcht_vrf_id_t switch_api_device_default_vrf_id_get(
                             1: switcht_device_t device);
    switcht_handle_t switch_api_device_default_vlan_handle_get(
                             1: switcht_device_t device);
    switcht_vlan_t switch_api_device_default_vlan_id_get(
                             1: switcht_device_t device);
    switcht_port_t switch_api_device_cpu_eth_port_get(
                             1: switcht_device_t device);
    switcht_port_t switch_api_device_cpu_pcie_port_get(
                             1: switcht_device_t device);
    i32 switch_api_device_counter_refresh_interval_get(
                             1: switcht_device_t device);
    switcht_handle_t switch_api_device_recirc_port_get(
                             1: switcht_device_t device,
                             2: switcht_pipe_t pipe_id);
    i16 switch_api_device_max_recirc_ports_get(
                             1: switcht_device_t device);
    switcht_acl_action_t switch_api_device_dmac_miss_packet_action_get(
                             1: switcht_device_t device,
                             2: switcht_packet_type_t pkt_type);
    bool switch_api_device_cut_through_mode_get(
                             1: switcht_device_t device);
    void switch_api_config_smac_program_set(
                             1: switcht_device_t device,
                             2: bool flag);
    switcht_status_t
    switch_api_device_mac_learning_set(
                             1: switcht_device_t device,
                             2: bool enable);
    bool
    switch_api_device_mac_learning_get(1: switcht_device_t device);

    /* Port */
    switcht_handle_t switch_api_port_add(
                             1: switcht_device_t device,
                             2: switcht_port_t port);
    switcht_handle_t switch_api_port_add_with_attribute(
                             1: switcht_device_t device,
                             2: switcht_api_port_info_t api_port_info);
    switcht_status_t switch_api_port_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_status_t switch_api_port_storm_control_set(
                             1: switcht_device_t device,
                             2: switcht_port_t port_id,
                             3: switcht_packet_type_t pkt_type,
                             4: switcht_handle_t meter_handle);
    list<switcht_counter_t> switch_api_storm_control_counters_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle,
                             3: list<i16> counter_ids);
    switcht_status_t switch_api_port_trust_dscp_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: bool trust_dscp);
    switcht_status_t switch_api_port_trust_pcp_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: bool trust_pcp);
    switcht_status_t switch_api_port_ingress_mirror_set(
                             1: switcht_device_t device,
			     2: switcht_handle_t port_handle,
			     3: switcht_handle_t mirror_handle);
    switcht_status_t switch_api_port_egress_mirror_set(
                             1: switcht_device_t device,
			     2: switcht_handle_t port_handle,
			     3: switcht_handle_t mirror_handle);
    switcht_status_t switch_api_port_learning_enabled_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: bool learning_enabled);
    switcht_status_t switch_api_port_drop_limit_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i32 num_bytes);
    switcht_status_t switch_api_port_drop_hysteresis_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i32 num_bytes);
    switcht_status_t switch_api_port_pfc_cos_mapping(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: list<byte> cos_to_icos);
    switcht_status_t switch_api_port_tc_default_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i16 tc);
    switcht_status_t switch_api_port_color_default_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_color_t color);
    switcht_status_t switch_api_port_qos_group_ingress_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_handle_t qos_handle);
    switcht_status_t switch_api_port_qos_group_tc_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_handle_t qos_handle);
    switcht_status_t switch_api_port_qos_group_egress_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_handle_t qos_handle);
    switcht_handle_t switch_api_port_id_to_handle_get(
                             1: switcht_device_t device,
                             2: switcht_port_t port);

    switcht_status_t switch_api_port_bind_mode_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i32 bind_mode);
    switcht_status_t switch_api_port_mtu_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i32 tx_mtu,
                             4: i32 rx_mtu);
    switcht_status_t switch_api_port_stats_clear(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_status_t switch_api_port_ingress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i16 label);
    switcht_status_t switch_api_port_egress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: i16 label);
    i16 switch_api_port_speed_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_auto_neg_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    bool switch_api_port_admin_state_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_oper_status_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_loopback_mode_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_rx_mtu_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_tx_mtu_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_api_port_info_t switch_api_port_get(
                             1: switcht_device_t device,
                             2: i32 port_number);
    switcht_handle_t switch_api_port_storm_control_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_packet_type_t pkt_type);
    switcht_port_t switch_api_port_handle_to_id_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    list<i64> switch_api_port_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: list<i16> counter_ids);
    switcht_handle_t switch_api_port_ingress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_egress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_ingress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_egress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_bind_mode_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_max_queues_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_pfc_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    bool switch_api_port_link_tx_pause_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    bool switch_api_port_link_rx_pause_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16  switch_api_port_fec_mode_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_ingress_mirror_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_egress_mirror_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_ingress_sflow_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_egress_sflow_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_ingress_qos_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_tc_queue_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_tc_ppg_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_egress_qos_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i16 switch_api_port_max_ppg_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    list<switcht_handle_t> switch_api_port_ppg_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i64 switch_api_ppg_drop_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle);
    switcht_handle_t switch_api_port_pfc_priority_to_ppg_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_pfc_priority_to_queue_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    list<switcht_handle_t> switch_api_port_qos_scheduler_group_handles_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i32 switch_api_port_queue_scheduler_group_handle_count_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_handle_t switch_api_port_scheduler_profile_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_status_t switch_api_port_ppg_create(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_status_t switch_api_port_ppg_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i64 switch_api_port_ppg_drop_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    i64 switch_api_port_queue_drop_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);

    /* vrf */
    switcht_handle_t switch_api_vrf_create(
                             1:switcht_device_t device,
                             2:switcht_vrf_id_t vrf);
    switcht_status_t switch_api_vrf_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle);
    switcht_vrf_id_t switch_api_default_vrf_id_get(
                             1:switcht_device_t device);
    switcht_vrf_id_t switch_api_vrf_handle_to_id_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle);
    switcht_handle_t switch_api_vrf_id_to_handle_get(
                             1:switcht_device_t device,
                             2:switcht_vrf_id_t vrf);
    switcht_status_t switch_api_vrf_rmac_handle_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle,
                             3:switcht_handle_t rmac_handle);
    switcht_handle_t switch_api_vrf_rmac_handle_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle);

    /* router mac */
    switcht_handle_t switch_api_router_mac_group_create(
                             1:switcht_device_t device,
                             2: i32 rmac_type);
    switcht_status_t switch_api_router_mac_group_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t rmac_handle);
    switcht_status_t switch_api_router_mac_add(
                             1:switcht_device_t device,
                             2:switcht_handle_t rmac_handle,
                             3:switcht_mac_addr_t mac);
    switcht_status_t switch_api_router_mac_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t rmac_handle,
                             3:switcht_mac_addr_t mac);
    switcht_handle_t switch_api_default_router_mac_handle_get(
                             1: switcht_device_t device);
    list<switcht_mac_addr_t> switch_api_rmac_macs_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t rmac_handle);

    /* interface */
    switcht_handle_t switch_api_interface_create(
                             1:switcht_device_t device,
                             2:switcht_interface_info_t interface_info);
    switcht_status_t switch_api_interface_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t interface_handle);
    switcht_status_t switch_api_interface_native_vlan_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle,
                             3: switcht_handle_t vlan_handle);
    switcht_ifindex_t switch_api_interface_ifindex_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t interface_handle);
    switcht_handle_t switch_api_interface_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle);
    switcht_handle_t switch_api_interface_by_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle,
                             3: switcht_interface_type_t intf_type);
    switcht_handle_t switch_api_interface_native_vlan_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle);
    switcht_vlan_t switch_api_interface_native_vlan_id_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle);
    switcht_interface_info_t switch_api_interface_attribute_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle,
                             3: i64 intf_flags);
    switcht_handle_t switch_api_interface_ln_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle);

    /* rif */
    switcht_handle_t switch_api_rif_create(1:switcht_device_t device, 2:switcht_rif_info_t rif_info);
    switcht_status_t switch_api_rif_delete(1:switcht_device_t device, 2:switcht_handle_t rif_handle);
    switcht_status_t switch_api_rif_ipv4_unicast_enabled_set(1: switcht_handle_t rif_handle, 2: i64 value);
    switcht_status_t switch_api_rif_ipv6_unicast_enabled_set(1: switcht_handle_t rif_handle, 2: i64 value);
    switcht_status_t switch_api_rif_ipv4_urpf_mode_set(1: switcht_device_t device, 2: switcht_handle_t rif_handle, 3: i64 value);
    switcht_status_t switch_api_rif_ipv6_urpf_mode_set(1: switcht_device_t device, 2: switcht_handle_t rif_handle, 3: i64 value);
    switcht_status_t switch_api_rif_mtu_set(1: switcht_device_t device, 2: switcht_handle_t rif_handle, 3: switcht_handle_t mtu_handle);
    i32 switch_api_rif_bd_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_status_t switch_api_rif_ingress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle, 3: i16 label);
    switcht_status_t switch_api_rif_egress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle, 3: i16 label);
    switcht_handle_t switch_api_rif_vrf_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_handle_t switch_api_rif_intf_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    bool switch_api_rif_ipv4_unicast_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    bool switch_api_rif_ipv6_unicast_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    bool switch_api_rif_ipv4_multicast_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    bool switch_api_rif_ipv6_multicast_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_handle_t switch_api_rif_mtu_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    i16 switch_api_rif_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_rif_info_t switch_api_rif_attribute_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle,
                             3: i16 type);
    switcht_handle_t switch_api_rif_rmac_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_handle_t switch_api_rif_ingress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    i16 switch_api_rif_ingress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    switcht_handle_t switch_api_rif_egress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);
    i16 switch_api_rif_egress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle);

    /* ip address */
    switcht_status_t
    switch_api_l3_interface_address_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t rif_handle,
                             3: switcht_handle_t vrf,
                             4: switcht_ip_addr_t ip_addr);
    switcht_status_t
    switch_api_l3_interface_address_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t interface_handle,
                             3: switcht_handle_t vrf,
                             4:switcht_ip_addr_t ip_addr);

    /* next hop */
    switcht_handle_t switch_api_nhop_create(
                             1:switcht_device_t device,
                             2:switcht_api_nhop_info_t api_nhop_info);
    switcht_status_t switch_api_nhop_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t handle);
    switcht_api_nhop_info_t switch_api_nhop_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t nhop_handle);

    /* ARP */
    switcht_handle_t switch_api_nhop_handle_get(
                             1:switcht_device_t device,
                             2:switcht_nhop_key_t nhop_key);
    switcht_handle_t switch_api_neighbor_handle_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t handle);
    i16 switch_api_nhop_id_type_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t handle);
    i32 switch_api_nhop_table_size_get(
                             1:switcht_device_t device);
    list<switcht_handle_t> switch_api_ecmp_members_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t handle);

    /* ARP */
    switcht_handle_t switch_api_neighbor_create(
                             1:switcht_device_t device,
                             2:switcht_api_neighbor_info_t neighbor);
    switcht_status_t switch_api_neighbor_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t neighbor_handle);
    switcht_mac_addr_t switch_api_neighbor_entry_rewrite_mac_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t neighbor_handle);

    /* L3 */
    switcht_status_t
    switch_api_l3_route_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t vrf,
                             3: switcht_ip_addr_t ip_addr,
                             4: switcht_handle_t nhop_handle);

    switcht_status_t
    switch_api_l3_route_update(
                             1: switcht_device_t device,
                             2: switcht_handle_t vrf,
                             3: switcht_ip_addr_t ip_addr,
                             4: switcht_handle_t nhop_handle);
    switcht_status_t
    switch_api_l3_route_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t vrf,
                             3: switcht_ip_addr_t ip_addr,
                             4: switcht_handle_t nhop_handle);
    switcht_handle_t
    switch_api_l3_route_lookup(
                             1: switcht_device_t device,
                             2: switcht_handle_t vrf,
                             3:switcht_ip_addr_t ip_addr);

    switcht_handle_t
    switch_api_l3_route_nhop_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vrf,
                             3: switcht_ip_addr_t ip_addr);

    i32
    switch_api_route_table_size_get(
                             1: switcht_device_t device);

    /* VLAN */
    switcht_handle_t
    switch_api_vlan_create(1: switcht_device_t device,
                           2: switcht_vlan_t vlan_id);
    switcht_status_t
    switch_api_vlan_delete(1: switcht_device_t device,
                           2: switcht_handle_t vlan_handle);
    switcht_handle_t
    switch_api_vlan_id_to_handle_get(
                           1: switcht_device_t device,
                           2: switcht_vlan_t vlan_id);
    switcht_vlan_t
    switch_api_vlan_handle_to_id_get(
                           1: switcht_device_t device,
                           2: switcht_handle_t vlan_handle);
    switcht_status_t
    switch_api_vlan_member_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_handle_t intf_handle);
    switcht_status_t
    switch_api_vlan_member_remove(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_handle_t intf_handle);
    list<switcht_handle_t>
    switch_api_vlan_interfaces_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);
    switcht_vlan_t
    switch_api_vlan_member_vlan_id_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_member_handle);
    bool
    switch_api_vlan_member_vlan_tagging_mode_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_member_handle);
    switcht_handle_t
    switch_api_vlan_member_intf_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_member_handle);

    /* VLAN attribute */
    switcht_status_t
    switch_api_vlan_learning_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: bool enable);

    bool
    switch_api_vlan_learning_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_aging_interval_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: i32 value);

    i32
    switch_api_vlan_aging_interval_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);
    switcht_status_t
    switch_api_vlan_stats_enable(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_stats_disable(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    list<switcht_counter_t>
    switch_api_vlan_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: list<i16> counter_ids);

    switcht_api_vlan_info_t
    switch_api_vlan_attribute_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: i64 flags);

    switcht_status_t
    switch_api_vlan_igmp_snooping_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: bool enable);

    bool
    switch_api_vlan_igmp_snooping_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_mld_snooping_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: bool enable);

    bool
    switch_api_vlan_mld_snooping_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_mrpf_group_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle,
                             3: i64 value);

    i64
    switch_api_vlan_mrpf_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_stp_handle_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t intf_handle,
                             3: switcht_handle_t stp_handle);

    switcht_handle_t
    switch_api_vlan_stp_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_ingress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: i16 label);

    switcht_handle_t
    switch_api_vlan_ingress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    i16
    switch_api_vlan_ingress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    switcht_status_t
    switch_api_vlan_egress_acl_label_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: i16 label);

    switcht_handle_t
    switch_api_vlan_egress_acl_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    i16
    switch_api_vlan_egress_acl_label_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    i32 switch_api_vlan_bd_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle);

    /* L2 */
    switcht_status_t
    switch_api_mac_table_entry_create(
                             1: switcht_device_t device,
                             2: switcht_api_mac_entry_t mac_entry);

    switcht_status_t
    switch_api_mac_table_entry_update(
                             1: switcht_device_t device,
                             2: switcht_api_mac_entry_t mac_entry);

    switcht_status_t
    switch_api_mac_table_entry_delete(
                             1: switcht_device_t device,
                             2: switcht_api_mac_entry_t mac_entry);

    switcht_status_t
    switch_api_mac_table_entry_flush(
                             1: switcht_device_t device,
                             2: switcht_uint64_t flush_type,
                             3: switcht_handle_t network_handle,
                             4: switcht_handle_t intf_handle);

    switcht_status_t
    switch_api_mac_table_learning_timeout_set(
                             1: switcht_device_t device,
                             2: i32 learn_timeout);

    switcht_handle_t
    switch_api_mac_entry_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_mac_addr_t mac);

    i16
    switch_api_mac_entry_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_mac_addr_t mac);

    switcht_handle_t
    switch_api_mac_entry_intf_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_mac_addr_t mac);

    i16
    switch_api_mac_entry_packet_action_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t vlan_handle,
                             3: switcht_mac_addr_t mac);

    /* ECMP */

    switcht_handle_t
    switch_api_ecmp_create(
             1:switcht_device_t device);

    switcht_status_t
    switch_api_ecmp_delete(
             1:switcht_device_t device,
             2:switcht_handle_t handle);

    switcht_status_t
    switch_api_ecmp_member_add(
             1: switcht_device_t device,
             2:switcht_handle_t handle,
             3: i16 nhop_count,
             4:list<switcht_handle_t> nhop_handle);

    switcht_status_t
    switch_api_ecmp_member_delete(
             1: switcht_device_t device,
             2:switcht_handle_t handle,
             3: i16 nhop_count, 4:
             list<switcht_handle_t> nhop_handle);

    switcht_status_t
    switch_api_l3_ecmp_member_activate(
             1: switcht_device_t device,
             2:switcht_handle_t handle,
             3: i16 nhop_count,
             4: list<switcht_handle_t> nhop_handle);

    switcht_status_t
    switch_api_l3_ecmp_member_deactivate(
             1: switcht_device_t device,
             2:switcht_handle_t handle,
             3: i16 nhop_count,
             4: list<switcht_handle_t> nhop_handle);

    /* WCMP */
    switcht_handle_t switch_api_l3_wcmp_create(1:switcht_device_t device);
    switcht_status_t switch_api_l3_wcmp_delete(1:switcht_device_t device, 2:switcht_handle_t handle);
    switcht_status_t
    switch_api_l3_wcmp_member_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t handle,
                             3: i16 nhop_count,
                             4: list<switcht_handle_t> nhop_handle,
                             5:list<i16> nhop_weight);
    switcht_status_t
    switch_api_l3_wcmp_member_modify(
                             1: switcht_device_t device,
                             2: switcht_handle_t handle,
                             3: i16 nhop_count,
                             4:list<switcht_handle_t> nhop_handle,
                             5:list<i16> nhop_weight);
    switcht_status_t
    switch_api_l3_wcmp_member_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t handle,
                             3: i16 nhop_count,
                             4:list<switcht_handle_t> nhop_handle);

    /* LAG */
    switcht_handle_t switch_api_lag_create(1:switcht_device_t device);
    switcht_status_t switch_api_lag_delete(1:switcht_device_t device, 2:switcht_handle_t lag_handle);
    switcht_status_t
    switch_api_lag_member_add(
             1: switcht_device_t device,
             2:switcht_handle_t lag_handle,
             3:switcht_direction_t side,
             4:switcht_handle_t port);

    switcht_status_t switch_api_lag_member_delete(
             1: switcht_device_t device,
             2:switcht_handle_t lag_handle,
             3:switcht_direction_t side,
             4:switcht_handle_t port);

    switcht_status_t
    switch_api_lag_member_activate(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle,
             3: switcht_handle_t port_handle);

    switcht_status_t
    switch_api_lag_member_deactivate(
             1: switcht_device_t device,
             2:switcht_handle_t lag_handle,
             3:switcht_handle_t port_handle);

    switcht_status_t
    switch_api_lag_bind_mode_set(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle,
             3: i32 bind_mode);

    switcht_status_t
    switch_api_lag_ingress_acl_label_set(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle,
             3: i16 label);

    switcht_handle_t
    switch_api_lag_ingress_acl_group_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    i16
    switch_api_lag_ingress_acl_label_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    switcht_status_t
    switch_api_lag_egress_acl_label_set(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle,
             3: i16 label);

    switcht_handle_t
    switch_api_lag_egress_acl_group_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    i16
    switch_api_lag_egress_acl_label_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    i32
    switch_api_lag_bind_mode_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    list<switcht_handle_t>
    switch_api_lag_members_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    i32
    switch_api_lag_member_count_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_handle);

    switcht_handle_t
    switch_api_lag_member_port_handle_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_member_handle);

    switcht_handle_t
    swich_api_lag_handle_from_lag_member_get(
             1: switcht_device_t device,
             2: switcht_handle_t lag_member_handle);

    /* LAG failover */
    switcht_status_t
    switch_api_fast_failover_enable(
             1:switcht_device_t device);
    switcht_status_t
    switch_api_fast_failover_disable(
             1:switcht_device_t device);

    /* Tunnel API */
    switcht_handle_t
    switch_api_tunnel_create(
             1: switcht_device_t device,
             2: switcht_api_tunnel_info_t api_tunnel_info);

    switcht_status_t
    switch_api_tunnel_delete(
             1:switcht_device_t device,
             2:switcht_handle_t tunnel_handle);

    switcht_handle_t
    switch_api_tunnel_term_create(
             1: switcht_device_t device,
             2: switcht_api_tunnel_term_info_t api_tunnel_term_info);

    switcht_status_t
    switch_api_tunnel_term_delete(
             1:switcht_device_t device,
             2:switcht_handle_t tunnel_term_handle);

    switcht_handle_t
    switch_api_tunnel_mapper_create(
             1: switcht_device_t device,
             2: switcht_api_tunnel_mapper_t tunnel_mapper);

    switcht_status_t
    switch_api_tunnel_mapper_delete(
             1:switcht_device_t device,
             2:switcht_handle_t mapper_handle);

    switcht_handle_t
    switch_api_tunnel_mapper_entry_create(
             1: switcht_device_t device,
             2: switcht_api_tunnel_mapper_entry_t tunnel_mapper_entry);

    switcht_status_t
    switch_api_tunnel_mapper_entry_delete(
             1:switcht_device_t device,
             2:switcht_handle_t mapper_entry_handle);

    /* MPLS API */
    switcht_handle_t
    switch_api_mpls_tunnel_create(
             1:switcht_device_t device,
             2:switcht_api_mpls_info_t api_mpls_info);

    switcht_status_t
    switch_api_mpls_tunnel_delete(
             1:switcht_device_t device,
             2:switcht_handle_t mpls_handle);

    switcht_handle_t
    switch_api_mpls_label_stack_create(
             1:switcht_device_t device,
             2:switcht_mpls_label_stack_t label_stack);

    switcht_status_t
    switch_api_mpls_label_stack_delete(
             1:switcht_device_t device,
             2:switcht_handle_t label_stack_handle);

    /* Logical Network */
    switcht_handle_t
    switch_api_logical_network_create(
             1:switcht_device_t device,
             2:switcht_logical_network_t info);

    switcht_status_t
    switch_api_logical_network_delete(
             1:switcht_device_t device,
             2:switcht_handle_t network_handle);

    switcht_status_t
    switch_api_logical_network_member_add(
             1:switcht_device_t device,
             2:switcht_handle_t network_handle,
             3:switcht_handle_t interface_handle);

    switcht_status_t
    switch_api_logical_network_member_remove(
             1:switcht_device_t device,
             2:switcht_handle_t network_handle,
             3:switcht_handle_t interface_handle);

    switcht_status_t
    switch_api_logical_network_learning_set(
             1: switcht_device_t device,
             2: switcht_handle_t network_handle,
             3: bool enable);

    bool
    switch_api_logical_network_learning_get(
             1: switcht_device_t device,
             2: switcht_handle_t network_handle);

    i32
    switch_api_logical_network_bd_get(
             1: switcht_device_t device,
             2: switcht_handle_t network_handle);

    list<switcht_counter_t>
    switch_api_logical_network_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ln_handle,
                             3: list<i16> counter_ids);

    /* STP API */
    switcht_handle_t
    switch_api_stp_group_create(
                             1:switcht_device_t device,
                             2:switcht_stp_mode_t stp_mode);
    switcht_status_t
    switch_api_stp_group_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t stp_handle);
    switcht_status_t
    switch_api_stp_group_member_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle,
                             3: switcht_handle_t network_handle);
    switcht_status_t
    switch_api_stp_group_member_remove(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle,
                             3: switcht_handle_t network_handle);
    switcht_status_t
    switch_api_stp_port_state_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle,
                             3: switcht_handle_t intf_handle,
                             4:switcht_stp_state_t stp_state);
    switcht_stp_state_t
    switch_api_stp_port_state_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle,
                             3: switcht_handle_t intf_handle);
    list<switcht_handle_t>
    switch_api_stp_group_members_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle);
    list<switcht_handle_t>
    switch_api_stp_interfaces_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t stp_handle);

    /* NAT API */
    switcht_status_t switch_api_nat_create(1:switcht_device_t device, 2:switcht_nat_info_t nat_info);
    switcht_status_t switch_api_nat_delete(1:switcht_device_t device, 2:switcht_nat_info_t nat_info);

    /* ILA API */
    switcht_status_t
    switch_api_ila_add(
                   1: switcht_device_t device,
                   2: switcht_handle_t vrf_handle,
                   3: switcht_ip_addr_t sir,
                   4: switcht_ip_addr_t ila_addr,
                   5: switcht_handle_t nhop_handle);
    switcht_status_t
    switch_api_ila_update(
                   1: switcht_device_t device,
                   2: switcht_handle_t vrf_handle,
                   3: switcht_ip_addr_t sir,
                   4: switcht_ip_addr_t ila_addr,
                   5:switcht_handle_t nhop_handle);

    switcht_status_t switch_api_ila_delete(1:switcht_device_t device, 2:switcht_handle_t vrf_handle, 3:switcht_ip_addr_t sir);
    switcht_handle_t switch_api_ila_lookup(1:switcht_device_t device, 2:switcht_handle_t vrf_handle, 3:switcht_ip_addr_t sir);

    /* ACL API */
    switcht_handle_t switch_api_acl_list_create(
                             1:switcht_device_t device,
                             2:switcht_direction_t direction,
                             3:switcht_acl_type_t type,
                             4:switcht_handle_type_t bp_type);
    switcht_status_t switch_api_acl_list_delete(1:switcht_device_t device, 2:switcht_handle_t handle);
    switcht_handle_t switch_api_acl_mac_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_mac_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);
    switcht_handle_t switch_api_acl_ip_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ip_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);
    switcht_handle_t switch_api_acl_ipv6_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ipv6_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);
    switcht_handle_t switch_api_acl_ipracl_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ipracl_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);
    switcht_handle_t switch_api_acl_ipv6racl_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ipv6racl_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);
    switcht_handle_t switch_api_acl_system_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_system_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);

    switcht_handle_t switch_api_acl_ip_mirror_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ip_mirror_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);

    switcht_handle_t switch_api_acl_ipv6_mirror_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_ipv6_mirror_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);

    switcht_handle_t switch_api_acl_egress_system_rule_create(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:i32 priority,
                             4:i32 key_value_count,
                             5:list<switcht_acl_egress_system_key_value_pair_t> acl_kvp,
                             6:switcht_acl_action_t action,
                             7:switcht_acl_action_params_t action_params,
                             8:switcht_acl_opt_action_params_t opt_action_params);

    switcht_status_t switch_api_acl_rule_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);

    switcht_status_t switch_api_acl_entry_action_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t ace_handle,
                             3:i32 priority,
                             4:switcht_acl_action_t action,
                             5:switcht_acl_action_params_t action_params,
                             6:switcht_acl_opt_action_params_t opt_action_params);

    switcht_status_t switch_api_acl_entry_egress_system_action_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t ace_handle,
                             3:i32 priority,
                             4:switcht_acl_action_t action,
                             5:switcht_acl_action_params_t action_params,
                             6:switcht_acl_opt_action_params_t opt_action_params);

    switcht_status_t switch_api_acl_reference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_status_t switch_api_acl_dereference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_status_t switch_api_ingress_acl_reference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_status_t switch_api_ingress_acl_dereference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_status_t switch_api_egress_acl_reference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_status_t switch_api_egress_acl_dereference(
                             1:switcht_device_t device,
                             2:switcht_handle_t acl_handle,
                             3:switcht_handle_t handle);
    switcht_handle_t switch_api_acl_counter_create(
                             1: switcht_device_t device);
    switcht_status_t switch_api_acl_counter_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t counter_handle);
    switcht_counter_t switch_api_acl_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t counter_handle);
    switcht_acl_type_t switch_api_acl_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t acl_handle);
    switcht_range_type_t switch_api_acl_range_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t range_handle);
    switcht_range_t switch_api_acl_range_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t range_handle);
    switcht_acl_action_spec_t switch_api_acl_entry_action_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ace_handle);
    i16 switch_api_acl_entry_rules_count_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ace_handle);
    switcht_handle_t switch_api_acl_entry_acl_table_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ace_handle);
    switcht_handle_t switch_api_racl_counter_create(1 : switcht_device_t device);
    switcht_status_t switch_api_racl_counter_delete(
      1: switcht_device_t device,
      2: switcht_handle_t counter_handle);
    switcht_counter_t switch_api_racl_stats_get(
      1: switcht_device_t device,
      2: switcht_handle_t counter_handle);

    switcht_handle_t switch_api_egress_acl_counter_create(
      1: switcht_device_t device);

    switcht_status_t switch_api_egress_acl_counter_delete(
      1: switcht_device_t device,
      2: switcht_handle_t counter_handle);
    switcht_counter_t switch_api_egress_acl_stats_get(
      1: switcht_device_t device,
      2: switcht_handle_t counter_handle);

    switcht_handle_t switch_api_acl_range_create(
                             1: switcht_device_t device,
                             2: switcht_direction_t direction,
                             3: switcht_range_type_t range_type,
                             4: switcht_range_t range);
    switcht_status_t switch_api_acl_range_update(
                             1: switcht_device_t device,
                             2: switcht_handle_t range_handle,
                             3: switcht_range_t range);
    switcht_status_t switch_api_acl_range_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t range_handle);

    /* HOSTIF API */
    switcht_handle_t switch_api_hostif_group_create(
                             1:switcht_device_t device,
                             2:switcht_hostif_group_t hostif_group);
    switcht_status_t switch_api_hostif_group_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t hostif_group_handle);
    switcht_handle_t switch_api_hostif_reason_code_create(
                             1:switcht_device_t device,
                             2: switcht_uint64_t flags,
                             3:switcht_hostif_rcode_info_t rcode_api_info);
    switcht_status_t switch_api_hostif_reason_code_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t rcode_handle);
    switcht_handle_t switch_api_hostif_create(
                             1:switcht_device_t device,
                             2: switcht_uint64_t flags,
                             3:switcht_hostif_t hostif);
    switcht_status_t switch_api_hostif_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t hostif_handle);
    switcht_handle_t switch_api_hostif_rx_filter_create(
                             1: switcht_device_t device,
                             2: switcht_uint64_t flags,
                             3: i32 priority,
                             4: switcht_hostif_rx_filter_key_t rx_key,
                             5: switcht_hostif_rx_filter_action_t rx_action);
    switcht_status_t switch_api_hostif_rx_filter_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t filter_handle);
    switcht_handle_t switch_api_hostif_tx_filter_create(
                             1: switcht_device_t device,
                             2: switcht_uint64_t flags,
                             3: i32 priority,
                             4: switcht_hostif_tx_filter_key_t tx_key,
                             5: switcht_hostif_tx_filter_action_t tx_action);
    switcht_status_t switch_api_hostif_tx_filter_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t filter_handle);
    switcht_handle_t switch_api_hostif_meter_create(
                             1: switcht_device_t device,
                             2: switcht_meter_info_t api_meter_info);
    switcht_status_t switch_api_hostif_meter_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle);
    switcht_handle_t switch_api_hostif_nhop_get(
                             1: switcht_device_t device,
                             2: switcht_hostif_reason_code_t reason_code);
    switcht_handle_t switch_api_hostif_handle_get(
                             1: switcht_device_t device,
                             2: string intf_name);
    switcht_hostif_group_t switch_api_hostif_group_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t hostif_group_handle);
    bool switch_api_hostif_oper_state_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t hostif_handle);

    /* RPF group API */
    switcht_handle_t switch_api_rpf_create(
                             1: switcht_device_t device,
                             2: switcht_rpf_type_t rpf_type,
                             3: switcht_mcast_mode_t pim_mode);

    switcht_status_t switch_api_rpf_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t rpf_group_handle);

    switcht_status_t switch_api_rpf_member_add(
                             1: switcht_device_t device,
                             2: switcht_handle_t rpf_group_handle,
                             3: switcht_handle_t rif_handle);

    switcht_status_t switch_api_rpf_member_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t rpf_group_handle,
                             3: switcht_handle_t rif_handle);
    list<switcht_handle_t> switch_api_rpf_members_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t rpf_group_handle);

    /* Multicast API */
    switcht_handle_t switch_api_multicast_tree_create(
                             1:switcht_device_t device);
    switcht_status_t switch_api_multicast_tree_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle);

    switcht_status_t switch_api_multicast_member_add(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:list<switcht_mcast_member_t> mbrs);

    switcht_status_t switch_api_multicast_member_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:list<switcht_mcast_member_t> mbrs);

    switcht_status_t switch_api_multicast_ecmp_nhop_add(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t ecmp_nhop_handle);

    switcht_status_t switch_api_multicast_ecmp_nhop_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t ecmp_nhop_handle);

    switcht_status_t switch_api_multicast_mroute_add(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t rpf_handle,
                             4:switcht_handle_t vrf_handle,
                             5:switcht_ip_addr_t src_ip,
                             6:switcht_ip_addr_t grp_ip,
                             7:switcht_mcast_mode_t mc_mode);

    switcht_status_t switch_api_multicast_mroute_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle,
                             3:switcht_ip_addr_t src_ip,
                             4:switcht_ip_addr_t grp_ip);

    switcht_status_t switch_api_multicast_mroute_miss_mgid_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t vlan_handle);

    switcht_counter_t switch_api_multicast_mroute_stats_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle,
                             3:switcht_ip_addr_t src_ip,
                             4:switcht_ip_addr_t grp_ip);

    switcht_mroute_tree_t switch_api_multicast_mroute_tree_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t vrf_handle,
                             3:switcht_ip_addr_t src_ip,
                             4:switcht_ip_addr_t grp_ip);

    switcht_status_t switch_api_multicast_mroute_mgid_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t vrf_handle,
                             4:switcht_ip_addr_t src_ip,
                             5:switcht_ip_addr_t grp_ip,
                             6:switcht_mcast_mode_t mc_mode);

    switcht_status_t switch_api_multicast_mroute_rpf_set(
                             1:switcht_device_t device,
                             2:switcht_handle_t rpf_handle,
                             3:switcht_handle_t vrf_handle,
                             4:switcht_ip_addr_t src_ip,
                             5:switcht_ip_addr_t grp_ip,
                             6:switcht_mcast_mode_t mc_mode);

    switcht_status_t switch_api_multicast_l2route_add(
                             1:switcht_device_t device,
                             2:switcht_handle_t mgid_handle,
                             3:switcht_handle_t bd_handle,
                             4:switcht_ip_addr_t src_ip,
                             5:switcht_ip_addr_t grp_ip);

    switcht_status_t switch_api_multicast_l2route_delete(
                             1:switcht_device_t device,
                             2:switcht_handle_t bd_handle,
                             3:switcht_ip_addr_t src_ip,
                             4:switcht_ip_addr_t grp_ip);

    switcht_handle_t switch_api_multicast_l2route_tree_get(
                             1:switcht_device_t device,
                             2:switcht_handle_t vlan_handle,
                             3:switcht_ip_addr_t src_ip,
                             4:switcht_ip_addr_t grp_ip);

    /* MIRROR API */

    switcht_handle_t switch_api_mirror_session_create(
                            1:switcht_device_t device,
                            2:switcht_mirror_info_t api_mirror_info);

    switcht_status_t switch_api_mirror_session_update(
                            1:switcht_device_t device,
                            2:switcht_handle_t mirror_handle,
                            3:switcht_mirror_info_t api_mirror_info);

    switcht_status_t switch_api_mirror_session_delete(
                            1:switcht_device_t device,
                            2:switcht_handle_t mirror_handle);

    i16 switch_api_mirror_session_type_get(
                            1:switcht_device_t device,
                            2:switcht_handle_t mirror_handle);

    switcht_mirror_info_t switch_api_mirror_session_info_get(
                            1:switcht_device_t device,
                            2:switcht_handle_t mirror_handle);

    /* DTel shared API */
    switcht_status_t switch_api_dtel_switch_id_set(
                            1:switcht_device_t device,
                            2:i32 switch_id);

    switcht_status_t switch_api_dtel_report_session_add(
                            1:switcht_device_t device,
                            2:switcht_mirror_id_t mirror_id);
    switcht_status_t switch_api_dtel_report_session_delete(
                            1:switcht_device_t device,
                            2:switcht_mirror_id_t mirror_id);

    switcht_status_t switch_api_dtel_report_sequence_number_set(
                            1:switcht_device_t device,
                            2:i16 mirror_session_id,
                            3:i32 value);

    list<i32> switch_api_dtel_report_sequence_number_get(
                            1:switcht_device_t device,
                            2:i16 mirror_session_id,
                            3:byte max_num);

    switcht_status_t switch_api_dtel_queue_report_create(
                            1:switcht_device_t device,
                            2:i16 port,
                            3:i16 queue,
                            4:i32 depth_threshold,
                            5:i32 latency_threshold,
                            6:i16 report_quota_during_breach,
                            7:bool report_tail_drops);
    switcht_status_t switch_api_dtel_queue_report_update(
                            1:switcht_device_t device,
                            2:i16 port,
                            3:i16 queue,
                            4:i32 depth_threshold,
                            5:i32 latency_threshold,
                            6:i16 report_quota_during_breach,
                            7:bool report_tail_drops);
    switcht_status_t switch_api_dtel_queue_report_delete(
                            1:switcht_device_t device,
                            2:i16 port,
                            3:i16 queue);
    i16 switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                            1:switcht_device_t device,
                            2:i16 port,
                            3:i16 queue);

    switcht_status_t switch_api_dtel_flow_state_clear_cycle(
                            1:switcht_device_t device,
                            2:i16 cycle);

    switcht_status_t switch_api_dtel_latency_quantization_shift(
                            1:switcht_device_t device,
                            2:byte quant_shift);

    switcht_status_t switch_api_dtel_report_udp_dstport_set(
                            1:switcht_device_t device,
                            2:i16 dest_udp_port);

    byte switch_api_dtel_event_get_dscp(
                            1:switcht_device_t device,
                            2:switcht_dtel_event_type_t event_type);

    switcht_status_t switch_api_dtel_event_set_dscp(
                            1:switcht_device_t device,
                            2:switcht_dtel_event_type_t event_type,
                            3:byte dscp);

    /* DTel INT API */
    switcht_status_t switch_api_dtel_int_transit_enable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_int_transit_disable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_int_endpoint_enable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_int_endpoint_disable(
                            1:switcht_device_t device);

    switcht_status_t switch_api_dtel_int_session_create(
                            1:switcht_device_t device,
                            2:i16 session_id,
                            3:i16 instruction,
                            4:byte max_hop);
    switcht_status_t switch_api_dtel_int_session_update(
                            1:switcht_device_t device,
                            2:i16 session_id,
                            3:i16 instruction,
                            4:byte max_hop);
    switcht_status_t switch_api_dtel_int_session_delete(
                            1:switcht_device_t device,
                            2:i16 session_id);

    switcht_status_t switch_api_dtel_int_sink_ports_add(
                            1:switcht_device_t device,
                            2:i16 port);
    switcht_status_t switch_api_dtel_int_sink_ports_delete(
                            1:switcht_device_t device,
                            2:i16 port);

    switcht_status_t switch_api_dtel_int_watchlist_entry_create(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_int_params_t action_params);
    switcht_status_t switch_api_dtel_int_watchlist_entry_update(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_int_params_t action_params);
    switcht_status_t switch_api_dtel_int_watchlist_entry_delete(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp);
    switcht_status_t switch_api_dtel_int_watchlist_clear(
                            1:switcht_device_t device)
    switcht_status_t switch_api_dtel_int_dscp_value_set(
                             1:switcht_device_t device,
                             2:byte value,
                             3:byte mask);

    switcht_status_t switch_api_dtel_int_marker_set(
                             1:switcht_device_t device,
                             2:byte proto,
                             3:i64 marker);
    i64 switch_api_dtel_int_marker_get(
                             1:switcht_device_t device,
                             2:byte proto);
    switcht_status_t switch_api_dtel_int_marker_delete(
                             1:switcht_device_t device,
                             2:byte proto);
    switcht_status_t switch_api_dtel_int_marker_port_add(
        1:switcht_device_t device,
        2:byte proto,
        3:i16 value,
        4:i16 mask);

    switcht_status_t switch_api_dtel_int_marker_port_delete(
        1:switcht_device_t device,
        2:byte proto,
        3:i16 value,
        4:i16 mask);

    switcht_status_t switch_api_dtel_int_marker_port_clear(
        1:switcht_device_t device,
        2:byte proto);

    /* DTel Postcard API */
    switcht_status_t switch_api_dtel_postcard_enable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_postcard_disable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_postcard_watchlist_entry_create(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_postcard_params_t action_params);
    switcht_status_t switch_api_dtel_postcard_watchlist_entry_update(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_postcard_params_t action_params);
    switcht_status_t switch_api_dtel_postcard_watchlist_entry_delete(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp);
    switcht_status_t switch_api_dtel_postcard_watchlist_clear(
                            1:switcht_device_t device)

    /* DTel Mirror on Drop API */
    switcht_status_t switch_api_dtel_drop_report_enable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_drop_report_disable(
                            1:switcht_device_t device);
    switcht_status_t switch_api_dtel_drop_watchlist_entry_create(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_mod_params_t action_params);
    switcht_status_t switch_api_dtel_drop_watchlist_entry_update(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp,
                            3:i32 priority,
                            4:bool watch,
                            5:switcht_twl_mod_params_t action_params);
    switcht_status_t switch_api_dtel_drop_watchlist_entry_delete(
                            1:switcht_device_t device,
                            2:list<switcht_twl_key_value_pair_t> twl_kvp);
    switcht_status_t switch_api_dtel_drop_watchlist_clear(
                            1:switcht_device_t device)

    /* WRED APIs */
    switcht_handle_t switch_api_wred_create(
                             1: switcht_device_t device,
                             2: switcht_wred_info_t api_wred_info)

    switcht_status_t switch_api_wred_update(
                             1: switcht_device_t device,
                             2: switcht_handle_t wred_handle,
                             3: switcht_wred_info_t api_wred_info);

    switcht_status_t switch_api_wred_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t wred_handle);

    switcht_wred_info_t switch_api_wred_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t wred_handle);

    switcht_status_t switch_api_wred_attach(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle,
                             3: switcht_meter_counter_t packet_color,
                             4: switcht_handle_t wred_handle);

    switcht_status_t switch_api_wred_detach(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle,
                             3: switcht_meter_counter_t packet_color);

    list<switcht_counter_t> switch_api_wred_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle,
                             3: list<i16> counter_ids);

    switcht_status_t switch_api_wred_stats_clear(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle,
                             3: list<i16> counter_id);

    switcht_wred_profile_info_t switch_api_wred_profile_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t profile_handle);

    /* Meter APS */
    switcht_handle_t switch_api_meter_create(
                             1: switcht_device_t device,
                             2: switcht_meter_info_t api_meter_info);

    switcht_status_t switch_api_meter_update(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle,
                             3: switcht_uint64_t flags,
                             4: switcht_meter_info_t api_meter_info);

    switcht_status_t switch_api_meter_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle);

    switcht_meter_info_t switch_api_meter_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle);

    list<switcht_counter_t> switch_api_meter_stats_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t meter_handle,
                             3: list<i16> counter_ids);

    /* Global config */
    switcht_status_t switch_api_flowlet_switching_set(
                             1:switcht_device_t device,
                             2:i32 enable_flowlet);
    switcht_status_t switch_api_set_switch_id(1:switcht_device_t device, 2:i32 switch_id);
    list<switcht_handle_t> switch_api_handles_get(1:switcht_device_t device,
                                                  2:switcht_handle_type_t type);

    /* SFLOW APIs */
    switcht_handle_t switch_api_sflow_session_create(1:switcht_device_t device, 2:switcht_sflow_info_t api_sflow_info);

    switcht_status_t switch_api_sflow_session_delete(1:switcht_device_t device, 2:switcht_handle_t sflow_hdl, 3:bool all_cleanup);

    switcht_handle_t switch_api_sflow_session_attach(
                             1:switcht_device_t device,
                             2:switcht_handle_t sflow_handle,
                             3:switcht_direction_t direction,
                             4:i32 priority,
                             5:i32 sample_rate,
                             6:list<switcht_sflow_key_value_pair_t> sflow_kvp);

    switcht_status_t switch_api_sflow_session_detach(
                             1:switcht_device_t device,
                             2:switcht_handle_t sflow_handle
                             3:switcht_handle_t entry_hdl);
    /* BFD APIs */
    switcht_handle_t switch_api_bfd_session_create(1:switcht_device_t device, 2:switcht_bfd_session_info_t api_bfd_info);
    switcht_status_t switch_api_bfd_session_delete(1:switcht_device_t device, 2:switcht_handle_t bfd_hdl);

    /* PPG */
    switcht_status_t switch_api_port_cos_mapping(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle,
                             3: switcht_handle_t ppg_handle,
                             4: byte cos_bmp);
    switcht_status_t switch_api_ppg_lossless_enable(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle,
                             3: bool enabled);
    list<switcht_handle_t> switch_api_ppg_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t port_handle);
    switcht_status_t switch_api_ppg_guaranteed_limit_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle,
                             3: i32 num_bytes);
    switcht_status_t switch_api_ppg_skid_limit_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle,
                             3: i32 num_bytes);
    switcht_status_t switch_api_ppg_skid_hysteresis_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle,
                             3: i32 num_bytes);

    /* Buffer */
    switcht_handle_t switch_api_buffer_pool_create(
                             1: switcht_device_t device,
                             2: switcht_buffer_pool_t api_buffer_pool);
    switcht_status_t switch_api_buffer_pool_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t buffer_pool_handle);
    switcht_handle_t switch_api_buffer_profile_create(
                             1: switcht_device_t device,
                             2: switcht_buffer_profile_t api_buffer_info);
    switcht_status_t switch_api_buffer_profile_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t buffer_profile_handle);
    switcht_status_t switch_api_ppg_buffer_profile_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle,
                             3: switcht_handle_t buffer_profile_handle);
    switcht_status_t switch_api_queue_buffer_profile_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle,
                             3: switcht_handle_t buffer_profile_handle);
    switcht_status_t switch_api_buffer_skid_limit_set(
                             1: switcht_device_t device,
                             2: i32 num_bytes);
    switcht_status_t switch_api_buffer_skid_hysteresis_set(
                             1: switcht_device_t device,
                             2: i32 num_bytes);
    switcht_status_t switch_api_buffer_pool_pfc_limit(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle,
                             3: byte icos,
                             4: i32 num_bytes);
    switcht_status_t switch_api_buffer_pool_color_drop_enable(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle,
                             3: bool enable);
    switcht_status_t switch_api_buffer_pool_color_limit_set(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle,
                             3: switcht_color_t color,
                             4: i32 num_bytes);
    switcht_status_t switch_api_buffer_pool_color_hysteresis_set(
                             1: switcht_device_t device,
                             2: switcht_color_t color,
                             3: i32 num_bytes);
    i16 switch_api_buffer_pool_threshold_mode_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle);
    i32 switch_api_buffer_pool_size_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle);
    switcht_direction_t switch_api_buffer_pool_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle);
    i32 switch_api_buffer_pool_xoff_size_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t pool_handle);
    switcht_handle_t switch_api_ppg_buffer_profile_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle);
    switcht_handle_t switch_api_queue_buffer_profile_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle);
    switcht_handle_t switch_api_priority_group_port_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle);
    i64 switch_api_queue_drop_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t queue_handle);
    i32 switch_api_priority_group_index_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t ppg_handle);
    switcht_buffer_profile_t switch_api_buffer_profile_info_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t profile_handle);
    i32 switch_api_buffer_pool_usage_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t profile_handle);
    i16 switch_api_max_ingress_pool_get(
                             1: switcht_device_t device);
    i16 switch_api_max_egress_pool_get(
                             1: switcht_device_t device);
    i64 switch_api_total_buffer_size_get(
                             1: switcht_device_t device);

    /* Qos */
    switcht_handle_t switch_api_qos_map_ingress_create(
                             1: switcht_device_t device,
                             2: switcht_qos_map_type_t qos_map_type,
                             3: list<switcht_qos_map_t> qos_map);
    switcht_status_t switch_api_qos_map_ingress_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);
    switcht_handle_t switch_api_qos_map_egress_create(
                             1: switcht_device_t device,
                             2: switcht_qos_map_type_t qos_map_type,
                             3: list<switcht_qos_map_t> qos_map);
    switcht_status_t switch_api_qos_map_egress_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);
    i16 switch_api_qos_map_dir_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);
    i16 switch_api_qos_map_ig_map_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);
    i16 switch_api_qos_map_eg_map_type_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);
    list<switcht_qos_map_t> switch_api_qos_map_list_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t qos_map_handle);

    /* Scheduler */
    switcht_handle_t switch_api_scheduler_create(
                             1: switcht_device_t device,
                             2: switcht_scheduler_info_t api_scheduler_info);
    switcht_status_t switch_api_scheduler_delete(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_handle);
    switcht_scheduler_info_t switch_api_scheduler_config_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_handle);
    switcht_handle_t switch_api_scheduler_group_child_handle_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_group_handle);
    i32 switch_api_scheduler_group_child_count_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_group_handle);
    switcht_handle_t switch_api_scheduler_group_profile_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_group_handle);
    switcht_scheduler_group_info_t switch_api_scheduler_group_config_get(
                             1: switcht_device_t device,
                             2: switcht_handle_t scheduler_group_handle);

    /* Queues */
    list<switcht_handle_t> switch_api_queues_get(
                            1: switcht_device_t device,
                            2: switcht_handle_t port_handle);
    switcht_status_t switch_api_queue_color_drop_enable(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle,
                            3: bool enable);
    switcht_status_t switch_api_queue_color_limit_set(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle,
                            3: switcht_color_t color,
                            4: i32 limit);
    switcht_status_t switch_api_queue_color_hysteresis_set(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle,
                            3: switcht_color_t color,
                            4: i32 limit);
    switcht_status_t switch_api_queue_pfc_cos_mapping(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle,
                            3: byte cos);
    switcht_status_t switch_api_dtel_tail_drop_deflection_queue_set(
                            1: switcht_device_t device,
			                2: switcht_pipe_t pipe,
                            3: switcht_handle_t queue_handle);
    i32 switch_api_max_queues_get(
                            1: switcht_device_t device);
    i32 switch_api_max_cpu_queues_get(
                            1: switcht_device_t device);
    i32 switch_api_max_traffic_class_get(
                            1: switcht_device_t device);
    i16 switch_api_queue_index_get(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle);
    switcht_handle_t switch_api_queue_port_get(
                            1: switcht_device_t device,
                            2: switcht_handle_t queue_handle);

    /* MTU */
    switcht_handle_t switch_api_l3_mtu_create(
                            1: switcht_device_t device,
                            2: i64 flags,
                            3: i32 mtu);

    switcht_status_t switch_api_l3_mtu_update(
                            1: switcht_device_t device,
                            2: switcht_handle_t mtu_handle,
                            3: i32 mtu);

    switcht_status_t switch_api_l3_mtu_delete(
                            1: switcht_device_t device,
                            2: switcht_handle_t mtu_handle);

    i16 switch_api_l3_mtu_get(
                            1: switcht_device_t device,
                            2: switcht_handle_t mtu_handle);

    /* Perf Test */
    i32 switch_api_route_entry_add_perf_test(
                             1: switcht_device_t device,
                             2: list<switcht_route_entry_t> route_entries);

    i32 switch_api_mac_entry_add_perf_test(
                             1: switcht_device_t device,
                             2: list<switcht_api_mac_entry_t> mac_entries);

  switcht_status_t switch_api_ipv6_hash_input_fields_set(
			     1: switcht_device_t device,
			     2: i32 fields);
  switcht_status_t switch_api_ipv4_hash_input_fields_set(
			     1: switcht_device_t device,
			     2: i32 fields);
  switcht_status_t switch_api_non_ip_hash_input_fields_set(
			     1: switcht_device_t device,
			     2: i32 fields);

 switcht_status_t switch_api_ipv6_hash_algorithm_set(
			     1: switcht_device_t device,
			     2: i32 algorithm);

 switcht_status_t switch_api_ipv4_hash_algorithm_set(
			     1: switcht_device_t device,
			     2: i32 algorithm);

 switcht_status_t switch_api_non_ip_hash_algorithm_set(
			     1: switcht_device_t device,
			     2: i32 algorithm);

 switcht_status_t switch_api_ipv6_hash_input_fields_attribute_set(
			     1: switcht_device_t device, 2: i32 fields, 3: i32 attr_flags);

 switcht_status_t switch_api_ipv4_hash_input_fields_attribute_set(
			     1: switcht_device_t device, 2: i32 fields, 3: i32 attr_flags);

 switcht_status_t switch_api_non_ip_hash_input_fields_attribute_set(
			     1: switcht_device_t device, 2: i32 fields, 3: i32 attr_flags);
 void switch_api_lag_hash_seed_set(
                             1: switcht_device_t device,
                             2: i64 seed) throws(1: InvalidSwitchOperation ouch);

 void switch_api_ecmp_hash_seed_set(
                             1: switcht_device_t device,
                             2: i64 seed) throws(1: InvalidSwitchOperation ouch);

 i64 switch_api_lag_hash_seed_get(
                             1: switcht_device_t device) throws(1: InvalidSwitchOperation ouch);

 i64 switch_api_ecmp_hash_seed_get(
                             1: switcht_device_t device) throws(1: InvalidSwitchOperation ouch);

 switcht_status_t switch_api_ipv6_hash_seed_set(
			     1: switcht_device_t device,
			     2: i64 seed);
 switcht_status_t switch_api_ipv4_hash_seed_set(
			     1: switcht_device_t device,
			     2: i64 seed);
 switcht_status_t switch_api_non_ip_hash_seed_set(
			     1: switcht_device_t device,
			     2: i64 seed);
 switch_hash_ipv6_input_fields_res_t switch_api_ipv6_hash_input_fields_get(
			     1: switcht_device_t device);

 switch_hash_ipv4_input_fields_res_t switch_api_ipv4_hash_input_fields_get(
			     1: switcht_device_t device);

 switch_hash_non_ip_input_fields_res_t switch_api_non_ip_hash_input_fields_get(
			     1: switcht_device_t device);

 switch_hash_input_fields_attribute_res_t switch_api_ipv6_hash_input_fields_attribute_get(
			     1: switcht_device_t device, 2: i32 fields);

 switch_hash_input_fields_attribute_res_t switch_api_ipv4_hash_input_fields_attribute_get(
			     1: switcht_device_t device, 2: i32 fields);

 switch_hash_input_fields_attribute_res_t switch_api_non_ip_hash_input_fields_attribute_get(
			     1: switcht_device_t device, 2: i32 fields);

 switch_hash_ipv6_algo_res_t switch_api_ipv6_hash_algorithm_get(
			     1: switcht_device_t device);

 switch_hash_ipv4_algo_res_t switch_api_ipv4_hash_algorithm_get(
			     1: switcht_device_t device);

 switch_hash_non_ip_algo_res_t switch_api_non_ip_hash_algorithm_get(
			     1: switcht_device_t device);

 switch_hash_ipv6_seed_res_t switch_api_ipv6_hash_seed_get(
			     1: switcht_device_t device);

 switch_hash_ipv4_seed_res_t switch_api_ipv4_hash_seed_get(
			     1: switcht_device_t device);

 switch_hash_non_ip_seed_res_t switch_api_non_ip_hash_seed_get(
			     1: switcht_device_t device);
}
