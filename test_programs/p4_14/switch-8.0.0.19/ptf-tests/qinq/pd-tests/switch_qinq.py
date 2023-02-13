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

from ptf.testutils import *
from ptf.thriftutils import *

import os

from switch.p4_pd_rpc.ttypes import *

from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
sys.path.append(os.path.join(this_dir, '../../base'))


from common.utils import *
from common.pd_utils import *



default_entries = {}

def add_vlan_mapping(
        client, sess_hdl, dev_tgt, vrf, vlan, rmac, port_lag_index, vlan_tag):
    action_spec = dc_set_bd_properties_action_spec_t(
        action_bd=vlan,
        action_vrf=vrf,
        action_rmac_group=rmac,
        action_ipv4_unicast_enabled=1,
        action_ipv6_unicast_enabled=0,
        action_bd_label=0,
        action_igmp_snooping_enabled=0,
        action_mld_snooping_enabled=0,
        action_ipv4_urpf_mode=0,
        action_ipv6_urpf_mode=0,
        action_stp_group=0,
        action_ipv4_multicast_enabled=0,
        action_ipv6_multicast_enabled=0,
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
        vlan_tag__0__valid=1,
        vlan_tag__0__vid=vlan_tag)
    hdl = client.port_vlan_to_bd_mapping_add_entry(
        sess_hdl, dev_tgt, match_spec, mbr_hdl)
    return hdl, mbr_hdl

def add_port_mapping(
    client, sess_hdl, dev_tgt, vrf, vlan, rmac, port_lag_index): 
    action_spec = dc_set_bd_properties_action_spec_t(
        action_bd=vlan,
        action_vrf=vrf,
        action_rmac_group=rmac,
        action_ipv4_unicast_enabled=1,
        action_ipv6_unicast_enabled=0,
        action_bd_label=0,
        action_igmp_snooping_enabled=0,
        action_mld_snooping_enabled=0,
        action_ipv4_urpf_mode=0,
        action_ipv6_urpf_mode=0,
        action_stp_group=0,
        action_ipv4_multicast_enabled=0,
        action_ipv6_multicast_enabled=0,
        action_mrpf_group=0,
        action_ipv4_mcast_key_type=0,
        action_ipv4_mcast_key=0,
        action_ipv6_mcast_key_type=0,
        action_ipv6_mcast_key=0,
        action_stats_idx=0,
        action_learning_enabled=0)
    match_spec = dc_port_to_bd_mapping_match_spec_t(
        ingress_metadata_port_lag_index=port_lag_index)
    hdl = client.port_to_bd_mapping_table_add_with_set_bd_properties(
        sess_hdl, dev_tgt, match_spec, action_spec)
    return hdl

def delete_port_mapping(client, sess_hdl, device, hdl):
    client.port_to_bd_mapping_table_delete(sess_hdl, device, hdl)

def add_ifindex_mapping(client,
                        sess_hdl,
                        dev_tgt,
                        port_lag_index,
                        ifindex,
                        vlan_tag,
                        rid=0):

    match_spec = dc_port_vlan_to_ifindex_mapping_match_spec_t(
        ingress_metadata_port_lag_index=port_lag_index,
        vlan_tag__0__valid=1,
        vlan_tag__0__vid=vlan_tag)

    action_spec = dc_set_ingress_interface_properties_action_spec_t(
        action_ifindex=ifindex, action_ingress_rid=rid)

    hdl = client.port_vlan_to_ifindex_mapping_table_add_with_set_ingress_interface_properties(
        sess_hdl, dev_tgt, match_spec, action_spec)
    return hdl

def add_vlan_xlate(client,
                   sess_hdl,
                   dev_tgt,
                   egress_ifindex,
                   bd,
                   vlan_tag):
    match_spec = dc_egress_vlan_xlate_match_spec_t(
        ingress_metadata_egress_ifindex=egress_ifindex, egress_metadata_bd=bd)
   
    action_spec = dc_set_egress_if_params_qinq_tagged_action_spec_t(
        action_s_tag=vlan_tag)
    hdl = client.egress_vlan_xlate_table_add_with_set_egress_if_params_qinq_tagged(
        sess_hdl, dev_tgt, match_spec, action_spec)
    return hdl


def add_default_entries(client, sess_hdl, dev_tgt):
    index = 0
    action_spec = dc_set_config_parameters_action_spec_t(
        action_switch_id=0)
    client.switch_config_params_set_default_action_set_config_parameters(
        sess_hdl, dev_tgt, action_spec)
    client.validate_outer_ethernet_set_default_action_set_valid_outer_unicast_packet_untagged(
        sess_hdl, dev_tgt)
    client.validate_outer_ipv4_packet_set_default_action_set_valid_outer_ipv4_packet(
        sess_hdl, dev_tgt)

    client.smac_set_default_action_smac_miss(sess_hdl, dev_tgt)
    client.dmac_set_default_action_dmac_miss(sess_hdl, dev_tgt)
    client.rmac_set_default_action_rmac_miss(sess_hdl, dev_tgt)
    client.ipv4_fib_set_default_action_on_miss(sess_hdl, dev_tgt)
    client.egress_vlan_xlate_set_default_action_set_egress_if_params_untagged(
        sess_hdl, dev_tgt)
    client.rewrite_set_default_action_set_l2_rewrite(sess_hdl, dev_tgt)
    action_spec = dc_egress_port_type_normal_action_spec_t(
        action_qos_group=0, action_port_lag_label=0)
    client.egress_port_mapping_set_default_action_egress_port_type_normal(
        sess_hdl, dev_tgt, action_spec)
    client.mtu_set_default_action_mtu_miss(sess_hdl, dev_tgt)

    mbr_hdl = client.bd_action_profile_add_member_with_port_vlan_mapping_miss(
        sess_hdl, dev_tgt)
    client.port_vlan_to_bd_mapping_set_default_entry(sess_hdl, dev_tgt, mbr_hdl)

    match_spec = dc_compute_ipv4_hashes_match_spec_t(ethernet_valid=1)
    entry_hdl = client.compute_ipv4_hashes_table_add_with_compute_lkp_ipv4_hash(
        sess_hdl, dev_tgt, match_spec)
    default_entries[index] = ['client.compute_ipv4_hashes_table_delete', entry_hdl]
    index += 1

    match_spec = dc_compute_non_ip_hashes_match_spec_t(ethernet_valid=1)
    entry_hdl = client.compute_non_ip_hashes_table_add_with_compute_lkp_non_ip_hash(
       sess_hdl, dev_tgt, match_spec)
    default_entries[index] = ['client.compute_non_ip_hashes_table_delete', entry_hdl]
    index += 1

    match_spec = dc_compute_other_hashes_match_spec_t(ethernet_valid=1)
    entry_hdl = client.compute_other_hashes_table_add_with_compute_other_hashes(
        sess_hdl, dev_tgt, match_spec)
    default_entries[index] = ['client.compute_other_hashes_table_delete', entry_hdl]
    index += 1

    client.adjust_lkp_fields_set_default_action_non_ip_lkp(sess_hdl, dev_tgt)
    match_spec = dc_adjust_lkp_fields_match_spec_t(ipv4_valid=1, ipv6_valid=0)
    entry_hdl = client.adjust_lkp_fields_table_add_with_ipv4_lkp(
        sess_hdl, dev_tgt, match_spec)
    default_entries[index] = ['client.adjust_lkp_fields_table_delete', entry_hdl]
    index += 1
    match_spec = dc_adjust_lkp_fields_match_spec_t(ipv4_valid=0, ipv6_valid=1)
    entry_hdl = client.adjust_lkp_fields_table_add_with_ipv6_lkp(
        sess_hdl, dev_tgt, match_spec)
    default_entries[index] = ['client.adjust_lkp_fields_table_delete', entry_hdl]
    index += 1

def delete_default_entries(client, sess_hdl, dev_id):
    for value in default_entries.itervalues():
        eval(value[0])(sess_hdl, dev_id, value[1])

def add_init_entries(client, sess_hdl, dev_tgt, rmac, rmac_group, smac_index):
    ret = []
    match_spec = dc_smac_rewrite_match_spec_t(
        egress_metadata_smac_idx=smac_index)
    action_spec = dc_rewrite_smac_action_spec_t(
        action_smac=macAddr_to_string(rmac))
    ret.append(
        client.smac_rewrite_table_add_with_rewrite_smac(
            sess_hdl, dev_tgt, match_spec, action_spec))

    match_spec = dc_fwd_result_match_spec_t(
        l2_metadata_l2_redirect=0,
        l2_metadata_l2_redirect_mask=0,
        acl_metadata_acl_redirect=0,
        acl_metadata_acl_redirect_mask=0,
        acl_metadata_racl_redirect=0,
        acl_metadata_racl_redirect_mask=0,
        l3_metadata_fib_hit=1,
        l3_metadata_fib_hit_mask=1,
        l3_metadata_rmac_hit=0,
        l3_metadata_rmac_hit_mask=0,
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
    ret.append(
        client.fwd_result_table_add_with_set_fib_redirect_action(
            sess_hdl, dev_tgt, match_spec, 1000))

    match_spec = dc_fwd_result_match_spec_t(
        l2_metadata_l2_redirect=1,
        l2_metadata_l2_redirect_mask=1,
        acl_metadata_acl_redirect=0,
        acl_metadata_acl_redirect_mask=0,
        acl_metadata_racl_redirect=0,
        acl_metadata_racl_redirect_mask=0,
        l3_metadata_fib_hit=0,
        l3_metadata_fib_hit_mask=0,
        l3_metadata_rmac_hit=0,
        l3_metadata_rmac_hit_mask=0,
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
    ret.append(
        client.fwd_result_table_add_with_set_l2_redirect_action(
            sess_hdl, dev_tgt, match_spec, 1000))

    #Add default inner rmac entry
    match_spec = dc_rmac_match_spec_t(
        l3_metadata_rmac_group=rmac_group,
        l2_metadata_lkp_mac_da=macAddr_to_string(rmac))
    ret.append(
        client.rmac_table_add_with_rmac_hit(sess_hdl, dev_tgt, match_spec))

    #Initialize L3 rewrite table
    match_spec = dc_l3_rewrite_match_spec_t(
                ipv4_valid=1,
                ipv6_valid=0,
                ipv4_dstAddr=0,
                ipv4_dstAddr_mask=0,
                ipv6_dstAddr=ipv6Addr_to_string('::'),
                ipv6_dstAddr_mask=ipv6Addr_to_string('::'))
    ret.append(
        client.l3_rewrite_table_add_with_ipv4_unicast_rewrite(
            sess_hdl, dev_tgt, match_spec, 10))
    return ret

def delete_init_entries(client, sess_hdl, dev, ret_list):
    client.smac_rewrite_table_delete(sess_hdl, dev, ret_list[0])
    client.fwd_result_table_delete(sess_hdl, dev, ret_list[1])
    client.fwd_result_table_delete(sess_hdl, dev, ret_list[2])
    client.rmac_table_delete(sess_hdl, dev, ret_list[3])
    client.l3_rewrite_table_delete(sess_hdl, dev, ret_list[4])


@group('l2')
class L2QinQTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        smac_index = 1
        vrf = 1
        rmac = '00:33:33:33:33:33'
        rmac_group = 1
 
        print
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        add_default_entries(self.client, sess_hdl, dev_tgt)
        ret_init = add_init_entries(
            self.client, sess_hdl, dev_tgt, rmac, rmac_group, smac_index)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan = 10
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 0
        v6_enabled = 0

        # Add bd entry
        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, 0)
        
        p_hdl = add_port_mapping(
            self.client, sess_hdl, dev_tgt, vrf, vlan, rmac_group, port1) 
 
        #Add ports to vlan
        hdl1, mbr_hdl1 = add_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            rmac_group,
            port1,
            vlan_tag=10)

        if_hdl1 = add_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            vlan_tag=10)

        hdl2, mbr_hdl2 = add_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            rmac_group,
            port2,
            vlan_tag=20)

        if_hdl2 = add_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            vlan_tag=20)

        match_spec = dc_vlan_decap_match_spec_t(
            ig_intr_md_ingress_port=port1, vlan_tag__0__valid=1)
        v_hdl1 = self.client.vlan_decap_table_add_with_nop(
            sess_hdl, dev_tgt, match_spec)
        match_spec = dc_vlan_decap_match_spec_t(
                ig_intr_md_ingress_port=port2, vlan_tag__0__valid=1)
        v_hdl2 = self.client.vlan_decap_table_add_with_remove_vlan_single_tagged(
            sess_hdl, dev_tgt, match_spec)
        xlate_hdl = add_vlan_xlate(
            self.client, sess_hdl, dev_tgt, port2, bd=10, vlan_tag=20)

        #Add static macs to ports. (vlan, mac -> port)
        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:11:11:11:11:11', port1, ifindex1)
        dmac_hdl2, smac_hdl2 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:22:22:22:22:22', port2, ifindex2)

        self.conn_mgr.complete_operations(sess_hdl)

        pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:11:11:11:11:11',
            dl_vlan_enable=True,
            vlan_vid=10,
            vlan_pcp=0,
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_ttl=64,
            pktlen=100)

        exp_pkt = simple_qinq_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:11:11:11:11:11',
            dl_vlan_outer=20,
            dl_vlan_pcp_outer=0,
            dl_vlan_cfi_outer=0,
            vlan_vid=10,
            vlan_pcp=0,
            dl_vlan_cfi=0,
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_ttl=64,
            pktlen=100 + 4)
        exp_pkt['Ethernet'].type = 0x9100

        pkt2 = simple_qinq_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            dl_vlan_outer=20,
            dl_vlan_pcp_outer=0,
            dl_vlan_cfi_outer=0,
            vlan_vid=10,
            vlan_pcp=0,
            dl_vlan_cfi=0,
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_ttl=64,
            pktlen=100)
        pkt2[Ether].type = 0x9100
        
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=10,
            vlan_pcp=0,
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_ttl=64,
            pktlen=100 - 4)
        
        try:
            print "Sending packet port 1 (.1Q tunnel) -> port 2 (trunk)"
            send_packet(self, 1, str(pkt))
            verify_packets(self, exp_pkt, [2])
            print "Sending packet port 2 (trunk) -> port 2 (.1Q tunnel)"
            send_packet(self, 2, str(pkt2))
            verify_packets(self, exp_pkt2, [1])

        finally:
            delete_default_entries(self.client, sess_hdl, device)
            delete_init_entries(self.client, sess_hdl, device, ret_init)
            delete_mac(self.client, sess_hdl, device, dmac_hdl2, smac_hdl2)
            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)

            delete_egress_vlan_xlate(self.client, sess_hdl, device, xlate_hdl)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_port_mapping(self.client, sess_hdl, device, p_hdl)

            self.client.vlan_decap_table_delete(sess_hdl, device, v_hdl1)
            self.client.vlan_decap_table_delete(sess_hdl, device, v_hdl2)

            # delete BD
            delete_bd(self.client, sess_hdl, device, vlan_hdl)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_default_entries(self.client, sess_hdl, device)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


