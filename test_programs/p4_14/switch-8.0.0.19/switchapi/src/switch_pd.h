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

#ifndef _switch_pd_api_
#define _switch_pd_api_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Batch APIs */

switch_status_t switch_pd_batch_begin();

switch_status_t switch_pd_batch_end(bool hw_synchronous);

/* Dmac table PD API's */
switch_status_t switch_pd_dmac_table_entry_add(
    switch_device_t device,
    switch_handle_type_t handle_type,
    switch_bd_t bd,
    switch_mac_info_t *mac_info,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_t nhop_index,
    switch_mgid_t mgid_index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_dmac_table_entry_update(
    switch_device_t device,
    switch_handle_type_t handle_type,
    switch_bd_t bd,
    switch_mac_info_t *mac_info,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_t nhop_index,
    switch_mgid_t mgid_index,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_dmac_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl);

/* Smac table PD API's */
switch_status_t switch_pd_smac_table_entry_add(switch_device_t device,
                                               switch_bd_t bd,
                                               switch_mac_info_t *mac_info,
                                               switch_ifindex_t ifindex,
                                               switch_uint32_t aging_time,
                                               switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_smac_table_entry_update(switch_device_t device,
                                                  switch_bd_t bd,
                                                  switch_mac_info_t *mac_info,
                                                  switch_ifindex_t ifindex,
                                                  switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_smac_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_learn_callback_register(switch_device_t device,
                                                      void *client_data);
switch_status_t switch_pd_mac_learn_callback_deregister(switch_device_t device);

switch_status_t switch_pd_mac_aging_callback_register(
    switch_device_t device,
    switch_uint32_t min_aging_time,
    switch_uint32_t max_aging_time,
    switch_uint32_t query_interval,
    void *client_data);

switch_status_t switch_pd_smac_hit_state_get(switch_device_t device,
                                             switch_pd_hdl_t pd_hdl,
                                             bool *is_hit);

switch_status_t switch_pd_mac_table_set_learning_timeout(switch_device_t device,
                                                         uint32_t timeout);

switch_status_t switch_pd_nexthop_table_entry_add(
    switch_device_t device,
    switch_nhop_t nhop_index,
    switch_bd_t bd,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_pd_action_t pd_action,
    switch_mgid_t mc_index,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_nexthop_table_entry_update(
    switch_device_t device,
    switch_nhop_t nhop_index,
    switch_bd_t bd,
    switch_ifindex_t ifindex,
    switch_port_lag_index_t port_lag_index,
    switch_nhop_pd_action_t pd_action,
    switch_mgid_t mc_index,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_nexthop_table_entry_delete(switch_device_t device,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ecmp_group_create(switch_device_t device,
                                            switch_pd_grp_hdl_t *group_hdl);

switch_status_t switch_pd_ecmp_group_delete(switch_device_t device,
                                            switch_pd_grp_hdl_t group_hdl);

switch_status_t switch_pd_ecmp_member_add(switch_device_t device,
                                          switch_pd_grp_hdl_t pd_grp_hdl,
                                          switch_nhop_t nhop_index,
                                          switch_spath_info_t *spath_info,
                                          switch_pd_mbr_hdl_t *pd_mbr_hdl);

switch_status_t switch_pd_ecmp_member_update(switch_device_t device,
                                             switch_pd_grp_hdl_t pd_grp_hdl,
                                             switch_nhop_t nhop_index,
                                             switch_spath_info_t *spath_info,
                                             switch_pd_mbr_hdl_t pd_mbr_hdl);

switch_status_t switch_pd_ecmp_member_delete(switch_device_t device,
                                             switch_pd_grp_hdl_t group_hdl,
                                             switch_pd_mbr_hdl_t mbr_hdl);

switch_status_t switch_pd_ecmp_group_table_with_selector_add(
    switch_device_t device,
    switch_ecmp_t ecmp_index,
    switch_pd_grp_hdl_t switch_group_hdl,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_wcmp_group_create(switch_device_t device,
                                            switch_wcmp_t wcmp_index,
                                            switch_pd_grp_hdl_t *entry_hdl,
                                            switch_pd_mbr_hdl_t *mbr_hdl);

switch_status_t switch_pd_wcmp_member_add(switch_device_t device,
                                          switch_nhop_t nhop_index,
                                          switch_wcmp_t wcmp_index,
                                          switch_uint8_t start,
                                          switch_uint8_t end,
                                          switch_spath_info_t *spath_info,
                                          switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_wcmp_group_delete(switch_device_t device,
                                            switch_pd_mbr_hdl_t mbr_hdl,
                                            switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_wcmp_member_delete(switch_device_t device,
                                             switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ecmp_group_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ip_fib_entry_add(switch_device_t device,
                                           switch_vrf_id_t vrf,
                                           switch_ip_addr_t *ipaddr,
                                           bool ecmp,
                                           switch_nhop_t nexthop,
                                           switch_route_type_t type,
                                           switch_pd_hdl_t *entry_hdl,
                                           uint32_t flags);

switch_status_t switch_pd_ip_fib_entry_update(switch_device_t device,
                                              switch_vrf_id_t vrf,
                                              switch_ip_addr_t *ipaddr,
                                              bool ecmp,
                                              switch_nhop_t nexthop,
                                              switch_pd_hdl_t entry_hdl,
                                              uint32_t flags);

switch_status_t switch_pd_ip_fib_entry_delete(switch_device_t device,
                                              switch_ip_addr_t *ip_addr,
                                              switch_pd_hdl_t entry_hdl,
                                              uint32_t flags);

switch_status_t switch_pd_inner_rmac_table_entry_add(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_inner_rmac_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_outer_rmac_table_entry_add(
    switch_device_t device,
    switch_rmac_group_t rmac_group,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_outer_rmac_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_ip_src_table_entry_add(
    switch_device_t device,
    switch_vrf_t vrf,
    const switch_ip_addr_t *ip_addr,
    switch_tunnel_type_ingress_t tunnel_type,
    switch_ifindex_t ifindex,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_ip_src_table_entry_delete(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_ip_dst_table_entry_add(
    switch_device_t device,
    switch_vrf_t vrf,
    const switch_ip_addr_t *ip_addr,
    switch_tunnel_type_ingress_t tunnel_type,
    switch_tunnel_term_entry_type_t tunnel_term_type,
    switch_vni_t tunnel_vni,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_ip_dst_table_entry_delete(
    switch_device_t device,
    switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_rewrite_table_entry_add(
    const switch_device_t device,
    const switch_tunnel_t tunnel_index,
    const switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rewrite_table_fabric_entry_add(
    switch_device_t device,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_table_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_vni_t tunnel_vni,
    switch_rid_t rid,
    switch_tunnel_pd_type_t pd_type,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_table_entry_delete(switch_device_t device,
                                                    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_vni_table_entry_add(
    switch_device_t device,
    switch_bd_t egress_bd,
    switch_vni_t tunnel_vni,
    switch_tunnel_pd_type_t pd_type,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_vni_table_entry_delete(
    switch_device_t device_id, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_ip_dst_rewrite_table_entry_add(
    const switch_device_t device,
    const switch_id_t tunnel_dst_index,
    const switch_ip_addr_t *ip_addr,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_ip_dst_rewrite_table_entry_delete(
    const switch_device_t device, const switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_smac_rewrite_table_entry_add(
    switch_device_t device,
    switch_id_t smac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_smac_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_dmac_rewrite_table_entry_add(
    switch_device_t device,
    switch_id_t dmac_index,
    switch_mac_addr_t *mac,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_dmac_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_bd_table_entry_add(switch_device_t device,
                                             switch_bd_t bd,
                                             switch_bd_info_t *bd_info,
                                             switch_pd_mbr_hdl_t *pd_mbr_hdl);

switch_status_t switch_pd_bd_table_entry_update(switch_device_t device,
                                                switch_bd_t bd,
                                                switch_bd_info_t *bd_info,
                                                switch_pd_mbr_hdl_t pd_mbr_hdl);

switch_status_t switch_pd_bd_table_entry_delete(switch_device_t device,
                                                switch_bd_info_t *bd_info);

switch_status_t switch_pd_egress_bd_table_entry_add(switch_device_t device,
                                                    switch_bd_t bd,
                                                    switch_bd_info_t *bd_info,
                                                    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_bd_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_bd_info_t *bd_info,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_bd_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_outer_bd_table_entry_add(
    switch_device_t device,
    switch_bd_t outer_bd,
    switch_bd_info_t *outer_bd_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_outer_bd_table_entry_update(
    switch_device_t device,
    switch_bd_t outer_bd,
    switch_bd_info_t *outer_bd_info,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_outer_bd_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_bd_flood_table_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_packet_type_t packet_type,
    bool flood_to_mrouters,
    switch_mgid_t mgid,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_bd_flood_table_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_packet_type_t packet_type,
    bool flood_to_mrouters,
    switch_mgid_t mgid,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_bd_flood_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_port_vlan_to_ifindex_mapping_table_entry_add(
    switch_device_t device,
    switch_port_lag_index_t port_lag_index,
    switch_vlan_t inner_vlan,
    switch_vlan_t outer_vlan,
    switch_ifindex_t ifindex,
    switch_rid_t ingress_rid,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_port_vlan_to_ifindex_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_port_vlan_to_bd_mapping_table_entry_add(
    switch_device_t device,
    switch_ifindex_t ifindex,
    switch_vlan_t inner_vlan,
    switch_vlan_t outer_vlan,
    switch_pd_mbr_hdl_t bd_hdl,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_port_vlan_to_bd_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_vlan_xlate_table_entry_add(
    switch_device_t device,
    switch_ifindex_t egress_ifindex,
    switch_bd_t bd,
    switch_vlan_t inner_vlan,
    switch_vlan_t outer_vlan,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_vlan_xlate_table_entry_update(
    switch_device_t device,
    switch_ifindex_t egress_ifindex,
    switch_bd_t bd,
    switch_vlan_t inner_vlan,
    switch_vlan_t outer_vlan,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_vlan_xlate_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_port_add(switch_device_t device,
                                   switch_dev_port_t dev_port,
                                   switch_port_speed_t port_speed);

switch_status_t switch_pd_port_enable(switch_device_t device,
                                      switch_dev_port_t dev_port);

switch_status_t switch_pd_port_delete(switch_device_t device,
                                      switch_dev_port_t dev_port);

switch_status_t switch_pd_port_disable(switch_device_t device,
                                       switch_dev_port_t dev_port);

switch_status_t switch_pd_port_stats_get(switch_device_t device,
                                         switch_dev_port_t dev_port,
                                         switch_uint16_t num_entries,
                                         switch_port_counter_id_t *counter_ids,
                                         switch_uint64_t *counters);

switch_status_t switch_pd_ingress_port_mapping_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_index_t port_lag_index,
    switch_port_type_t port_type,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ingress_port_mapping_table_entry_update(
    switch_device_t device,
    switch_port_t port_id,
    switch_port_lag_index_t port_lag_index,
    switch_port_type_t port_type,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ingress_port_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ingress_port_properties_table_entry_add(
    switch_device_t device,
    switch_yid_t yid,
    switch_port_info_t *port_info,
    switch_port_lag_label_t port_lag_label,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ingress_port_properties_table_entry_update(
    switch_device_t device,
    switch_yid_t yid,
    switch_port_info_t *port_info,
    switch_port_lag_label_t port_lag_label,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ingress_port_properties_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_port_mapping_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_label_t port_lag_label,
    switch_port_type_t port_type,
    switch_qos_group_t qos_group,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_port_mapping_table_entry_update(
    switch_device_t device,
    switch_port_t port_id,
    switch_port_lag_label_t port_lag_label,
    switch_port_type_t port_type,
    switch_qos_group_t qos_group,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_port_mapping_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ecmp_member_activate(switch_device_t device,
                                               switch_pd_grp_hdl_t pd_group_hdl,
                                               switch_pd_mbr_hdl_t *mbr_hdl);

switch_status_t switch_pd_ecmp_member_deactivate(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_group_hdl,
    switch_pd_mbr_hdl_t *mbr_hdl);

/*
 * Rewrite table
 */
switch_status_t switch_pd_rewrite_table_unicast_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_rw_type_t rw_type,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rewrite_table_unicast_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_rw_type_t rw_type,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_rewrite_table_tunnel_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_api_neighbor_info_t *api_neighbor_info,
    switch_tunnel_t tunnel_id,
    switch_vni_t tunnel_vni,
    switch_id_t tunnel_dst_index,
    switch_tunnel_type_egress_t tunnel_type,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rewrite_table_tunnel_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_mac_addr_t dmac,
    switch_neighbor_type_t neigh_type,
    switch_neighbor_rw_type_t rw_type,
    switch_tunnel_t tunnel_id,
    switch_vni_t tunnel_vni,
    switch_id_t tunnel_dst_index,
    switch_tunnel_type_egress_t tunnel_type,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_rewrite_table_entry_delete(switch_device_t device,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_lag_group_create(switch_device_t device,
                                           switch_pd_grp_hdl_t *pd_grp_hdl);

switch_status_t switch_pd_lag_group_delete(switch_device_t device,
                                           switch_pd_grp_hdl_t pd_group_hdl);

switch_status_t switch_pd_lag_member_add(switch_device_t device,
                                         switch_pd_grp_hdl_t pd_grp_hdl,
                                         switch_dev_port_t dev_port,
                                         switch_pd_mbr_hdl_t *pd_mbr_hdl);

switch_status_t switch_pd_lag_member_delete(switch_device_t device,
                                            switch_pd_grp_hdl_t pd_grp_hdl,
                                            switch_pd_mbr_hdl_t pd_mbr_hdl);

switch_status_t switch_pd_lag_group_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_lag_index_t port_lag_index,
    switch_pd_mbr_hdl_t *pd_mbr_hdl,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_lag_group_table_with_selector_add(
    switch_device_t device,
    switch_port_lag_index_t port_lag_index,
    switch_pd_grp_hdl_t pd_grp_hdl,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_lag_group_table_entry_delete(
    switch_device_t device,
    bool port,
    switch_pd_hdl_t entry_hdl,
    switch_pd_mbr_hdl_t mbr_hdl);

switch_status_t switch_pd_smac_rewrite_table_entry_add(
    switch_device_t device,
    switch_smac_entry_t *smac_entry,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_smac_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_nat_init(switch_device_t device);

switch_status_t switch_pd_nat_table_entry_add(switch_device_t device,
                                              switch_nat_info_t *nat_info,
                                              switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_nat_table_entry_delete(switch_device_t device,
                                                 switch_nat_info_t *nat_info,
                                                 switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_nat_rewrite_table_entry_add(
    switch_device_t device,
    switch_nat_info_t *nat_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_nat_rewrite_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mcast_egress_ifindex_table_entry_add(
    switch_device_t device,
    switch_rid_t rid,
    switch_ifindex_t ifindex,
    switch_pd_hdl_t *entry_hdl);
switch_status_t switch_pd_mcast_egress_ifindex_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_rid_table_entry_add(
    switch_device_t device,
    switch_rid_type_t rid_type,
    switch_rid_t rid,
    switch_bd_t bd,
    switch_tunnel_type_egress_t tunnel_type,
    switch_tunnel_t tunnel_index,
    switch_id_t dmac_index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rid_table_entry_delete(switch_device_t device,
                                                 switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mcast_table_entry_add(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_mode_t mc_mode,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_mrpf_group_t mrpf_group);

switch_status_t switch_pd_mcast_table_entry_delete(
    switch_device_t device,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry);

switch_status_t switch_pd_mcast_table_entry_update(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_mode_t mc_mode,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_mrpf_group_t mrpf_group);

switch_status_t switch_pd_mcast_table_entry_stats_get(
    switch_device_t device,
    switch_mcast_group_info_t *group_info,
    bool core_entry,
    bool vrf_entry,
    switch_counter_t *counter);

switch_status_t switch_pd_multicast_rpf_entry_add(
    switch_device_t device,
    switch_rpf_type_t rpf_type,
    switch_mrpf_group_t mrpf_group,
    switch_bd_t bd,
    switch_pd_hdl_t *pd_hdl);

switch_status_t switch_pd_multicast_rpf_entry_delete(switch_device_t device,
                                                     switch_rpf_type_t rpf_type,
                                                     switch_pd_hdl_t pd_hdl);

switch_status_t switch_pd_prune_mask_table_update(
    switch_device_t device, switch_yid_t yid, switch_mc_port_map_t port_map);

switch_status_t switch_pd_spanning_tree_table_entry_add(
    switch_device_t device,
    switch_stp_group_t stp_group,
    switch_ifindex_t ifindex,
    switch_stp_state_t stp_state,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_spanning_tree_table_entry_update(
    switch_device_t device,
    switch_stp_group_t stp_group,
    switch_ifindex_t ifindex,
    switch_stp_state_t stp_state,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_spanning_tree_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_urpf_bd_table_entry_add(
    switch_device_t device,
    switch_urpf_group_t urpf_group,
    switch_bd_t bd,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_urpf_bd_table_entry_delete(switch_device_t device,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_urpf_entry_add(switch_device_t device,
                                         switch_vrf_t vrf_id,
                                         switch_ip_addr_t *ip_addr,
                                         switch_urpf_group_t urpf_group,
                                         switch_pd_hdl_t *entry_hdl,
                                         uint32_t flags);

switch_status_t switch_pd_urpf_entry_update(switch_device_t device,
                                            switch_vrf_t vrf_id,
                                            switch_ip_addr_t *ip_addr,
                                            switch_urpf_group_t urpf_group,
                                            switch_pd_hdl_t entry_hdl,
                                            uint32_t flags);

switch_status_t switch_pd_urpf_entry_delete(switch_device_t device,
                                            switch_vrf_id_t vrf_id,
                                            switch_ip_addr_t *ip_addr,
                                            switch_pd_hdl_t entry_hdl,
                                            uint32_t flags);

switch_status_t switch_pd_mcast_global_rid_set(switch_device_t device,
                                               switch_rid_t rid);

switch_status_t switch_pd_mcast_mgrp_tree_create(
    switch_device_t device,
    switch_mgid_t mgid_index,
    switch_mcast_info_t *mcast_info);

switch_status_t switch_pd_mcast_mgrp_tree_delete(
    switch_device_t device, switch_mcast_info_t *mcast_info);

switch_status_t switch_pd_mcast_entry_add(switch_device_t device,
                                          switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_entry_update(switch_device_t device,
                                             switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_entry_delete(switch_device_t device,
                                             switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_mgid_table_entry_add(switch_device_t device,
                                                     mc_mgrp_hdl_t mgid,
                                                     switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_mgid_table_entry_delete(
    switch_device_t device, mc_mgrp_hdl_t mgid_hdl, switch_mcast_node_t *node);

switch_status_t switch_pd_lag_mcast_port_map_update(
    switch_device_t device,
    switch_lag_t lag_index,
    switch_mc_port_map_t port_map);

switch_status_t switch_pd_mcast_ecmp_group_create(switch_device_t device,
                                                  switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_ecmp_group_delete(switch_device_t device,
                                                  switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_ecmp_entry_add(switch_device_t device,
                                               switch_mcast_node_t *ecmp_node,
                                               switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_ecmp_entry_remove(
    switch_device_t device,
    switch_mcast_node_t *ecmp_node,
    switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_mgid_table_ecmp_entry_add(
    switch_device_t device,
    switch_pd_hdl_t mgrp_hdl,
    switch_mcast_node_t *node);

switch_status_t switch_pd_mcast_mgid_table_ecmp_entry_remove(
    switch_device_t device,
    switch_pd_hdl_t mgrp_hdl,
    switch_mcast_node_t *node);

switch_status_t switch_pd_mpls_table_entry_add(
    switch_device_t device,
    switch_mpls_tunnel_type_ingress_t ingress_tunnel_type,
    switch_mpls_tunnel_subtype_ingress_t mpls_tunnel_type,
    switch_bd_t bd,
    switch_api_mpls_info_t *tunnel_info,
    switch_bd_info_t *bd_info,
    switch_mpls_label_t label,
    switch_ifindex_t egress_ifindex,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_mpls_table_entry_update(
    switch_device_t device,
    switch_mpls_tunnel_subtype_ingress_t mpls_tunnel_type,
    switch_bd_t bd,
    switch_api_mpls_info_t *tunnel_info,
    switch_bd_info_t *bd_info,
    switch_mpls_label_t label,
    switch_ifindex_t egress_ifindex,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mpls_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_rewrite_table_mpls_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_uint16_t num_labels,
    switch_mpls_t *mpls_header,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_rewrite_table_mpls_udp_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_id_t smac_index,
    switch_id_t dmac_index,
    switch_id_t sip_index,
    switch_id_t dip_index,
    switch_uint8_t header_count,
    switch_mpls_t *mpls_header,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rewrite_table_mpls_rewrite_entry_add(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_tunnel_t tunnel_index,
    switch_neighbor_tunnel_type_t neigh_type,
    switch_mac_addr_t dmac,
    switch_mpls_label_t mpls_label,
    switch_uint8_t header_count,
    switch_id_t tunnel_dmac_index,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_rewrite_table_mpls_rewrite_entry_update(
    switch_device_t device,
    switch_bd_t bd,
    switch_nhop_t nhop_index,
    switch_tunnel_t tunnel_index,
    switch_neighbor_tunnel_type_t neigh_type,
    switch_mac_addr_t dmac,
    switch_mpls_label_t label,
    switch_uint8_t header_count,
    switch_id_t tunnel_dmac_index,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_ipv4_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv4_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_ipv4_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ip_key_value_pair_t *ip_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_ipv6_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_ipv6_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv6_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_key_value_pair_t *ipv6_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_racl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_racl_key_value_pair_t *ip_racl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv4_racl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_racl_key_value_pair_t *ip_racl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_racl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_racl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_racl_key_value_pair_t *ip_racl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv6_racl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_racl_key_value_pair_t *ip_racl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_racl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_mirror_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_mirror_acl_key_value_pair_t *ip_mirror_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_mirror_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_qos_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv4_qos_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ip_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ip_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv4_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_qos_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ipv6_qos_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ipv6_qos_acl_key_value_pair_t *ip_qos_acl,
    switch_acl_ipv6_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ipv6_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_mac_qos_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_mac_qos_acl_key_value_pair_t *mac_acl,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_mac_acl_table_entry_add(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_mac_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_qos_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_mac_qos_acl_key_value_pair_t *mac_acl,
    switch_acl_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_mac_acl_table_entry_update(
    switch_device_t device,
    switch_uint16_t priority,
    switch_int32_t count,
    switch_acl_mac_key_value_pair_t *mac_acl,
    switch_acl_mac_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_qos_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_system_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_system_key_value_pair_t *system_acl,
    switch_acl_system_action_t action_type,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_system_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_system_key_value_pair_t *system_acl,
    switch_acl_system_action_t action_type,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_system_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_egress_system_key_value_pair_t *egr_acl,
    switch_acl_egress_system_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_egress_system_key_value_pair_t *egr_acl,
    switch_acl_egress_system_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ecn_acl_table_entry_add(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ecn_key_value_pair_t *ecn_acl,
    switch_acl_ecn_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_ecn_acl_table_entry_update(
    switch_device_t device,
    uint16_t priority,
    unsigned int count,
    switch_acl_ecn_key_value_pair_t *ecn_acl,
    switch_acl_ecn_action_t action,
    switch_acl_action_params_t *action_params,
    switch_acl_opt_action_params_t *opt_action_params,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_ecn_acl_table_entry_delete(
    switch_device_t device,
    switch_direction_t direction,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_bd_stats_get(switch_device_t device,
                                       switch_bd_stats_t *bd_stats);

switch_status_t switch_pd_bd_stats_clear(switch_device_t device,
                                         switch_bd_stats_t *bd_stats);

switch_status_t switch_pd_drop_stats_get(switch_device_t device,
                                         switch_uint32_t num_counters,
                                         switch_uint64_t *counters);

switch_status_t switch_pd_ingress_fabric_table_entry_add(
    switch_device_t device, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_lag_member_activate(switch_device_t device,
                                              switch_pd_grp_hdl_t pd_group_hdl,
                                              switch_pd_mbr_hdl_t mbr_hdl);

switch_status_t switch_pd_lag_member_deactivate(
    switch_device_t device,
    switch_pd_grp_hdl_t pd_group_hdl,
    switch_pd_mbr_hdl_t mbr_hdl);

// Default Entries
switch_status_t switch_pd_ip_mcast_default_entry_add(switch_device_t device);

switch_status_t switch_pd_validate_outer_ethernet_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_validate_outer_ip_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_outer_rmac_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_src_vtep_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_dest_vtep_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_validate_packet_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_port_vlan_to_bd_mapping_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_port_vlan_to_ifindex_mapping_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_acl_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_inner_rmac_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_fwd_result_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_nexthop_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_flowlet_default_entry_add(switch_device_t device);

switch_status_t switch_pd_lag_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_rid_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_replica_type_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_mac_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_egress_bd_map_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_ip_fib_default_entry_add(switch_device_t device);

switch_status_t switch_pd_ip_urpf_default_entry_add(switch_device_t device);

switch_status_t switch_pd_tunnel_smac_rewrite_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_tunnel_dmac_rewrite_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_tunnel_rewrite_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_fwd_result_table_entry_init(switch_device_t device);

switch_status_t switch_pd_rewrite_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_egress_vlan_xlate_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_cpu_rewrite_default_entry_add(switch_device_t device);

switch_status_t switch_pd_egress_acl_default_entry_add(switch_device_t device);

switch_status_t switch_pd_vlan_decap_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_tunnel_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_adjust_lkp_fields_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_bd_stats_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_bd_flood_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_mtu_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_l3_rewrite_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_learn_notify_table_entry_init(switch_device_t device);

switch_status_t switch_pd_tunnel_encap_table_entry_init(switch_device_t device);

switch_status_t switch_pd_tunnel_decap_table_entry_init(switch_device_t device);

switch_status_t switch_pd_validate_outer_ethernet_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_vlan_decap_table_entry_init(switch_device_t device);

switch_status_t switch_pd_validate_mpls_packet_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_egress_filter_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_fabric_header_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_egress_port_mapping_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_compute_hashes_entry_init(switch_device_t device);

switch_status_t switch_pd_switch_config_params_update(
    switch_device_t device, switch_config_params_t *config_params);

switch_status_t switch_pd_replica_type_table_entry_init(switch_device_t device);

switch_status_t switch_pd_l3_rewrite_table_entry_init(switch_device_t device);

switch_status_t switch_pd_sflow_tables_init(switch_device_t device);

// mirroring apis
switch_status_t switch_pd_mirror_session_update(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_dev_port_t dev_port,
    switch_mirror_info_t *mirror_info);

switch_status_t switch_pd_mirror_session_delete(switch_device_t device,
                                                switch_handle_t mirror_handle);

switch_status_t switch_pd_mirror_table_entry_add(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_info_t *mirror_info);

switch_status_t switch_pd_mirror_table_entry_delete(
    switch_device_t device, switch_mirror_info_t *mirror_info);

switch_status_t switch_pd_mirror_table_entry_update(
    switch_device_t device,
    switch_handle_t mirror_handle,
    switch_mirror_info_t *mirror_info);

switch_status_t switch_pd_mirror_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_storm_control_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_storm_control_meter_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info);

switch_status_t switch_pd_storm_control_table_entry_add(
    switch_device_t device,
    switch_dev_port_t dev_port,
    uint16_t priority,
    switch_packet_type_t pkt_type,
    switch_meter_id_t meter_idx,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_storm_control_table_entry_update(
    switch_device_t device,
    switch_port_t port,
    switch_uint16_t priority,
    switch_packet_type_t pkt_type,
    switch_meter_id_t meter_idx,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_storm_control_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_meter_index_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_meter_index_table_default_entry_delete(
    switch_device_t device);

switch_status_t switch_pd_meter_index_table_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_meter_index_table_entry_update(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_meter_index_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_meter_action_table_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_meter_action_table_default_entry_delete(
    switch_device_t device);

switch_status_t switch_pd_meter_action_table_entry_add(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_meter_action_table_entry_update(
    switch_device_t device,
    switch_meter_id_t meter_idx,
    switch_meter_info_t *meter_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_meter_action_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_sflow_ingress_table_add(
    switch_device_t device,
    switch_sflow_match_key_t *match_key,
    switch_ifindex_t ifindex,
    switch_uint16_t priority,
    switch_uint32_t sample_rate,
    switch_sflow_info_t *sflow_info,
    switch_sflow_match_entry_t *entry);

switch_status_t switch_pd_sflow_ingress_table_delete(
    switch_device_t device, switch_sflow_match_entry_t *match_entry);

switch_status_t switch_pd_sflow_session_create(switch_device_t device,
                                               switch_sflow_info_t *sflow_info);

switch_status_t switch_pd_sflow_session_delete(switch_device_t device,
                                               switch_sflow_info_t *sflow_info);

switch_status_t switch_pd_stats_update(switch_device_t device);

switch_status_t switch_pd_meter_counters_get(switch_device_t device,
                                             switch_meter_info_t *meter_info);

switch_status_t switch_pd_storm_control_stats_get(
    switch_device_t device, switch_meter_info_t *meter_info);

switch_status_t switch_pd_egress_bd_stats_table_entry_add(
    switch_device_t device, switch_bd_t bd, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_bd_stats_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_outer_bd_stats_table_entry_add(
    switch_device_t device, switch_bd_t bd, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_egress_outer_bd_stats_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_acl_stats_get(switch_device_t device,
                                        switch_counter_id_t acl_stats_index,
                                        switch_counter_t *acl_counter);

switch_status_t switch_pd_acl_stats_clear(switch_device_t device,
                                          switch_counter_id_t acl_stats_index);

switch_status_t switch_pd_racl_stats_get(switch_device_t device,
                                         switch_counter_id_t racl_stats_index,
                                         switch_counter_t *racl_counter);

switch_status_t switch_pd_racl_stats_clear(
    switch_device_t device, switch_counter_id_t racl_stats_index);

switch_status_t switch_pd_egress_acl_stats_get(
    switch_device_t device,
    switch_counter_id_t egress_acl_stats_index,
    switch_counter_t *egress_acl_counter);

switch_status_t switch_pd_egress_acl_stats_clear(
    switch_device_t device, switch_counter_id_t egress_acl_stats_index);

switch_status_t switch_pd_ingress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pool_info);

switch_status_t switch_pd_egress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pool_info);

switch_status_t switch_pd_buffer_pool_set(switch_device_t device,
                                          switch_pd_pool_id_t pool_id,
                                          switch_uint32_t pool_size);

switch_status_t switch_pd_buffer_pool_color_drop_enable(
    switch_device_t device, switch_pd_pool_id_t pool_id, bool enable);

switch_status_t switch_pd_buffer_pool_pfc_limit(switch_device_t device,
                                                switch_pd_pool_id_t pool_id,
                                                switch_uint8_t icos,
                                                switch_uint32_t num_bytes);

switch_status_t switch_pd_buffer_skid_limit_set(switch_device_t device,
                                                switch_uint32_t num_bytes);

switch_status_t switch_pd_buffer_skid_hysteresis_set(switch_device_t device,
                                                     switch_uint32_t num_bytes);

switch_status_t switch_pd_buffer_pool_color_limit_set(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_color_t color,
    switch_uint32_t num_bytes);

switch_status_t switch_pd_buffer_pool_color_hysteresis_set(
    switch_device_t device, switch_color_t color, switch_uint32_t num_bytes);

switch_status_t switch_pd_qos_default_entry_add(switch_device_t device);

switch_status_t switch_pd_qos_map_egress_default_entries_add(
    switch_device_t device);

switch_status_t switch_pd_qos_map_ingress_entry_add(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_qos_map_ingress_entry_update(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_qos_map_ingress_entry_delete(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_qos_map_egress_entry_add(
    switch_device_t device,
    switch_qos_map_egress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_qos_map_egress_entry_delete(
    switch_device_t device,
    switch_qos_map_egress_t qos_map_type,
    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_port_drop_limit_set(switch_device_t device,
                                              switch_handle_t port_handle,
                                              uint32_t num_bytes);

switch_status_t switch_pd_port_drop_hysteresis_set(switch_device_t device,
                                                   switch_handle_t port_handle,
                                                   uint32_t num_bytes);

switch_status_t switch_pd_port_pfc_cos_mapping(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               uint8_t *cos_to_icos);

switch_status_t switch_pd_port_flowcontrol_mode_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_flowcontrol_type_t flow_control);

switch_status_t switch_pd_ppg_create(switch_device_t device,
                                     switch_dev_port_t dev_port,
                                     switch_tm_ppg_hdl_t *ppg_handle);

switch_status_t switch_pd_ppg_delete(switch_device_t device,
                                     switch_tm_ppg_hdl_t ppg_handle);

switch_status_t switch_pd_port_ppg_icos_mapping(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint8_t icos_bmp);

switch_status_t switch_pd_port_cut_through_set(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               bool enable);

switch_status_t switch_pd_port_cut_through_get(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               bool *enable);

switch_status_t switch_pd_ppg_lossless_enable(switch_device_t device,
                                              switch_tm_ppg_hdl_t tm_ppg_handle,
                                              bool enable);

switch_status_t switch_pd_ppg_guaranteed_limit_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes);

switch_status_t switch_pd_ppg_skid_limit_set(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint32_t num_bytes);

switch_status_t switch_pd_ppg_skid_hysteresis_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes);

switch_status_t switch_pd_ppg_drop_count_get(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint64_t *num_packets);

#define DYNAMIC_THRESHOLD_FACTOR 32
switch_status_t switch_pd_ppg_pool_usage_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable);

switch_status_t switch_pd_queue_pool_usage_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t qid,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable);

switch_status_t switch_pd_buffer_bytes_to_cells(
    switch_device_t device,
    switch_uint32_t bytes_threshold,
    switch_uint32_t *cell_threshold);

switch_status_t switch_pd_queue_color_drop_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable);

switch_status_t switch_pd_queue_color_limit_set(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                switch_color_t color,
                                                uint32_t limit);

switch_status_t switch_pd_queue_color_hysteresis_set(switch_device_t device,
                                                     switch_dev_port_t dev_port,
                                                     switch_qid_t queue_id,
                                                     switch_color_t color,
                                                     uint32_t limit);

switch_status_t switch_pd_queue_pfc_cos_mapping(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                uint8_t cos);

switch_status_t switch_pd_queue_port_mapping(switch_device_t device,
                                             switch_dev_port_t dev_port,
                                             uint8_t queue_count,
                                             switch_qid_t *queue_mapping);

switch_status_t switch_pd_queue_scheduling_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable);

switch_status_t switch_pd_queue_drop_count_get(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               switch_qid_t queue_id,
                                               uint64_t *num_packets);

switch_status_t switch_pd_queue_scheduling_strict_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority);

switch_status_t switch_pd_queue_scheduling_remaining_bw_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority);

switch_status_t switch_pd_queue_scheduling_dwrr_weight_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint16_t weight);

switch_status_t switch_pd_queue_guaranteed_rate_set(switch_device_t device,
                                                    switch_dev_port_t dev_port,
                                                    switch_qid_t queue_id,
                                                    bool pps,
                                                    uint32_t burst_size,
                                                    uint64_t rate);

switch_status_t switch_pd_queue_scheduling_guaranteed_shaping_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    bool pps,
    uint32_t burst_size,
    uint32_t rate);

switch_status_t switch_pd_queue_shaping_set(switch_device_t device,
                                            switch_dev_port_t dev_port,
                                            switch_qid_t queue_id,
                                            bool pps,
                                            uint32_t burst_size,
                                            uint64_t rate);

switch_status_t switch_pd_port_shaping_set(switch_device_t device,
                                           switch_dev_port_t dev_port,
                                           bool pps,
                                           uint32_t burst_size,
                                           uint64_t rate);

switch_status_t switch_pd_dtel_tail_drop_deflection_queue_set(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id);

switch_status_t switch_pd_hostif_meter_set(switch_device_t device,
                                           switch_meter_id_t meter_id,
                                           switch_meter_info_t *meter_info,
                                           bool enable);

switch_status_t switch_pd_range_entry_add(switch_device_t device,
                                          switch_direction_t direction,
                                          switch_uint16_t range_id,
                                          switch_range_type_t range_type,
                                          switch_range_t *range,
                                          switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_range_entry_update(switch_device_t device,
                                             switch_direction_t direction,
                                             switch_uint16_t range_id,
                                             switch_range_type_t range_type,
                                             switch_range_t *range,
                                             switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_range_entry_delete(switch_device_t device,
                                             switch_direction_t direction,
                                             switch_range_type_t range_type,
                                             switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_l4port_fields_entry_init(
    switch_device_t device);

switch_status_t switch_pd_l4port_default_entry_add(switch_device_t device);

switch_status_t switch_pd_ila_table_init(switch_device_t device);

switch_status_t switch_pd_ila_table_entry_delete(switch_device_t device,
                                                 switch_ila_info_t *ila_info);

switch_status_t switch_pd_ila_table_entry_add(switch_device_t device,
                                              switch_ila_info_t *ila_info);

switch_status_t switch_pd_ila_table_entry_update(switch_device_t device,
                                                 switch_ila_info_t *ila_info);

switch_status_t switch_pd_mtu_table_entry_add(switch_device_t device,
                                              switch_mtu_id_t mtu_index,
                                              switch_mtu_t mtu,
                                              bool ipv4,
                                              switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_mtu_table_entry_update(switch_device_t device,
                                                 switch_mtu_id_t mtu_index,
                                                 switch_mtu_t mtu,
                                                 bool ipv4,
                                                 switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mtu_table_entry_delete(switch_device_t device,
                                                 switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_rewrite_table_srv6_entry_add(
    switch_device_t device,
    switch_tunnel_t tunnel_index,
    switch_id_t sip_index,
    switch_id_t dip_index,
    switch_id_t smac_index,
    switch_uint8_t first_seg,
    switch_srv6_segment_t *seg_list,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_srv6_table_default_entry_add(switch_device_t device);

switch_status_t switch_pd_srv6_table_entry_delete(switch_device_t device,
                                                  switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_srv6_table_entry_add(
    switch_device_t device,
    switch_interface_ip_addr_t *ip_addr_info,
    switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_srv6_table_entry_init(switch_device_t device);

switch_status_t switch_pd_srv6_rewrite_table_entry_init(switch_device_t device);

switch_status_t switch_pd_process_srh_len_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_packet_length_adjust_default_entry_add(
    switch_device_t device);

switch_status_t switch_pd_wred_early_drop_set(switch_device_t device,
                                              switch_handle_t wred_handle,
                                              switch_wred_info_t *wred_info);

switch_status_t switch_pd_wred_index_table_entry_delete(
    switch_device_t device, p4_pd_entry_hdl_t entry_hdl);

switch_status_t switch_pd_wred_index_table_entry_update(
    switch_device_t device,
    switch_handle_t wred_handle,
    p4_pd_entry_hdl_t entry_hdl);

switch_status_t switch_pd_wred_index_table_entry_add(
    switch_device_t device,
    switch_wred_queue_entry_t *queue_entry,
    switch_handle_t wred_handle,
    switch_handle_t wred_stats_handle);

switch_status_t switch_pd_wred_action_table_entry_add(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_wred_info_t *wred_info);

switch_status_t switch_pd_wred_action_table_entry_update(
    switch_device_t device,
    switch_handle_t wred_handle,
    switch_wred_info_t *wred_info);

switch_status_t switch_pd_wred_action_table_entry_delete(
    switch_device_t device, switch_wred_info_t *wred_info);

switch_status_t switch_pd_wred_action_default_entry_add(switch_device_t device);

switch_status_t switch_pd_total_buffer_size_get(switch_device_t device,
                                                switch_uint64_t *size);

switch_status_t switch_pd_buffer_pool_usage_get(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes);

switch_status_t switch_pd_wred_stats_get(switch_device_t device,
                                         switch_wred_counter_t counter_id,
                                         switch_handle_t stats_handle,
                                         switch_counter_t *counter);

switch_status_t switch_pd_wred_stats_clear(switch_device_t device,
                                           switch_wred_counter_t counter_id,
                                           switch_handle_t stats_handle);

switch_status_t switch_pd_port_auto_neg_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_auto_neg_mode_t an_mode);

switch_status_t switch_pd_port_state_change_notification_register(
    switch_device_t device, void *cookie);

switch_status_t switch_pd_port_stats_counter_id_clear(
    const switch_device_t device,
    const switch_dev_port_t dev_port,
    const switch_uint16_t num_counters,
    const switch_port_counter_id_t *counter_ids);

switch_status_t switch_pd_port_stats_clear_all(switch_device_t device,
                                               switch_dev_port_t dev_port);

switch_status_t switch_pd_port_loopback_mode_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_port_loopback_mode_t lb_mode);

switch_status_t switch_pd_port_pfc_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_uint32_t rx_pfc_map,
                                       switch_uint32_t tx_pfc_map);

switch_status_t switch_pd_port_link_pause_set(switch_device_t device,
                                              switch_dev_port_t dev_port,
                                              bool rx_pause_en,
                                              bool tx_pause_en);

switch_status_t switch_pd_port_mtu_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_uint32_t tx_mtu,
                                       switch_uint32_t rx_mtu);

switch_status_t switch_pd_tunnel_mgid_entry_add(switch_device_t device,
                                                switch_tunnel_t tunnel_index,
                                                switch_mgid_t mc_index,
                                                switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_tunnel_mgid_entry_update(switch_device_t device,
                                                   switch_tunnel_t tunnel_index,
                                                   switch_mgid_t mc_index,
                                                   switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_tunnel_mgid_entry_delete(switch_device_t device,
                                                   switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_port_fec_set(switch_device_t device,
                                       switch_dev_port_t dev_port,
                                       switch_port_fec_mode_t fec_mode);

switch_status_t switch_pd_ingress_port_mirror_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_port_ingress_mirror_set(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_handle_t mirror_handle,
                                                  switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_port_ingress_mirror_delete(switch_device_t device,
                                                     switch_dev_port_t dev_port,
                                                     switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_egress_port_mirror_table_entry_init(
    switch_device_t device);

switch_status_t switch_pd_port_egress_mirror_set(switch_device_t device,
                                                 switch_dev_port_t dev_port,
                                                 switch_handle_t mirror_handle,
                                                 switch_pd_hdl_t *entry_hdl);

switch_status_t switch_pd_port_egress_mirror_delete(switch_device_t device,
                                                    switch_dev_port_t dev_port,
                                                    switch_pd_hdl_t entry_hdl);

switch_status_t switch_pd_mac_entry_aging_time_set(
    switch_device_t device,
    switch_pd_hdl_t pd_hdl,
    switch_uint32_t aging_interval);

// Dynamic Hash apis
switch_status_t switch_pd_ipv6_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv6_input_fields_t input);

switch_status_t switch_pd_ipv4_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv4_input_fields_t input);

switch_status_t switch_pd_non_ip_hash_input_fields_set(
    switch_device_t device, switch_hash_non_ip_input_fields_t input);

switch_status_t switch_pd_ipv6_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_pd_ipv4_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_pd_non_ip_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_pd_ipv6_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv6_algorithm_t algorithm);

switch_status_t switch_pd_ipv4_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv4_algorithm_t algorithm);

switch_status_t switch_pd_non_ip_hash_algorithm_set(
    switch_device_t device, switch_hash_non_ip_algorithm_t algorithm);

switch_status_t switch_pd_ipv6_hash_seed_set(switch_device_t device,
                                             uint64_t seed);

switch_status_t switch_pd_ipv4_hash_seed_set(switch_device_t device,
                                             uint64_t seed);

switch_status_t switch_pd_non_ip_hash_seed_set(switch_device_t device,
                                               uint64_t seed);

switch_status_t switch_pd_ipv6_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv6_input_fields_t *input);

switch_status_t switch_pd_ipv4_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv4_input_fields_t *input);

switch_status_t switch_pd_non_ip_hash_input_fields_get(
    switch_device_t device, switch_hash_non_ip_input_fields_t *input);

switch_status_t switch_pd_ipv6_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_pd_ipv4_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_pd_non_ip_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_pd_ipv6_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv6_algorithm_t *algorithm);

switch_status_t switch_pd_ipv4_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv4_algorithm_t *algorithm);

switch_status_t switch_pd_non_ip_hash_algorithm_get(
    switch_device_t device, switch_hash_non_ip_algorithm_t *algorithm);

switch_status_t switch_pd_ipv6_hash_seed_get(switch_device_t device,
                                             uint64_t *seed);

switch_status_t switch_pd_ipv4_hash_seed_get(switch_device_t device,
                                             uint64_t *seed);

switch_status_t switch_pd_non_ip_hash_seed_get(switch_device_t device,
                                               uint64_t *seed);
switch_status_t switch_pd_lag_hash_seed_set(switch_device_t device,
                                            uint64_t seed);

switch_status_t switch_pd_ecmp_hash_seed_set(switch_device_t device,
                                             uint64_t seed);

switch_status_t switch_pd_lag_hash_seed_get(switch_device_t device,
                                            uint64_t *seed);

switch_status_t switch_pd_ecmp_hash_seed_get(switch_device_t device,
                                             uint64_t *seed);

bool switch_pd_platform_type_model(switch_device_t device);

switch_status_t switch_pd_port_tm_drop_get(switch_device_t device,
                                           switch_dev_port_t dev_port,
                                           uint64_t *idrop_count,
                                           uint64_t *edrop_count);

switch_int32_t switch_pd_counter_read_flags(switch_device_t device);

switch_status_t switch_pd_l2_mac_learning_set(switch_device_t device,
                                              bool enable);

#ifdef __cplusplus
}
#endif

#endif
