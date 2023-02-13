################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2016 Barefoot Networks, Inc.

# All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
# technical concepts contained herein are proprietary to Barefoot Networks,
# Inc.
# and its suppliers and may be covered by U.S. and Foreign Patents, patents in
# process, and are protected by trade secret or copyright law.
# Dissemination of this information or reproduction of this material is
# strictly forbidden unless prior written permission is obtained from
# Barefoot Networks, Inc.
#
# No warranty, explicit or implicit is provided, unless granted under a
# written agreement with Barefoot Networks, Inc.
#
# $Id: $
#
###############################################################################
"""
Thrift PD interface basic tests
"""

import time
import sys
import logging
import unittest
import random
import pd_base_tests
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
import os
from switch.p4_pd_rpc.ttypes import *
from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *


def enum(**enums):
    return type('Enum', (), enums)


this_dir = os.path.dirname(os.path.abspath(__file__))

# features enabled based on p4src/p4feature.h
tunnel_enabled = 1
ipv6_enabled = 0
acl_enabled = 0
stats_enabled = 0

# global defaults
g_inner_rmac_grp = 1
g_outer_rmac_grp = 2
g_smac_index = 1
g_vrf = 1
g_rmac = '00:77:66:55:44:33'
g_fabric_mcast_device_id = 127
g_fabric_mgid = 3333
g_unicast_fabric_tunnel_rewrite_index = 66
g_multicast_fabric_tunnel_rewrite_index = 77

# config across devices
vlan1 = 10
vlan2 = 11
g_nhop1 = 302
g_nhop2 = 303
g_nhop3 = 102
g_nhop4 = 101
vlan1_uuc_mc_index = 12345
vlan2_uuc_mc_index = 4321

PortType = enum(Normal=0, Fabric=1)


def populate_default_entries(client, sess_hdl, dev_tgt):
    client.validate_outer_ethernet_set_default_action_set_valid_outer_unicast_packet_untagged(
        sess_hdl, dev_tgt)
    client.validate_outer_ipv4_packet_set_default_action_set_valid_outer_ipv4_packet(
        sess_hdl, dev_tgt)
    if ipv6_enabled:
        client.validate_outer_ipv6_packet_set_default_action_set_valid_outer_ipv6_packet(
            sess_hdl, dev_tgt)
    client.smac_set_default_action_smac_miss(sess_hdl, dev_tgt)
    client.learn_notify_set_default_action_nop(sess_hdl, dev_tgt)
    client.dmac_set_default_action_dmac_miss(sess_hdl, dev_tgt)
    client.rmac_set_default_action_rmac_miss(sess_hdl, dev_tgt)
    client.ipv4_fib_set_default_action_on_miss(sess_hdl, dev_tgt)
    client.fwd_result_set_default_action_nop(sess_hdl, dev_tgt)
    client.nexthop_set_default_action_nop(sess_hdl, dev_tgt)
    client.fabric_ingress_dst_lkp_set_default_action_nop(sess_hdl, dev_tgt)
    client.fabric_ingress_src_lkp_set_default_action_nop(sess_hdl, dev_tgt)
    mbr_hdl = client.fabric_lag_action_profile_add_member_with_nop(sess_hdl,
                                                                   dev_tgt)
    client.fabric_lag_set_default_entry(sess_hdl, dev_tgt, mbr_hdl)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_miss(sess_hdl,
                                                                     dev_tgt)
    client.lag_group_set_default_entry(sess_hdl, dev_tgt, mbr_hdl)
    client.vlan_decap_set_default_action_nop(sess_hdl, dev_tgt)
    if acl_enabled:
        client.ip_acl_set_default_action_nop(sess_hdl, dev_tgt)
        client.ipv4_racl_set_default_action_nop(sess_hdl, dev_tgt)
        client.egress_acl_set_default_action_nop(sess_hdl, dev_tgt)
        client.qos_set_default_action_nop(sess_hdl, dev_tgt)
    client.validate_packet_set_default_action_nop(sess_hdl, dev_tgt)
    client.adjust_lkp_fields_set_default_action_non_ip_lkp(sess_hdl, dev_tgt)
    match_spec = dc_adjust_lkp_fields_match_spec_t(ipv4_valid=1, ipv6_valid=0)
    client.adjust_lkp_fields_table_add_with_ipv4_lkp(sess_hdl, dev_tgt,
                                                     match_spec)
    if ipv6_enabled:
        match_spec = dc_adjust_lkp_fields_match_spec_t(
            ipv4_valid=0, ipv6_valid=1)
        client.adjust_lkp_fields_table_add_with_ipv6_lkp(sess_hdl, dev_tgt,
                                                         match_spec)
    if tunnel_enabled:
        client.outer_rmac_set_default_action_on_miss(sess_hdl, dev_tgt)
        client.ipv4_src_vtep_set_default_action_on_miss(sess_hdl, dev_tgt)
        client.ipv4_dest_vtep_set_default_action_nop(sess_hdl, dev_tgt)
        client.tunnel_src_rewrite_set_default_action_nop(sess_hdl, dev_tgt)
        client.tunnel_smac_rewrite_set_default_action_nop(sess_hdl, dev_tgt)
        client.tunnel_dmac_rewrite_set_default_action_nop(sess_hdl, dev_tgt)
        client.tunnel_lookup_miss_set_default_action_non_ip_lkp(sess_hdl,
                                                                dev_tgt)
        match_spec = dc_tunnel_lookup_miss_match_spec_t(
            ipv4_valid=1, ipv6_valid=0)
        client.tunnel_lookup_miss_table_add_with_ipv4_lkp(sess_hdl, dev_tgt,
                                                          match_spec)
    if ipv6_enabled and tunnel_enabled:
        client.ipv6_src_vtep_set_default_action_nop(sess_hdl, dev_tgt)
        client.ipv6_dest_vtep_set_default_action_nop(sess_hdl, dev_tgt)
        match_spec = dc_tunnel_lookup_miss_match_spec_t(
            ipv4_valid=0, ipv6_valid=1)
        client.tunnel_lookup_miss_table_add_with_ipv6_lkp(sess_hdl, dev_tgt,
                                                          match_spec)
    if ipv6_enabled and acl_enabled:
        client.ipv6_acl_set_default_action_nop(sess_hdl, dev_tgt)
        client.ipv6_racl_set_default_action_nop(sess_hdl, dev_tgt)
    match_spec = dc_compute_ipv4_hashes_match_spec_t(ethernet_valid=1)
    client.compute_ipv4_hashes_table_add_with_compute_lkp_ipv4_hash(
        sess_hdl, dev_tgt, match_spec)
    if ipv6_enabled == 1:
        match_spec = dc_compute_ipv6_hashes_match_spec_t(ethernet_valid=1)
        client.compute_ipv6_hashes_table_add_with_compute_lkp_ipv6_hash(
            sess_hdl, dev_tgt, match_spec)
    match_spec = dc_compute_non_ip_hashes_match_spec_t(ethernet_valid=1)
    client.compute_non_ip_hashes_table_add_with_compute_lkp_non_ip_hash(
        sess_hdl, dev_tgt, match_spec)
    client.egress_vni_set_default_action_nop(sess_hdl, dev_tgt)
    client.system_acl_set_default_action_nop(sess_hdl, dev_tgt)
    client.outer_ipv4_multicast_set_default_action_nop(sess_hdl, dev_tgt)
    #client.storm_control_set_default_action_nop(sess_hdl, dev_tgt)
    client.egress_vlan_xlate_set_default_action_set_egress_if_params_untagged(
        sess_hdl, dev_tgt)
    if stats_enabled:
        client.ingress_bd_stats_set_default_action_update_ingress_bd_stats(
            sess_hdl, dev_tgt)
    action_spec = dc_set_config_parameters_action_spec_t(action_switch_id=0)
    client.switch_config_params_set_default_action_set_config_parameters(
          sess_hdl, dev_tgt, action_spec);


def populate_init_entries(client, sess_hdl, dev_tgt):
    match_spec = dc_smac_rewrite_match_spec_t(
        egress_metadata_smac_idx=g_smac_index)
    action_spec = dc_rewrite_smac_action_spec_t(
        action_smac=macAddr_to_string(g_rmac))
    client.smac_rewrite_table_add_with_rewrite_smac(sess_hdl, dev_tgt,
                                                    match_spec, action_spec)

    match_spec = dc_fwd_result_match_spec_t(
        l2_metadata_l2_redirect=0,
        l2_metadata_l2_redirect_mask=0,
        acl_metadata_acl_redirect=0,
        acl_metadata_acl_redirect_mask=0,
        acl_metadata_racl_redirect=0,
        acl_metadata_racl_redirect_mask=0,
        l3_metadata_rmac_hit=1,
        l3_metadata_rmac_hit_mask=1,
        l3_metadata_fib_hit=1,
        l3_metadata_fib_hit_mask=1,
#        nat_metadata_nat_hit=0,
#        nat_metadata_nat_hit_mask=0,
        l2_metadata_lkp_pkt_type=0,
        l2_metadata_lkp_pkt_type_mask=0,
        l3_metadata_lkp_ip_type=0,
        l3_metadata_lkp_ip_type_mask=0,
        multicast_metadata_igmp_snooping_enabled=0,
        multicast_metadata_igmp_snooping_enabled_mask=0,
        multicast_metadata_mld_snooping_enabled=0,
        multicast_metadata_mld_snooping_enabled_mask=0,
        multicast_metadata_mcast_route_hit=0,
        multicast_metadata_mcast_route_hit_mask=0,
        multicast_metadata_mcast_bridge_hit=0,
        multicast_metadata_mcast_bridge_hit_mask=0,
        multicast_metadata_mcast_rpf_group=0,
        multicast_metadata_mcast_rpf_group_mask=0,
        multicast_metadata_mcast_mode=0,
        multicast_metadata_mcast_mode_mask=0)
    client.fwd_result_table_add_with_set_fib_redirect_action(sess_hdl, dev_tgt,
                                                             match_spec, 1000)

    match_spec = dc_fwd_result_match_spec_t(
        l2_metadata_l2_redirect=1,
        l2_metadata_l2_redirect_mask=1,
        acl_metadata_acl_redirect=0,
        acl_metadata_acl_redirect_mask=0,
        acl_metadata_racl_redirect=0,
        acl_metadata_racl_redirect_mask=0,
        l3_metadata_rmac_hit=0,
        l3_metadata_rmac_hit_mask=0,
        l3_metadata_fib_hit=0,
        l3_metadata_fib_hit_mask=0,
#        nat_metadata_nat_hit=0,
#        nat_metadata_nat_hit_mask=0,
        l2_metadata_lkp_pkt_type=0,
        l2_metadata_lkp_pkt_type_mask=0,
        l3_metadata_lkp_ip_type=0,
        l3_metadata_lkp_ip_type_mask=0,
        multicast_metadata_igmp_snooping_enabled=0,
        multicast_metadata_igmp_snooping_enabled_mask=0,
        multicast_metadata_mld_snooping_enabled=0,
        multicast_metadata_mld_snooping_enabled_mask=0,
        multicast_metadata_mcast_route_hit=0,
        multicast_metadata_mcast_route_hit_mask=0,
        multicast_metadata_mcast_bridge_hit=0,
        multicast_metadata_mcast_bridge_hit_mask=0,
        multicast_metadata_mcast_rpf_group=0,
        multicast_metadata_mcast_rpf_group_mask=0,
        multicast_metadata_mcast_mode=0,
        multicast_metadata_mcast_mode_mask=0)
    client.fwd_result_table_add_with_set_l2_redirect_action(sess_hdl, dev_tgt,
                                                            match_spec, 1000)

    # add default inner rmac entry
    match_spec = dc_rmac_match_spec_t(
        l3_metadata_rmac_group=g_inner_rmac_grp,
        l2_metadata_lkp_mac_da=macAddr_to_string(g_rmac))
    client.rmac_table_add_with_rmac_hit(sess_hdl, dev_tgt, match_spec)

    if tunnel_enabled:
        # add default outer rmac entry
        match_spec = dc_outer_rmac_match_spec_t(
            l3_metadata_rmac_group=g_outer_rmac_grp,
            ethernet_dstAddr=macAddr_to_string(g_rmac))
        client.outer_rmac_table_add_with_outer_rmac_hit(sess_hdl, dev_tgt,
                                                        match_spec)

    # initialize fabric tunnel rewrite table for unicast
    match_spec = dc_tunnel_encap_process_outer_match_spec_t(
        tunnel_metadata_egress_tunnel_type=15,
        tunnel_metadata_egress_header_count=0,
        multicast_metadata_replica=0)
    action_spec = dc_fabric_rewrite_action_spec_t(
        action_tunnel_index=g_unicast_fabric_tunnel_rewrite_index)
    client.tunnel_encap_process_outer_table_add_with_fabric_rewrite(
        sess_hdl, dev_tgt, match_spec, action_spec)

    match_spec = dc_tunnel_rewrite_match_spec_t(
        tunnel_metadata_tunnel_index=g_unicast_fabric_tunnel_rewrite_index)
    client.tunnel_rewrite_table_add_with_fabric_unicast_rewrite(
        sess_hdl, dev_tgt, match_spec)

    # initialize fabric tunnel rewrite table for multicast
    match_spec = dc_tunnel_encap_process_outer_match_spec_t(
        tunnel_metadata_egress_tunnel_type=15,
        tunnel_metadata_egress_header_count=0,
        multicast_metadata_replica=1)
    action_spec = dc_fabric_rewrite_action_spec_t(
        action_tunnel_index=g_multicast_fabric_tunnel_rewrite_index)
    client.tunnel_encap_process_outer_table_add_with_fabric_rewrite(
        sess_hdl, dev_tgt, match_spec, action_spec)

    match_spec = dc_tunnel_rewrite_match_spec_t(
        tunnel_metadata_tunnel_index=g_multicast_fabric_tunnel_rewrite_index)
    action_spec = dc_fabric_multicast_rewrite_action_spec_t(
        action_fabric_mgid=g_fabric_mgid)
    client.tunnel_rewrite_table_add_with_fabric_multicast_rewrite(
        sess_hdl, dev_tgt, match_spec, action_spec)

    # initalize l3 rewrite table
    match_spec = dc_l3_rewrite_match_spec_t(
        ipv4_valid=1, ipv4_dstAddr=0, ipv4_dstAddr_mask=0)
    client.l3_rewrite_table_add_with_ipv4_unicast_rewrite(sess_hdl, dev_tgt,
                                                          match_spec, 100)


def add_ports(client, sess_hdl, dev_tgt, port_count, port_type, l2xid):
    count = 0
    while (count < port_count):
        port_lag_index = count + 1
        match_spec = dc_ingress_port_mapping_match_spec_t(
            ig_intr_md_ingress_port=count)
        action_spec = dc_set_port_lag_index_action_spec_t(
            action_port_lag_index=port_lag_index, action_port_type=port_type[count])
        client.ingress_port_mapping_table_add_with_set_port_lag_index(
            sess_hdl, dev_tgt, match_spec, action_spec)

        action_spec = dc_set_ingress_port_properties_action_spec_t(
	    action_port_lag_label=0,
            action_exclusion_id=l2xid[count],
            action_qos_group=0,
            action_tc_qos_group=0,
            action_tc=0,
            action_color=0,
            action_trust_dscp=0,
            action_trust_pcp=0)
        client.ingress_port_properties_table_add_with_set_ingress_port_properties(
            sess_hdl, dev_tgt, match_spec, action_spec)
        action_spec = dc_set_lag_port_action_spec_t(action_port=count)
        mbr_hdl = client.lag_action_profile_add_member_with_set_lag_port(
            sess_hdl, dev_tgt, action_spec)
        match_spec = dc_lag_group_match_spec_t(
            ingress_metadata_egress_port_lag_index=port_lag_index)
        client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

        match_spec = dc_egress_port_mapping_match_spec_t(
            eg_intr_md_egress_port=count)
        if port_type[count] == PortType.Normal:
            action_spec = dc_egress_port_type_normal_action_spec_t(
                action_qos_group=0, action_port_lag_label=0)
            client.egress_port_mapping_table_add_with_egress_port_type_normal(
                sess_hdl, dev_tgt, match_spec, action_spec)
        elif port_type[count] == PortType.Fabric:
            client.egress_port_mapping_table_add_with_egress_port_type_fabric(
                sess_hdl, dev_tgt, match_spec)

        count = count + 1


def program_bd(client, sess_hdl, dev_tgt, vlan, uuc_mc_index):
    match_spec = dc_bd_flood_match_spec_t(
        ingress_metadata_bd=vlan, l2_metadata_lkp_pkt_type=0x1)
    action_spec = dc_set_bd_flood_mc_index_action_spec_t(
        action_mc_index=uuc_mc_index)
    client.bd_flood_table_add_with_set_bd_flood_mc_index(
        sess_hdl, dev_tgt, match_spec, action_spec)


def program_vlan_mapping(client, sess_hdl, dev_tgt, vlan, port_lag_index, v4_enabled,
                         v6_enabled, ifindex):
    action_spec = dc_set_bd_properties_action_spec_t(
        action_bd=vlan,
        action_vrf=g_vrf,
        action_rmac_group=g_inner_rmac_grp,
        action_bd_label=0,
        action_ipv4_unicast_enabled=v4_enabled,
        action_ipv6_unicast_enabled=v6_enabled,
        action_ipv4_multicast_enabled=0,
        action_ipv6_multicast_enabled=0,
        action_igmp_snooping_enabled=0,
        action_mld_snooping_enabled=0,
        action_ipv4_urpf_mode=0,
        action_ipv6_urpf_mode=0,
        action_stp_group=0,
        action_mrpf_group=0,
        action_ipv4_mcast_key_type=0,
        action_ipv4_mcast_key=0,
        action_ipv6_mcast_key_type=0,
        action_ipv6_mcast_key=0,
        action_stats_idx=0,
        action_learning_enabled=0)
    mbr_hdl = client.bd_action_profile_add_member_with_set_bd_properties(
        sess_hdl, dev_tgt, action_spec)

    match_spec = dc_port_vlan_to_bd_mapping_match_spec_t(
        ingress_metadata_port_lag_index=port_lag_index,
        vlan_tag__0__valid=0,
        vlan_tag__0__vid=0,
        vlan_tag__1__valid=0,
        vlan_tag__1__vid=0)
    client.port_vlan_to_bd_mapping_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    action_spec = dc_set_ingress_interface_properties_action_spec_t(
        action_ifindex=ifindex, action_ingress_rid=0)

    match_spec = dc_port_vlan_to_ifindex_mapping_match_spec_t(
        ingress_metadata_port_lag_index=port_lag_index,
        vlan_tag__0__valid=0,
        vlan_tag__0__vid=0,
        vlan_tag__1__valid=0,
        vlan_tag__1__vid=0)

    client.port_vlan_to_ifindex_mapping_table_add_with_set_ingress_interface_properties(
        sess_hdl, dev_tgt, match_spec, action_spec)


def program_egress_bd_map(client, sess_hdl, dev_tgt, vlan):
    match_spec = dc_egress_bd_map_match_spec_t(egress_metadata_bd=vlan)
    action_spec = dc_set_egress_bd_properties_action_spec_t(
        action_smac_idx=g_smac_index,
        action_nat_mode=0,
        action_bd_label=0,
        action_mtu_index=0)
    client.egress_bd_map_table_add_with_set_egress_bd_properties(
        sess_hdl, dev_tgt, match_spec, action_spec)


def program_tunnel_vlan(client, sess_hdl, dev_tgt, vlan, port, vni, ttype,
                        v4_enabled, inner_rmac):
    match_spec = dc_tunnel_match_spec_t(
        tunnel_metadata_tunnel_vni=vni,
        tunnel_metadata_ingress_tunnel_type=ttype,
        inner_ipv4_valid=1)
    action_spec = dc_terminate_tunnel_inner_ipv4_action_spec_t(
        action_bd=vlan,
        action_vrf=g_vrf,
        action_rmac_group=inner_rmac,
        action_bd_label=0,
        action_uuc_mc_index=0,
        action_umc_mc_index=0,
        action_bcast_mc_index=0,
        action_ipv4_unicast_enabled=v4_enabled,
        action_igmp_snooping_enabled=0)
    client.tunnel_table_add_with_terminate_tunnel_inner_ipv4(
        sess_hdl, dev_tgt, match_spec, action_spec)


def add_mac(client, sess_hdl, dev_tgt, vlan, mac, port):
    match_spec = dc_dmac_match_spec_t(
        l2_metadata_lkp_mac_da=macAddr_to_string(mac), l2_metadata_bd=vlan)
    action_spec = dc_dmac_hit_action_spec_t(action_port_lag_index=port)
    client.dmac_table_add_with_dmac_hit(sess_hdl, dev_tgt, match_spec,
                                        action_spec)

    match_spec = dc_smac_match_spec_t(
        l2_metadata_lkp_mac_sa=macAddr_to_string(mac), l2_metadata_bd=vlan)
    action_spec = dc_smac_hit_action_spec_t(action_port_lag_index=port)
    client.smac_table_add_with_smac_hit(sess_hdl, dev_tgt, match_spec,
                                        action_spec, 0)


def add_mac_with_nexthop(client, sess_hdl, dev_tgt, vlan, mac, port, nhop):
    match_spec = dc_dmac_match_spec_t(
        l2_metadata_lkp_mac_da=macAddr_to_string(mac), l2_metadata_bd=vlan)
    action_spec = dc_dmac_redirect_nexthop_action_spec_t(
        action_nexthop_index=nhop)
    client.dmac_table_add_with_dmac_redirect_nexthop(sess_hdl, dev_tgt,
                                                     match_spec, action_spec)

    match_spec = dc_smac_match_spec_t(
        l2_metadata_lkp_mac_sa=macAddr_to_string(mac), l2_metadata_bd=vlan)
    action_spec = dc_smac_hit_action_spec_t(action_port_lag_index=port)
    client.smac_table_add_with_smac_hit(sess_hdl, dev_tgt, match_spec,
                                        action_spec, 0)


def add_v4_route(client, sess_hdl, dev_tgt, vrf, ip, prefix, nhop):
    if prefix == 32:
        match_spec = dc_ipv4_fib_match_spec_t(
            l3_metadata_vrf=vrf, ipv4_metadata_lkp_ipv4_da=ip)
        action_spec = dc_fib_hit_nexthop_action_spec_t(
            action_nexthop_index=nhop)
        client.ipv4_fib_table_add_with_fib_hit_nexthop(sess_hdl, dev_tgt,
                                                       match_spec, action_spec)
    else:
        match_spec = dc_ipv4_fib_lpm_match_spec_t(
            l3_metadata_vrf=vrf,
            ipv4_metadata_lkp_ipv4_da=ip,
            ipv4_metadata_lkp_ipv4_da_prefix_length=prefix)
        action_spec = dc_fib_hit_nexthop_action_spec_t(
            action_nexthop_index=nhop)
        client.ipv4_fib_lpm_table_add_with_fib_hit_nexthop(
            sess_hdl, dev_tgt, match_spec, action_spec)


def add_v6_route(client, sess_hdl, dev_tgt, vrf, ip, prefix, nhop):
    if ipv6_enabled == 0:
        return
    if prefix == 128:
        match_spec = dc_ipv6_fib_match_spec_t(
            l3_metadata_vrf=vrf,
            ipv6_metadata_lkp_ipv6_da=ipv6Addr_to_string(ip))
        action_spec = dc_fib_hit_nexthop_action_spec_t(
            action_nexthop_index=nhop)
        client.ipv6_fib_table_add_with_fib_hit_nexthop(sess_hdl, dev_tgt,
                                                       match_spec, action_spec)
    else:
        match_spec = dc_ipv6_fib_lpm_match_spec_t(
            l3_metadata_vrf=vrf,
            ipv6_metadata_lkp_ipv6_da=ip,
            ipv6_metadata_lkp_ipv6_da_prefix_length=prefix)
        action_spec = dc_fib_hit_nexthop_action_spec_t(
            action_nexthop_index=nhop)
        client.ipv6_fib_lpm_table_add_with_fib_hit_nexthop(
            sess_hdl, dev_tgt, match_spec, action_spec)


def add_nexthop(client, sess_hdl, dev_tgt, nhop, vlan, port_lag_index, ifindex):
    match_spec = dc_nexthop_match_spec_t(l3_metadata_nexthop_index=nhop)
    action_spec = dc_set_nexthop_details_action_spec_t(
	action_ifindex=ifindex, action_port_lag_index=port_lag_index, action_bd=vlan, action_tunnel=0)
    client.nexthop_table_add_with_set_nexthop_details(sess_hdl, dev_tgt,
                                                      match_spec, action_spec)


def add_v4_unicast_rewrite(client, sess_hdl, dev_tgt, vlan, nhop, dmac):
    match_spec = dc_rewrite_match_spec_t(l3_metadata_nexthop_index=nhop)
    action_spec = dc_set_l3_rewrite_action_spec_t(
        action_bd=vlan, action_dmac=macAddr_to_string(dmac))
    client.rewrite_table_add_with_set_l3_rewrite(sess_hdl, dev_tgt, match_spec,
                                                 action_spec)


def add_fabric_lag(client, sess_hdl, dev_tgt, src_device, port_list):
    grp_hdl = client.fabric_lag_action_profile_create_group(sess_hdl, dev_tgt,
                                                            len(port_list))
    for port in port_list:
        action_spec = dc_set_fabric_lag_port_action_spec_t(action_port=port)
        mbr_hdl = client.fabric_lag_action_profile_add_member_with_set_fabric_lag_port(
            sess_hdl, dev_tgt, action_spec)
        client.fabric_lag_action_profile_add_member_to_group(
            sess_hdl, dev_tgt.dev_id, grp_hdl, mbr_hdl)
    return grp_hdl


# multicast related apis
def port_to_pipe(port):
    return port >> 7


def port_to_pipe_local_id(port):
    return port & 0x7F


def port_to_bit_idx(port):
    pipe = port_to_pipe(port)
    index = port_to_pipe_local_id(port)
    return 72 * pipe + index


def set_port_or_lag_bitmap(bit_map_size, indicies):
    bit_map = [0] * ((bit_map_size + 7) / 8)
    for i in indicies:
        index = port_to_bit_idx(i)
        bit_map[index / 8] = (bit_map[index / 8] | (1 << (index % 8))) & 0xFF
    return bytes_to_string(bit_map)


def config_port_asic_0(client, sess_hdl, mc, mc_sess_hdl):
    dev_id = 0
    my_device_id = 1
    mc_vlan2_rid = vlan2

    dev_tgt = DevTarget_t(dev_id, hex_to_i16(0xFFFF))
    add_ports(
        client,
        sess_hdl,
        dev_tgt,
        port_count=8,
        l2xid=[1, 2, 3, 4, 9, 9, 9, 9],
        port_type=[PortType.Normal] * 4 + [PortType.Fabric] * 4)

    # program fabric lag for all leaf devices
    grp_hdl = add_fabric_lag(client, sess_hdl, dev_tgt, my_device_id,
                             [4, 5, 6, 7])
    match_spec = dc_fabric_lag_match_spec_t(fabric_metadata_dst_device=3)
    client.fabric_lag_add_entry_with_selector(sess_hdl, dev_tgt, match_spec,
                                              grp_hdl)
    match_spec = dc_fabric_lag_match_spec_t(fabric_metadata_dst_device=1)
    client.fabric_lag_add_entry_with_selector(sess_hdl, dev_tgt, match_spec,
                                              grp_hdl)

    # program entry to terminate packet from fabric and switch it out
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=my_device_id)
    client.fabric_ingress_dst_lkp_table_add_with_terminate_fabric_unicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program multicast device id (egress)
    mbr_hdl = client.fabric_lag_action_profile_add_member_with_set_fabric_multicast(
        sess_hdl, dev_tgt)
    match_spec = dc_fabric_lag_match_spec_t(
        fabric_metadata_dst_device=g_fabric_mcast_device_id)
    client.fabric_lag_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    # program multicast device id (ingress)
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=g_fabric_mcast_device_id)
    client.fabric_ingress_dst_lkp_table_add_with_terminate_fabric_multicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program multicast tree for vlan flood (vlan2)
    vlan_port_map = set_port_or_lag_bitmap(288, [0, 2, 3])
    fab_lag_port_map = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_set_lag_membership(mc_sess_hdl, dev_id, 5, fab_lag_port_map)
    fab_lag_map = set_port_or_lag_bitmap(256, [5])
    mc_node_hdl = mc.mc_node_create(mc_sess_hdl, dev_id, mc_vlan2_rid,
                                    vlan_port_map, fab_lag_map)
    mc_grp_hdl = mc.mc_mgrp_create(mc_sess_hdl, dev_id, vlan2_uuc_mc_index)
    mc.mc_associate_node(mc_sess_hdl, dev_id, mc_grp_hdl, mc_node_hdl, 0, 0)

    # program rid table
    match_spec = dc_rid_match_spec_t(eg_intr_md_egress_rid=mc_vlan2_rid)
    action_spec = dc_inner_replica_from_rid_action_spec_t(
        action_bd=vlan2,
        action_dmac_idx=0,
        action_tunnel_index=0,
        action_tunnel_type=0,
        action_header_count=0)
    client.rid_table_add_with_inner_replica_from_rid(sess_hdl, dev_tgt,
                                                     match_spec, action_spec)

    # program port prune table
    prune_port_map = set_port_or_lag_bitmap(288, [0])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 1, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [1])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 2, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [2])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 3, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [3])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 4, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 9, prune_port_map)

    # add bd
    program_bd(
        client, sess_hdl, dev_tgt, vlan=vlan1, uuc_mc_index=vlan1_uuc_mc_index)
    program_bd(
        client, sess_hdl, dev_tgt, vlan=vlan2, uuc_mc_index=vlan2_uuc_mc_index)

    # add vlan mappings
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        port_lag_index=1,
        v4_enabled=1,
        v6_enabled=0,
	ifindex=1)
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan1,
        port_lag_index=2,
        v4_enabled=1,
        v6_enabled=0,
	ifindex=2)
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        port_lag_index=3,
        v4_enabled=1,
        v6_enabled=0,
	ifindex=3)
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        port_lag_index=4,
        v4_enabled=1,
        v6_enabled=0,
        ifindex=4)

    # program egress bd map
    program_egress_bd_map(client, sess_hdl, dev_tgt, vlan=vlan1)
    program_egress_bd_map(client, sess_hdl, dev_tgt, vlan=vlan2)

    dst_port_lag_index1 = 500
    dst_port_lag_index2 = 501
    dst_port_lag_index3 = 502

    # dest_index1 => remote (device=3, port=2)
    action_spec = dc_set_lag_remote_port_action_spec_t(
        action_device=3, action_port=2)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_remote_port(
        sess_hdl, dev_tgt, action_spec)
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=dst_port_lag_index1)
    client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)
    # dest_index2 => remote (device=3, port=3)
    action_spec = dc_set_lag_remote_port_action_spec_t(
        action_device=3, action_port=3)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_remote_port(
        sess_hdl, dev_tgt, action_spec)
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=dst_port_lag_index2)
    client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)
    # dest_index3 => local (device=1, port=2)
    action_spec = dc_set_lag_port_action_spec_t(action_port=2)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_port(
        sess_hdl, dev_tgt, action_spec)
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=dst_port_lag_index3)
    client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    # add nexthops
    add_nexthop(
        client,
        sess_hdl,
        dev_tgt,
        nhop=g_nhop1,
        vlan=vlan2,
        port_lag_index=dst_port_lag_index1,
	ifindex=0)
    add_nexthop(
        client,
        sess_hdl,
        dev_tgt,
        nhop=g_nhop2,
        vlan=vlan2,
        port_lag_index=dst_port_lag_index2,
	ifindex=0)
    add_nexthop(
        client,
        sess_hdl,
        dev_tgt,
        nhop=g_nhop3,
        vlan=vlan2,
        port_lag_index=dst_port_lag_index3,
	ifindex=0)

    # add rewrite information for local ports
    add_v4_unicast_rewrite(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        nhop=g_nhop3,
        dmac='00:00:00:00:01:02')
    add_v4_unicast_rewrite(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan1,
        nhop=g_nhop4,
        dmac='00:00:00:00:01:01')

    # add routes
    add_v4_route(
        client,
        sess_hdl,
        dev_tgt,
        vrf=1,
        ip=0x0a0a0302,
        prefix=32,
        nhop=g_nhop1)
    add_v4_route(
        client,
        sess_hdl,
        dev_tgt,
        vrf=1,
        ip=0x0a0a0303,
        prefix=32,
        nhop=g_nhop2)
    add_v4_route(
        client,
        sess_hdl,
        dev_tgt,
        vrf=1,
        ip=0x0a0a0102,
        prefix=32,
        nhop=g_nhop3)


def config_port_asic_1(client, sess_hdl, mc, mc_sess_hdl):
    dev_id = 1
    my_device_id = 2
    dev_tgt = DevTarget_t(dev_id, hex_to_i16(0xFFFF))
    add_ports(
        client,
        sess_hdl,
        dev_tgt,
        port_count=8,
        l2xid=[1, 1, 1, 1, 2, 2, 2, 2],
        port_type=[PortType.Fabric] * 8)

    # add an entry to switch the fabric packet based on dst device id
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=1)
    client.fabric_ingress_dst_lkp_table_add_with_switch_fabric_unicast_packet(
        sess_hdl, dev_tgt, match_spec)
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=3)
    client.fabric_ingress_dst_lkp_table_add_with_switch_fabric_unicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program multicast device id
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=g_fabric_mcast_device_id)
    client.fabric_ingress_dst_lkp_table_add_with_switch_fabric_multicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program fabric lag for the leaf devices
    grp_hdl = add_fabric_lag(client, sess_hdl, dev_tgt, my_device_id,
                             [0, 1, 2, 3])
    match_spec = dc_fabric_lag_match_spec_t(fabric_metadata_dst_device=1)
    client.fabric_lag_add_entry_with_selector(sess_hdl, dev_tgt, match_spec,
                                              grp_hdl)
    grp_hdl = add_fabric_lag(client, sess_hdl, dev_tgt, my_device_id,
                             [4, 5, 6, 7])
    match_spec = dc_fabric_lag_match_spec_t(fabric_metadata_dst_device=3)
    client.fabric_lag_add_entry_with_selector(sess_hdl, dev_tgt, match_spec,
                                              grp_hdl)

    # program fabric multicast tree
    fab_lag_port_map1 = set_port_or_lag_bitmap(288, [0, 1, 2, 3])
    fab_lag_port_map2 = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_set_lag_membership(mc_sess_hdl, dev_id, 15, fab_lag_port_map1)
    mc.mc_set_lag_membership(mc_sess_hdl, dev_id, 55, fab_lag_port_map2)
    fab_port_map = set_port_or_lag_bitmap(288, [])
    fab_lag_map = set_port_or_lag_bitmap(256, [15, 55])
    fab_mc_node_hdl = mc.mc_node_create(mc_sess_hdl, dev_id, 0, fab_port_map,
                                        fab_lag_map)
    fab_mc_grp_hdl = mc.mc_mgrp_create(mc_sess_hdl, dev_id, g_fabric_mgid)
    mc.mc_associate_node(mc_sess_hdl, dev_id, fab_mc_grp_hdl, fab_mc_node_hdl,
                         0, 0)

    # program port prune table
    prune_port_map = set_port_or_lag_bitmap(288, [0, 1, 2, 3])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 1, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 2, prune_port_map)


def config_port_asic_2(client, sess_hdl, mc, mc_sess_hdl):
    dev_id = 2
    my_device_id = 3
    mc_vlan2_rid = 747

    dev_tgt = DevTarget_t(dev_id, hex_to_i16(0xFFFF))
    add_ports(
        client,
        sess_hdl,
        dev_tgt,
        port_count=8,
        l2xid=[1, 2, 3, 4, 9, 9, 9, 9],
        port_type=[PortType.Normal] * 4 + [PortType.Fabric] * 4)

    # program fabric lag for other leaf devices
    grp_hdl = add_fabric_lag(client, sess_hdl, dev_tgt, my_device_id,
                             [4, 5, 6, 7])
    match_spec = dc_fabric_lag_match_spec_t(fabric_metadata_dst_device=1)
    client.fabric_lag_add_entry_with_selector(sess_hdl, dev_tgt, match_spec,
                                              grp_hdl)

    # program entry to terminate packet from fabric and switch it out
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=my_device_id)
    client.fabric_ingress_dst_lkp_table_add_with_terminate_fabric_unicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program multicast device id (egress)
    mbr_hdl = client.fabric_lag_action_profile_add_member_with_set_fabric_multicast(
        sess_hdl, dev_tgt)
    match_spec = dc_fabric_lag_match_spec_t(
        fabric_metadata_dst_device=g_fabric_mcast_device_id)
    client.fabric_lag_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    # program multicast device id (ingress)
    match_spec = dc_fabric_ingress_dst_lkp_match_spec_t(
        fabric_header_dstDevice=g_fabric_mcast_device_id)
    client.fabric_ingress_dst_lkp_table_add_with_terminate_fabric_multicast_packet(
        sess_hdl, dev_tgt, match_spec)

    # program multicast tree for vlan flood (vlan2)
    vlan_port_map = set_port_or_lag_bitmap(288, [2, 3])
    fab_lag_port_map = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_set_lag_membership(mc_sess_hdl, dev_id, 37, fab_lag_port_map)
    fab_lag_map = set_port_or_lag_bitmap(256, [37])
    mc_node_hdl = mc.mc_node_create(mc_sess_hdl, dev_id, mc_vlan2_rid,
                                    vlan_port_map, fab_lag_map)
    mc_grp_hdl = mc.mc_mgrp_create(mc_sess_hdl, dev_id, vlan2_uuc_mc_index)
    mc.mc_associate_node(mc_sess_hdl, dev_id, mc_grp_hdl, mc_node_hdl, 0, 0)

    # program rid table
    match_spec = dc_rid_match_spec_t(eg_intr_md_egress_rid=mc_vlan2_rid)
    action_spec = dc_inner_replica_from_rid_action_spec_t(
        action_bd=vlan2,
        action_dmac_idx=0,
        action_tunnel_index=0,
        action_tunnel_type=0,
        action_header_count=0)
    client.rid_table_add_with_inner_replica_from_rid(sess_hdl, dev_tgt,
                                                     match_spec, action_spec)
    # program port prune table
    prune_port_map = set_port_or_lag_bitmap(288, [0])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 1, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [1])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 2, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [2])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 3, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [3])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 4, prune_port_map)
    prune_port_map = set_port_or_lag_bitmap(288, [4, 5, 6, 7])
    mc.mc_update_port_prune_table(mc_sess_hdl, dev_id, 9, prune_port_map)

    # add bd
    program_bd(
        client, sess_hdl, dev_tgt, vlan=vlan1, uuc_mc_index=vlan1_uuc_mc_index)
    program_bd(
        client, sess_hdl, dev_tgt, vlan=vlan2, uuc_mc_index=vlan2_uuc_mc_index)

    # add vlan mappings
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan1,
        port_lag_index=1,
        v4_enabled=1,
        v6_enabled=0,
        ifindex=1)

    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan1,
        port_lag_index=2,
        v4_enabled=1,
        v6_enabled=0,
        ifindex=2)

    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        port_lag_index=3,
        v4_enabled=1,
        v6_enabled=0,
        ifindex=3)
    program_vlan_mapping(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        port_lag_index=4,
        v4_enabled=1,
        v6_enabled=0,
        ifindex=4)

    # program egress bd map
    program_egress_bd_map(client, sess_hdl, dev_tgt, vlan=vlan1)
    program_egress_bd_map(client, sess_hdl, dev_tgt, vlan=vlan2)

    dst_port_lag_index1 = 300

    # dest_index1 => remote (device=1, port=1)
    action_spec = dc_set_lag_remote_port_action_spec_t(
        action_device=1, action_port=1)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_remote_port(
        sess_hdl, dev_tgt, action_spec)
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=dst_port_lag_index1)
    client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    # add nexthops
    add_nexthop(
        client,
        sess_hdl,
        dev_tgt,
        nhop=g_nhop4,
        vlan=vlan1,
        port_lag_index=dst_port_lag_index1,
	ifindex=0)

    # add rewrite information for local ports
    add_v4_unicast_rewrite(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        nhop=g_nhop1,
        dmac='00:00:00:00:03:02')
    add_v4_unicast_rewrite(
        client,
        sess_hdl,
        dev_tgt,
        vlan=vlan2,
        nhop=g_nhop2,
        dmac='00:00:00:00:03:03')

    # add routes
    add_v4_route(
        client,
        sess_hdl,
        dev_tgt,
        vrf=1,
        ip=0x0a0a0101,
        prefix=32,
        nhop=g_nhop4)


class Run(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        print
        print 'Configuring the devices'
        sess_hdl = self.conn_mgr.client_init()
        mc_sess_hdl = self.mc.mc_create_session()
        for dev_id in range(0, 3):
            dev_tgt = DevTarget_t(dev_id, hex_to_i16(0xFFFF))
            # add default entries
            populate_default_entries(self.client, sess_hdl, dev_tgt)
            populate_init_entries(self.client, sess_hdl, dev_tgt)

        config_port_asic_0(self.client, sess_hdl, self.mc, mc_sess_hdl)
        config_port_asic_1(self.client, sess_hdl, self.mc, mc_sess_hdl)
        config_port_asic_2(self.client, sess_hdl, self.mc, mc_sess_hdl)

    def runTest(self):
        print
        print "L3 routing from device 1 port 1 to remote device 3 port 2"
        pkt = simple_udp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:00:00:00:01:01',
            ip_dst='10.10.3.2',
            ip_src='10.10.1.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:00:00:00:03:02',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.3.2',
            ip_src='10.10.1.1',
            ip_ttl=63)
        send_packet(self, 1, str(pkt))
        verify_packets(self, exp_pkt, [914])

        print "L3 routing from device 1 port 1 to remote device 3 port 3"
        pkt = simple_udp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:00:00:00:01:01',
            ip_dst='10.10.3.3',
            ip_src='10.10.1.1',
            ip_ttl=4)
        exp_pkt = simple_udp_packet(
            eth_dst='00:00:00:00:03:03',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.3.3',
            ip_src='10.10.1.1',
            ip_ttl=3)
        send_packet(self, 1, str(pkt))
        verify_packets(self, exp_pkt, [915])

        print "L3 routing from device 1 port 1 to local device 1 port 2"
        pkt = simple_udp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:00:00:00:01:01',
            ip_dst='10.10.1.2',
            ip_src='10.10.1.1',
            ip_ttl=255)
        exp_pkt = simple_udp_packet(
            eth_dst='00:00:00:00:01:02',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.1.2',
            ip_src='10.10.1.1',
            ip_ttl=254)
        send_packet(self, 1, str(pkt))
        verify_packets(self, exp_pkt, [2])

        print "L3 routing from device 3 port 2 to remote device 1 port 3"
        pkt = simple_udp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:00:00:00:03:02',
            ip_dst='10.10.1.1',
            ip_src='10.10.3.3',
            ip_ttl=25)
        exp_pkt = simple_udp_packet(
            eth_dst='00:00:00:00:01:01',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.1.1',
            ip_src='10.10.3.3',
            ip_ttl=24)
        send_packet(self, 915, str(pkt))
        verify_packets(self, exp_pkt, [1])

        print "Vlan flood from device 1 port 2"
        pkt = simple_udp_packet(
            eth_dst='00:00:00:aa:aa:aa',
            eth_src='00:00:00:00:01:01',
            ip_dst='10.10.3.200',
            ip_src='10.10.1.1',
            ip_ttl=64)
        send_packet(self, 2, str(pkt))
        verify_packets(self, pkt, [0, 3, 914, 915])

        print "Vlan flood from device 3 port 2"
        pkt = simple_udp_packet(
            eth_dst='00:00:00:aa:aa:aa',
            eth_src='00:00:00:00:01:01',
            ip_dst='10.10.3.200',
            ip_src='10.10.1.1',
            ip_ttl=64)
        send_packet(self, 914, str(pkt))
        verify_packets(self, pkt, [0, 2, 3, 915])
