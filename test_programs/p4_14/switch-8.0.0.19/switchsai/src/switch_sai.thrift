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

namespace py switch_sai namespace cpp switch_sai

/*
Copyright 2013-present Barefoot Networks, Inc.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

namespace py switch_sai
namespace cpp switch_sai

typedef i64 sai_thrift_object_id_t
typedef i64 sai_thrift_uint64_t
typedef i16 sai_thrift_vlan_id_t
typedef string sai_thrift_mac_t
typedef byte sai_thrift_vlan_tagging_mode_t
typedef i32 sai_thrift_status_t
typedef string sai_thrift_ip4_t
typedef string sai_thrift_ip6_t
typedef byte sai_thrift_ip_addr_family_t
typedef byte sai_thrift_port_stp_port_state_t
typedef i32 sai_thrift_hostif_trap_id_t
typedef i32 sai_thrift_next_hop_type_t
typedef i32 sai_thrift_vlan_stat_counter_t
typedef i32 sai_thrift_bridge_stat_counter_t
typedef i32 sai_thrift_bridge_port_stat_counter_t
typedef i32 sai_thrift_policer_stat_counter_t
typedef i32 sai_thrift_port_stat_counter_t
typedef i32 sai_thrift_queue_stat_counter_t
typedef i32 sai_thrift_pg_stat_counter_t
typedef i32 sai_thrift_policer_stat_t


struct sai_thrift_fdb_entry_t {
    1: sai_thrift_mac_t mac_address;
    2: sai_thrift_vlan_id_t vlan_id;
    3: i16 bridge_type;
    4: sai_thrift_object_id_t bridge_id;
}

struct sai_thrift_vlan_port_t {
    1: sai_thrift_object_id_t port_id;
    2: sai_thrift_vlan_tagging_mode_t tagging_mode;
}

union sai_thrift_ip_t {
    1: sai_thrift_ip4_t ip4;
    2: sai_thrift_ip6_t ip6;
}

struct sai_thrift_ip_address_t {
    1: sai_thrift_ip_addr_family_t addr_family;
    2: sai_thrift_ip_t addr;
}

struct sai_thrift_ip_prefix_t {
    1: sai_thrift_ip_addr_family_t addr_family;
    2: sai_thrift_ip_t addr;
    3: sai_thrift_ip_t mask;
}

struct sai_thrift_object_list_t {
    1: i32 count;
    2: list<sai_thrift_object_id_t> object_id_list;
}

struct sai_thrift_vlan_list_t {
    1: i32 vlan_count;
    2: list<sai_thrift_vlan_id_t> vlan_list;
}

struct sai_thrift_s32_list_t {
    1: i32 count;
    2: list<i32> s32list;
}

struct sai_thrift_ipaddr_list_t {
    1: i32 count;
    2: list<sai_thrift_ip_address_t> ipaddr_list;
}

union sai_thrift_acl_mask_t {
    1: byte u8;
    2: byte s8;
    3: i16 u16;
    4: i16 s16;
    5: i32 u32;
    6: i32 s32;
    7: sai_thrift_mac_t mac;
    8: sai_thrift_ip4_t ip4;
    9: sai_thrift_ip6_t ip6;
}

union sai_thrift_acl_data_t {
    1: byte u8;
    2: byte s8;
    3: i16 u16;
    4: i16 s16;
    5: i32 u32;
    6: i32 s32;
    7: sai_thrift_mac_t mac;
    8: sai_thrift_ip4_t ip4;
    9: sai_thrift_ip6_t ip6;
    10: sai_thrift_object_id_t oid;
    11: sai_thrift_object_list_t objlist;
}

struct sai_thrift_acl_field_data_t
{
    1: bool enable;
    2: sai_thrift_acl_mask_t mask;
    3: sai_thrift_acl_data_t data;
}

union sai_thrift_acl_parameter_t {
    1: bool booldata;
    2: byte u8;
    3: byte s8;
    4: i16 u16;
    5: i16 s16;
    6: i32 u32;
    7: i32 s32;
    8: sai_thrift_mac_t mac;
    9: sai_thrift_ip4_t ip4;
    10: sai_thrift_ip6_t ip6;
    11: sai_thrift_object_id_t oid;
}

struct sai_thrift_acl_action_data_t {
    1: bool enable;
    2: sai_thrift_acl_parameter_t parameter;
}

struct sai_thrift_u32_list_t {
    1: i32 count;
    2: list<i32> u32list;
}

struct sai_thrift_qos_map_params_t {
    1: byte tc;
    2: byte dscp;
    3: byte dot1p;
    4: byte prio;
    5: byte pg;
    6: byte queue_index;
    7: byte color;
}

struct sai_thrift_qos_map_t {
    1: sai_thrift_qos_map_params_t key;
    2: sai_thrift_qos_map_params_t value;
}

struct sai_thrift_qos_map_list_t {
  1 : list<sai_thrift_qos_map_params_t> key;
  2 : list<sai_thrift_qos_map_params_t> data;
}

struct sai_thrift_range_t {
    1: i32 min;
    2: i32 max;
}

union sai_thrift_attribute_value_t {
    1:  bool booldata;
    2:  string chardata;
    3:  byte u8;
    4:  byte s8;
    5:  i16 u16;
    6:  i16 s16;
    7:  i32 u32;
    8:  i32 s32;
    9:  i64 u64;
    10: i64 s64;
    11: sai_thrift_mac_t mac;
    12: sai_thrift_object_id_t oid;
    13: sai_thrift_ip4_t ip4;
    14: sai_thrift_ip6_t ip6;
    15: sai_thrift_ip_address_t ipaddr;
    16: sai_thrift_object_list_t objlist;
    17: sai_thrift_vlan_list_t vlanlist;
    18: sai_thrift_acl_field_data_t aclfield;
    19: sai_thrift_acl_action_data_t aclaction;
    20: sai_thrift_u32_list_t u32list;
    21: sai_thrift_s32_list_t s32list;
    22: sai_thrift_qos_map_list_t qosmap;
    23: sai_thrift_range_t s32range;
    24: sai_thrift_range_t u32range;
    25: sai_thrift_ipaddr_list_t ipaddrlist;
}

struct sai_thrift_attribute_t {
    1: i32 id;
    2: sai_thrift_attribute_value_t value;
}

struct sai_thrift_route_entry_t {
    1: sai_thrift_object_id_t vr_id;
    2: sai_thrift_ip_prefix_t destination;
}

struct sai_thrift_neighbor_entry_t {
    1: sai_thrift_object_id_t rif_id;
    2: sai_thrift_ip_address_t ip_address;
}

struct sai_thrift_attribute_list_t {
    1: list<sai_thrift_attribute_t> attr_list;
    2: i32 attr_count; // redundant
}

union sai_thrift_result_data_t {
    1: sai_thrift_object_list_t objlist;
    2: sai_thrift_object_id_t oid;
    3: i16 u16;
}

struct sai_thrift_result_t {
    1: sai_thrift_result_data_t data;
    2: sai_thrift_status_t status;
}

service switch_sai_rpc {
    //port API
    sai_thrift_status_t sai_thrift_set_port_attribute(1: sai_thrift_object_id_t port_id, 2: sai_thrift_attribute_t thrift_attr);
    sai_thrift_attribute_list_t sai_thrift_get_port_attribute(1: sai_thrift_object_id_t port_id);
    list<i64> sai_thrift_get_port_stats(
                             1: sai_thrift_object_id_t port_id,
                             2: list<sai_thrift_port_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);
    sai_thrift_status_t sai_thrift_clear_port_all_stats(1: sai_thrift_object_id_t port_id)

    sai_thrift_attribute_t sai_thrift_get_port_handles_attribute(
        1: sai_thrift_object_id_t port_id, 2: sai_thrift_attribute_t attr);

    //fdb API
    sai_thrift_status_t sai_thrift_create_fdb_entry(1: sai_thrift_fdb_entry_t thrift_fdb_entry, 2: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_delete_fdb_entry(1: sai_thrift_fdb_entry_t thrift_fdb_entry);
    sai_thrift_status_t sai_thrift_flush_fdb_entries(1: list <sai_thrift_attribute_t> thrift_attr_list);

    //vlan API
    sai_thrift_object_id_t sai_thrift_create_vlan(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_vlan(1: sai_thrift_object_id_t vlan_oid);
    list<i64> sai_thrift_get_vlan_stats(
                             1: sai_thrift_vlan_id_t vlan_id,
                             2: list<sai_thrift_vlan_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);
    sai_thrift_object_id_t sai_thrift_create_vlan_member(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_attribute_list_t sai_thrift_get_vlan_member_attribute(1: sai_thrift_object_id_t vlan_member_id);
    sai_thrift_status_t sai_thrift_remove_vlan_member(1: sai_thrift_object_id_t vlan_member_id);
    sai_thrift_attribute_list_t sai_thrift_get_vlan_attribute(1: sai_thrift_object_id_t vlan_id);
    sai_thrift_result_t sai_thrift_get_vlan_id(1: sai_thrift_object_id_t vlan_id);

    //virtual router API
    sai_thrift_object_id_t sai_thrift_create_virtual_router(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_virtual_router(1: sai_thrift_object_id_t vr_id);

    //route API
    sai_thrift_status_t sai_thrift_create_route(1: sai_thrift_route_entry_t thrift_route_entry, 2: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_route(1: sai_thrift_route_entry_t thrift_route_entry);

    //router interface API
    sai_thrift_object_id_t sai_thrift_create_router_interface(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_router_interface(1: sai_thrift_object_id_t rif_id);
    sai_thrift_status_t sai_thrift_set_router_interface_attribute(1: sai_thrift_object_id_t rif_id, 2: sai_thrift_attribute_t thrift_attr);

    //next hop API
    sai_thrift_object_id_t sai_thrift_create_next_hop(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_next_hop(1: sai_thrift_object_id_t next_hop_id);

    // Next Hop Group API.
    sai_thrift_object_id_t sai_thrift_create_next_hop_group(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_next_hop_group(1: sai_thrift_object_id_t nhop_group_oid);
    sai_thrift_object_id_t sai_thrift_create_next_hop_group_member(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_next_hop_group_member(1: sai_thrift_object_id_t nhop_group_member_oid);

    //lag API
    sai_thrift_object_id_t sai_thrift_create_lag(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_lag(1: sai_thrift_object_id_t lag_id);
    sai_thrift_object_id_t sai_thrift_create_lag_member(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_lag_member(1: sai_thrift_object_id_t lag_member_id);
    sai_thrift_attribute_list_t sai_thrift_get_lag_member_attribute(1: sai_thrift_object_id_t lag_member_id);

    //stp API
    sai_thrift_object_id_t sai_thrift_create_stp_entry(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_stp_entry(1: sai_thrift_object_id_t stp_id);
    sai_thrift_status_t sai_thrift_set_stp_port_state(1: sai_thrift_object_id_t stp_id, 2: sai_thrift_object_id_t port_id, 3: sai_thrift_port_stp_port_state_t stp_port_state);
    sai_thrift_port_stp_port_state_t sai_thrift_get_stp_port_state(1: sai_thrift_object_id_t stp_id, 2: sai_thrift_object_id_t port_id);

    //neighbor API
    sai_thrift_status_t sai_thrift_create_neighbor_entry(1: sai_thrift_neighbor_entry_t thrift_neighbor_entry, 2: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_neighbor_entry(1: sai_thrift_neighbor_entry_t thrift_neighbor_entry);

    //switch API
    sai_thrift_attribute_list_t sai_thrift_get_switch_attribute();
    sai_thrift_attribute_t sai_thrift_get_switch_attribute_by_id(i32 id);
    sai_thrift_attribute_t sai_thrift_get_port_list_by_front_port();
    sai_thrift_object_id_t sai_thrift_get_cpu_port_id();
    sai_thrift_object_id_t sai_thrift_get_default_trap_group();
    sai_thrift_object_id_t sai_thrift_get_default_router_id();
    sai_thrift_object_id_t sai_thrift_get_default_1q_bridge_id();
    sai_thrift_result_t sai_thrift_get_default_vlan_id();
    sai_thrift_object_id_t sai_thrift_get_port_id_by_front_port(1: string port_name);
    sai_thrift_status_t sai_thrift_set_switch_attribute(1: sai_thrift_attribute_t attribute);

    //bridge API
    sai_thrift_object_id_t sai_thrift_create_bridge(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_bridge(1: sai_thrift_object_id_t bridge_id);
    sai_thrift_result_t sai_thrift_create_bridge_port(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_bridge_port(1: sai_thrift_object_id_t bridge_port_id);
    sai_thrift_result_t sai_thrift_get_bridge_port_list(1: sai_thrift_object_id_t bridge_id);
    sai_thrift_attribute_list_t sai_thrift_get_bridge_port_attribute(1: sai_thrift_object_id_t bridge_port_id);
    sai_thrift_status_t sai_thrift_set_bridge_port_attribute(1: sai_thrift_object_id_t bridge_port_id,
                                                             2: sai_thrift_attribute_t thrift_attr);
    list<i64> sai_thrift_get_bridge_stats(
                             1: sai_thrift_object_id_t bridge_id,
                             2: list<sai_thrift_bridge_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);

    list<i64> sai_thrift_get_bridge_port_stats(
                             1: sai_thrift_object_id_t bridge_port_id,
                             2: list<sai_thrift_bridge_port_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);

    //Trap API
    sai_thrift_object_id_t sai_thrift_create_hostif(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_hostif(1: sai_thrift_object_id_t thrift_hif_id);
    sai_thrift_status_t sai_thrift_set_hostif_attribute(1: sai_thrift_object_id_t thrift_hif_id,
                                                        2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_hostif_trap_group(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_hostif_trap_group(1: sai_thrift_object_id_t thrift_hostif_trap_group_id);
    sai_thrift_status_t sai_thrift_set_hostif_trap_group_attribute(1: sai_thrift_object_id_t thrift_hostif_trap_group_id,
                                                                   2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_hostif_trap(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_hostif_trap(1: sai_thrift_object_id_t thrift_hostif_trap_id);
    sai_thrift_status_t sai_thrift_set_hostif_trap_attribute(1: sai_thrift_object_id_t thrift_hostif_trap_id,
                                                             2: sai_thrift_attribute_t thrift_attr);
    sai_thrift_object_id_t sai_thrift_create_hostif_table_entry(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_hostif_table_entry(1: sai_thrift_object_id_t hif_table_entry_id);

  // ACL API

    // ACL API
    sai_thrift_object_id_t sai_thrift_create_acl_table(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_acl_table(1: sai_thrift_object_id_t acl_table_id);

    sai_thrift_object_id_t sai_thrift_create_acl_entry(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_set_acl_entry_attribute(1 : sai_thrift_object_id_t acl_entry_id,
                                                           2: sai_thrift_attribute_t thrift_attr);
    sai_thrift_status_t sai_thrift_remove_acl_entry(1: sai_thrift_object_id_t acl_entry);

    sai_thrift_object_id_t sai_thrift_create_acl_table_group(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_acl_table_group(1: sai_thrift_object_id_t acl_table_group_id);

    sai_thrift_object_id_t sai_thrift_create_acl_table_group_member(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_acl_table_group_member(1: sai_thrift_object_id_t acl_table_group_member_id);

    sai_thrift_object_id_t sai_thrift_create_acl_counter(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_acl_counter(1: sai_thrift_object_id_t acl_counter_id);
    list<sai_thrift_attribute_value_t> sai_thrift_get_acl_counter_attribute(
                             1: sai_thrift_object_id_t acl_counter_id,
                             2: list<i32> thrift_attr_ids);
    sai_thrift_object_id_t sai_thrift_create_acl_range(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_delete_acl_range(1: sai_thrift_object_id_t acl_range_id);

    // Mirror API
    sai_thrift_object_id_t sai_thrift_create_mirror_session(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_mirror_session(1: sai_thrift_object_id_t session_id);
    sai_thrift_status_t sai_thrift_set_mirror_attribute(1: sai_thrift_object_id_t thrift_mirror_id,
                                                         2: sai_thrift_attribute_t thrift_attr);

    // Policer API
    sai_thrift_object_id_t sai_thrift_create_policer(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_policer(1: sai_thrift_object_id_t thrift_policer_id);

    sai_thrift_status_t sai_thrift_set_policer_attribute(1: sai_thrift_object_id_t thrift_policer_id,
                                                         2: sai_thrift_attribute_t thrift_attr);
    list<sai_thrift_uint64_t> sai_thrift_get_policer_stats(1: sai_thrift_object_id_t thrift_policer_id,
                                                           2: list<sai_thrift_policer_stat_t> thrift_counter_ids);

    sai_thrift_status_t sai_thrift_clear_policer_stats(1: sai_thrift_object_id_t thrift_policer_id,
                                                       2: list<sai_thrift_policer_stat_t> thrift_counter_ids);

    // Scheduler API
    sai_thrift_object_id_t sai_thrift_create_scheduler_profile(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_scheduler_profile(1: sai_thrift_object_id_t scheduler_id);
    sai_thrift_status_t sai_thrift_set_scheduler_group(
        1: sai_thrift_object_id_t scheduler_group_id, 2: sai_thrift_attribute_t attr);

    sai_thrift_attribute_t sai_thrift_get_scheduler_profile(
        1: sai_thrift_object_id_t scheduler_id, 2: sai_thrift_attribute_t attr);

    sai_thrift_attribute_t sai_thrift_get_scheduler_group_attribute(
        1: sai_thrift_object_id_t group_id, 2: sai_thrift_attribute_t attr);

    sai_thrift_status_t sai_thrift_set_scheduler_profile(
        1: sai_thrift_object_id_t scheduler_id, 2: sai_thrift_attribute_t attr);

    // Queue API
    list<i64> sai_thrift_get_queue_stats(
                             1: sai_thrift_object_id_t queue_id,
                             2: list<sai_thrift_queue_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);
    sai_thrift_status_t sai_thrift_clear_queue_stats(
                             1: sai_thrift_object_id_t queue_id,
                             2: list<sai_thrift_queue_stat_counter_t> counter_ids,
                             3: i32 number_of_counters);
    sai_thrift_status_t sai_thrift_set_queue_attribute(1: sai_thrift_object_id_t queue_id,
                                                       2: sai_thrift_attribute_t thrift_attr)

    // Buffer API
    sai_thrift_object_id_t sai_thrift_create_buffer_profile(1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_object_id_t sai_thrift_create_pool_profile(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_attribute_t sai_thrift_get_buffer_pool_attribute(
        1: sai_thrift_object_id_t pool_id, 2: sai_thrift_attribute_t thrift_attr);
    sai_thrift_status_t sai_thrift_remove_buffer_pool(1: sai_thrift_object_id_t pool_id);

    sai_thrift_status_t sai_thrift_set_priority_group_attribute(1: sai_thrift_object_id_t pg_id,
                                                                2: sai_thrift_attribute_t thrift_attr)
    list<i64> sai_thrift_get_pg_stats(
                         1: sai_thrift_object_id_t pg_id,
                         2: list<sai_thrift_pg_stat_counter_t> counter_ids,
                         3: i32 number_of_counters);

    // WRED API
    sai_thrift_object_id_t sai_thrift_create_wred_profile(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_wred_profile(1: sai_thrift_object_id_t wred_id);

    // QoS Map API
    sai_thrift_object_id_t sai_thrift_create_qos_map(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_attribute_t sai_thrift_get_qos_map(
        1: sai_thrift_object_id_t qos_map_id,
        2: i32 map_count,
        3: sai_thrift_attribute_t thrift_attr);
    sai_thrift_status_t sai_thrift_set_qos_map(1: sai_thrift_object_id_t qos_map, 2: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_qos_map(1: sai_thrift_object_id_t qos_map_id);

    // DTEL API
    sai_thrift_object_id_t sai_thrift_create_dtel(
        1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_dtel(
        1: sai_thrift_object_id_t dtel_id);

    sai_thrift_attribute_list_t sai_thrift_get_dtel_attribute(
        1: sai_thrift_object_id_t dtel_id,
        2: list<i32> thrift_attr_ids);

    sai_thrift_status_t sai_thrift_set_dtel_attribute(
        1: sai_thrift_object_id_t dtel_id,
        2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_dtel_queue_report(
        1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_dtel_queue_report(
        1: sai_thrift_object_id_t dtel_queue_report_id);

    sai_thrift_attribute_list_t sai_thrift_get_dtel_queue_report_attribute(
        1: sai_thrift_object_id_t dtel_queue_report_id,
        2: list<i32> thrift_attr_ids);

    sai_thrift_status_t sai_thrift_set_dtel_queue_report_attribute(
        1: sai_thrift_object_id_t dtel_queue_report_id,
        2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_dtel_int_session(
        1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_dtel_int_session(
        1: sai_thrift_object_id_t dtel_int_session_id);

    sai_thrift_attribute_list_t sai_thrift_get_dtel_int_session_attribute(
        1: sai_thrift_object_id_t dtel_int_session_id,
        2: list<i32> thrift_attr_ids);

    sai_thrift_status_t sai_thrift_set_dtel_int_session_attribute(
        1: sai_thrift_object_id_t dtel_int_session_id,
        2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_dtel_report_session(
        1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_dtel_report_session(
        1: sai_thrift_object_id_t dtel_report_session_id);

    sai_thrift_object_id_t sai_thrift_create_dtel_event(
        1: list<sai_thrift_attribute_t> thrift_attr_list);

    sai_thrift_status_t sai_thrift_remove_dtel_event(
        1: sai_thrift_object_id_t dtel_event_id);

    sai_thrift_status_t sai_thrift_set_dtel_event_attribute(
        1: sai_thrift_object_id_t dtel_event_id,
        2: sai_thrift_attribute_t thrift_attr);

    //Tunnel API
    sai_thrift_object_id_t sai_thrift_create_tunnel_map(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_tunnel_map(1: sai_thrift_object_id_t tunnel_map_id);
    list<sai_thrift_attribute_t> sai_thrift_get_tunnel_map(1: sai_thrift_object_id_t tunnel_map_id, 2: list<i32> thrift_attr_ids);
    sai_thrift_status_t sai_thrift_set_tunnel_map(1: sai_thrift_object_id_t tunnel_map_id, 2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_tunnel(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_tunnel(1: sai_thrift_object_id_t tunnel_id);
    list<sai_thrift_attribute_t> sai_thrift_get_tunnel(1: sai_thrift_object_id_t tunnel_id, 2: list<i32> thrift_attr_ids);
    sai_thrift_status_t sai_thrift_set_tunnel(1: sai_thrift_object_id_t tunnel_id, 2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_tunnel_term(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_tunnel_term(1: sai_thrift_object_id_t tunnel_term_id);
    list<sai_thrift_attribute_t> sai_thrift_get_tunnel_term(1: sai_thrift_object_id_t tunnel_term_id, 2: list<i32> thrift_attr_ids);
    sai_thrift_status_t sai_thrift_set_tunnel_term(1: sai_thrift_object_id_t tunnel_term_id, 2: sai_thrift_attribute_t thrift_attr);

    sai_thrift_object_id_t sai_thrift_create_tunnel_map_entry(1: list<sai_thrift_attribute_t> thrift_attr_list);
    sai_thrift_status_t sai_thrift_remove_tunnel_map_entry(1: sai_thrift_object_id_t tunnel_map_entry_id);
    list<sai_thrift_attribute_t> sai_thrift_get_tunnel_map_entry(1: sai_thrift_object_id_t tunnel_map_entry_id, 2: list<i32> thrift_attr_ids);
    sai_thrift_status_t sai_thrift_set_tunnel_map_entry(1: sai_thrift_object_id_t tunnel_map_entry_id, 2: sai_thrift_attribute_t thrift_attr);

}
