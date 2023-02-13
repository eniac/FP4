################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2017-present Barefoot Networks, Inc.
#
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

import os
import time
import sys
import logging
import unittest
import random
import pd_base_tests
import pltfm_pm_rpc

from pltfm_pm_rpc.ttypes import *
from pal_rpc.ttypes import *
from port_mapping import *
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switch.p4_pd_rpc.ttypes import *
from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.pd_utils import *
from common.utils import *

ipv6_enabled = 1
acl_enabled = 1
tunnel_enabled = 1
mc_tunnel_enabled = 0
multicast_enabled = 1
int_enabled = 1

frontPanelPorts = [
    "1/0", "1/1", "1/2", "1/3", "2/0", "2/1", "2/2", "2/3", "17/0"
]

################################################################################
# This script uses PD thrift interface to demonstrate L3 routing over ECMP.
# The ECMP path contains both regular ports and LAGs. It uses 9 ports. The
# ports are specified in list "frontPanelPorts". Ports 0 thru 7 are egress
# ports and port 8 is the ingress port. Ports 0 thru 3 are member ports of a
# LAG. Likewise ports 4 and 5 are members of another LAG. See figure below for
# how the ports are configured.
#
#                     +--- frontPanelPorts[0]
#                     |
#                     +--- frontPanelPorts[1]
#          +-- LAG1 --|
#          |          +--- frontPanelPorts[2]
#          |          |
#          |          +--- frontPanelPorts[3]
#    ECMP--+
#          |          +--- frontPanelPorts[4]
#          +-- LAG2 --|
#          |          +--- frontPanelPorts[5]
#          |
#          +-------------- frontPanelPorts[6]
#          |
#          +-------------- frontPanelPorts[7]
#
# Traffic ingress port : frontPanelPorts[8]
#
# The device's MAC address is configured as 00:77:66:55:44:33
#
# First, the script configures 5 VLANs 10, 11, 12, 13, 14.
#
# Next, lag1  is added to VLAN 10,
#       lag2  is added to VLAN 11,
#       port6 is added to VLAN 12,
#       port7 is added to VLAN 13,
#   and port8 is addded to VLAN 14.
#
# 4 Nexthop interfaces are created with the following rewrites.
#     33 : 00:11:11:11:11:11 
#     34 : 00:11:11:11:11:22
#     35 : 00:11:11:11:11:33 
#     36 : 00:11:11:11:11:44 
# 
# An ECMP group with index 777 is created that contains the above 4 nexthops.
#
# Finally, a route is added with the ECMP group.
#   ip route add 10.100.0.0/16 ===> ECMP index 777
#
# To verify the configuration, the following traffic is sent on the input port.
#     Input port               : frontPanelPort[8]
#     MAC source               : 00:11:11:11:11:55
#     MAC destination addresss : 00:77:66:55:44:33
#     IP source address        : 192.168.0.0/16 subnet
#     IP destination address   : 10.100.0.0/16 subnet
#
# Packets are expected on frontPanelPorts 0 thru 7 with the following weights:
#     frontPanelPort[0] : 1/16
#     frontPanelPort[1] : 1/16
#     frontPanelPort[2] : 1/16
#     frontPanelPort[3] : 1/16
#     frontPanelPort[4] : 1/8
#     frontPanelPort[5] : 1/8
#     frontPanelPort[6] : 1/4
#     frontPanelPort[7] : 1/4
################################################################################


################################################################################
# function : create_port
# in  : port
# out : interface index corresponding to the port
################################################################################
def create_port(client, sess_hdl, dev_tgt, port):
    ifindex = port + 1

    # program port_mapping and port_properties tables
    match_spec = dc_ingress_port_mapping_match_spec_t(
        ig_intr_md_ingress_port=port)
    action_spec = dc_set_port_lag_index_action_spec_t(
        action_port_lag_index=ifindex, action_port_type=0)
    port_hdl = client.ingress_port_mapping_table_add_with_set_port_lag_index(
        sess_hdl, dev_tgt, match_spec, action_spec)
    action_spec = dc_set_ingress_port_properties_action_spec_t(
        action_port_lag_label=0,
        action_exclusion_id=ifindex,
        action_qos_group=0,
        action_tc_qos_group=0,
        action_tc=0,
        action_color=0,
        action_trust_dscp=0,
        action_trust_pcp=0)
    port2_hdl = client.ingress_port_properties_table_add_with_set_ingress_port_properties(
        sess_hdl, dev_tgt, match_spec, action_spec)

    # program lag_group table
    action_spec = dc_set_lag_port_action_spec_t(action_port=port)
    mbr_hdl = client.lag_action_profile_add_member_with_set_lag_port(
        sess_hdl, dev_tgt, action_spec)
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=ifindex)
    lag_hdl = client.lag_group_add_entry(sess_hdl, dev_tgt, match_spec, mbr_hdl)

    # program egress_port_mapping table
    match_spec = dc_egress_port_mapping_match_spec_t(
        eg_intr_md_egress_port=port)
    action_spec = dc_egress_port_type_normal_action_spec_t(
        action_qos_group=0, action_port_lag_label=0)
    egress_hdl = client.egress_port_mapping_table_add_with_egress_port_type_normal(
        sess_hdl, dev_tgt, match_spec, action_spec)

    return ifindex


################################################################################
# function : create_lag
# in  : list of port in the LAG
# out : interface index corresponding to the LAG
################################################################################
def create_lag(client, sess_hdl, dev_tgt, port_list):
    if 'ifindex' not in vars(create_lag):
        create_lag.ifindex = 128
        ifidx = create_lag.ifindex
    else:
        create_lag.ifindex += 1
    ifidx = create_lag.ifindex

    # create lag group
    lag_grp_hdl = client.lag_action_profile_create_group(sess_hdl, dev_tgt, 8)

    for port in port_list:
        # program port_mapping and port_properties tables
        match_spec = dc_ingress_port_mapping_match_spec_t(
            ig_intr_md_ingress_port=port)
        action_spec = dc_set_port_lag_index_action_spec_t(
            action_port_lag_index=ifidx, action_port_type=0)
        port_hdl = client.ingress_port_mapping_table_add_with_set_port_lag_index(
            sess_hdl, dev_tgt, match_spec, action_spec)
        action_spec = dc_set_ingress_port_properties_action_spec_t(
            action_port_lag_label=0,
            action_exclusion_id=ifidx,
            action_qos_group=0,
            action_tc_qos_group=0,
            action_tc=0,
            action_color=0,
            action_trust_dscp=0,
            action_trust_pcp=0)
        port2_hdl = client.ingress_port_properties_table_add_with_set_ingress_port_properties(
            sess_hdl, dev_tgt, match_spec, action_spec)

        # add port to lag_group
        action_spec = dc_set_lag_port_action_spec_t(action_port=port)
        mbr_hdl = client.lag_action_profile_add_member_with_set_lag_port(
            sess_hdl, dev_tgt, action_spec)
        client.lag_action_profile_add_member_to_group(sess_hdl, dev_tgt.dev_id,
                                                      lag_grp_hdl, mbr_hdl)

        # program egress_port_mapping table
        match_spec = dc_egress_port_mapping_match_spec_t(
            eg_intr_md_egress_port=port)
        action_spec = dc_egress_port_type_normal_action_spec_t(
            action_qos_group=0, action_port_lag_label=0)
        egress_hdl = client.egress_port_mapping_table_add_with_egress_port_type_normal(
            sess_hdl, dev_tgt, match_spec, action_spec)

    # program lag_group table
    match_spec = dc_lag_group_match_spec_t(
        ingress_metadata_egress_port_lag_index=ifidx)
    lag_grp_entry_hdl = client.lag_group_add_entry_with_selector(
        sess_hdl, dev_tgt, match_spec, lag_grp_hdl)

    return ifidx


################################################################################
# function : create_lag
# in  : list of nexthops (egress BD, interface index and nexthop index)
# out : index corresponding to ECMP group
################################################################################
def create_ecmp_group(client, sess_hdl, dev_tgt, nhop_list):
    ecmp_idx = 777

    # create ecmp group
    ecmp_hdl = client.ecmp_action_profile_create_group(sess_hdl, dev_tgt, 8)

    # add nexthops to ecmp group
    for nhop in nhop_list:
        action_spec = dc_set_ecmp_nexthop_details_action_spec_t(
            action_ifindex=nhop['ifidx'],
            action_port_lag_index=nhop['port_lag_idx'],
            action_bd=nhop['bd'],
            action_nhop_index=nhop['nhop'],
            action_tunnel=0)
        mbr_hdl = client.ecmp_action_profile_add_member_with_set_ecmp_nexthop_details(
            sess_hdl, dev_tgt, action_spec)
        client.ecmp_action_profile_add_member_to_group(sess_hdl, dev_tgt.dev_id,
                                                       ecmp_hdl, mbr_hdl)

    # program ecmp entry
    match_spec = dc_ecmp_group_match_spec_t(l3_metadata_nexthop_index=ecmp_idx)
    ecmp_grp_entry_hdl = client.ecmp_group_add_entry_with_selector(
        sess_hdl, dev_tgt, match_spec, ecmp_hdl)

    return ecmp_idx


################################################################################
# function : create_ipv4_route_with_ecmp
# in  : vrf, IPv4 prefix (in hex), length of prefix, ecmp index
# out : PD handle corresponding to the fib entry
################################################################################
def create_ipv4_route_with_ecmp(client, sess_hdl, dev_tgt, vrf, prefix,
                                prefix_len, ecmp_idx):
    match_spec = dc_ipv4_fib_lpm_match_spec_t(
        l3_metadata_vrf=vrf,
        ipv4_metadata_lkp_ipv4_da=prefix,
        ipv4_metadata_lkp_ipv4_da_prefix_length=prefix_len)
    action_spec = dc_fib_hit_ecmp_action_spec_t(action_ecmp_index=ecmp_idx)
    fib_hdl = client.ipv4_fib_lpm_table_add_with_fib_hit_ecmp(
        sess_hdl, dev_tgt, match_spec, action_spec)
    print 'fib entry handle:', format(fib_hdl, '#010x')
    return fib_hdl


class L3EcmpLagTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        # initialize the thrift data plane
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring the system'
        self.DEVICE = 0
        self.PIPE = 0xFFFF
        self.INNER_RMAC_GRP = 1
        self.OUTER_RMAC_GRP = 2
        self.SMAC_IDX = 1
        self.VRF = 1
        self.RMAC = '00:77:66:55:44:33'
        self.VLANS = [10, 11, 12, 13, 14]
        self.NHOPS = [33, 34, 35, 36]
        self.NEIGHBOR_MAC = [
            '00:11:11:11:11:11', '00:11:11:11:11:22', '00:11:11:11:11:33',
            '00:11:11:11:11:44'
        ]
        self.ROUTE = {'PREFIX': 0x0a640000, 'LEN': 16}

        # initialize the connection
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()

        self.platform_type = "mavericks"
        board_type = self.pltfm_pm.pltfm_pm_board_type_get()
        if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
            self.platform_type = "mavericks"
        elif re.search("0x2234|0x3234", hex(board_type)):
            self.platform_type = "montara"

        # get the device ports from front panel ports
        self.devPorts = getDevPorts(self.platform_type, frontPanelPorts)

        if test_param_get('setup') == True or (
                test_param_get('setup') != True and
                test_param_get('cleanup') != True):


            # add platform ports
            for devport in self.devPorts:
                self.pal.pal_port_add(
                    0, devport, pal_port_speed_t.BF_SPEED_10G,
                    pal_fec_type_t.BF_FEC_TYP_NONE)
                self.pal.pal_port_enable(0, devport)

            # initialize the client
            dev_tgt = DevTarget_t(self.DEVICE, hex_to_i16(self.PIPE))
            client_init(self.client, self.sess_hdl, dev_tgt)

            # program default actions and add default entries
            populate_default_entries(self.client, self.sess_hdl, dev_tgt,
                                     ipv6_enabled, acl_enabled, tunnel_enabled,
                                     mc_tunnel_enabled, multicast_enabled, int_enabled)

            # initialize tables
            populate_init_entries(self.client, self.sess_hdl, dev_tgt,
                                  self.SMAC_IDX, self.RMAC, self.INNER_RMAC_GRP,
                                  self.OUTER_RMAC_GRP, ipv6_enabled,
                                  tunnel_enabled)

            # create interfaces
            lag1_idx = create_lag(self.client, self.sess_hdl, dev_tgt, [
                self.devPorts[0], self.devPorts[1], self.devPorts[2],
                self.devPorts[3]
            ])
            lag2_idx = create_lag(self.client, self.sess_hdl, dev_tgt,
                                  [self.devPorts[4], self.devPorts[5]])
            port1_idx = create_port(self.client, self.sess_hdl, dev_tgt,
                                    self.devPorts[6])
            port2_idx = create_port(self.client, self.sess_hdl, dev_tgt,
                                    self.devPorts[7])
            port3_idx = create_port(self.client, self.sess_hdl, dev_tgt,
                                    self.devPorts[8])

            # program ingress and egress bd properties
            program_bd(self.client, self.sess_hdl, dev_tgt, self.VLANS[0], 0)
            program_bd(self.client, self.sess_hdl, dev_tgt, self.VLANS[1], 0)
            program_bd(self.client, self.sess_hdl, dev_tgt, self.VLANS[2], 0)
            program_bd(self.client, self.sess_hdl, dev_tgt, self.VLANS[3], 0)
            program_bd(self.client, self.sess_hdl, dev_tgt, self.VLANS[4], 0)
            program_egress_bd_properties(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[0], self.SMAC_IDX)
            program_egress_bd_properties(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[1], self.SMAC_IDX)
            program_egress_bd_properties(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[2], self.SMAC_IDX)
            program_egress_bd_properties(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[3], self.SMAC_IDX)

            # program port vlan mapping entries
            program_vlan_mapping(
                self.client,
                self.sess_hdl,
                dev_tgt,
                self.VRF,
                self.VLANS[0],
                lag1_idx,
                1,
                0,
                self.INNER_RMAC_GRP,
                0,
                ctag=None,
                stag=None)
            program_vlan_mapping(
                self.client,
                self.sess_hdl,
                dev_tgt,
                self.VRF,
                self.VLANS[1],
                lag2_idx,
                1,
                0,
                self.INNER_RMAC_GRP,
                0,
                ctag=None,
                stag=None)
            program_vlan_mapping(
                self.client,
                self.sess_hdl,
                dev_tgt,
                self.VRF,
                self.VLANS[2],
                port1_idx,
                1,
                0,
                self.INNER_RMAC_GRP,
                0,
                ctag=None,
                stag=None)
            program_vlan_mapping(
                self.client,
                self.sess_hdl,
                dev_tgt,
                self.VRF,
                self.VLANS[3],
                port2_idx,
                1,
                0,
                self.INNER_RMAC_GRP,
                0,
                ctag=None,
                stag=None)
            program_vlan_mapping(
                self.client,
                self.sess_hdl,
                dev_tgt,
                self.VRF,
                self.VLANS[4],
                port3_idx,
                1,
                0,
                self.INNER_RMAC_GRP,
                0,
                ctag=None,
                stag=None)

            # program nexthops
            program_nexthop(self.client, self.sess_hdl, dev_tgt, self.NHOPS[0],
                            self.VLANS[0], lag1_idx, lag1_idx, 0)
            program_nexthop(self.client, self.sess_hdl, dev_tgt, self.NHOPS[1],
                            self.VLANS[1], lag2_idx, lag2_idx, 0)
            program_nexthop(self.client, self.sess_hdl, dev_tgt, self.NHOPS[2],
                            self.VLANS[2], port1_idx, port1_idx, 0)
            program_nexthop(self.client, self.sess_hdl, dev_tgt, self.NHOPS[3],
                            self.VLANS[3], port2_idx, port2_idx, 0)

            # program L3 rewrites
            program_ipv4_unicast_rewrite(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[0], self.NHOPS[0],
                                         self.NEIGHBOR_MAC[0])
            program_ipv4_unicast_rewrite(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[1], self.NHOPS[1],
                                         self.NEIGHBOR_MAC[1])
            program_ipv4_unicast_rewrite(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[2], self.NHOPS[2],
                                         self.NEIGHBOR_MAC[2])
            program_ipv4_unicast_rewrite(self.client, self.sess_hdl, dev_tgt,
                                         self.VLANS[3], self.NHOPS[3],
                                         self.NEIGHBOR_MAC[3])

            nhop_list = \
                [{'bd':self.VLANS[0], 'nhop':self.NHOPS[0], 'ifidx':lag1_idx, 'port_lag_idx': lag1_idx},{'bd':self.VLANS[1], 'nhop':self.NHOPS[1], 'ifidx':lag2_idx, 'port_lag_idx': lag2_idx},{'bd':self.VLANS[2], 'nhop':self.NHOPS[2], 'ifidx':port1_idx, 'port_lag_idx': port1_idx},{'bd':self.VLANS[3], 'nhop':self.NHOPS[3], 'ifidx':port2_idx, 'port_lag_idx': port2_idx}]

            ecmp_idx = create_ecmp_group(self.client, self.sess_hdl, dev_tgt,
                                         nhop_list)

            create_ipv4_route_with_ecmp(self.client, self.sess_hdl, dev_tgt,
                                        self.VRF, self.ROUTE['PREFIX'],
                                        self.ROUTE['LEN'], ecmp_idx)

            self.conn_mgr.complete_operations(self.sess_hdl)
            self.conn_mgr.client_cleanup(self.sess_hdl)

    def runTest(self):
        if test_param_get('setup') != True and \
            test_param_get('cleanup') != True:

            print 'Verifying configuration by sending packets'
            count = [0] * 8
            dst_ip = int(socket.inet_aton('10.100.1.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 500
            for i in range(0, max_itrs):
                # ingress packet
                dst_ip_addr = \
                    socket.inet_ntoa(format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = \
                    socket.inet_ntoa(format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:11:11:11:11:55',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=64)

                # egress packets
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:22',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt3 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:33',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt4 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:44',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)

                send_packet(self, self.devPorts[8], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4], [
                        self.devPorts[0], self.devPorts[1], self.devPorts[2],
                        self.devPorts[3], self.devPorts[4], self.devPorts[5],
                        self.devPorts[6], self.devPorts[7]
                    ])
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            print 'Packets per port: ', count
